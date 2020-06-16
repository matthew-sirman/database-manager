//
// Created by matthew on 12/06/2020.
//

#include "../include/NetworkMessage.h"

NetworkMessage::NetworkMessage(const void *messageData, uint32 messageSize, MessageProtocol protocol) {
    if (messageSize > MAX_MESSAGE_LENGTH) {
        std::cerr << "ERROR: Attempted to create a message longer than the maximum allowable message length (65532)." << std::endl;
        return;
    }

    this->messageData = malloc(BUFFER_PADDED_SIZE(messageSize + sizeof(uint32)));
    memcpy((uint8 *) this->messageData, &protocol, sizeof(uint16));
    memcpy(((uint8 *) this->messageData) + sizeof(uint16), &messageSize, sizeof(uint16));
    memcpy(((uint8 *) this->messageData) + sizeof(uint32), messageData, messageSize);

    this->messageSize = messageSize;
    this->_protocol = protocol;

    // Set the dirty flag
    messageStateFlags |= MESSAGE_DIRTY;
}

NetworkMessage::NetworkMessage(const std::string &message, MessageProtocol protocol)
    : NetworkMessage(message.c_str(), message.size(), protocol) {

}

NetworkMessage::~NetworkMessage() {
    if (messageData) {
        free(messageData);
        messageData = nullptr;
    }
}

const void *NetworkMessage::dataStream() const {
    return messageData;
}

uint32 NetworkMessage::dataStreamSize() const {
    return BUFFER_PADDED_SIZE(messageSize + sizeof(uint32));
}

void NetworkMessage::clear() {
    // Clear the error, dirty and decoding bits
    messageStateFlags = 0x00u;

    // free the memory we currently have stored if we have assigned the pointer
    if (messageData) {
        free(messageData);
        messageData = nullptr;
    }
}

DecodeStatus NetworkMessage::decode(uint8 *buffer, uint16 bufferSize) {
    // Initialise the read index to 0
    uint16 readIndex = 0;

    // If we haven't started decoding the message, this should be the first message, and so we should start
    // by retrieving the size
    if (!(messageStateFlags & MESSAGE_DECODING)) {
        // Copy the first bytes into the messageSize and protocol variables
        memcpy(&this->_protocol, &buffer[readIndex], sizeof(uint16));
        readIndex += sizeof(uint16);
        memcpy(&this->messageSize, &buffer[readIndex], sizeof(uint16));
        readIndex += sizeof(uint16);

        switch (_protocol) {
            case KEY_MESSAGE:
                // If we are receiving a key, the message should be exactly the size of a public key.
                // Otherwise, there is an error, so break out.
                if (messageSize != sizeof(PublicKey)) {
                    return DECODE_ERROR;
                }
                break;
            case RSA_MESSAGE:
                // If we are receiving an RSA encrypted message, the encrypted message should be exactly the
                // size of a uin2048, otherwise there is an error
                if (messageSize != sizeof(uint2048)) {
                    return DECODE_ERROR;
                }
                break;
            case AES_MESSAGE:
            case RAW_MESSAGE:
                // If we are receiving an AES encrypted message, or a raw message, we must simply check that the
                // message is not too large
                if (messageSize > MAX_MESSAGE_LENGTH) {
                    return DECODE_ERROR;
                }
                break;
            default:
                // If we didn't receive any protocol type from above, the protocol was invalid.
                return DECODE_ERROR;
        }

        // Set that we have the entire message left to read
        readLeft = messageSize;
        // Allocate enough memory in the message buffer to write into
        messageData = malloc(messageSize);

        // Set the decoding flag
        messageStateFlags |= MESSAGE_DECODING;
        // Set the dirty flag
        messageStateFlags |= MESSAGE_DIRTY;
    }

    // Get the amount we are going to read which is either the remainder of the buffer or however much we have left
    // to read; whichever is smaller
    uint16 readAmount = MIN(bufferSize - readIndex, readLeft);
    // Copy into the data buffer from the buffer passed in
    memcpy(((uint8 *) messageData) + messageSize - readLeft, &buffer[readIndex], readAmount);

    // Decrease the amount we have left to read
    readLeft -= readAmount;

    // Return true if and only if we have nothing left to read - we are done reading
    return (readLeft == 0) ? DECODED : DECODING;
}

const void *NetworkMessage::getMessageData() const {
    return messageData;
}

uint32 NetworkMessage::getMessageSize() const {
    return messageSize;
}

bool NetworkMessage::error() const {
    return messageStateFlags & MESSAGE_ERROR;
}

