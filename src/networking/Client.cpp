//
// Created by matthew on 12/05/2020.
//

#include "../../include/networking/Client.h"

Client::Client(float refreshRate, RSAKeyPair clientKey,
               DigitalSignatureKeyPair::Public serverSignature) {
  this->refreshRate = refreshRate;
  this->clientKey = clientKey;
  this->serverSignature = serverSignature;
}

Client::~Client() { clientSocket.closeSocket(); }

void Client::heartbeat() { clientSocket.heartbeat(); }

bool Client::hasFullAccess() const { return access == ClientAccess::FULL; }

void Client::initialiseClient() {
  // If we already have a client socket, don't create another
  if (clientSocket.open()) {
    return;
  }

  // Create a TCP socket with the default protocol
  guardTCPSocketCode(clientSocket.create());
}

Client::ConnectionStatus Client::connectToServer(
    const std::string &ipAddress, unsigned port,
    const std::function<void(const std::string &)> &authStringCallback) {
  if (!safeGuardTCPSocketCode(clientSocket.connectToServer(ipAddress, port))) {
    clientSocket.closeSocket();
    return ConnectionStatus::NO_CONNECTION;
  }

  // Protocol description found in the server accept method.
  //
  // 1. C -> S: KC
  // 2. S -> C: KS
  // 3. C -> S: {NC}_KS
  // 4. S -> C: {{NC, NS, K, T}_SIG}_KC
  //    if NC invalid:
  //      client terminates connection
  //      end
  // 5. C -> S: {T, JWT}_K
  //    if JWT invalid:
  //      S -> C: ERROR
  //      server terminates connection
  // Both communicate with AES key from here, with messages starting with token

  // 1: Send client's public key to server
  clientSocket.sendMessage(NetworkMessage(&clientKey.publicKey,
                                          sizeof(RSAKeyPair::Public),
                                          MessageProtocol::KEY_MESSAGE));

  // 2: Receive server's public key
  RSAKeyPair::Public serverKey;
  NetworkMessage serverKeyMessage;

  if (clientSocket.waitForMessage(
          serverKeyMessage, MessageProtocol::KEY_MESSAGE) != SOCKET_SUCCESS) {
    std::cerr << "ERROR::Client.cpp: Server key message transfer failed."
              << std::endl;
    clientSocket.closeSocket();
    return ConnectionStatus::CREDS_EXCHANGE_FAILED;
  }

  // If the received message was in any way erroneous, terminate the connection
  if (serverKeyMessage.error()) {
    std::cerr << "ERROR::Client.cpp: Server key message transfer failed."
              << std::endl;
    clientSocket.closeSocket();
    return ConnectionStatus::CREDS_EXCHANGE_FAILED;
  }

  memcpy(&serverKey, serverKeyMessage.getMessageData(),
         serverKeyMessage.getMessageSize());

  // 3: Send random challenge to server encrypted under its public key
  uint2048 challengeMessage;
  uint64 *challenge = (uint64 *)&challengeMessage;
  // We only want a 64 bit challenge, so leave the rest as 0s
  CryptoSafeRandom::random(challenge, sizeof(uint64));
  uint2048 encryptedChallenge = encrypt(challengeMessage, serverKey);
  clientSocket.sendMessage(NetworkMessage(encryptedChallenge.rawData(),
                                          sizeof(uint2048),
                                          MessageProtocol::RSA_MESSAGE));

  // 4: Receive signed challenge, nonce, session key and token, encrypted under
  // client's public key
  uint2048 signedEncryptedResponse;
  NetworkMessage signedEncryptedResponseMessage;

  if (clientSocket.waitForMessage(signedEncryptedResponseMessage,
                                  MessageProtocol::RSA_MESSAGE) !=
      SOCKET_SUCCESS) {
    std::cerr << "ERROR::Client.cpp: Failed to retrieve signed response from "
                 "the server."
              << std::endl;
    clientSocket.closeSocket();
    return ConnectionStatus::CREDS_EXCHANGE_FAILED;
  }

  // If the received message was in any way erroneous, terminate the connection
  if (signedEncryptedResponseMessage.error()) {
    std::cerr << "ERROR::Client.cpp: Failed to retrieve signed response from "
                 "the server."
              << std::endl;
    clientSocket.closeSocket();
    return ConnectionStatus::CREDS_EXCHANGE_FAILED;
  }

  memcpy(&signedEncryptedResponse,
         signedEncryptedResponseMessage.getMessageData(),
         signedEncryptedResponseMessage.getMessageSize());

  uint2048 responseValue = checkSignature(
      decrypt(signedEncryptedResponse, clientKey.privateKey), serverSignature);
  uint8 *responseData = (uint8 *)&responseValue;
  uint64 responseChallenge;
  uint32 nonce;

  memcpy(&responseChallenge, responseData, sizeof(uint64));
  memcpy(&nonce, responseData + sizeof(uint64), sizeof(uint32));
  memcpy(&sessionKey, responseData + sizeof(uint32) + sizeof(uint64),
         sizeof(AESKey));
  memcpy(&sessionToken,
         responseData + sizeof(uint32) + sizeof(uint64) + sizeof(AESKey),
         sizeof(uint64));

  if (responseChallenge != *challenge) {
    clientSocket.closeSocket();
    std::cerr << "ERROR::Client.cpp: Returned challenge from server incorrect. "
                 "Server failed to authenticate itself."
              << std::endl;
    return ConnectionStatus::CREDS_EXCHANGE_FAILED;
  }

  // 5: Client now has to gather and send a JWT to the server to authenticate
  // themselves
  QueryURL authUrl =
      getMicrosoftAccountIDQueryURL(CLIENT_APPLICATION_ID, REDIRECT_URL, nonce);

  authStringCallback(authUrl.url);

  std::string authToken = openOneShotHTTPAuthenticationServer(
      "127.0.0.1", 5000, "<html><body><h1>Thank you</h1></body></html>");

  unsigned char *authMessageBuff =
      (unsigned char *)alloca(sizeof(AuthMode) + authToken.size());

  *((AuthMode *)authMessageBuff) = AuthMode::JWT;
  memcpy(authMessageBuff + sizeof(AuthMode), authToken.c_str(),
         authToken.size());

  EncryptedNetworkMessage authMessage(
      authMessageBuff, sizeof(AuthMode) + authToken.size(), sessionKey);
  clientSocket.sendMessage(authMessage);

  NetworkMessage authResponse;

  if (clientSocket.receiveMessage(
          authResponse, MessageProtocol::CONNECTION_RESPONSE_MESSAGE) ==
      SOCKET_SUCCESS) {
    if (!authResponse.error()) {
      ConnectionResponse response =
          *((ConnectionResponse *)authResponse.getMessageData());

      switch (response) {
        case ConnectionResponse::SUCCESS:
          access = ClientAccess::LIMITED;
          return ConnectionStatus::SUCCESS;
          break;
        case ConnectionResponse::SUCCESS_ADMIN:
          access = ClientAccess::FULL;
          return ConnectionStatus::SUCCESS;
        case ConnectionResponse::FAILED:
          return ConnectionStatus::INVALID_JWT;
          break;
      }
    }
  }

  return ConnectionStatus::INVALID_JWT;
}

