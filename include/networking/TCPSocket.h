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

void guardTCPSocketCode(TCPSocketCode code, std::ostream &errorStream = std::cerr);

bool safeGuardTCPSocketCode(TCPSocketCode code, std::ostream &errorStream = std::cerr);

struct TCPSocket {
public:
    TCPSocket();

    explicit TCPSocket(float connectionTimeout);

    ~TCPSocket();

    // Setup Functions

    TCPSocketCode create();

    TCPSocketCode bindToAddress(unsigned short port, const std::string &ip = "");

    TCPSocketCode connectToServer(const std::string &ip, unsigned short port);

    TCPSocketCode setNonBlocking();

    // Destruction Functions

    void closeSocket();

    // Operation Functions

    TCPSocketCode beginListen();

    TCPSocketCode tryAccept(bool callbackAsync = false) const;

    TCPSocketCode sendMessage(const NetworkMessage &message);

    TCPSocketCode receiveMessage(NetworkMessage &message, MessageProtocol expectedProtocol);

    TCPSocketCode waitForMessage(NetworkMessage &message, MessageProtocol expectedProtocol);

    void heartbeat();

    // Properties

    bool open() const;

    bool bound() const;

    bool connected() const;

    bool blocking() const;

    bool listening() const;

    bool dead() const;

    void setAcceptCallback(const std::function<void(TCPSocket &)> &callback);

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

    std::function<void(TCPSocket &)> acceptCallback;

    std::chrono::system_clock::time_point lastHeard;

    float connectionTimeout = 30.0f;

#ifdef _WIN32
    // Windows Specific

public:
    static int initialiseWSA();

    static void cleanupWSA();

private:
    static WSADATA wsaData;

#endif
};

#endif //DATABASE_MANAGER_TCPSOCKET_H
