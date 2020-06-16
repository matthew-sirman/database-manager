//
// Created by matthew on 12/06/2020.
//

#ifndef DATABASE_MANAGER_TCPSOCKET_H
#define DATABASE_MANAGER_TCPSOCKET_H

#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <functional>

#include "NetworkMessage.h"
#include "../guard.h"

#define BACKLOG_QUEUE_SIZE 8

struct TCPSocket {
public:
    TCPSocket();

    ~TCPSocket();

    // Setup Functions

    void create();

    void bindToAddress(unsigned short port, const std::string &ip = "");

    void connectToServer(const std::string &ip, unsigned short port);

    void setNonBlocking();

    // Destruction Functions

    void closeSocket();

    void shutdownServerSocket();

    // Operation Functions

    void beginListen();

    bool tryAccept() const;

    bool sendMessage(const NetworkMessage &message) const;

    bool receiveMessage(NetworkMessage &message) const;

    // Properties

    bool open() const;

    bool bound() const;

    bool connected() const;

    bool blocking() const;

    bool listening() const;

    void setAcceptCallback(const std::function<void(TCPSocket &)> &callback);

//    int TEST_getFd() const {return fd;}

private:
    enum SocketFlags {
        SOCKET_OPEN = 0x01u,
        SOCKET_BOUND = 0x02u,
        SOCKET_CONNECTED = 0x04u,
        SOCKET_LISTENING = 0x08u,
        SOCKET_BLOCKING = 0x10u
    };

    int fd = -1;

    // FLAGS: UNUSED, UNUSED, UNUSED, Blocking, Listening, Connected, Bound, Socket Open
    unsigned char flags = SOCKET_BLOCKING;

    std::function<void(TCPSocket &)> acceptCallback;
};


#endif //DATABASE_MANAGER_TCPSOCKET_H