Client::ConnectionStatus Client::connectWithToken(const std::string &ipAddress,
                                                  unsigned port,
                                                  uint256 repeatToken) {
  if (!safeGuardTCPSocketCode(clientSocket.connectToServer(ipAddress, port))) {
    clientSocket.closeSocket();
    return ConnectionStatus::NO_CONNECTION;
  }

  // Protocol description found in the server accept method.
  //
  // 1. C -> S: KC
  // 2. S -> C: KS
  // 3. C -> S: {NC}_KS
  // 4. S -> C: {{NC, NS, K, T}_SIG}_KC
  //    if NC invalid:
  //      client terminates connection
  //      end
  // 5. C -> S: {T, RT}_K
  //    if RT invalid:
  //      S -> C: ERROR
  //      server terminates connection
  // Both communicate with AES key from here, with messages starting with token

  // 1: Send client's public key to server
  clientSocket.sendMessage(NetworkMessage(&clientKey.publicKey,
                                          sizeof(RSAKeyPair::Public),
                                          MessageProtocol::KEY_MESSAGE));

  // 2: Receive server's public key
  RSAKeyPair::Public serverKey;
  NetworkMessage serverKeyMessage;

  clientSocket.waitForMessage(serverKeyMessage, MessageProtocol::KEY_MESSAGE);

  // If the received message was in any way erroneous, terminate the connection
  if (serverKeyMessage.error()) {
    std::cerr << "ERROR::Client.cpp: Server key message transfer failed."
              << std::endl;
    clientSocket.closeSocket();
    return ConnectionStatus::CREDS_EXCHANGE_FAILED;
  }

  memcpy(&serverKey, serverKeyMessage.getMessageData(),
         serverKeyMessage.getMessageSize());

  // 3: Send random challenge to server encrypted under its public key
  uint2048 challengeMessage;
  uint64 *challenge = (uint64 *)&challengeMessage;
  // We only want a 64 bit challenge, so leave the rest as 0s
  CryptoSafeRandom::random(challenge, sizeof(uint64));
  uint2048 encryptedChallenge = encrypt(challengeMessage, serverKey);
  clientSocket.sendMessage(NetworkMessage(encryptedChallenge.rawData(),
                                          sizeof(uint2048),
                                          MessageProtocol::RSA_MESSAGE));

  // 4: Receive signed challenge, nonce, session key and token, encrypted under
  // client's public key
  uint2048 signedEncryptedResponse;
  NetworkMessage signedEncryptedResponseMessage;

  clientSocket.waitForMessage(signedEncryptedResponseMessage,
                              MessageProtocol::RSA_MESSAGE);

  // If the received message was in any way erroneous, terminate the connection
  if (signedEncryptedResponseMessage.error()) {
    std::cerr << "ERROR::Client.cpp: Failed to retrieve signed response from "
                 "the server."
              << std::endl;
    clientSocket.closeSocket();
    return ConnectionStatus::CREDS_EXCHANGE_FAILED;
  }

  memcpy(&signedEncryptedResponse,
         signedEncryptedResponseMessage.getMessageData(),
         signedEncryptedResponseMessage.getMessageSize());

  uint2048 responseValue = checkSignature(
      decrypt(signedEncryptedResponse, clientKey.privateKey), serverSignature);
  uint8 *responseData = (uint8 *)&responseValue;
  uint64 responseChallenge;
  uint32 nonce;

  memcpy(&responseChallenge, responseData, sizeof(uint64));
  memcpy(&nonce, responseData + sizeof(uint64), sizeof(uint32));
  memcpy(&sessionKey, responseData + sizeof(uint32) + sizeof(uint64),
         sizeof(AESKey));
  memcpy(&sessionToken,
         responseData + sizeof(uint32) + sizeof(uint64) + sizeof(AESKey),
         sizeof(uint64));

  if (responseChallenge != *challenge) {
    clientSocket.closeSocket();
    std::cerr << "ERROR::Client.cpp: Returned challenge from server incorrect. "
                 "Server failed to authenticate itself."
              << std::endl;
    return ConnectionStatus::CREDS_EXCHANGE_FAILED;
  }

  // 5: Client now sends their repeat token to authenticate themselves

  unsigned char *authMessageBuff =
      (unsigned char *)alloca(sizeof(AuthMode) + sizeof(uint256));

  *((AuthMode *)authMessageBuff) = AuthMode::REPEAT_TOKEN;
  memcpy(authMessageBuff + sizeof(AuthMode), &repeatToken, sizeof(uint256));

  EncryptedNetworkMessage authMessage(
      authMessageBuff, sizeof(AuthMode) + sizeof(uint256), sessionKey);
  clientSocket.sendMessage(authMessage);

  NetworkMessage authResponse;

  if (clientSocket.receiveMessage(
          authResponse, MessageProtocol::CONNECTION_RESPONSE_MESSAGE) ==
      SOCKET_SUCCESS) {
    if (!authResponse.error()) {
      ConnectionResponse response =
          *((ConnectionResponse *)authResponse.getMessageData());

      switch (response) {
        case ConnectionResponse::SUCCESS:
          access = ClientAccess::LIMITED;
          return ConnectionStatus::SUCCESS;
        case ConnectionResponse::SUCCESS_ADMIN:
          access = ClientAccess::FULL;
          return ConnectionStatus::SUCCESS;
        case ConnectionResponse::FAILED:
          return ConnectionStatus::INVALID_REPEAT_TOKEN;
      }
    }
  }

  return ConnectionStatus::INVALID_JWT;
}

