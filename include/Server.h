//
// Created by matthew on 12/05/2020.
//

#ifndef DATABASE_SERVER_SERVER_H
#define DATABASE_SERVER_SERVER_H

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <future>
#include <chrono>

#include <encrypt.h>

#include "TCPSocket.h"
#include "../guard.h"

static std::string getNonBlockingInput();

//struct ClientData {
////    int clientSocketFd;
//    TCPSocket clientSocket;
//    sockaddr_in clientAddress;
//};

// TCP Server class
class Server {
public:
    Server(float refreshRate, RSAKeyPair serverKey, RSAKeyPair serverSignature);

    ~Server();

    // Setup the server by creating the non-blocking TCP socket, binding the address
    // and setting the socket to listen.
    void initialiseServer(unsigned short serverPort);

    // Start the main server loop
    void startServer();

    // Close the server
    void closeServer();

private:
    // The two keys this server has for communicating with clients and signing messages for authenticity
    RSAKeyPair serverKey, serverSignature;

//    // The port this server is hosted on
//    int port{};
//
//    // The full address object of this server
//    sockaddr_in address{};
//
//    // The file descriptor for the TCP socket
//    int serverSocketFd{};
//
//    // The server's TCP options (set to reuse address and port)
//    unsigned int tcpOptions{};
//
//    // A flag to indicate whether the server is currently alive or not
//    bool serverOpen = false;

    // The server's TCP socket
    TCPSocket serverSocket;

    // The refresh rate of the server (Hz) - giving this a higher value minimises CPU usage but reduces responsiveness
    float refreshRate;

    // A list of all the currently connected clients
    std::vector<TCPSocket> connectedClients;

    // Accepts a new client into the server
    bool acceptClient(const TCPSocket& clientSocket);
};


#endif //DATABASE_SERVER_SERVER_H
