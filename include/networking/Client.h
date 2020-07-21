//
// Created by matthew on 12/05/2020.
//

#ifndef DATABASE_CLIENT_CLIENT_H
#define DATABASE_CLIENT_CLIENT_H

#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include <queue>

#include <encrypt.h>
#include <authenticate.h>

#include "TCPSocket.h"
#include "../../guard.h"

#define CLIENT_APPLICATION_ID "1f4c53b0-56be-4ecb-9c90-3c7b1294da44"
#define REDIRECT_URL "http://localhost:5000/login/authorize"

class ClientResponseHandler {
public:
    virtual void onMessageReceived(void *message, unsigned messageSize) = 0;
};

class Client {
public:
    Client(float refreshRate, RSAKeyPair clientKey, PublicKey serverSignature);

    ~Client();

    // Setup the client by crating the non-blocking TCP socket
    void initialiseClient();

    // Connect to the server. Returns whether the connection was successful
    bool connectToServer(const std::string &ipAddress, unsigned port, const std::function<void(const std::string &)> &authStringCallback);

    // Connect to the server with a repeat token. Returns whether the connection was successful
    bool connectWithToken(const std::string &ipAddress, unsigned port, uint256 repeatToken);

    // Begins the client loop in another thread (so applications can run as usual)
    void startClientLoop();

    // Force close the client loop thread
    void stopClientLoop();

    void addMessageToSendQueue(const void *message, unsigned messageLength);

    void addMessageToSendQueue(const std::string &message);

    void setResponseHandler(ClientResponseHandler &handler);

private:
    // The RSA key for establishing a secure and authenticated connection with the server
    RSAKeyPair clientKey;

    // The public key for the server's signature
    PublicKey serverSignature;

    // The address of the server stored when we connect
    sockaddr_in serverAddress{};

    // The client's TCP socket
    TCPSocket clientSocket;

    // The session key provided by the server for this communication
    AESKey sessionKey;
    // The session token provided by the server for this communication to verify this client is still who they claim
    uint64 sessionToken{};

    // The thread containing the client server loop
    std::thread clientLoopThread;

    // Flag to determine if the client loop is currently active
    bool clientLoopRunning = false;

    // A queue of messages to be sent to the server from this client
    std::queue<NetworkMessage *> sendQueue;
    std::mutex sendQueueMutex;

    ClientResponseHandler *responseHandler = nullptr;

    // The refresh of the client loop (Hz) - trade-off between CPU usage and responsiveness
    float refreshRate;

    // Internal function to manage the client loop
    void clientLoop();
};


#endif //DATABASE_CLIENT_CLIENT_H
