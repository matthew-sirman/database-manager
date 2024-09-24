//
// Created by matthew on 11/07/2020.
//

#ifndef DATABASE_MANAGER_DATABASERESPONSEHANDLER_H
#define DATABASE_MANAGER_DATABASERESPONSEHANDLER_H

#include "DatabaseQuery.h"
#include "../networking/Client.h"

/// <summary>
/// DatabaseResponseHandler
/// This object is bound to the Client object for the application. Any message received from the server
/// (after the initial key exchanges and connection messages) will be forwarded to the bound ClientResponseHandler
/// and sent to the onMessageReceived function. 
/// The DatabaseResponseHandler is owned by the MainMenu object and handles all responses from the server.
/// </summary>
class DatabaseResponseHandler : public ClientResponseHandler {
public:
    /// <summary>
    /// Callback function invoked every time the client receives a message from the server which should
    /// be forwarded to the application. Any invalid message is caught by the client object itself and never
    /// forwarded to this function, so by this stage, we know that the message is valid (though it could potentially
    /// still be corrupted.)
    /// </summary>
    /// <param name="message">A bytestream of the raw (decrypted) message data. The client handles the decryption.
    /// It is a rvalue reference to indicate the transfer of ownership.</param>
    /// <param name="messageSize">The size of the message bytestream.</param>
    void onMessageReceived(void*&& message, unsigned int messageSize) override;

    /// <summary>
    /// Sets a function to move a buffer into to, with the results from a search.
    /// </summary>
    /// <param name="func">The function to be ran with the buffer.</param>
    void setPopulateResultsModel(std::function<void(void*&&)> func);

    /// <summary>
    /// Setter for the drawing received callback. This callback (if set) will be invoked when the client receives
    /// a DrawingRequest response object.
    /// </summary>
    /// <param name="callback">The callback function to invoke. DrawingRequest parameter is filled by the decoded drawing request
    /// object.</param>
    void setDrawingReceivedHandler(const std::function<void(DrawingRequest &)> &callback);

    /// <summary>
    /// Setter for the drawing insert response callback. This callback (if set) will be invoked when the client receives
    /// a DrawingInsert response object with the response code returned by it, indicating whether the insertion was successful.
    /// </summary>
    /// <param name="callback">The callback function to invoke InsertResponseCode parameter is filled by the decoded code, and the 
    /// unsigned value is the response echo code used for matching requests and responses.</param>
    void setDrawingInsertResponseHandler(const std::function<void(DrawingInsert::InsertResponseCode, unsigned)> &callback);

    /// <summary>
    /// Setter for the repeat token received callback. This callback (if set) will be invoked when the client receives
    /// a repeat token from the server for use in repeat logins.
    /// </summary>
    /// <param name="callback">The callback function to invoke. The uint256 is the token data sent by the server.</param>
    void setRepeatTokenResponseCallback(const std::function<void(const uint256 &token)> &callback);

    /// <summary>
    /// Setter for the email received callback. This callback (if set) will be invoked when the client receives
    /// an email address for the authenticated user.
    /// </summary>
    /// <param name="callback">The callback function to invoke. The std::string parameter is filled by the decoded email address
    /// sent by the server.</param>
    void setEmailReceivedCallback(const std::function<void(const std::string &)> &callback);

    /// <summary>
    /// Setter for the add component response received callback. This callback (if set) will be invoked when
    /// the client receives a response code indicating whether an insertion of a component was successful or not.
    /// </summary>
    /// <param name="callback">The callback function to invoke. The ComponentInsertResponse code is filled by the success code
    /// from the decoded response object.</param>
    void setAddComponentResponseCallback(const std::function<void(ComponentInsert::ComponentInsertResponse)> &callback);

    /// <summary>
    /// Setter for the backup response received callback. This callback (if set) will be invoked when the client
    /// receives a response code indicating whether a backup was successfully created or not.
    /// </summary>
    /// <param name="callback">The callback function to invoke. The BackupResponse code is filled by the success code 
    /// from the decoded response object.</param>
    void setBackupResponseCallback(const std::function<void(DatabaseBackup::BackupResponse)> &callback);

    /// <summary>
    /// Setter for the next drawing number received callback. This callback (if set) will be invoked when the client
    /// receives a NextDrawing object from the server, containing the next automatic or manual drawing number to use.
    /// </summary>
    /// <param name="callback">The callback function to invoke. The NextDrawing parameter is filled by the decoded 
    /// object containing the required information about the drawing number.</param>
    void setNextDrawingNumberCallback(const std::function<void(const NextDrawing &)> &callback);
private:
    /// <summary>
    /// Static function to read the RequestType from the start of the message data stream.
    /// </summary>
    /// <param name="data">The data stream forwarded from the server by the client.</param>
    /// <returns>The request code encoded at the start of this data stream</returns>
    static RequestType getDeserialiseType(void *data);

    // A pointer to the results model to write to when receiving a drawing search query
    //DrawingSearchResultsModel *resultsModel = nullptr;

    std::function<void(void*&&)> populateResultsModel = nullptr;

    // Callback invoked when a drawing object is received
    std::function<void(DrawingRequest &)> drawingReceivedCallback = nullptr;

    // Callback invoked when a drawing insert response code is received
    std::function<void(DrawingInsert::InsertResponseCode, unsigned)> drawingInsertCallback = nullptr;

    // Callback invoked when a repeat token is received
    std::function<void(const uint256 &token)> repeatTokenCallback = nullptr;

    // Callback invoked when an email address is received
    std::function<void(const std::string &)> emailReceivedCallback = nullptr;

    // Callback invoked when a component addition response is received
    std::function<void(ComponentInsert::ComponentInsertResponse)> addComponentCallback = nullptr;

    // Callback invoked when a backup success response is received
    std::function<void(DatabaseBackup::BackupResponse)> backupResponseCallback = nullptr;

    // Callback invoked when a next drawing response is received
    std::function<void(const NextDrawing &)> nextDrawingResponseCallback = nullptr;
};


#endif //DATABASE_MANAGER_DATABASERESPONSEHANDLER_H
