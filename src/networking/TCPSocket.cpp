#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
//
// Created by matthew on 12/06/2020.
//

#include "../../include/networking/TCPSocket.h"

// Set the SIGPIPE signal to ignore
#ifdef _WIN32

#else
void (*SIG_PIPE_HANDLER)(int) = signal(SIGPIPE, SIG_IGN);
#endif

void guardTCPSocketCode(TCPSocketCode code, std::ostream &errorStream) {
    switch (code) {
        case ERR_SOCKET_DEAD:
        case ERR_SEND_FAILED:
        case S_NO_DATA:
        case WAS_HEARTBEAT:
        case SOCKET_SUCCESS:
            return;
        case ERR_CREATE_SOCKET: SOCK_ERROR_TO("Failed to create socket", errorStream)
        case ERR_SET_SOCKET_OPTIONS: SOCK_ERROR_TO("Failed to set TCP socket to reuse address and port", errorStream)
        case ERR_PARSE_IP: SOCK_ERROR_TO("Failed to parse IP address. Check that it is in the correct format", errorStream)
        case ERR_BIND_SOCKET: SOCK_ERROR_TO("Failed to bind socket to provided address and port.", errorStream)
        case ERR_CONNECT: SOCK_ERROR_TO("Failed to connect to server", errorStream)
        case ERR_GET_FD_FLAGS: SOCK_ERROR_TO("Failed to get flags from TCP socket file descriptor", errorStream)
        case ERR_SET_NON_BLOCKING: SOCK_ERROR_TO("Failed to set TCP socket to non blocking", errorStream)
        case ERR_LISTEN: SOCK_ERROR_TO("Failed to set server to listen", errorStream)
        case ERR_ACCEPT: SOCK_ERROR_TO("Failed to accept client", errorStream)
        case ERR_RECEIVE_FAILED: SOCK_ERROR_TO("Failed to receive data from socket", errorStream)
    }
}

bool safeGuardTCPSocketCode(TCPSocketCode code, std::ostream &errorStream) {
    switch (code) {
        case ERR_SOCKET_DEAD:
        case ERR_SEND_FAILED:
        case S_NO_DATA:
        case WAS_HEARTBEAT:
        case SOCKET_SUCCESS:
            return true;
        case ERR_CREATE_SOCKET: SAFE_SOCK_ERROR_TO("Failed to create socket", errorStream)
            return false;
        case ERR_SET_SOCKET_OPTIONS: SAFE_SOCK_ERROR_TO("Failed to set TCP socket to reuse address and port", errorStream)
            return false;
        case ERR_PARSE_IP: SAFE_SOCK_ERROR_TO("Failed to parse IP address. Check that it is in the correct format",
                                         errorStream)
            return false;
        case ERR_BIND_SOCKET: SAFE_SOCK_ERROR_TO("Failed to bind socket to provided address and port", errorStream)
            return false;
        case ERR_CONNECT: SAFE_SOCK_ERROR_TO("Failed to connect to server", errorStream)
            return false;
        case ERR_GET_FD_FLAGS: SAFE_SOCK_ERROR_TO("Failed to get flags from TCP socket file descriptor", errorStream)
            return false;
        case ERR_SET_NON_BLOCKING: SAFE_SOCK_ERROR_TO("Failed to set TCP socket to non blocking", errorStream)
            return false;
        case ERR_LISTEN: SAFE_SOCK_ERROR_TO("Failed to set server to listen", errorStream)
            return false;
        case ERR_ACCEPT: SAFE_SOCK_ERROR_TO("Failed to accept client", errorStream)
            return false;
        case ERR_RECEIVE_FAILED: SAFE_SOCK_ERROR_TO("Failed to receive data from socket", errorStream)
            return false;
    }
    return false;
}

TCPSocket::TCPSocket() = default;

TCPSocket::TCPSocket(float connectionTimeout) {
    this->connectionTimeout = connectionTimeout;
}

TCPSocket::~TCPSocket() = default;

TCPSocketCode TCPSocket::create() {
#ifdef _WIN32
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        return ERR_CREATE_SOCKET;
    }

    BOOL reuseAddr = TRUE;

    if ((setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &reuseAddr, sizeof(BOOL))) != 0) {
        return ERR_SET_SOCKET_OPTIONS;
    }
#else
    // Create a TCP socket with the default protocol
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return ERR_CREATE_SOCKET;
    }

    // Set the TCP socket to reuse the port and address (if necessary)
    int tcpOptions = 0;

    if ((setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &tcpOptions, sizeof(tcpOptions))) < 0) {
        return ERR_SET_SOCKET_OPTIONS;
    }
#endif

    flags |= SOCKET_OPEN;

    return SOCKET_SUCCESS;
}

