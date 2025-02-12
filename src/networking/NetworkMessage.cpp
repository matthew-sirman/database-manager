//
// Created by matthew on 12/06/2020.
//

#include "../../include/networking/NetworkMessage.h"

NetworkMessage::NetworkMessage(const void *messageData, uint32 messageSize, MessageProtocol protocol) {
    if (messageSize > MAX_MESSAGE_LENGTH) {
        std::cerr << "ERROR: Attempted to create a message longer than the maximum allowable message length (" << MAX_MESSAGE_LENGTH << ")." << std::endl;
        return;
    }

    this->messageData = malloc(BUFFER_PADDED_SIZE(messageSize + sizeof(uint32)));
    memcpy((uint8 *) this->messageData, &protocol, sizeof(uint8));
    memcpy(((uint8 *) this->messageData) + sizeof(uint8), &messageSize, sizeof(uint8) + sizeof(uint16));
    memcpy(((uint8 *) this->messageData) + sizeof(uint32), messageData, messageSize);

    this->messageSize = messageSize;
    this->_protocol = protocol;

    // Set the dirty flag
    messageStateFlags |= MESSAGE_DIRTY;
}

NetworkMessage::NetworkMessage(const std::string &message, MessageProtocol protocol)
    : NetworkMessage(message.c_str(), message.size() + 1, protocol) {

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

DecodeStatus NetworkMessage::decode(uint8 *buffer, uint32 bufferSize) {
    // Initialise the read index to 0
    uint32 readIndex = 0;

    // If we haven't started decoding the message, this should be the first message, and so we should start
    // by retrieving the size
    if (!(messageStateFlags & MESSAGE_DECODING)) {
        // Copy the first bytes into the messageSize and protocol variables
        memset(&this->_protocol, 0, sizeof(MessageProtocol));
        memcpy(&this->_protocol, &buffer[readIndex], sizeof(uint8));
        readIndex += sizeof(uint8);
        this->messageSize = 0;
        memcpy(&this->messageSize, &buffer[readIndex], sizeof(uint8) + sizeof(uint16));
        readIndex += sizeof(uint8) + sizeof(uint16);

        switch (_protocol) {
            case MessageProtocol::KEY_MESSAGE:
                // If we are receiving a key, the message should be exactly the size of a public key.
                // Otherwise, there is an error, so break out.
                if (messageSize != sizeof(RSAKeyPair::Public)) {
                    return DecodeStatus::DECODE_ERROR;
                }
                break;
            case MessageProtocol::RSA_MESSAGE:
                // If we are receiving an RSA encrypted message, the encrypted message should be exactly the
                // size of a uin2048, otherwise there is an error
                if (messageSize != sizeof(uint2048)) {
                    return DecodeStatus::DECODE_ERROR;
                }
                break;
            case MessageProtocol::AES_MESSAGE:
            case MessageProtocol::RAW_MESSAGE:
                // If we are receiving an AES encrypted message, or a raw message, we must simply check that the
                // message is not too large
                if (messageSize > MAX_MESSAGE_LENGTH) {
                    return DecodeStatus::DECODE_ERROR;
                }
                break;
            case MessageProtocol::CONNECTION_RESPONSE_MESSAGE:
                // If we are receiving a key, the message should be exactly the size of a ConnectionResponse.
                // Otherwise, there is an error, so break out.
                if (messageSize != sizeof(ConnectionResponse)) {
                    return DecodeStatus::DECODE_ERROR;
                }
                break;
            case MessageProtocol::DISCONNECT_MESSAGE:
                if (messageSize != sizeof(DisconnectCode)) {
                    return DecodeStatus::DECODE_ERROR;
                }
                break;
            case MessageProtocol::HEARTBEAT:
                if (messageSize != sizeof(HeartbeatMode)) {
                    return DecodeStatus::DECODE_ERROR;
                }
                break;
            default:
                // If we didn't receive any protocol type from above, the protocol was invalid.
                return DecodeStatus::DECODE_ERROR;
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
    uint32 readAmount = MIN(bufferSize - readIndex, readLeft);
    // Copy into the data buffer from the buffer passed in
    memcpy(((uint8 *) messageData) + messageSize - readLeft, &buffer[readIndex], readAmount);

    // Decrease the amount we have left to read
    readLeft -= readAmount;

    // Return true if and only if we have nothing left to read - we are done reading
    return (readLeft == 0) ? DecodeStatus::DECODED : DecodeStatus::DECODING;
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

__declspec(no_sanitize_address) EncryptedNetworkMessage::EncryptedNetworkMessage(const void *messageData, uint32 messageSize, AESKey encryptionKey) {
    if (messageSize > MAX_MESSAGE_LENGTH) {
        std::cerr << "ERROR: Attempted to create a message longer than the maximum allowable message length (" << MAX_MESSAGE_LENGTH << ")." << std::endl;
        return;
    }

    // The protocol will always be AES for an encrypted message
    this->_protocol = MessageProtocol::AES_MESSAGE;

    size_t encryptedMessageSize = PADDED_SIZE(messageSize, AES_CHUNK_SIZE);
    CryptoSafeRandom::random(&initialisationVector, sizeof(uint64));

    uint32 usedBufferSize = sizeof(uint32) + sizeof(uint64) + encryptedMessageSize;
    uint32 paddedBufferSize = BUFFER_PADDED_SIZE(usedBufferSize);

    this->messageData = malloc(paddedBufferSize);
    memcpy((uint8 *) this->messageData, &this->_protocol, sizeof(uint8));
    memcpy((uint8 *) this->messageData + sizeof(uint8), &messageSize, sizeof(uint8) + sizeof(uint16));
    memcpy((uint8 *) this->messageData + sizeof(uint32), &initialisationVector, sizeof(uint64));
    encrypt((uint8 *) messageData, encryptedMessageSize, (uint8 *) this->messageData + sizeof(uint32) + sizeof(uint64), initialisationVector, encryptionKey);

    this->messageSize = messageSize;

    // Set the dirty flag
    messageStateFlags |= MESSAGE_DIRTY;
}

EncryptedNetworkMessage::EncryptedNetworkMessage(const std::string &message, AESKey encryptionKey)
    : EncryptedNetworkMessage(message.c_str(), message.size() + 1, encryptionKey) {

}

uint32 EncryptedNetworkMessage::dataStreamSize() const {
    size_t encryptedMessageSize = PADDED_SIZE(messageSize, AES_CHUNK_SIZE);
    return BUFFER_PADDED_SIZE(sizeof(uint32) + sizeof(uint64) + encryptedMessageSize);
}

DecodeStatus EncryptedNetworkMessage::decode(uint8 *buffer, uint32 bufferSize) {
    // Initialise the read index to 0
    uint8 readIndex = 0;

    // If we haven't started decoding the message, this should be the first message, and so we should start
    // by retrieving the size
    if (!(messageStateFlags & MESSAGE_DECODING)) {
        // Copy the first bytes into the messageSize variable
        memset(&this->_protocol, 0, sizeof(MessageProtocol));
        memcpy(&this->_protocol, &buffer[readIndex], sizeof(uint8));
        readIndex += sizeof(uint8);
        this->messageSize = 0;
        memcpy(&this->messageSize, &buffer[readIndex], sizeof(uint8) + sizeof(uint16));
        readIndex += sizeof(uint8) + sizeof(uint16);

        switch (_protocol) {
            case MessageProtocol::AES_MESSAGE:
                // If we are receiving an AES encrypted message, or a raw message, we must simply check that the
                // message is not too large
                if (messageSize > MAX_MESSAGE_LENGTH) {
                    return DecodeStatus::DECODE_ERROR;
                }
                break;
                if (messageSize != sizeof(HeartbeatMode)) {
                    return DecodeStatus::DECODE_ERROR;
                }
            case MessageProtocol::DISCONNECT_MESSAGE:
            case MessageProtocol::HEARTBEAT:
                return NetworkMessage::decode(buffer, bufferSize);
            case MessageProtocol::KEY_MESSAGE:
            case MessageProtocol::RSA_MESSAGE:
            case MessageProtocol::RAW_MESSAGE:
            case MessageProtocol::CONNECTION_RESPONSE_MESSAGE:
            default:
                // If we received anything other than the AES message protocol, the protocol was invalid, as this is
                // supposed to be an encrypted message.
                return DecodeStatus::DECODE_ERROR;
        }

        // Copy the next bytes into the init vector
        memcpy(&this->initialisationVector, &buffer[readIndex], sizeof(uint64));
        readIndex += sizeof(uint64);
        // Set that we have the entire message left to read
        readLeft = PADDED_SIZE(messageSize, AES_CHUNK_SIZE);
        // Allocate enough memory in the message buffer to write into
        messageData = malloc(readLeft);

        // Set the decoding flag
        messageStateFlags |= MESSAGE_DECODING;
        // Set the dirty flag
        messageStateFlags |= MESSAGE_DIRTY;
    }

    // Get the amount we are going to read which is either the remainder of the buffer or however much we have left
    // to read; whichever is smaller
    uint32 readAmount = MIN(bufferSize - readIndex, readLeft);
    // Copy into the data buffer from the buffer passed in
    memcpy(((uint8 *) messageData) + PADDED_SIZE(messageSize, AES_CHUNK_SIZE) - readLeft, &buffer[readIndex], readAmount);

    // Decrease the amount we have left to read
    readLeft -= readAmount;

    // Return true if and only if we have nothing left to read - we are done reading
    return (readLeft == 0) ? DecodeStatus::DECODED : DecodeStatus::DECODING;
}

const void *EncryptedNetworkMessage::decryptMessageData(AESKey decryptionKey) const {
    return decrypt((const uint8 *) messageData, messageSize, initialisationVector, decryptionKey);
}
