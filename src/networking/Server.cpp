//
// Created by matthew on 12/05/2020.
//

#include "../../include/networking/Server.h"

std::string getNonBlockingInput() {
    std::string result;
    std::getline(std::cin, result);
    return result;
}

ClientHandle::ClientHandle(unsigned id) {
    this->clientID = id;
}

ClientData::ClientData(unsigned handleID, const TCPSocket &socket, const AESKey &sessionKey, uint64 sessionToken,
                       uint64 authNonce)
        : handle(handleID) {
    clientSocket = socket;
    clientSessionKey = sessionKey;
    clientSessionToken = sessionToken;
    clientAuthNonce = authNonce;
}

Server::Server(float refreshRate, RSAKeyPair serverKey, DigitalSignatureKeyPair serverSignature) {
    this->refreshRate = refreshRate;
    this->serverKey = serverKey;
    this->serverSignature = serverSignature;
}

Server::~Server() {
    closeServer();
}

void Server::initialiseServer(unsigned short serverPort) {
    if (serverSocket.bound()) {
        return;
    }

    guardTCPSocketCode(serverSocket.create(), *errorStream);
    guardTCPSocketCode(serverSocket.setNonBlocking(), *errorStream);
    guardTCPSocketCode(serverSocket.bindToAddress(serverPort), *errorStream);

    guardTCPSocketCode(serverSocket.beginListen(), *errorStream);

    serverSocket.setAcceptCallback([this](TCPSocket &s) {
        std::future<void> acceptAsync = std::async(std::launch::async, &Server::acceptClient, this, std::ref(s));

        // TODO: Make non constant
        if (acceptAsync.wait_for(std::chrono::milliseconds(60000)) != std::future_status::ready) {
            s.closeSocket();
        }
    });

    *logStream << timestamp() << "Server initialised" << std::endl;
}

void Server::startServer() {
    *logStream << timestamp() << "Server started successfully" << std::endl;

    // Asynchronous call to console read to make console inputs non-blocking
    std::future<std::string> nonBlockingInput = std::async(std::launch::async, getNonBlockingInput);

    // Variables to hold the start and end times of each update frame (for calculating frame delta time)
    std::chrono::time_point<std::chrono::system_clock> start, end;

    unsigned cycle = 0;

    while (true) {
        // Get the start time of this frame
        start = std::chrono::system_clock::now();

        // Check if any client is trying to connect to the server
        switch (serverSocket.tryAccept(true)) {
            case ERR_ACCEPT: ERROR_TO("Failed to accept client", *errorStream)
            case SOCKET_SUCCESS:
                // *logStream << timestamp << ": Accepted a new client" << std::endl;
            case S_NO_DATA:
            default:
                break;
        }

        // For every client waiting for authentication, try to authenticate them.
        for (int ic = 0; ic < waitingClients.size(); ic++) {
            if (tryAuthenticateClient(*waitingClients[ic])) {
                waitingClients.erase(waitingClients.begin() + ic);
            }
        }

        // Check all connected client channels for incoming messages
        for (int ic = 0; ic < connectedClients.size(); ic++) {
            ClientData &connectedClient = *connectedClients[ic];

            // First, check if the client socket connection is dead
            /*if (connectedClient.clientSocket.dead()) {
                *logStream << "Client " << connectedClient.clientEmail << " disconnected." << std::endl;
                connectedClient.clientSocket.closeSocket();
                connectedClients.erase(connectedClients.begin() + ic);
                handleMap.erase(connectedClient.handle.clientID);
            }

            EncryptedNetworkMessage message;
            // If we haven't received a message, continue
            if (connectedClient.clientSocket.receiveMessage(message, MessageProtocol::AES_MESSAGE) != SOCKET_SUCCESS) {
                continue;
            }*/

            EncryptedNetworkMessage message;

            switch (connectedClient.clientSocket.receiveMessage(message, MessageProtocol::AES_MESSAGE)) {
                case SOCKET_SUCCESS:
                    break;
                case ERR_SOCKET_DEAD:
                    *logStream << timestamp() << "Client " << connectedClient.clientEmail << " disconnected (timed out)." << std::endl;
                    connectedClient.clientSocket.closeSocket();
                    delete &connectedClient;
                    connectedClients.erase(connectedClients.begin() + ic);
                    handleMap.erase(connectedClient.handle.clientID);
                    continue;
                case SOCKET_DISCONNECTED:
                    switch (*((DisconnectCode *)message.getMessageData())) {
                        case DisconnectCode::CLIENT_EXIT:
                            *logStream << timestamp() << "Client " << connectedClient.clientEmail << " disconnected." << std::endl;
                            break;
                        default:
                            *logStream << timestamp() << "Client " << connectedClient.clientEmail << " disconnected (with error code)." << std::endl;
                            break;
                    }
                    connectedClient.clientSocket.closeSocket();
                    delete &connectedClient;
                    connectedClients.erase(connectedClients.begin() + ic);
                    handleMap.erase(connectedClient.handle.clientID);
                    continue;
                default:
                    continue;
            }

            // If the message received successfully
            if (!message.error()) {
                // Get the message and decrypt it with this client's key
                uint8 *decryptedMessage = (uint8 *) message.decryptMessageData(connectedClient.clientSessionKey);
                uint64 messageToken = *((uint64 *) decryptedMessage);

                // If the message starts with the client's secret session token, the message is valid
                if (messageToken == connectedClient.clientSessionToken) {
                    // Assuming we have set an appropriate handler, call it with the decrypted message
                    if (requestHandler) {
                        requestHandler->onMessageReceived(*this, connectedClient.handle,
                                                          decryptedMessage + sizeof(uint64),
                                                          message.getMessageSize() - sizeof(uint64));
                    }
                }

                delete decryptedMessage;
            }
        }

        // Send any messages in the send queue
        sendQueueMutex.lock();
        while (!sendQueue.empty()) {
            std::pair<unsigned, NetworkMessage *> message = sendQueue.front();
            handleMap[message.first]->clientSocket.sendMessage(*message.second);
            sendQueue.pop();
            delete message.second;
        }
        sendQueueMutex.unlock();

        // Every n cycles
        if (cycle % heartBeatCycles == 0) {
            for (ClientData *connectedClient : connectedClients) {
                connectedClient->clientSocket.heartbeat();
            }
        }

        if (nonBlockingInput.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
            std::string input = nonBlockingInput.get();

            if (input == "quit" || input == "exit") {
                *logStream << timestamp() << "Server closed" << std::endl << std::endl;
                break;
            }
            if (input == "list users") {
                for (ClientData *connectedClient : connectedClients) {
                    std::cout << "Client: " << connectedClient->clientEmail << std::endl;
                }
            }

            nonBlockingInput = std::async(std::launch::async, getNonBlockingInput);
        }

        // If the refresh rate is either negative or 0 it is invalid, so raise an error (otherwise there is the risk
        // of division by 0 or running a while loop at maximum speed causing the process to overload the CPU!)
        if (refreshRate <= 0) {
            ERROR_TO("Refresh rate is <= 0. This is not allowed", *errorStream)
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
            // Sleep function in microseconds, so multiply by 1e6
            // usleep(delta * 1e6);
            std::this_thread::sleep_for(std::chrono::microseconds((long long) (delta * 1e6)));
        }

        cycle++;
    }
}