TCPSocketCode TCPSocket::bindToAddress(unsigned short port, const std::string &ip) {
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

#ifdef _WIN32
    if (ip.empty()) {
        address.sin_addr.s_addr = INADDR_ANY;
    }
    else {
        address.sin_addr.s_addr = inet_addr(ip.c_str());
    }

    if (bind(sock, (SOCKADDR *) &address, sizeof(address)) == SOCKET_ERROR) {
        return ERR_BIND_SOCKET;
    }
#else
    if (ip.empty()) {
        address.sin_addr.s_addr = INADDR_ANY;
    }
    else {
        if (inet_pton(AF_INET, ip.c_str(), &address.sin_addr) < 0) {
            return ERR_PARSE_IP;
        }
    }

    if (bind(fd, (struct sockaddr *) &address, (socklen_t) sizeof(sockaddr_in)) < 0) {
        return ERR_BIND_SOCKET;
    }
#endif

    flags |= SOCKET_BOUND;

    return SOCKET_SUCCESS;
}

TCPSocketCode TCPSocket::connectToServer(const std::string &ip, unsigned short port) {
    sockaddr_in serverAddress{};

    // Set the address family
    serverAddress.sin_family = AF_INET;

    // Set the port
    serverAddress.sin_port = htons(port);

#ifdef _WIN32

    serverAddress.sin_addr.s_addr = inet_addr(ip.c_str());

    if (connect(sock, (SOCKADDR *) &serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        return ERR_CONNECT;
    }

#else

    // Set the address through converting the string to the correct format
    if (inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr.s_addr) < 0) {
        return ERR_PARSE_IP;
    }

    // Connect to the server address
    if (connect(fd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        return ERR_CONNECT;
    }
#endif

    flags |= SOCKET_CONNECTED;

    return SOCKET_SUCCESS;
}

TCPSocketCode TCPSocket::setNonBlocking() {
    unsigned int socketFlags;

#ifdef _WIN32

    unsigned long nonBlocking = 1;

    if (ioctlsocket(sock, FIONBIO, (unsigned long *) &nonBlocking) == SOCKET_ERROR) {
        return ERR_SET_NON_BLOCKING;
    }

#else

    // Get the current flags from the file descriptor
    if ((socketFlags = fcntl(fd, F_GETFL)) < 0) {
        return ERR_GET_FD_FLAGS;
    }

    // Set the flags to the original flags with the O_NONBLOCK bit set to true, hence
    // making the socket non blocking
    if ((fcntl(fd, F_SETFL, socketFlags | O_NONBLOCK)) < 0) {
        return ERR_SET_NON_BLOCKING;
    }

#endif

    flags &= (unsigned char) (~SOCKET_BLOCKING);

    return SOCKET_SUCCESS;
}

void TCPSocket::closeSocket() {
    if (open()) {

#ifdef _WIN32
        shutdown(sock, SD_BOTH);
        closesocket(sock);
#else
        shutdown(fd, SHUT_RDWR);
        close(fd);
#endif

        flags &= (unsigned char) (~SOCKET_OPEN);
    }
}

TCPSocketCode TCPSocket::beginListen() {
    // Set the server to begin listening for clients

#ifdef _WIN32
    if (listen(sock, BACKLOG_QUEUE_SIZE) == SOCKET_ERROR) {
        return ERR_LISTEN;
    }
#else
    if ((listen(fd, BACKLOG_QUEUE_SIZE)) < 0) {
        return ERR_LISTEN;
    }
#endif

    flags |= SOCKET_LISTENING;

    return SOCKET_SUCCESS;
}

TCPSocketCode TCPSocket::tryAccept(bool callbackAsync) const {
    sockaddr_in clientAddress{};
    socklen_t clientAddressSize = sizeof(clientAddress);

    TCPSocket *acceptedSocket = new TCPSocket();


#ifdef _WIN32
    SOCKET clientSocket = INVALID_SOCKET;

    if ((clientSocket = accept(sock, (SOCKADDR *) &clientAddress, &clientAddressSize)) == INVALID_SOCKET) {
        if (sock_errno != WSAEWOULDBLOCK) {
            return ERR_ACCEPT;
        }
        return S_NO_DATA;
    }

    acceptedSocket->sock = clientSocket;

    // There is no way to determine if windows sockets are non blocking.

#else
    int clientSocketFd = accept(fd, (struct sockaddr *) &clientAddress, &clientAddressSize);

    if (clientSocketFd == -1) {
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            return ERR_ACCEPT;
        }
        return S_NO_DATA;
    }

    acceptedSocket->fd = clientSocketFd;

    unsigned int socketFlags;

    // Get the current flags from the file descriptor
    if ((socketFlags = fcntl(fd, F_GETFL)) < 0) {
        return ERR_GET_FD_FLAGS;
    }

    if (socketFlags & O_NONBLOCK) {
        acceptedSocket->flags &= (unsigned char) (~SOCKET_BLOCKING);
    }
#endif

    acceptedSocket->flags |= SOCKET_OPEN;
    acceptedSocket->flags |= SOCKET_CONNECTED;

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

#ifdef _WIN32
    FD_SET writeSet;
    FD_ZERO(&writeSet);

    unsigned total;

    FD_SET(sock, &writeSet);

    if ((total = select(0, nullptr, &writeSet, nullptr, nullptr)) == SOCKET_ERROR) {
        return ERR_SEND_FAILED;
    }
    if (!FD_ISSET(sock, &writeSet)) {
        return ERR_SEND_FAILED;
    }

    bool success = send(sock, (const char *) message.dataStream(), message.dataStreamSize(), 0) != SOCKET_ERROR;

    if (sock_errno == WSAECONNRESET) {
        flags |= SOCKET_DEAD;
        return ERR_SOCKET_DEAD;
    }
#else
    bool success = send(fd, message.dataStream(), message.dataStreamSize(), 0) >= 0;

    if (errno == EPIPE) {
        flags |= SOCKET_DEAD;
        return ERR_SOCKET_DEAD;
    }
#endif

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

#ifdef _WIN32
    if (recv(sock, (char *) readBuffer, BUFFER_CHUNK_SIZE, 0) <= 0) {
        return S_NO_DATA;
    }
#else
    if (recv(fd, readBuffer, BUFFER_CHUNK_SIZE, 0) <= 0) {
        return S_NO_DATA;
    }
#endif

    message.clear();

    while (true) {
        DecodeStatus status = message.decode(readBuffer, BUFFER_CHUNK_SIZE);

        if (message.protocol() == MessageProtocol::HEARTBEAT && status == DecodeStatus::DECODED) {
            switch (*((HeartbeatMode *)message.getMessageData())) {
                case HeartbeatMode::REQUEST: {
                    HeartbeatMode response = HeartbeatMode::RESPONSE;
                    sendMessage(NetworkMessage(&response, sizeof(HeartbeatMode), MessageProtocol::HEARTBEAT));
                    return WAS_HEARTBEAT;
                }
                case HeartbeatMode::RESPONSE:
                    flags &= (unsigned char)(~SOCKET_WAITING);
                    return WAS_HEARTBEAT;
            }
        }

        if (message.protocol() != expectedProtocol) {
            // If the message we are receiving isn't using the correct protocol, then there is an error
            message.clear();
            message.setError();
            return ERR_RECEIVE_FAILED;
        }

        if (status == DecodeStatus::DECODED) {
            break;
        }
        if (status == DecodeStatus::DECODE_ERROR) {
            // If the decoding was erroneous, clear the message and set the error bit. We clear the message
            // to avoid attacks which flood the server's memory by sending messages with large sized headers
            // but no content
            message.clear();
            message.setError();
            return ERR_RECEIVE_FAILED;
        }

#ifdef _WIN32
        if (recv(sock, (char *) readBuffer, BUFFER_CHUNK_SIZE, 0) <= 0) {
            message.clear();
            message.setError();
            return ERR_RECEIVE_FAILED;
        }
#else
        if (recv(fd, readBuffer, BUFFER_CHUNK_SIZE, 0) <= 0) {
            // If we are expecting to receive more data, but there is no more data, then there is an error.
            message.clear();
            message.setError();
            return ERR_RECEIVE_FAILED;
        }
#endif
    }

    return SOCKET_SUCCESS;
}

