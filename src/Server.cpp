#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
//
// Created by matthew on 12/05/2020.
//

#include "../include/Server.h"

std::string getNonBlockingInput() {
    std:: string result;
    std::getline(std::cin, result);
    return result;
}

Server::Server(float refreshRate, RSAKeyPair serverKey, RSAKeyPair serverSignature) {
    this->refreshRate = refreshRate;
    this->serverKey = serverKey;
    this->serverSignature = serverSignature;

    serverSocket = TCPSocket();
}

Server::~Server() {
    serverSocket.shutdownServerSocket();
    serverSocket.closeSocket();
}

void Server::initialiseServer(unsigned short serverPort) {
    if (serverSocket.bound()) {
        return;
    }

    serverSocket.create();

    serverSocket.setNonBlocking();
    serverSocket.bindToAddress(serverPort);

    serverSocket.beginListen();

    serverSocket.setAcceptCallback([this](TCPSocket& s) { acceptClient(s); });
}

void Server::startServer() {
    std::cout << "Server started successfully." << std::endl;

    // Asynchronous call to console read to make console inputs non-blocking
    std::future<std::string> nonBlockingInput = std::async(std::launch::async, getNonBlockingInput);

    // Variables to hold the start and end times of each update frame (for calculating frame delta time)
    std::chrono::time_point<std::chrono::system_clock> start, end;

    while (true) {
        // Get the start time of this frame
        start = std::chrono::system_clock::now();

        if (serverSocket.tryAccept()) {
            printf("Accepted a new client\n");
        }

        if (nonBlockingInput.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
            std::string input = nonBlockingInput.get();

            printf("You wrote: %s\n", input.c_str());

            if (input == "quit" || input == "exit") {
                break;
            }

            nonBlockingInput = std::async(std::launch::async, getNonBlockingInput);
        }

        // If the refresh rate is either negative or 0 it is invalid, so raise an error (otherwise there is the risk
        // of division by 0 or running a while loop at maximum speed causing the process to overload the CPU!)
        if (refreshRate <= 0) {
            ERROR("Refresh rate is <= 0. This is not allowed")
        }

        // Get the end time of this frame
        end = std::chrono::system_clock::now();

        // Calculate how long this frame took to execute
        std::chrono::duration<double> frameElapsed = end - start;

        // Calculate how much extra time must be waited to have the frame take the correct
        // total time
        double delta = (1.0 / refreshRate) - frameElapsed.count();

        // If we have time to wait, wait it, otherwise don't (can't wait a negative time!)
        if (delta > 0) {
            // Sleep function in microseconds, so multiply by 10e6
            usleep(delta * 1e6);
        }
    }
}

void Server::closeServer() {
    serverSocket.closeSocket();
    serverSocket.shutdownServerSocket();
}

