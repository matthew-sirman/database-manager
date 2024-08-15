//
// Created by matthew on 12/05/2020.
//

#ifndef DATABASE_SERVER_SERVER_H
#define DATABASE_SERVER_SERVER_H

#include <string>
#include <vector>
#include <future>
#include <chrono>
#include <time.h>
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

/// <summary>\ingroup networking
/// ClientHandle
/// Keeps track of individual clients through handles.
/// </summary>
struct ClientHandle {
    friend class Server;

    /// <summary>
    /// Constructs a new client with a specified ID.
    /// </summary>
    /// <param name="id">ID of client.</param>
    ClientHandle(unsigned id);

private:
    unsigned clientID;
};

/// <summary>\ingroup networking
/// ClientData
/// Stores all data relevant to a client's connection.
/// </summary>
struct ClientData {
    friend class Server;

    /// <summary>
    /// Constructs new ClientData with all relevant information.
    /// </summary>
    /// <param name="handleID">The ID to construct a new ClientHandle.</param>
    /// <param name="socket">The TCPSocket connected through.</param>
    /// <param name="sessionKey">The session key.</param>
    /// <param name="sessionToken">The session token.</param>
    /// <param name="authNonce">The clients authNonce.</param>
    ClientData(unsigned handleID, const TCPSocket &socket, const AESKey &sessionKey, uint64 sessionToken, uint64 authNonce);

    /// <summary>
    /// Stores the client's users' email.
    /// </summary>
    std::string clientEmail;

    /// <summary>
    /// Stores the individual ClientHandle.
    /// </summary>
    ClientHandle handle;

private:
    TCPSocket clientSocket;
    AESKey clientSessionKey;
    uint64 clientSessionToken;
    uint64 clientAuthNonce;
};

/// <summary>\ingroup networking
/// ServerRequestHandler
/// Pure virtual class to help with server request handling.
/// </summary>
class ServerRequestHandler {
public:
    /// <summary>
    /// pure virtual function for handling requests.
    /// </summary>
    /// <param name="caller">A reference to the server that called this function.</param>
    /// <param name="clientHandle">The handle of the client that made the initial request.</param>
    /// <param name="message">The message from the client, as a rvalue reference to indicate
    /// transfer of ownership.</param>
    /// <param name="messageSize">The size of the message.</param>
    virtual void onMessageReceived(Server &caller, const ClientHandle &clientHandle, void*&& message, unsigned messageSize) = 0;
};

/// <summary>\ingroup networking
/// TCP Server class
///</summary>
class Server {
public:
    /// <summary>
    /// Constructs a new server.
    /// </summary>
    /// <param name="refreshRate">How often the server should check for new messages (in Hz).</param>
    /// <param name="serverKey">The server's RSA key.</param>
    /// <param name="serverSignature">The signature of the server.</param>
    Server(float refreshRate, RSAKeyPair serverKey, DigitalSignatureKeyPair serverSignature);

    /// <summary>
    /// Destructor that closes the server.
    /// </summary>
    ~Server();


    /// <summary>
    /// Setup the server by creating the non-blocking TCP socket, binding the address
    /// and setting the socket to listen.
    /// </summary>
    /// <param name="serverPort">The port to open the server on.</param>
    void initialiseServer(unsigned short serverPort);

    /// <summary>
    /// Starts the main server loop.
    /// </summary>
    void startServer();

    /// <summary>
    /// Close the server.
    /// </summary>
    void closeServer();

    /// <summary>
    /// Adds a message to be sent ASAP.
    /// </summary>
    /// <param name="clientHandle">The client to send the message to.</param>
    /// <param name="message">The message as a buffer.</param>
    /// <param name="messageLength">The size of the buffer.</param>
    void addMessageToSendQueue(const ClientHandle &clientHandle, const void *message, unsigned messageLength);

    /// <summary>
    /// Adds a message to be sent ASAP.
    /// </summary>
    /// <param name="clientHandle">The client to send the message to.</param>
    /// <param name="message">The message as a string.</param>
    void addMessageToSendQueue(const ClientHandle &clientHandle, const std::string &message);

    /// <summary>
    /// Broadcast a message to all clients connected to this server.
    /// </summary>
    /// <param name="message">The message to be broadcasted as a buffer.</param>
    /// <param name="messageLength">The size of the buffer.</param>
    void broadcastMessage(const void *message, unsigned messageLength);

    /// <summary>
    /// Broadcast a message to all clients connected to this server.
    /// </summary>
    /// <param name="message">The message to be broadcasted as a string.</param>
    void broadcastMessage(const std::string &message);

    /// <summary>
    /// Send a repeat token to a client, for faster reconnection.
    /// </summary>
    /// <param name="clientHandle">The client to send the repeat token to.</param>
    /// <param name="responseCode">The response code.</param>
    void sendRepeatToken(const ClientHandle &clientHandle, unsigned responseCode = 0);

    /// <summary>
    /// Sends the users email to a client.
    /// </summary>
    /// <param name="clientHandle">The client that requested their email.</param>
    /// <param name="responseCode">The response code.</param>
    void sendEmailAddress(const ClientHandle &clientHandle, unsigned responseCode = 0);

    /// <summary>
    /// Connets this server to its database.
    /// </summary>
    /// <param name="database">the name of the schema to connect to.</param>
    /// <param name="user">The user to connect as.</param>
    /// <param name="password">The password of the user.</param>
    /// <param name="host">The host of the database.</param>
    void connectToDatabaseServer(const std::string &database, const std::string &user, const std::string &password,
                                 const std::string &host = "localhost");

    /// <summary>
    /// Sets a handler for request messages.
    /// </summary>
    /// <param name="handler">The handler to handle requests.</param>
    void setRequestHandler(ServerRequestHandler &handler);

    /// <summary>
    /// Sets output streams for logs, changelogs and errors.
    /// </summary>
    /// <param name="log">A stream to write logs to.</param>
    /// <param name="changelog">A stream to write changelogs to.</param>
    /// <param name="errStream">A stream to write errors to.</param>
    void setLoggingStream(std::ostream *log = &std::cout, std::ostream *changelog = nullptr, std::ostream *errStream = &std::cerr);

    /// <summary>
    /// Writes a message to the changelog, with the client that requested the change.
    /// </summary>
    /// <param name="clientHandle">The handle of the client that requested the change.</param>
    /// <param name="message">The message to log to the changelog.</param>
    void changelogMessage(const ClientHandle &clientHandle, const std::string &message);

    /// <summary>
    /// Gets the current time, formatted as a string.
    /// </summary>
    /// <returns></returns>
    std::string timestamp() const;

    /// <summary>
    /// Sets how many loops are completed by the server loop before sending another heartbeat.
    /// </summary>
    /// <param name="cycles">The number of cycles per heartbeat.</param>
    void setHeartBeatCycles(int cycles);

    /// <summary>
    /// Getter for the database mananger.
    /// </summary>
    /// <returns>The database manager.</returns>
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
    std::ostream *changelogStream = nullptr;
    std::ostream *errorStream = &std::cerr;

    DatabaseManager *dbManager = nullptr;
    std::string databaseHost, databaseUsername, databasePassword, databaseSchema;

    // Accepts a new client into the server
    void acceptClient(TCPSocket& clientSocket);

    // Attempt to authenticate a client. Returns true if the client attempts to authenticate; not if it is successful
    bool tryAuthenticateClient(ClientData &clientData);
};


#endif //DATABASE_SERVER_SERVER_H
