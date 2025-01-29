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

/// <summary>
/// Enum of all message protocols.
/// </summary>
enum class MessageProtocol {
    /// <summary>
    /// A message containing a RSA public key.
    /// </summary>
    KEY_MESSAGE,
    /// <summary>
    /// A message encrypted with Rivest-Shamir-Adleman encryption, an asymetric encryption method.
    /// </summary>
    RSA_MESSAGE,
    /// <summary>
    /// A message encrypted with Advanced Encrpytion Standard, a symmetric encryption method.
    /// </summary>
    AES_MESSAGE,
    /// <summary>
    /// An unencrypted message
    /// </summary>
    RAW_MESSAGE,
    /// <summary>
    /// This message indiates a connection response.
    /// </summary>
    CONNECTION_RESPONSE_MESSAGE,
    /// <summary>
    /// This message indiates a disconnection.
    /// </summary>
    DISCONNECT_MESSAGE,
    /// <summary>
    /// This message was a heartbeat.
    /// </summary>
    HEARTBEAT
};

/// <summary>
/// Enum of heartbeat options.
/// </summary>
enum class HeartbeatMode {
    /// <summary>
    /// A reuqest for a heartbeat.
    /// </summary>
    REQUEST,
    /// <summary>
    /// A response to the request, aknowledging the heartbeat.
    /// </summary>
    RESPONSE
};

/// <summary>
/// Enum of how the client is connecting.
/// </summary>
enum class AuthMode {
    /// <summary>
    /// Authenticate using JSON Web Token's
    /// </summary>
    JWT,
    /// <summary>
    /// Authenticate using a repeat token provided by the server.
    /// </summary>
    REPEAT_TOKEN
};

/// <summary>
/// Enum for connection reponse.
/// </summary>
enum class ConnectionResponse {
    /// <summary>
    /// The connection succeeded as a limited user.
    /// </summary>
    SUCCESS,
    /// <summary>
    /// The connection succeeded as an admin user.
    /// </summary>
    SUCCESS_ADMIN,
    /// <summary>
    /// The connection failed.
    /// </summary>
    FAILED
};

/// <summary>
/// Enum for disconnections.
/// </summary>
enum class DisconnectCode {
    /// <summary>
    /// The client has disconnected.
    /// </summary>
    CLIENT_EXIT
};

/// <summary>
/// Enum for reporting decoding status.
/// </summary>
enum class DecodeStatus {
    /// <summary>
    /// The message has been decoded.
    /// </summary>
    DECODED,
    /// <summary>
    /// The message is being decoded.
    /// </summary>
    DECODING,
    /// <summary>
    /// The message failed to decode.
    /// </summary>
    DECODE_ERROR
};

/// <summary>
/// Indicates whether or not this client has full access to features.
/// </summary>
enum class ClientAccess {
    /// <summary>
    /// The client has full access.
    /// </summary>
    FULL,
    /// <summary>
    /// The client has limited access.
    /// </summary>
    LIMITED
};

/// <summary>
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
        /// <summary>
        /// The message is dirty.
        /// </summary>
        MESSAGE_DIRTY = 0x01u,
        /// <summary>
        /// The message is currently being decoded.
        /// </summary>
        MESSAGE_DECODING = 0x02u,
        /// <summary>
        /// Decoding the message resulted in an error.
        /// </summary>
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

/// <summary>
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
    __declspec(no_sanitize_address) EncryptedNetworkMessage(const void *messageData, uint32 messageSize, AESKey encryptionKey);

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