void Server::acceptClient(TCPSocket& clientSocket) {
    // PROTOCOL:
    // Client and server send each other their respective public keys
    // Client sends a random challenge to the server, encrypted under the server's public key.
    // The server decrypts this message and generates a random session token for this client along with a random
    // AES session key for this client. It then concatenates the challenge, session key and token, and signs them
    // all under the digital signature key for the server. Then, it encrypts this signed value under the client's
    // public key and sends it back to the client.
    // At this point, neither the client nor the server have authenticated themselves to each other. The server notes
    // that this client is currently not authenticated and waits for proof of who they are.
    // The client receives the encrypted and signed data from the server. It first decrypts using its private key to
    // see the message, and then decrypts the signature with the server's public signature key. It expects to see the
    // number it generated. If it does not see the number, then the server has failed to authenticate itself and
    // the client terminates the connection; it is not communicating with the correct server.
    // If it does see its challenge, the server must have signed it, so the client knows they are talking to the server.
    // They now have an AES session key in common with the server, and they also have their session token allowing
    // them to communicate.
    // Next, in order to access any resource from the server, the client must also authenticate itself to the server.
    // It knows by this point that the server is trusted. The client first gathers a JWT from the Microsoft
    // graph authentication network. This JWT will be signed by Microsoft for a specific account. The data in the JWT
    // cannot be tampered with, else the signature will be invalid.
    // The client then constructs a message containing first its token, followed by the JWT it has received. It then
    // encrypts this under the symmetric AES key and sends to the server.
    // The server then decrypts this message, and if it sees the client's token, it knows that this client must have
    // sent the message, as no one else knows the token number.
    // The server then authenticates the JWT.
    // If the authentication is successful, the client has successfully authenticated themselves to the server and they
    // may now access resources. Otherwise, the server informs the client that they have failed, and terminates the
    // connection.
    //
    // 1. C -> S: KC
    // 2. S -> C: KS
    // 3. C -> S: {NC}_KS
    // 4. S -> C: {{NC, K, T}_SIG}_KC
    //    if NC invalid:
    //      client terminates connection
    //      end
    // 5. C -> S: {T, JWT}_K
    //    if JWT invalid:
    //      S -> C: ERROR
    //      server terminates connection
    //
    // Both communicate with AES key from here, with messages starting with token

    // 1: Receive client's public key
    PublicKey clientKey;
    NetworkMessage clientKeyMessage;
    clientSocket.receiveMessage(clientKeyMessage);

    // If this message was in any way erroneous, close the connection and return
    if (clientKeyMessage.error() || clientKeyMessage.protocol() != KEY_MESSAGE) {
        clientSocket.closeSocket();
        return;
    }

    memcpy(&clientKey, clientKeyMessage.getMessageData(), clientKeyMessage.getMessageSize());

    // 2: Send server's public key
    clientSocket.sendMessage(NetworkMessage(&serverKey.publicKey, sizeof(PublicKey), KEY_MESSAGE));

    // 3: Receive the client's random challenge, encrypted under server's public key
    uint2048 encryptedChallenge;
    NetworkMessage challengeMessage;

    clientSocket.receiveMessage(challengeMessage);

    // If this message was in any way erroneous, close the connection and return
    if (challengeMessage.error() || challengeMessage.protocol() != RSA_MESSAGE) {
        clientSocket.closeSocket();
        return;
    }

    memcpy(&encryptedChallenge, challengeMessage.getMessageData(), challengeMessage.getMessageSize());

    uint2048 decryptedChallenge = decrypt(encryptedChallenge, serverKey.privateKey);

    // Check that the challenge is valid. If it isn't a uint64 padded by 0's, is is not correct;
    // terminate the connection
    uint2048 mask = 0xFFFFFFFFFFFFFFFFul;

    if ((decryptedChallenge & (~mask)) != 0) {
        clientSocket.closeSocket();
        return;
    }

    // 4: Send signed challenge, session key and token
    AESKey clientSessionKey = generateAESKey();
    uint64 clientSessionToken;
    CryptoSafeRandom::random(&clientSessionToken, sizeof(uint64));

    // TODO: Maybe supply the number used once for the client's JWT request
    uint2048 response;
    uint8 *responseBuffer = (uint8 *) &response;
    memcpy(responseBuffer, &decryptedChallenge, sizeof(uint64));
    memcpy(&responseBuffer[sizeof(uint64)], &clientSessionKey, sizeof(AESKey));
    memcpy(&responseBuffer[sizeof(uint64) + sizeof(AESKey)], &clientSessionToken, sizeof(uint64));

    // TODO: This might not work if the signed message is greater than the client key's n value!
    uint2048 signedEncryptedResponse = encrypt(sign(response, serverSignature.privateKey), clientKey);
    clientSocket.sendMessage(NetworkMessage(signedEncryptedResponse.rawData(), sizeof(uint2048), RSA_MESSAGE));

    connectedClients.push_back(clientSocket);
}

// Old protocol. See above for fixed protocol
// PROTOCOL: (this protocol might not work as the messages in steps 3 and 5 may be too long to encrypt
//            asymmetrically. See the second protocol which avoids this issue)
// Client and server send each other their respective public keys
// Client sends a JWT to authenticate itself, along with a random
// challenge, all encrypted under the server's public key. The server then decrypts this, authenticates
// the JWT.
// If the JWT authentication fails, the server sends an error to the client and refuses access.
// If the authentication succeeds, the server signs the random challenge with its signing private key,
// and sends the signed challenge and a session AES key, as well as a session token for this client
// all encrypted under the client's public key back
// to the client.
// The client decrypts the message it receives, and then verifies that the server did in fact sign the challenge
// it send. Now, both the server and client are authenticated to each other, and both have a common shared
// AES key for communication. Additionally, there is now a token that both the server and client know, so
// all messages should start with this token.
// 1. C -> S: KC
// 2. S -> C: KS
// 3. C -> S: {JWT, NC}_KS
//    if not authenticated:
// 4.   S -> C: ERROR
//      terminate connection
//    else:
// 5.   S -> C: {{NC}_SIG, K, T}_KC
// Both communicate with the session AES key K from here.
#pragma clang diagnostic pop