void Client::disconnect() {
  if (clientSocket.connected()) {
    DisconnectCode code = DisconnectCode::CLIENT_EXIT;
    clientSocket.sendMessage(NetworkMessage(
        &code, sizeof(DisconnectCode), MessageProtocol::DISCONNECT_MESSAGE));
  }
  clientSocket.closeSocket();
}

void Client::startClientLoop() {
  // If we are already running, simply return
  if (clientLoopRunning) {
    return;
  }

  // Set the socket to non blocking
  guardTCPSocketCode(clientSocket.setNonBlocking());

  // Activate the loop thread - the loop will run for as long as this is true
  clientLoopRunning = true;

  // Create the thread pointing to the clientLoop function, called on this
  // object
  clientLoopThread = std::thread(&Client::clientLoop, this);
}

void Client::stopClientLoop() {
  // If we aren't running, don't attempt to stop, simply return
  if (!clientLoopRunning) {
    return;
  }

  // Kill the loop - set this flag to false so the thread knows to exit
  clientLoopRunning = false;

  // Join and close the thread
  clientLoopThread.join();
}

void Client::addMessageToSendQueue(const void *message,
                                   unsigned messageLength) {
  uint8 *sendBuffer = (uint8 *)alloca(sizeof(uint64) + messageLength);
  memcpy(sendBuffer, &sessionToken, sizeof(uint64));
  memcpy(sendBuffer + sizeof(uint64), message, messageLength);

  sendQueueMutex.lock();
  sendQueue.push(new EncryptedNetworkMessage(
      sendBuffer, sizeof(uint64) + messageLength, sessionKey));
  sendQueueMutex.unlock();
}

