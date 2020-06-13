//
// Created by matthew on 12/06/2020.
//

#include "../include/TCPSocket.h"

TCPSocket::TCPSocket() = default;

TCPSocket::~TCPSocket() {
    closeSocket();
    shutdownServerSocket();
}

void TCPSocket::create() {
    // Create a TCP socket with the default protocol
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        ERROR("Failed to create socket.")
    }

    // Set the TCP socket to reuse the port and address (if necessary)
    int tcpOptions;

    if ((setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &tcpOptions, sizeof(tcpOptions))) < 0) {
        ERROR("Failed to set TCP socket to reuse address and port")
    }

    flags |= SOCKET_OPEN;
}

void TCPSocket::bindToAddress(unsigned short port, const std::string &ip) {
    sockaddr_in address{};
    address.sin_port = htons(port);

    if (ip.empty()) {
        address.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, ip.c_str(), &address.sin_addr) < 0) {
            ERROR("Failed to parse IP address. Check that it is in the correct format.")
        }
    }

    if (bind(fd, (struct sockaddr *) &address, (socklen_t) sizeof(sockaddr_in)) < 0) {
        ERROR("Failed to bind socket to provided address and port.")
    }

    flags |= SOCKET_BOUND;
}

void TCPSocket::connectToServer(const std::string &ip, unsigned short port) {
    sockaddr_in serverAddress{};

    // Set the address family
    serverAddress.sin_family = AF_INET;
    // Set the port
    serverAddress.sin_port = htons(port);

    // Set the address through converting the string to the correct format
    if (inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr.s_addr) < 0) {
        ERROR("Invalid IP address")
    }

    // Connect to the server address
    if (connect(fd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        ERROR("Failed to connect to server")
    }

    flags |= SOCKET_CONNECTED;
}

void TCPSocket::setNonBlocking() {
    unsigned int socketFlags;

    // Get the current flags from the file descriptor
    if ((socketFlags = fcntl(fd, F_GETFL)) < 0) {
        ERROR("Failed to get flags from TCP socket file descriptor")
    }

    // Set the flags to the original flags with the O_NONBLOCK bit set to true, hence
    // making the socket non blocking
    if ((fcntl(fd, F_SETFL, socketFlags | O_NONBLOCK)) < 0) {
        ERROR("Failed to set TCP socket to non blocking")
    }

    flags &= (unsigned char) (~SOCKET_BLOCKING);
}

void TCPSocket::closeSocket() {
    if (open()) {
        close(fd);
        flags &= (unsigned char) (~SOCKET_OPEN);
    }
}

void TCPSocket::shutdownServerSocket() {
    if (bound()) {
        shutdown(fd, SHUT_RDWR);
        flags &= (unsigned char) (~SOCKET_BOUND);
    }
}

void TCPSocket::beginListen() {
    // Set the server to begin listening for clients
    if ((listen(fd, BACKLOG_QUEUE_SIZE)) < 0) {
        ERROR("Failed to set server to listen")
    }

    flags |= SOCKET_LISTENING;
}

bool TCPSocket::tryAccept() const {
    sockaddr_in clientAddress{};
    socklen_t clientAddressSize = sizeof(clientAddress);

    int clientSocketFd = accept(fd, (struct sockaddr *) &clientAddress, &clientAddressSize);

    if (clientSocketFd == -1) {
        if (errno != EWOULDBLOCK) {
            ERROR("Failed to accept client")
        }
        return false;
    }

    TCPSocket acceptedSocket;
    acceptedSocket.fd = clientSocketFd;

    if (acceptCallback) {
        acceptCallback(acceptedSocket);
    }

    return true;
}

bool TCPSocket::sendMessage(const NetworkMessage &message) const {
    return send(fd, message.dataStream(), message.dataStreamSize(), 0) >= 0;
}

bool TCPSocket::receiveMessage(NetworkMessage &message) const {
    unsigned char readBuffer[BUFFER_CHUNK_SIZE];

    if (recv(fd, readBuffer, BUFFER_CHUNK_SIZE, 0) < 0) {
        return false;
    }

    message.clear();

    while (!message.decode(readBuffer, BUFFER_CHUNK_SIZE)) {
        recv(fd, readBuffer, BUFFER_CHUNK_SIZE, 0);
    }

    return true;
}

bool TCPSocket::open() const {
    return flags & SOCKET_OPEN;
}

bool TCPSocket::bound() const {
    return flags & SOCKET_BOUND;
}

bool TCPSocket::connected() const {
    return flags & SOCKET_CONNECTED;
}

bool TCPSocket::blocking() const {
    return flags & SOCKET_BLOCKING;
}

bool TCPSocket::listening() const {
    return flags & SOCKET_LISTENING;
}

void TCPSocket::setAcceptCallback(const std::function<void(const TCPSocket &)> &callback) {
    this->acceptCallback = callback;
}
