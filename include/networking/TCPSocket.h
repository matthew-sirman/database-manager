//
// Created by matthew on 12/06/2020.
//

#ifndef DATABASE_MANAGER_TCPSOCKET_H
#define DATABASE_MANAGER_TCPSOCKET_H

#include <iostream>
#ifdef _WIN32
#include <Windows.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#endif
#include <functional>
#include <chrono>
#include <signal.h>
#include <thread>
#include <future>

#include "NetworkMessage.h"
#include "../../guard.h"

#define BACKLOG_QUEUE_SIZE 8

#ifdef _WIN32
#define sock_errno WSAGetLastError()
#endif

/// <summary>\ingroup networking
/// Enum for all TCP socket codes.
/// </summary>
enum TCPSocketCode {
    SOCKET_SUCCESS,
    WAS_HEARTBEAT,
    SOCKET_DISCONNECTED,
    ERR_CREATE_SOCKET,
    ERR_SET_SOCKET_OPTIONS,
    ERR_PARSE_IP,
    ERR_BIND_SOCKET,
    ERR_CONNECT,
    ERR_GET_FD_FLAGS,
    ERR_SET_NON_BLOCKING,
    ERR_LISTEN,
    ERR_ACCEPT,
    ERR_SOCKET_DEAD,
    ERR_SEND_FAILED,
    ERR_RECEIVE_FAILED,
    S_NO_DATA
};

/// <summary>\ingroup networking
/// Checks if the code is an error, and if it is it writes the error to errorStream.
/// </summary>
/// <param name="code">The code to check.</param>
/// <param name="errorStream">Stream to write errors to.</param>
void guardTCPSocketCode(TCPSocketCode code, std::ostream &errorStream = std::cerr);

/// <summary>\ingroup networking
/// Checks if the code is an error, and if it is it writes the error to errorStream.
/// </summary>
/// <param name="code">The code to check.</param>
/// <param name="errorStream">Stream to write errors to.</param>
/// <returns>True if no error was found, false otherwise.</returns>
bool safeGuardTCPSocketCode(TCPSocketCode code, std::ostream &errorStream = std::cerr);

/// <summary>\ingroup networking
/// TCPSocket
/// An individual socket.
/// </summary>
struct TCPSocket {
public:
    /// <summary>
    /// Default Constructor.
    /// </summary>
    TCPSocket();

    /// <summary>
    /// Constructs a new TCPSocket with a timeout.
    /// </summary>
    /// <param name="connectionTimeout"></param>
    explicit TCPSocket(float connectionTimeout);

    /// <summary>
    /// Default Destructor.
    /// </summary>
    ~TCPSocket();

    // Setup Functions


    /// <summary>
    /// Creates a new socket.
    /// </summary>
    /// <returns>Status code from creating socket.</returns>
    TCPSocketCode create();

    /// <summary>
    /// Sets the sockets port and ip.
    /// </summary>
    /// <param name="port">Port to bind the socket to.</param>
    /// <param name="ip">IP to bind the socket to.</param>
    /// <returns></returns>
    TCPSocketCode bindToAddress(unsigned short port, const std::string &ip = "");

    /// <summary>
    /// Connects the socket to the server on the given ip and port.
    /// </summary>
    /// <param name="ip">IP of server.</param>
    /// <param name="port">The port of the server.</param>
    /// <returns></returns>
    TCPSocketCode connectToServer(const std::string &ip, unsigned short port);

    /// <summary>
    /// Sets the socket into a nonblocking mode.
    /// </summary>
    /// <returns></returns>
    TCPSocketCode setNonBlocking();

    // Destruction Functions

    /// <summary>
    /// Closes the socket.
    /// </summary>
    void closeSocket();

    // Operation Functions

    /// <summary>
    /// Starts the socket listening for a message.
    /// </summary>
    /// <returns>Status of starting listening.</returns>
    TCPSocketCode beginListen();

