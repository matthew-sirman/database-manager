//
// Created by matthew on 12/06/2020.
//

#include "../include/NetworkMessage.h"

NetworkMessage::NetworkMessage(const void *messageData, size_t messageSize) {
    this->messageData = malloc(BUFFER_PADDED_SIZE(messageSize + sizeof(size_t)));
    memcpy((uint8 *) this->messageData, &messageSize, sizeof(size_t));
    memcpy((uint8 *) this->messageData + sizeof(size_t), messageData, messageSize);

    this->messageSize = messageSize;
}

NetworkMessage::NetworkMessage(const void *messageData, size_t messageSize, AESKey encryptionKey) {
    size_t encryptedMessageSize = PADDED_SIZE(messageSize, 16u);
    uint64 initialisationVector;
    CryptoSafeRandom::random(&initialisationVector, sizeof(uint64));

    this->messageData = malloc(BUFFER_PADDED_SIZE(encryptedMessageSize + sizeof(size_t)));
    memcpy((uint8 *) this->messageData, &messageSize, sizeof(size_t));
    memcpy((uint8 *) this->messageData + sizeof(size_t), encrypt((uint8 *) messageData, messageSize, initialisationVector, encryptionKey), encryptedMessageSize);

    this->messageSize = messageSize;
}

NetworkMessage::NetworkMessage(const std::string &message)
    : NetworkMessage(message.c_str(), message.size()) {

}

NetworkMessage::NetworkMessage(const std::string &message, AESKey encryptionKey)
    : NetworkMessage(message.c_str(), message.size(), encryptionKey) {

}

NetworkMessage::~NetworkMessage() {
    free(messageData);
}

void *NetworkMessage::receiveMessage() {
    return nullptr;
}

uint8 *NetworkMessage::dataStream() const {
    return nullptr;
}

size_t NetworkMessage::dataStreamSize() const {
    return 0;
}

void NetworkMessage::clear() {

}

bool NetworkMessage::decode(uint8 *buffer, size_t bufferSize) {
    return false;
}