void Server::closeServer() {
    serverSocket.closeSocket();
    dbManager->closeConnection();
    delete dbManager;
    *logStream << timestamp() << "Server closed" << std::endl << std::endl;
}

void
Server::addMessageToSendQueue(const ClientHandle &clientHandle, const void *message, unsigned messageLength) {
    sendQueueMutex.lock();
    sendQueue.emplace(clientHandle.clientID, new EncryptedNetworkMessage(message, sizeof(uint64) + messageLength,
                                                                         handleMap[clientHandle.clientID]->clientSessionKey));
    sendQueueMutex.unlock();
}

void Server::addMessageToSendQueue(const ClientHandle &clientHandle, const std::string &message) {
    addMessageToSendQueue(clientHandle, message.c_str(), message.size());
}

void Server::broadcastMessage(const void *message, unsigned messageLength) {
    for (const ClientData *client : connectedClients) {
        addMessageToSendQueue(client->handle, message, messageLength);
    }
}

void Server::broadcastMessage(const std::string &message) {
    broadcastMessage(message.c_str(), message.size());
}

void Server::sendRepeatToken(const ClientHandle &clientHandle, unsigned responseCode) {
    uint256 token;
    CryptoSafeRandom::random(&token, sizeof(uint256));

    repeatTokenMap[token] = handleMap[clientHandle.clientID]->clientEmail;

    uint8 *buffer = (uint8 *)alloca(sizeof(unsigned) + sizeof(uint256));
    *((unsigned *)buffer) = responseCode;
    memcpy(buffer + sizeof(unsigned), &token, sizeof(uint256));

    addMessageToSendQueue(clientHandle, buffer, sizeof(unsigned) + sizeof(uint256));
}