    /// <summary>
    /// Listens for any message, and if recieved assigns the socket to a new acceptCallback.
    /// </summary>
    /// <param name="callbackAsync">If true, the acceptCallback is opened on a new thread, if not it takes over this thread.</param>
    /// <returns></returns>
    TCPSocketCode tryAccept(bool callbackAsync = false) const;

    /// <summary>
    /// sends a message through this socket.
    /// </summary>
    /// <param name="message">The message to send.</param>
    /// <returns></returns>
    TCPSocketCode sendMessage(const NetworkMessage &message);

    /// <summary>
    /// Recieves a message from socket.
    /// </summary>
    /// <param name="message">Decodes data from socket into message.</param>
    /// <param name="expectedProtocol">Protocol to expect, fails if its not this protocol or a heartbeat.</param>
    /// <returns>Status code from recieving message.</returns>
    TCPSocketCode receiveMessage(NetworkMessage &message, MessageProtocol expectedProtocol);

    /// <summary>
    /// Waits for a message, then one is recieved calls recieveMessage.
    /// </summary>
    /// <param name="message">Message to pass to recieveMessage.</param>
    /// <param name="expectedProtocol">Protocol to pass to recieveMessage.</param>
    /// <returns>Status code for waiting then recieving message.</returns>
    TCPSocketCode waitForMessage(NetworkMessage &message, MessageProtocol expectedProtocol);

    /// <summary>
    /// sends a heartbeat.
    /// </summary>
    void heartbeat();

    // Properties

    /// <summary>
    /// Checks if socket is open.
    /// </summary>
    /// <returns>True if socket is open, false otherwise.</returns>
    bool open() const;

    /// <summary>
    /// Checks if socket is bound.
    /// </summary>
    /// <returns>True if socket is bound, false otherwise.</returns>
    bool bound() const;

    /// <summary>
    /// Checks if socket is connected.
    /// </summary>
    /// <returns>True if socket is connected, false otherwise.</returns>
    bool connected() const;

    /// <summary>
    /// Checks if socket is blocking.
    /// </summary>
    /// <returns>True if socket is blocking, false otherwise.</returns>
    bool blocking() const;

    /// <summary>
    /// Checks if socket is actively listening.
    /// </summary>
    /// <returns>True if socket is listening, false otherwise.</returns>
    bool listening() const;

    /// <summary>
    /// Checks if socket is dead.
    /// </summary>
    /// <returns>True if socket is dead, false otherwise.</returns>
    bool dead() const;

    /// <summary>
    /// Sets the callback to handle an opened connection.
    /// </summary>
    /// <param name="callback">The callback function.</param>
    void setAcceptCallback(const std::function<void(TCPSocket &&)> &callback);

    /// <summary>
    /// Sets the timeout for the socket.
    /// </summary>
    /// <param name="timeout">The timeout (in ms).</param>
    void setConnectionTimeout(float timeout);

private:
    enum SocketFlags {
        SOCKET_OPEN = 0x01u,
        SOCKET_BOUND = 0x02u,
        SOCKET_CONNECTED = 0x04u,
        SOCKET_LISTENING = 0x08u,
        SOCKET_BLOCKING = 0x10u,
        SOCKET_WAITING = 0x20u,
        SOCKET_DEAD = 0x40u
    };

#ifdef _WIN32
    SOCKET sock;
#else
    int fd = -1;
#endif

    // FLAGS: UNUSED, Dead, Waiting, Blocking, Listening, Connected, Bound, Socket Open
    unsigned char flags = SOCKET_BLOCKING;

    std::function<void(TCPSocket &&)> acceptCallback;

    std::chrono::system_clock::time_point lastHeard;

    float connectionTimeout = 30.0f;

#ifdef _WIN32
    // Windows Specific

public:
    /// <summary>
    /// Starts windows socket api.
    /// </summary>
    /// <returns>0 for success, 1 for failiure.</returns>
    static int initialiseWSA();

    /// <summary>
    /// Cleans up windows socket api.
    /// </summary>
    static void cleanupWSA();

private:
    static WSADATA wsaData;

#endif
};

#endif //DATABASE_MANAGER_TCPSOCKET_H
