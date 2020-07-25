//
// Created by matthew on 12/05/2020.
//

#ifndef DATABASE_SERVER_SERVER_H
#define DATABASE_SERVER_SERVER_H

#include <string>
#include <vector>
#include <future>
#include <chrono>
#include <unordered_map>
#include <map>
#include <queue>

#include <encrypt.h>
#include <authenticate.h>

#include "TCPSocket.h"
#include "../database/DatabaseManager.h"
#include "../database/Drawing.h"
#include "../../guard.h"

#define CLIENT_APPLICATION_ID "e89163c2-86fd-4675-ad9e-0d0e7632b9a8"

static std::string getNonBlockingInput();

class Server;

struct ClientHandle {
    friend class Server;

    ClientHandle(unsigned id);

private:
    unsigned clientID;
};

struct ClientData {
    friend class Server;

    ClientData(unsigned handleID, const TCPSocket &socket, const AESKey &sessionKey, uint64 sessionToken, uint64 authNonce);

    std::string clientEmail;
    ClientHandle handle;

private:
    TCPSocket clientSocket;
    AESKey clientSessionKey;
    uint64 clientSessionToken;
    uint64 clientAuthNonce;
};

class ServerRequestHandler {
public:
    virtual void onMessageReceived(Server &caller, const ClientHandle &clientHandle, void *message, unsigned messageSize) = 0;
};

// TCP Server class
class Server {
public:
    Server(float refreshRate, RSAKeyPair serverKey, DigitalSignatureKeyPair serverSignature);

    ~Server();

    // Setup the server by creating the non-blocking TCP socket, binding the address
    // and setting the socket to listen.
    void initialiseServer(unsigned short serverPort);

    // Start the main server loop
    void startServer();

    // Close the server
    void closeServer();

    void addMessageToSendQueue(const ClientHandle &clientHandle, const void *message, unsigned messageLength);

    void addMessageToSendQueue(const ClientHandle &clientHandle, const std::string &message);

    void sendRepeatToken(const ClientHandle &clientHandle, unsigned responseCode = 0);

    void connectToDatabaseServer(const std::string &database, const std::string &user, const std::string &password,
                                 const std::string &host = "localhost");

    void setRequestHandler(ServerRequestHandler &handler);

    void setLoggingStream(const std::ostream &stream = std::cout, const std::ostream &errStream = std::cerr);

    void setHeartBeatCycles(int cycles);

    DatabaseManager &databaseManager();

private:
    // The two keys this server has for communicating with clients and signing messages for authenticity
    RSAKeyPair serverKey;
    DigitalSignatureKeyPair serverSignature;

    // The server's TCP socket
    TCPSocket serverSocket;

    // The refresh rate of the server (Hz) - giving this a higher value minimises CPU usage but reduces responsiveness
    float refreshRate;

    int heartBeatCycles = 128;

    // A list of all the clients which are connected, but haven't yet authenticated themselves
    std::vector<ClientData *> waitingClients;

    // A list of all the currently connected and authenticated clients
    std::vector<ClientData *> connectedClients;

    std::unordered_map<unsigned, ClientData *> handleMap;

    std::map<uint256, std::string> repeatTokenMap;

    std::queue<std::pair<unsigned, NetworkMessage *>> sendQueue;
    std::mutex sendQueueMutex;

    ServerRequestHandler *requestHandler = nullptr;

    std::ostream *logStream = &std::cout;
    std::ostream *errorStream = &std::cerr;

    DatabaseManager *dbManager = nullptr;

    // Accepts a new client into the server
    void acceptClient(TCPSocket& clientSocket);

    // Attempt to authenticate a client. Returns true if the client attempts to authenticate; not if it is successful
    bool tryAuthenticateClient(ClientData &clientData);
};


#endif //DATABASE_SERVER_SERVER_H