void Server::sendEmailAddress(const ClientHandle &clientHandle, unsigned int responseCode) {
    std::string email = handleMap[clientHandle.clientID]->clientEmail;
    void *buffer = (uint8 *)alloca(sizeof(unsigned) + sizeof(unsigned char) + email.size());
    unsigned char *buff = (unsigned char *)buffer;
    *((unsigned *) buff) = responseCode;
    buff += sizeof(unsigned);
    *buff++ = email.size();
    memcpy(buff, email.c_str(), email.size());

    addMessageToSendQueue(clientHandle, buffer, sizeof(unsigned) + sizeof(unsigned char) + email.size());
}

void Server::connectToDatabaseServer(const std::string &database, const std::string &user, const std::string &password,
                                     const std::string &host) {
    try {
        delete dbManager;
        dbManager = new DatabaseManager(database, user, password, host);
    } catch (mysqlx::Error &e) {
        SQL_ERROR(e, *errorStream);
    }
    dbManager->setErrorStream(*errorStream);
    databaseSchema = database;
    databaseUsername = user;
    databasePassword = password;
    databaseHost = host;

    *logStream << timestamp() << "Connected to database" << std::endl;
}

void Server::setRequestHandler(ServerRequestHandler &handler) {
    requestHandler = &handler;
}

void Server::setLoggingStream(std::ostream *log, std::ostream *changelog, std::ostream *errStream) {
    if (log) {
        logStream = log;
    }
    if (changelog) {
        changelogStream = changelog;
    }
    if (errorStream) {
        errorStream = errStream;
    }
    if (dbManager) {
        dbManager->setErrorStream(*errStream);
    }
}

void Server::changelogMessage(const ClientHandle &clientHandle, const std::string &message) {
    if (!changelogStream) {
        return;
    }
    *changelogStream << timestamp() << handleMap[clientHandle.clientID]->clientEmail << " " << message << std::endl;
}

std::string Server::timestamp() const {
    std::stringstream ss;
    std::time_t now = time(nullptr);
    std::tm *timePoint = std::localtime(&now);
    ss << "[" << std::put_time(timePoint, "%d/%m/%Y %H:%M:%S") << "] ";
    return ss.str();
}

void Server::setHeartBeatCycles(int cycles) {
    heartBeatCycles = cycles;
}

DatabaseManager &Server::databaseManager() {
    if (!dbManager) {
        ERROR_TO("Database manager not set up. No connection to database.", *errorStream)
    }

    if (!dbManager->testConnection()) {
        delete dbManager;

        try {
            dbManager = new DatabaseManager(databaseSchema, databaseUsername, databasePassword, databaseHost);
        } catch (mysqlx::Error &e) {
            SQL_ERROR(e, *errorStream);
        }
        dbManager->setErrorStream(*errorStream);

        *logStream << timestamp() << "Re-connected to database" << std::endl;
    }

    return *dbManager;
}