void NetworkMessage::setError() {
    messageStateFlags |= MESSAGE_ERROR;
}

MessageProtocol NetworkMessage::protocol() const {
    return _protocol;
}

EncryptedNetworkMessage::EncryptedNetworkMessage(const void *messageData, uint32 messageSize, MessageProtocol protocol, AESKey encryptionKey) {
    if (messageSize > MAX_MESSAGE_LENGTH) {
        std::cerr << "ERROR: Attempted to create a message longer than the maximum allowable message length (65532)." << std::endl;
        return;
    }

    size_t encryptedMessageSize = PADDED_SIZE(messageSize, AES_CHUNK_SIZE);
    CryptoSafeRandom::random(&initialisationVector, sizeof(uint64));

    this->messageData = malloc(BUFFER_PADDED_SIZE(sizeof(uint32) + sizeof(uint64) + encryptedMessageSize));
    memcpy((uint8 *) this->messageData, &protocol, sizeof(uint16));
    memcpy((uint8 *) this->messageData + sizeof(uint16), &messageSize, sizeof(uint16));
    memcpy((uint8 *) this->messageData + sizeof(uint32), &initialisationVector, sizeof(uint64));
    memcpy((uint8 *) this->messageData + sizeof(uint32) + sizeof(uint64), encrypt((uint8 *) messageData, messageSize, initialisationVector, encryptionKey), encryptedMessageSize);

    this->messageSize = messageSize;
    this->_protocol = protocol;

    // Set the dirty flag
    messageStateFlags |= MESSAGE_DIRTY;
}

EncryptedNetworkMessage::EncryptedNetworkMessage(const std::string &message, MessageProtocol protocol, AESKey encryptionKey)
    : EncryptedNetworkMessage(message.c_str(), message.size(), protocol, encryptionKey) {

}

uint32 EncryptedNetworkMessage::dataStreamSize() const {
    size_t encryptedMessageSize = PADDED_SIZE(messageSize, AES_CHUNK_SIZE);
    return BUFFER_PADDED_SIZE(sizeof(uint32) + sizeof(uint64) + encryptedMessageSize);
}

DecodeStatus EncryptedNetworkMessage::decode(uint8 *buffer, uint16 bufferSize) {
    // Initialise the read index to 0
    uint8 readIndex = 0;

    // If we haven't started decoding the message, this should be the first message, and so we should start
    // by retrieving the size
    if (!(messageStateFlags & MESSAGE_DECODING)) {
        // Copy the first bytes into the messageSize variable
        memcpy(&this->_protocol, &buffer[readIndex], sizeof(uint16));
        readIndex += sizeof(uint16);
        memcpy(&this->messageSize, &buffer[readIndex], sizeof(uint16));
        readIndex += sizeof(uint16);

        switch (_protocol) {
            case AES_MESSAGE:
                // If we are receiving an AES encrypted message, or a raw message, we must simply check that the
                // message is not too large
                if (messageSize > MAX_MESSAGE_LENGTH) {
                    return DECODE_ERROR;
                }
                break;
            case KEY_MESSAGE:
            case RSA_MESSAGE:
            case RAW_MESSAGE:
            default:
                // If we received anything other than the AES message protocol, the protocol was invalid, as this is
                // supposed to be an encrypted message.
                return DECODE_ERROR;
        }

        // Copy the next bytes into the init vector
        memcpy(&this->initialisationVector, &buffer[readIndex], sizeof(uint64));
        readIndex += sizeof(uint64);
        // Set that we have the entire message left to read
        readLeft = PADDED_SIZE(messageSize, AES_CHUNK_SIZE);
        // Allocate enough memory in the message buffer to write into
        messageData = malloc(messageSize);

        // Set the decoding flag
        messageStateFlags |= MESSAGE_DECODING;
        // Set the dirty flag
        messageStateFlags |= MESSAGE_DIRTY;
    }

    // Get the amount we are going to read which is either the remainder of the buffer or however much we have left
    // to read; whichever is smaller
    uint32 readAmount = MIN(bufferSize - readIndex, readLeft);
    // Copy into the data buffer from the buffer passed in
    memcpy(((uint8 *) messageData) + messageSize - readLeft, &buffer[readIndex], readAmount);

    // Decrease the amount we have left to read
    readLeft -= readAmount;

    // Return true if and only if we have nothing left to read - we are done reading
    return (readLeft == 0) ? DECODED : DECODING;
}

const void *EncryptedNetworkMessage::decryptMessageData(AESKey decryptionKey) const {
    return decrypt((const uint8 *) messageData, messageSize, initialisationVector, decryptionKey);
}
