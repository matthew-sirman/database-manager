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
#define MAX_MESSAGE_LENGTH 16777216

/* Message Structure:
 * HEADER:
 * Message Protocol (16 bit)
 * Message Size (16 bit)
 * PAYLOAD:
 * Data of size Message Size
 */

/// <summary>\ingroup networking
/// Enum of all message protocols.
/// </summary>
enum class MessageProtocol {
    KEY_MESSAGE,
    RSA_MESSAGE,
    AES_MESSAGE,
    RAW_MESSAGE,
    CONNECTION_RESPONSE_MESSAGE,
    DISCONNECT_MESSAGE,
    HEARTBEAT
};

/// <summary>\ingroup networking
/// Enum of heartbeat options.
/// </summary>
enum class HeartbeatMode {
    REQUEST,
    RESPONSE
};

/// <summary>\ingroup networking
/// Enum of how the client is connecting.
/// </summary>
enum class AuthMode {
    JWT,
    REPEAT_TOKEN
};

/// <summary>\ingroup networking
/// Enum for connection reponse.
/// </summary>
enum class ConnectionResponse {
    SUCCESS,
    FAILED
};

/// <summary>\ingroup networking
/// Enum for disconnections.
/// </summary>
enum class DisconnectCode {
    CLIENT_EXIT
};

/// <summary>\ingroup networking
/// Enum for reporting decoding status.
/// </summary>
enum class DecodeStatus {
    DECODED,
    DECODING,
    DECODE_ERROR
};

/// <summary>\ingroup networking
/// NetworkMessage
/// An individual network message.
/// </summary>
struct NetworkMessage {
    /// <summary>
    /// Default constructor.
    /// </summary>
    NetworkMessage() = default;

    /// <summary>
    /// Creates a NetworkMessage for a buffer and size message, with the corresponding protocol.
    /// </summary>
    /// <param name="messageData">The buffer holding the message.</param>
    /// <param name="messageSize">The size of the message on the buffer.</param>
    /// <param name="protocol">The protocol the message was sent with.</param>
    NetworkMessage(const void *messageData, uint32 messageSize, MessageProtocol protocol);

    /// <summary>
    /// Creates a NetworkMessage for a string message, with the corresponding protocol.
    /// </summary>
    /// <param name="message">The message.</param>
    /// <param name="protocol">Tge protocol the message was sent with.</param>
    NetworkMessage(const std::string &message, MessageProtocol protocol);

    /// <summary>
    /// Default destructor.
    /// </summary>
    virtual ~NetworkMessage();

    // Returns a padded data stream containing the relevant header and payload. Padded to a multiple of BUFFER_CHUNK_SIZE

    /// <summary>
    /// Returns the buffer with the message, padded with relevant header and payload.
    /// </summary>
    /// <returns>Message buffer.</returns>
    const void *dataStream() const;

    // Returns the number of bytes used in the padded data stream

    /// <summary>
    /// Returns the size of the buffer, with the padding.
    /// </summary>
    /// <returns>Message buffer size.</returns>
    virtual uint32 dataStreamSize() const;

    /// <summary>
    /// Clears this message to the state of a defaultly constructed network message.
    /// </summary>
    void clear();

    /// <summary>
    /// Decodes this message into an instance variable.
    /// </summary>
    /// <param name="buffer">The buffer the message is decoded from.</param>
    /// <param name="bufferSize">Size of the buffer.</param>
    /// <returns>Status code of decode.</returns>
    virtual DecodeStatus decode(uint8 *buffer, uint32 bufferSize);

    /// <summary>
    /// Gets the decoded message from this object.
    /// </summary>
    /// <returns>Message buffer.</returns>
    const void *getMessageData() const;

    /// <summary>
    /// Gets the size of the message's buffer.
    /// </summary>
    /// <returns>Buffer size.</returns>
    uint32 getMessageSize() const;

    /// <summary>
    /// Reports whether or not an error has occoured.
    /// </summary>
    /// <returns>True if error has occured, false otherwise.</returns>
    bool error() const;

    /// <summary>
    /// Sets the error status to true.
    /// </summary>
    void setError();

    /// <summary>
    /// Returns the protocol used by this message.
    /// </summary>
    /// <returns>Message's protocol.</returns>
    MessageProtocol protocol() const;

protected:
    /// <summary>
    /// Enum of the messages status.
    /// </summary>
    enum NetworkMessageFlag {
        MESSAGE_DIRTY = 0x01u,
        MESSAGE_DECODING = 0x02u,
        MESSAGE_ERROR = 0x04u
    };

    
    /// <summary>
    /// Stores the flags of the message.\n
    /// Flags: UNUSED, UNUSED, UNUSED, UNUSED, UNUSED, Error, Dirty, Decoding
    /// </summary>
    unsigned char messageStateFlags = 0x00u;

    /// <summary>
    /// Message data buffer.
    /// </summary>
    void *messageData = nullptr;
    /// <summary>
    /// Size of the \ref messageData buffer.
    /// </summary>
    uint32 messageSize;
    /// <summary>
    /// the protocol used by \ref messageData.
    /// </summary>
    MessageProtocol _protocol;

    /// <summary>
    /// How much of the message is left to read.
    /// </summary>
    uint32 readLeft = 0;
};

/// <summary>\ingroup networking
/// EncryptedNetworkMessage inherts NetworkMessage.
/// A network message that has been encrypted.
/// </summary>
struct EncryptedNetworkMessage : public NetworkMessage {
    /// <summary>
    /// Default constructor.
    /// </summary>
    EncryptedNetworkMessage() = default;

    /// <summary>
    /// Constructs a new EncryptedMessageObject using a buffer and buffer size, and the encryption key.
    /// </summary>
    /// <param name="messageData">Buffer holding the message.</param>
    /// <param name="messageSize">The size of the buffer.</param>
    /// <param name="encryptionKey">Key to encrypt the message.</param>
    EncryptedNetworkMessage(const void *messageData, uint32 messageSize, AESKey encryptionKey);

    /// <summary>
    /// Constructs a new EncryptedMessageObject using a string, and the encryption key.
    /// </summary>
    /// <param name="message">A string holding the message.</param>
    /// <param name="encryptionKey">Key to encrypt the message.</param>
    EncryptedNetworkMessage(const std::string &message, AESKey encryptionKey);

    /// <summary>
    /// Default destructor.
    /// </summary>
    ~EncryptedNetworkMessage() = default;

    /// <summary>
    /// Size of the buffer, including padding.
    /// </summary>
    /// <returns></returns>
    uint32 dataStreamSize() const override;

    /// <summary>
    /// Decodes a message from the buffer into a instance variable.
    /// </summary>
    /// <param name="buffer">Buffer to decrypt message from.</param>
    /// <param name="bufferSize">The size of the buffer.</param>
    /// <returns>Status code of decode.</returns>
    DecodeStatus decode(uint8 *buffer, uint32 bufferSize) override;

    /// <summary>
    /// Decrypts the data with a given key.
    /// </summary>
    /// <param name="decryptionKey">Key to decrypt data with.</param>
    /// <returns>Buffer with decrypted message.</returns>
    const void *decryptMessageData(AESKey decryptionKey) const;

private:
    uint64 initialisationVector = 0;
};


#endif //DATABASE_SERVER_NETWORKMESSAGE_H
