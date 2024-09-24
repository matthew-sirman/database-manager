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

#define CLIENT_APPLICATION_ID "e89163c2-86fd-4675-ad9e-0d0e7632b9a8"
#define REDIRECT_URL "http://localhost:5000/login/authorize"

/// <summary> 
/// ClientResponseHandler
/// A pure virtual class for setting up any response handling for the client.
/// </summary>
class ClientResponseHandler {
public:
    /// <summary>
    /// Pure virtual method that represents how to handle a message from the server.
    /// </summary>
    /// <param name="message">The message to be processed. It is a rvalue reference
    /// to indicate a transfer of ownership.</param>
    /// <param name="messageSize">The size of the message.</param>
    virtual void onMessageReceived(void*&&message, unsigned messageSize) = 0;
};

/// <summary> 
/// Client
/// Controls the client's networking. 
/// </summary>
class Client {
public:

    /// <summary>
    /// Enum of all connection statuses.
    /// </summary>
    enum class ConnectionStatus {
        /// <summary>
        /// The client successfully connected
        /// </summary>
        SUCCESS,
        /// <summary>
        /// The client is not connected.
        /// </summary>
        NO_CONNECTION,
        /// <summary>
        /// The client could not verify the server.
        /// </summary>
        CREDS_EXCHANGE_FAILED,
        /// <summary>
        /// The JSON web token was invalid.
        /// </summary>
        INVALID_JWT,
        /// <summary>
        /// The repeat token was invalid.
        /// </summary>
        INVALID_REPEAT_TOKEN
    };

    /// <summary>
    /// Constructs a new client, but does not initialise any connections.
    /// </summary>
    /// <param name="refreshRate"></param>
    /// <param name="clientKey"></param>
    /// <param name="serverSignature"></param>
    Client(float refreshRate, RSAKeyPair clientKey, DigitalSignatureKeyPair::Public serverSignature);

    /// <summary>
    /// Destructor the closes the connection.
    /// </summary>
    ~Client();

    /// <summary>
    /// Setup the client by crating the non-blocking TCP socket 
    /// </summary>
    void initialiseClient();

    /// <summary>
    /// Connects to the server.
    /// </summary>
    /// <param name="ipAddress">The IP of the server.</param>
    /// <param name="port">The port of the server.</param>
    /// <param name="authStringCallback">The a callback to open the url for the login page.</param>
    /// <returns>Returns whether the connection was successful.</returns>
    ConnectionStatus connectToServer(const std::string &ipAddress, unsigned port, const std::function<void(const std::string &)> &authStringCallback);

    /// <summary>
    /// Connects to the server using a repeat token.
    /// </summary>
    /// <param name="ipAddress">The IP of the server.</param>
    /// <param name="port">The port of the server.</param>
    /// <param name="repeatToken">The repeat token to reverify with.</param>
    /// <returns></returns>
    ConnectionStatus connectWithToken(const std::string &ipAddress, unsigned port, uint256 repeatToken);
    /// <summary>
    /// Disconnects from the server.
    /// </summary>
    void disconnect();

    /// <summary>
    /// Starts the repeating client loop in another thread.
    /// </summary>
    void startClientLoop();

    /// <summary>
    /// Forces the client loop to close ASAP.
    /// </summary>
    void stopClientLoop();

    /// <summary>
    /// Adds a message to be sent ASAP to the server, in the form of a void buffer and length.
    /// </summary>
    /// <param name="message">The meassage to be sent.</param>
    /// <param name="messageLength">The length of the message.</param>
    void addMessageToSendQueue(const void *message, unsigned messageLength);

    /// <summary>
    /// Adds a message to be sent ASAP to the server, in the form of a string.
    /// </summary>
    /// <param name="message">The message to send.</param>
    void addMessageToSendQueue(const std::string &message);

    /// <summary>
    /// Requests a repeat token from the server.
    /// </summary>
    /// <param name="responseCode"></param>
    void requestRepeatToken(unsigned responseCode = 0);

    /// <summary>
    /// Request the this clients email.
    /// </summary>
    /// <param name="responseCode"></param>
    void requestEmailAddress(unsigned responseCode = 0);

    /// <summary>
    /// Sets the response handler for recieved messages from the server.
    /// </summary>
    /// <param name="handler"></param>
    void setResponseHandler(ClientResponseHandler &handler);


    /// <summary>
    /// Sends a heartbeat to verify the server is alive.
    /// </summary>
    void heartbeat();

    /// <summary>
    /// Checks whether the client is allowed full access to features.
    /// </summary>
    /// <returns>True if the client has full access, false otherwise.</returns>
    bool hasFullAccess() const;

private:
    // The RSA key for establishing a secure and authenticated connection with the server
    RSAKeyPair clientKey;

    // The public key for the server's signature
    DigitalSignatureKeyPair::Public serverSignature;

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

    // Stores the clients access level
    ClientAccess access;
};


#endif //DATABASE_CLIENT_CLIENT_H