TCPSocketCode TCPSocket::waitForMessage(NetworkMessage &message, MessageProtocol expectedProtocol) {

#ifdef _WIN32

    FD_SET readSet;
    FD_ZERO(&readSet);

    unsigned total;

    FD_SET(sock, &readSet);

    if ((total = select(0, &readSet, nullptr, nullptr, nullptr)) == SOCKET_ERROR) {
        return ERR_RECEIVE_FAILED;
    }
    if (FD_ISSET(sock, &readSet)) {
        return receiveMessage(message, expectedProtocol);
    }

#else
    TCPSocketCode code = S_NO_DATA;

    while (code == S_NO_DATA) {
        code = receiveMessage(message, expectedProtocol);
    }

    return code;
#endif
}

void TCPSocket::heartbeat() {
    HeartbeatMode mode = HeartbeatMode::REQUEST;
    sendMessage(NetworkMessage(&mode, sizeof(HeartbeatMode), MessageProtocol::HEARTBEAT));
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

void TCPSocket::setConnectionTimeout(float timeout) {
    connectionTimeout = timeout;
}

#ifdef _WIN32

WSADATA TCPSocket::wsaData;

int TCPSocket::initialiseWSA() {
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to intialise WSA." << std::endl;
        return 1;
    }
    return 0;
}

void TCPSocket::cleanupWSA() {
    WSACleanup();
}

#endif

#pragma clang diagnostic pop