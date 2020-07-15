#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
//
// Created by matthew on 12/06/2020.
//

#include "../../include/networking/TCPSocket.h"

// Set the SIGPIPE signal to ignore
void (*SIG_PIPE_HANDLER)(int) = signal(SIGPIPE, SIG_IGN);

void guardTCPSocketCode(TCPSocketCode code, std::ostream &errorStream) {
    switch (code) {
        case ERR_SOCKET_DEAD:
        case ERR_SEND_FAILED:
        case NO_DATA:
        case SOCKET_SUCCESS:
            return;
        case ERR_CREATE_SOCKET: ERROR_TO("Failed to create socket", errorStream)
        case ERR_SET_SOCKET_OPTIONS: ERROR_TO("Failed to set TCP socket to reuse address and port", errorStream)
        case ERR_PARSE_IP: ERROR_TO("Failed to parse IP address. Check that it is in the correct format", errorStream)
        case ERR_BIND_SOCKET: ERROR_TO("Failed to bind socket to provided address and port.", errorStream)
        case ERR_CONNECT: ERROR_TO("Failed to connect to server", errorStream)
        case ERR_GET_FD_FLAGS: ERROR_TO("Failed to get flags from TCP socket file descriptor", errorStream)
        case ERR_SET_NON_BLOCKING: ERROR_TO("Failed to set TCP socket to non blocking", errorStream)
        case ERR_LISTEN: ERROR_TO("Failed to set server to listen", errorStream)
        case ERR_ACCEPT: ERROR_TO("Failed to accept client", errorStream)
        case ERR_RECEIVE_FAILED: ERROR_TO("Failed to receive data from socket", errorStream)
    }
}

bool safeGuardTCPSocketCode(TCPSocketCode code, std::ostream &errorStream) {
    switch (code) {
        case ERR_SOCKET_DEAD:
        case ERR_SEND_FAILED:
        case NO_DATA:
        case SOCKET_SUCCESS:
            return true;
        case ERR_CREATE_SOCKET: SAFE_ERROR_TO("Failed to create socket", errorStream)
            return false;
        case ERR_SET_SOCKET_OPTIONS: SAFE_ERROR_TO("Failed to set TCP socket to reuse address and port", errorStream)
            return false;
        case ERR_PARSE_IP: SAFE_ERROR_TO("Failed to parse IP address. Check that it is in the correct format",
                                         errorStream)
            return false;
        case ERR_BIND_SOCKET: SAFE_ERROR_TO("Failed to bind socket to provided address and port", errorStream)
            return false;
        case ERR_CONNECT: SAFE_ERROR_TO("Failed to connect to server", errorStream)
            return false;
        case ERR_GET_FD_FLAGS: SAFE_ERROR_TO("Failed to get flags from TCP socket file descriptor", errorStream)
            return false;
        case ERR_SET_NON_BLOCKING: SAFE_ERROR_TO("Failed to set TCP socket to non blocking", errorStream)
            return false;
        case ERR_LISTEN: SAFE_ERROR_TO("Failed to set server to listen", errorStream)
            return false;
        case ERR_ACCEPT: SAFE_ERROR_TO("Failed to accept client", errorStream)
            return false;
        case ERR_RECEIVE_FAILED: SAFE_ERROR_TO("Failed to receive data from socket", errorStream)
            return false;
    }
}

TCPSocket::TCPSocket() = default;

TCPSocket::TCPSocket(float connectionTimeout) {
    this->connectionTimeout = connectionTimeout;
}

TCPSocket::~TCPSocket() = default;

TCPSocketCode TCPSocket::create() {
    // Create a TCP socket with the default protocol
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return ERR_CREATE_SOCKET;
    }

    // Set the TCP socket to reuse the port and address (if necessary)
    int tcpOptions = 0;

    if ((setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &tcpOptions, sizeof(tcpOptions))) < 0) {
        return ERR_SET_SOCKET_OPTIONS;
    }

    flags |= SOCKET_OPEN;

    return SOCKET_SUCCESS;
}

TCPSocketCode TCPSocket::bindToAddress(unsigned short port, const std::string &ip) {
    sockaddr_in address{};
    address.sin_port = htons(port);

    if (ip.empty()) {
        address.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, ip.c_str(), &address.sin_addr) < 0) {
            return ERR_PARSE_IP;
        }
    }

    if (bind(fd, (struct sockaddr *) &address, (socklen_t) sizeof(sockaddr_in)) < 0) {
        return ERR_BIND_SOCKET;
    }

    flags |= SOCKET_BOUND;

    return SOCKET_SUCCESS;
}

TCPSocketCode TCPSocket::connectToServer(const std::string &ip, unsigned short port) {
    sockaddr_in serverAddress{};

    // Set the address family
    serverAddress.sin_family = AF_INET;
    // Set the port
    serverAddress.sin_port = htons(port);

    // Set the address through converting the string to the correct format
    if (inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr.s_addr) < 0) {
        return ERR_PARSE_IP;
    }

    // Connect to the server address
    if (connect(fd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        return ERR_CONNECT;
    }

    flags |= SOCKET_CONNECTED;

    return SOCKET_SUCCESS;
}