void Server::acceptClient(TCPSocket &clientSocket) {
    // PROTOCOL:
    // Client and server send each other their respective public keys
    // Client sends a random challenge to the server, encrypted under the server's public key.
    // The server decrypts this message and generates a random session token for this client along with a random
    // AES session key for this client. It also generates its own random number used once to be used in the JWT
    // request. It then concatenates the challenge, session key, token and new nonce, and signs them
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
    // 4. S -> C: {{NC, NS, K, T}_SIG}_KC
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
    RSAKeyPair::Public clientKey;
    NetworkMessage clientKeyMessage;

    // If the client socket doesn't send us a message, close the connection and return
    if (clientSocket.receiveMessage(clientKeyMessage, MessageProtocol::KEY_MESSAGE) != SOCKET_SUCCESS) {
        ConnectionResponse failResponse = ConnectionResponse::FAILED;
        NetworkMessage failedMessage(&failResponse, sizeof(ConnectionResponse), MessageProtocol::CONNECTION_RESPONSE_MESSAGE);
        clientSocket.sendMessage(failedMessage);

        clientSocket.closeSocket();
        return;
    }

    // If this message was in any way erroneous, close the connection and return
    if (clientKeyMessage.error()) {
        ConnectionResponse failResponse = ConnectionResponse::FAILED;
        NetworkMessage failedMessage(&failResponse, sizeof(ConnectionResponse), MessageProtocol::CONNECTION_RESPONSE_MESSAGE);
        clientSocket.sendMessage(failedMessage);

        clientSocket.closeSocket();
        return;
    }

    memcpy(&clientKey, clientKeyMessage.getMessageData(), clientKeyMessage.getMessageSize());

    // 2: Send server's public key
    if (clientSocket.sendMessage(NetworkMessage(&serverKey.publicKey, sizeof(RSAKeyPair::Public), MessageProtocol::KEY_MESSAGE)) !=
        SOCKET_SUCCESS) {
        ConnectionResponse failResponse = ConnectionResponse::FAILED;
        NetworkMessage failedMessage(&failResponse, sizeof(ConnectionResponse), MessageProtocol::CONNECTION_RESPONSE_MESSAGE);
        clientSocket.sendMessage(failedMessage);

        clientSocket.closeSocket();
        return;
    }

    // 3: Receive the client's random challenge, encrypted under server's public key
    uint2048 encryptedChallenge;
    NetworkMessage challengeMessage;

    if (clientSocket.waitForMessage(challengeMessage, MessageProtocol::RSA_MESSAGE) != SOCKET_SUCCESS) {
        ConnectionResponse failResponse = ConnectionResponse::FAILED;
        NetworkMessage failedMessage(&failResponse, sizeof(ConnectionResponse), MessageProtocol::CONNECTION_RESPONSE_MESSAGE);
        clientSocket.sendMessage(failedMessage);

        clientSocket.closeSocket();
        return;
    }

    // If this message was in any way erroneous, close the connection and return
    if (challengeMessage.error()) {
        ConnectionResponse failResponse = ConnectionResponse::FAILED;
        NetworkMessage failedMessage(&failResponse, sizeof(ConnectionResponse), MessageProtocol::CONNECTION_RESPONSE_MESSAGE);
        clientSocket.sendMessage(failedMessage);

        clientSocket.closeSocket();
        return;
    }

    memcpy(&encryptedChallenge, challengeMessage.getMessageData(), challengeMessage.getMessageSize());

    uint2048 decryptedChallenge = decrypt(encryptedChallenge, serverKey.privateKey);

    // Check that the challenge is valid. If it isn't a uint64 padded by 0's, is is not correct;
    // terminate the connection
    uint2048 mask = 0xFFFFFFFFFFFFFFFFull;

    if ((decryptedChallenge & (~mask)) != 0) {
        ConnectionResponse failResponse = ConnectionResponse::FAILED;
        NetworkMessage failedMessage(&failResponse, sizeof(ConnectionResponse), MessageProtocol::CONNECTION_RESPONSE_MESSAGE);
        clientSocket.sendMessage(failedMessage);

        clientSocket.closeSocket();
        return;
    }

    // 4: Send signed challenge, nonce, session key and token
    AESKey clientSessionKey = generateAESKey();
    uint64 clientSessionToken;
    uint32 clientNonce;
    CryptoSafeRandom::random(&clientSessionToken, sizeof(uint64));
    CryptoSafeRandom::random(&clientNonce, sizeof(uint32));

    uint2048 response;
    uint8 *responseBuffer = (uint8 *) &response;
    memcpy(responseBuffer, &decryptedChallenge, sizeof(uint64));
    memcpy(responseBuffer + sizeof(uint64), &clientNonce, sizeof(uint32));
    memcpy(responseBuffer + sizeof(uint64) + sizeof(uint32), &clientSessionKey, sizeof(AESKey));
    memcpy(responseBuffer + sizeof(uint64) + sizeof(uint32) + sizeof(AESKey), &clientSessionToken, sizeof(uint64));

    uint2048 signedEncryptedResponse = encrypt(sign(response, serverSignature.privateKey), clientKey);

    if (clientSocket.sendMessage(NetworkMessage(signedEncryptedResponse.rawData(), sizeof(uint2048), MessageProtocol::RSA_MESSAGE)) !=
        SOCKET_SUCCESS) {
        ConnectionResponse failResponse = ConnectionResponse::FAILED;
        NetworkMessage failedMessage(&failResponse, sizeof(ConnectionResponse), MessageProtocol::CONNECTION_RESPONSE_MESSAGE);
        clientSocket.sendMessage(failedMessage);

        clientSocket.closeSocket();
        return;
    }

    // After we have received all the protocol data, set this socket to be non blocking
    clientSocket.setNonBlocking();

    std::vector<unsigned> handles;

    for (std::pair<unsigned, ClientData *> handle : handleMap) {
        handles.push_back(handle.first);
    }
    for (unsigned i = 0; i < handles.size(); i++) {
        unsigned target = handles[i];
        while (target < handles.size() && target != handles[target]) {
            unsigned temp = handles[target];
            handles[target] = target;
            target = temp;
        }
    }
    unsigned handleID = handles.size();
    for (unsigned i = 0; i < handles.size(); i++) {
        if (handles[i] != i) {
            handleID = i;
            break;
        }
    }

    ClientData *client = new ClientData(handleID, clientSocket, clientSessionKey, clientSessionToken, clientNonce);

    waitingClients.push_back(client);

    handleMap[handleID] = client;
}

