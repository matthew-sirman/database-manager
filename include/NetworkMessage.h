//
// Created by matthew on 12/06/2020.
//

#ifndef DATABASE_SERVER_NETWORKMESSAGE_H
#define DATABASE_SERVER_NETWORKMESSAGE_H

#include <string>

#include <encrypt.h>

#define BUFFER_CHUNK_SIZE 128u

#define PADDED_SIZE(s, p) (((s / p) + ((s % p) != 0)) * p)

#define BUFFER_PADDED_SIZE(s) PADDED_SIZE(s, BUFFER_CHUNK_SIZE)

struct NetworkMessage {
    NetworkMessage(const void *messageData, size_t messageSize);

    NetworkMessage(const void *messageData, size_t messageSize, AESKey encryptionKey);

    NetworkMessage(const std::string &message);

    NetworkMessage(const std::string &message, AESKey encryptionKey);

    ~NetworkMessage();

    static void *receiveMessage();

    // Returns a padded data stream containing the relevant header and payload. Padded to a multiple of BUFFER_CHUNK_SIZE
    uint8 *dataStream() const;

    // Returns the number of bytes used in the padded data stream
    size_t dataStreamSize() const;

    void clear();

    bool decode(uint8 *buffer, size_t bufferSize);

private:
    enum NetworkMessageFlag {
        MESSAGE_DIRTY,
        MESSAGE_DECODING
    };

    // Flags: UNUSED, UNUSED, UNUSED, UNUSED, UNUSED, UNUSED, Dirty, Decoding
    unsigned char messageStateFlags = 0x00u;

    void *messageData;
    size_t messageSize;
};


#endif //DATABASE_SERVER_NETWORKMESSAGE_H