TCPSocketCode TCPSocket::setNonBlocking() {
    unsigned int socketFlags;

    // Get the current flags from the file descriptor
    if ((socketFlags = fcntl(fd, F_GETFL)) < 0) {
        return ERR_GET_FD_FLAGS;
    }

    // Set the flags to the original flags with the O_NONBLOCK bit set to true, hence
    // making the socket non blocking
    if ((fcntl(fd, F_SETFL, socketFlags | O_NONBLOCK)) < 0) {
        return ERR_SET_NON_BLOCKING;
    }

    flags &= (unsigned char) (~SOCKET_BLOCKING);

    return SOCKET_SUCCESS;
}

void TCPSocket::closeSocket() {
    if (open()) {
        shutdown(fd, SHUT_RDWR);
        close(fd);
        flags &= (unsigned char) (~SOCKET_OPEN);
    }
}

//void TCPSocket::shutdownSocket() {
//    if (open()) {
//        shutdown(fd, SHUT_RDWR);
//        flags &= (unsigned char) (~SOCKET_OPEN);
//    }
//}

TCPSocketCode TCPSocket::beginListen() {
    // Set the server to begin listening for clients
    if ((listen(fd, BACKLOG_QUEUE_SIZE)) < 0) {
        return ERR_LISTEN;
    }

    flags |= SOCKET_LISTENING;

    return SOCKET_SUCCESS;
}

TCPSocketCode TCPSocket::tryAccept(bool callbackAsync) const {
    sockaddr_in clientAddress{};
    socklen_t clientAddressSize = sizeof(clientAddress);

    int clientSocketFd = accept(fd, (struct sockaddr *) &clientAddress, &clientAddressSize);

    if (clientSocketFd == -1) {
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            return ERR_ACCEPT;
        }
        return NO_DATA;
    }

    TCPSocket *acceptedSocket = new TCPSocket();
    acceptedSocket->fd = clientSocketFd;

    unsigned int socketFlags;

    // Get the current flags from the file descriptor
    if ((socketFlags = fcntl(fd, F_GETFL)) < 0) {
//        STD_ERROR("Failed to get flags from TCP socket file descriptor")
        return ERR_GET_FD_FLAGS;
    }

    acceptedSocket->flags |= SOCKET_OPEN;
    acceptedSocket->flags |= SOCKET_CONNECTED;

    if (socketFlags & O_NONBLOCK) {
        acceptedSocket->flags &= (unsigned char) (~SOCKET_BLOCKING);
    }

    if (acceptCallback) {
        if (callbackAsync) {
            std::thread(acceptCallback, std::ref(*acceptedSocket)).detach();
        } else {
            acceptCallback(*acceptedSocket);
        }
    }

    return SOCKET_SUCCESS;
}

TCPSocketCode TCPSocket::sendMessage(const NetworkMessage &message) {
    if (flags & SOCKET_DEAD) {
        return ERR_SOCKET_DEAD;
    }

    bool success = send(fd, message.dataStream(), message.dataStreamSize(), 0) >= 0;

    if (errno == EPIPE) {
        flags |= SOCKET_DEAD;
        return ERR_SOCKET_DEAD;
    }

    return success ? SOCKET_SUCCESS : ERR_SEND_FAILED;
}

TCPSocketCode TCPSocket::receiveMessage(NetworkMessage &message, MessageProtocol expectedProtocol) {
    unsigned char readBuffer[BUFFER_CHUNK_SIZE];

    if (flags & SOCKET_WAITING) {
        std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - lastHeard;

        if (elapsed.count() > connectionTimeout) {
            flags |= SOCKET_DEAD;
            return ERR_SOCKET_DEAD;
        }
    }

    if (recv(fd, readBuffer, BUFFER_CHUNK_SIZE, 0) <= 0) {
        return NO_DATA;
    }

    message.clear();

    while (true) {
        DecodeStatus status = message.decode(readBuffer, BUFFER_CHUNK_SIZE);

        if (status == DECODED) {
            break;
        }
        if (status == DECODE_ERROR) {
            // If the decoding was erroneous, clear the message and set the error bit. We clear the message
            // to avoid attacks which flood the server's memory by sending messages with large sized headers
            // but no content
            message.clear();
            message.setError();
            return ERR_RECEIVE_FAILED;
        }

        if (recv(fd, readBuffer, BUFFER_CHUNK_SIZE, 0) <= 0) {
            // If we are expecting to receive more data, but there is no more data, then there is an error.
            message.clear();
            message.setError();
            return ERR_RECEIVE_FAILED;
        }

        if (message.protocol() != expectedProtocol) {
            // If the message we are receiving isn't using the correct protocol, then there is an error
            message.clear();
            message.setError();
            return ERR_RECEIVE_FAILED;
        }
    }

    if (message.protocol() == HEARTBEAT) {
        flags &= (unsigned char) (~SOCKET_WAITING);
    }

    return SOCKET_SUCCESS;
}

TCPSocketCode TCPSocket::waitForMessage(NetworkMessage &message, MessageProtocol expectedProtocol) {
    TCPSocketCode code = NO_DATA;

    while (code == NO_DATA) {
        code = receiveMessage(message, expectedProtocol);
    }

    return code;
}

void TCPSocket::heartbeat() {
    sendMessage(NetworkMessage("heartbeat", HEARTBEAT));
    flags |= SOCKET_WAITING;
    lastHeard = std::chrono::system_clock::now();
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

bool TCPSocket::dead() const {
    return flags & SOCKET_DEAD;
}

void TCPSocket::setAcceptCallback(const std::function<void(TCPSocket &)> &callback) {
    this->acceptCallback = callback;
}

#pragma clang diagnostic pop