#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
//
// Created by matthew on 12/05/2020.
//

#include "../include/Client.h"

Client::Client(float refreshRate, RSAKeyPair clientKey, PublicKey serverSignature) {
    this->refreshRate = refreshRate;
    this->clientKey = clientKey;
    this->serverSignature = serverSignature;

    clientSocket = TCPSocket();
}

Client::~Client() {
//    if (connected) {
//        close(clientSocketFd);
//    }
    clientSocket.closeSocket();
}

void Client::initialiseClient() {
    // If we already have a client socket, don't create another
    if (clientSocket.open()) {
        return;
    }
//    if (clientSocketFd != -1) {
//        return;
//    }

    // Create a TCP socket with the default protocol
//    clientSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    clientSocket.create();
}

bool Client::connectToServer(const std::string& ipAddress, int port) {
//    // Set the address family
//    serverAddress.sin_family = AF_INET;
//    // Set the port
//    serverAddress.sin_port = htons(port);
//
//    // Set the address through converting the string to the correct format
//    if (inet_pton(AF_INET, ipAddress.c_str(), &serverAddress.sin_addr.s_addr) < 0) {
//        ERROR("Invalid IP address")
//    }
//
//    // Connect to the server address
//    if (connect(clientSocketFd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
//        ERROR("Failed to connect to server")
//    }
    clientSocket.connectToServer(ipAddress, port);

    // Protocol description found in the server accept method.
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
    // Both communicate with AES key from here, with messages starting with token

    // 1: Send client's public key to server
    send(clientSocketFd, &clientKey.publicKey, sizeof(PublicKey), 0);

    // 2: Receive server's public key
    PublicKey serverKey;
    recv(clientSocketFd, &serverKey, sizeof(PublicKey), 0);

    // 3: Send random challenge to server encrypted under its public key
    uint2048 challengeMessage;
    uint64 *challenge = (uint64 *) &challengeMessage;
    // We only want a 64 bit challenge, so leave the rest as 0s
    CryptoSafeRandom::random(challenge, sizeof(uint64));
    uint2048 encryptedChallenge = encrypt(challengeMessage, serverKey);
    send(clientSocketFd, &encryptedChallenge, sizeof(uint2048), 0);

    std::cout << *challenge << std::endl;

    // 4: Receive signed challenge, session key and token, encrypted under client's public key
    uint2048 signedEncryptedResponse;
    recv(clientSocketFd, &signedEncryptedResponse, sizeof(uint2048), 0);

    uint2048 responseValue = checkSignature(decrypt(signedEncryptedResponse, clientKey.privateKey), serverSignature);
    uint8 *responseData = (uint8 *) &responseValue;
    uint64 responseChallenge;
    AESKey sessionKey;
    uint64 sessionToken;

    memcpy(&responseChallenge, responseData, sizeof(uint64));
    memcpy(&sessionKey, &responseData[sizeof(uint64)], sizeof(AESKey));
    memcpy(&sessionToken, &responseData[sizeof(uint64) + sizeof(AESKey)], sizeof(uint64));

    if (responseChallenge != *challenge) {
        close(clientSocketFd);
        std::cerr << "ERROR::Client.cpp: Returned challenge from server incorrect. Server failed to authenticate itself." << std::endl;
        return false;
    }

    // 5: Client now has to gather and send a JWT to the server to authenticate themselves

    // TODO: Make this section more dynamic
    QueryURL authUrl = getMicrosoftAccountIDQueryURL(CLIENT_APPLICATION_ID, REDIRECT_URL);

    std::cout << authUrl.url << std::endl;

    // TODO: Probably open this in another thread - currently this will hang the application
    std::string authToken = openOneShotHTTPAuthenticationServer("localhost", 5000);

    // TODO: Create a separate messaging system that handles the details of prepending the token etc
    size_t authTokenMessageSize = sizeof(uint64) + sizeof(uint32) + authToken.size();
    uint8 *authTokenMessage = (uint8 *) alloca(authTokenMessageSize);
    memcpy(authTokenMessage, &sessionToken, sizeof(uint64));
    memcpy(authTokenMessage + sizeof(uint64), &authUrl.numberUsedOnce, sizeof(uint32));
    memcpy(authTokenMessage + sizeof(uint64) + sizeof(uint32), authToken.c_str(), authToken.size());

    uint64 initialisationVector;
    CryptoSafeRandom::random(&initialisationVector, sizeof(uint64));

    size_t encryptedResponseSize = (authTokenMessageSize / 16 + (authTokenMessageSize % 16 != 0)) * 16;
    uint8 *tokenResponse = (uint8 *) alloca(sizeof(size_t) + sizeof(size_t) + sizeof(uint64) + encryptedResponseSize);
    memcpy(tokenResponse, &encryptedResponseSize, sizeof(size_t));
    memcpy(tokenResponse + sizeof(size_t), &initialisationVector, sizeof(uint64));
    memcpy(tokenResponse + sizeof(size_t) + sizeof(uint64), encrypt(authTokenMessage, authTokenMessageSize, initialisationVector, sessionKey), encryptedResponseSize);

    return true;
}

void Client::startClientLoop() {
    // Activate the loop thread - the loop will run for as long as this is true
    clientLoopRunning = true;

    // Create the thread pointing to the clientLoop function, called on this object
    clientLoopThread = std::thread(&Client::clientLoop, this);
}

void Client::stopClientLoop() {
    // Kill the loop - set this flag to false so the thread knows to exit
    clientLoopRunning = false;

    // Join and close the thread
    clientLoopThread.join();
}

void Client::clientLoop() {
    // Variables to hold the start and end times of each update frame (for calculating frame delta time)
    std::chrono::time_point<std::chrono::system_clock> start, end;

    while (clientLoopRunning) {
        // Get the start time of this frame
        start = std::chrono::system_clock::now();

        // do stuff!

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

#pragma clang diagnostic pop