bool Server::tryAuthenticateClient(ClientData &clientData) {
    // Attempt to receive a message from the client. If they aren't sending anything, or they send an erroneous message,
    // just return false.
    EncryptedNetworkMessage authMessage;
    if (clientData.clientSocket.receiveMessage(authMessage, MessageProtocol::AES_MESSAGE) == S_NO_DATA) {
        return false;
    }

    if (authMessage.error()) {
        return false;
    }

    unsigned char *authMessageBuff = (unsigned char *) authMessage.decryptMessageData(clientData.clientSessionKey);
    AuthMode authMode = *((AuthMode *) authMessageBuff);
    authMessageBuff += sizeof(AuthMode);

    switch (authMode) {
    case AuthMode::JWT: {
        // Get a string of the response
        std::string jwtResponse = std::string((const char *)authMessageBuff, authMessage.getMessageSize() - sizeof(AuthMode));

        nlohmann::json claims;

        // Attempt to authenticate this JWT
        MicrosoftAccountAuthState status = authenticateMicrosoftAccount(jwtResponse, CLIENT_APPLICATION_ID,
            clientData.clientAuthNonce, "", claims);

        // Check the status
        switch (status) {
        case AUTHENTICATED: {
            // If the client successfully authenticated themselves,
            // add them to the connected clients vector.
            // They may now access server resources
            clientData.clientEmail = claims["email"];
            connectedClients.push_back(&clientData);
            *logStream << timestamp() << "Client " << clientData.clientEmail << " successfully authenticated themselves."
                << std::endl;
            ConnectionResponse successResponse = ConnectionResponse::SUCCESS;
            NetworkMessage succeededMessage(&successResponse, sizeof(ConnectionResponse), MessageProtocol::CONNECTION_RESPONSE_MESSAGE);
            clientData.clientSocket.sendMessage(succeededMessage);

            return true;
        }
        case RECEIVED_ERRONEOUS_TOKEN:
            *logStream << timestamp() << "Client failed to authenticate themselves: Bad JWT (Erroneous Token). Terminating connection." << std::endl;
        case NO_MATCHING_KEY:
            *logStream << timestamp() << "Client failed to authenticate themselves: Bad JWT (No matching key). Terminating connection." << std::endl;
        case INVALID_TOKEN:
            *logStream << timestamp() << "Client failed to authenticate themselves: Bad JWT (Invalid token). Terminating connection." << std::endl;
        case INVALID_SIGNATURE:
            // If the authentication failed for any reason, the client did not authenticate themselves, so terminate
            // their connection. If they wish to retry, they must reconnect to the server.
            *logStream << timestamp() << "Client failed to authenticate themselves: Bad JWT (Invalid signature). Terminating connection." << std::endl;
            ConnectionResponse failResponse = ConnectionResponse::FAILED;
            NetworkMessage failedMessage(&failResponse, sizeof(ConnectionResponse), MessageProtocol::CONNECTION_RESPONSE_MESSAGE);
            clientData.clientSocket.sendMessage(failedMessage);

            clientData.clientSocket.closeSocket();
            break;
        }
        break;
    }
    case AuthMode::REPEAT_TOKEN:
        uint256 repeatToken;

        memcpy(&repeatToken, authMessageBuff, sizeof(uint256));

        for (std::map<uint256, std::string>::const_iterator it = repeatTokenMap.begin(); it != repeatTokenMap.end(); it++) {
            if (it->first == repeatToken) {
                clientData.clientEmail = repeatTokenMap[repeatToken];
                connectedClients.push_back(&clientData);
                *logStream << timestamp() << "Client " << clientData.clientEmail << " successfully authenticated themselves."
                    << std::endl;
                ConnectionResponse successResponse = ConnectionResponse::SUCCESS;
                NetworkMessage succeededMessage(&successResponse, sizeof(ConnectionResponse), MessageProtocol::CONNECTION_RESPONSE_MESSAGE);
                clientData.clientSocket.sendMessage(succeededMessage);

                return true;
            }
        }

        // There is no client with this repeat token
        *logStream << timestamp() << "Client failed to authenticate themselves: Invalid Repeat Token. Terminating connection." << std::endl;
        ConnectionResponse failResponse = ConnectionResponse::FAILED;
        NetworkMessage failedMessage(&failResponse, sizeof(ConnectionResponse), MessageProtocol::CONNECTION_RESPONSE_MESSAGE);
        clientData.clientSocket.sendMessage(failedMessage);

        clientData.clientSocket.closeSocket();
        break;
    }

    return true;
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