void Client::addMessageToSendQueue(const std::string &message) {
  addMessageToSendQueue(message.c_str(), message.size());
}

void Client::requestRepeatToken(unsigned responseCode) {
  addMessageToSendQueue(&responseCode, sizeof(unsigned));
}

void Client::requestEmailAddress(unsigned responseCode) {
  addMessageToSendQueue(&responseCode, sizeof(unsigned));
}

void Client::setResponseHandler(ClientResponseHandler &handler) {
  responseHandler = &handler;
}

void Client::clientLoop() {
  // Variables to hold the start and end times of each update frame (for
  // calculating frame delta time)
  std::chrono::time_point<std::chrono::system_clock> start, end;

  while (clientLoopRunning) {
    // Get the start time of this frame
    start = std::chrono::system_clock::now();

    sendQueueMutex.lock();
    while (!sendQueue.empty()) {
      NetworkMessage *message = sendQueue.front();
      clientSocket.sendMessage(*message);
      sendQueue.pop();
      delete message;
    }
    sendQueueMutex.unlock();

    EncryptedNetworkMessage rMessage;
    if (clientSocket.receiveMessage(rMessage, MessageProtocol::AES_MESSAGE) ==
        SOCKET_SUCCESS) {
      if (!rMessage.error()) {
        // decryptedMessage should not be owned by the client, and ownership
        // should be passed to the response handler
        void *&&decryptedMessage =
            (void *)rMessage.decryptMessageData(sessionKey);

        if (responseHandler) {
          // Obviously moving a pointer is useless, but it indicates transfer of
          // ownership, and that it is now the responsibility of the reciever to
          // free.
          responseHandler->onMessageReceived(std::move(decryptedMessage),
                                             rMessage.getMessageSize());
        }
      }
    }

    // If the refresh rate is either negative or 0 it is invalid, so raise an
    // error (otherwise there is the risk of division by 0 or running a while
    // loop at maximum speed causing the process to overload the CPU!)
    if (refreshRate <= 0) {
      STD_ERROR("Refresh rate is <= 0. This is not allowed")
    }

    // Get the end time of this frame
    end = std::chrono::system_clock::now();

    // Calculate how long this frame took to execute
    std::chrono::duration<double> frameElapsed = end - start;

    // Calculate how much extra time must be waited to have the frame take the
    // correct total time
    double delta = (1.0 / refreshRate) - frameElapsed.count();

    // If we have time to wait, wait it, otherwise don't (can't wait a negative
    // time!)
    if (delta > 0) {
      // Sleep function in microseconds, so multiply by 10e6
      // usleep(delta * 1e6);
      std::this_thread::sleep_for(
          std::chrono::microseconds((long long)(delta * 1e6)));
    }
  }
}