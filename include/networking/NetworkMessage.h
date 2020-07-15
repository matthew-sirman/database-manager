//
// Created by matthew on 12/06/2020.
//

#ifndef DATABASE_SERVER_NETWORKMESSAGE_H
#define DATABASE_SERVER_NETWORKMESSAGE_H

#include <string>

#include <encrypt.h>

#define PADDED_SIZE(s, p) ((((s) / (p)) + (((s) % (p)) != 0)) * (p))
#define BUFFER_PADDED_SIZE(s) PADDED_SIZE(s, BUFFER_CHUNK_SIZE)

#define BUFFER_CHUNK_SIZE 128u
#define AES_CHUNK_SIZE 16u
#define MAX_MESSAGE_LENGTH 65532

/* Message Structure:
 * HEADER:
 * Message Protocol (16 bit)
 * Message Size (16 bit)
 * PAYLOAD:
 * Data of size Message Size
 */

enum MessageProtocol {
    KEY_MESSAGE,
    RSA_MESSAGE,
    AES_MESSAGE,
    RAW_MESSAGE,
    HEARTBEAT
};

enum DecodeStatus {
    DECODED,
    DECODING,
    DECODE_ERROR
};

struct NetworkMessage {
    NetworkMessage() = default;

    NetworkMessage(const void *messageData, uint32 messageSize, MessageProtocol protocol);

    NetworkMessage(const std::string &message, MessageProtocol protocol);

    virtual ~NetworkMessage();

    // Returns a padded data stream containing the relevant header and payload. Padded to a multiple of BUFFER_CHUNK_SIZE
    const void *dataStream() const;

    // Returns the number of bytes used in the padded data stream
    virtual uint32 dataStreamSize() const;

    void clear();

    virtual DecodeStatus decode(uint8 *buffer, uint16 bufferSize);

    const void *getMessageData() const;

    uint32 getMessageSize() const;

    bool error() const;

    void setError();

    MessageProtocol protocol() const;

protected:
    enum NetworkMessageFlag {
        MESSAGE_DIRTY = 0x01u,
        MESSAGE_DECODING = 0x02u,
        MESSAGE_ERROR = 0x04u
    };

    // Flags: UNUSED, UNUSED, UNUSED, UNUSED, UNUSED, Error, Dirty, Decoding
    unsigned char messageStateFlags = 0x00u;

    void *messageData = nullptr;
    uint16 messageSize;
    MessageProtocol _protocol;

    uint16 readLeft = 0;
};

struct EncryptedNetworkMessage : public NetworkMessage {
    EncryptedNetworkMessage() = default;

    EncryptedNetworkMessage(const void *messageData, uint32 messageSize, AESKey encryptionKey);

    EncryptedNetworkMessage(const std::string &message, AESKey encryptionKey);

    ~EncryptedNetworkMessage() = default;

    uint32 dataStreamSize() const override;

    DecodeStatus decode(uint8 *buffer, uint16 bufferSize) override;

    const void *decryptMessageData(AESKey decryptionKey) const;

private:
    uint64 initialisationVector = 0;
};


#endif //DATABASE_SERVER_NETWORKMESSAGE_H
