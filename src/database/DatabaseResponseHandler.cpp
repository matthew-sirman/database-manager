//
// Created by matthew on 11/07/2020.
//

#include "../../include/database/DatabaseResponseHandler.h"

void DatabaseResponseHandler::onMessageReceived(void *message, unsigned int messageSize) {
    switch (getDeserialiseType(message)) {
    case RequestType::REPEAT_TOKEN_REQUEST: {
        uint256 token;

        memcpy(&token, (unsigned char *)message + sizeof(RequestType), sizeof(uint256));

        repeatTokenCallback(token);

        break;
    }
    case RequestType::DRAWING_SEARCH_QUERY:
        if (resultsModel) {
            resultsModel->sourceDataFromBuffer(message);
        }
        break;
    case RequestType::DRAWING_INSERT: {
        DrawingInsert response = DrawingInsert::deserialise(message);
        if (drawingInsertCallback) {
            drawingInsertCallback(response.insertResponseCode, response.responseEchoCode);
        }
        break;
    }
    case RequestType::SOURCE_PRODUCT_TABLE:
        DrawingComponentManager<Product>::sourceComponentTable(message, messageSize);
        break;
    case RequestType::SOURCE_APERTURE_TABLE:
        DrawingComponentManager<Aperture>::sourceComponentTable(message, messageSize);
        break;
    case RequestType::SOURCE_APERTURE_SHAPE_TABLE:
        DrawingComponentManager<ApertureShape>::sourceComponentTable(message, messageSize);
        break;
    case RequestType::SOURCE_MATERIAL_TABLE:
        DrawingComponentManager<Material>::sourceComponentTable(message, messageSize);
        break;
    case RequestType::SOURCE_SIDE_IRON_TABLE:
        DrawingComponentManager<SideIron>::sourceComponentTable(message, messageSize);
        break;
    case RequestType::SOURCE_MACHINE_TABLE:
        DrawingComponentManager<Machine>::sourceComponentTable(message, messageSize);
        break;
    case RequestType::SOURCE_MACHINE_DECK_TABLE:
        DrawingComponentManager<MachineDeck>::sourceComponentTable(message, messageSize);
        break;
    case RequestType::DRAWING_DETAILS:
        if (drawingReceivedCallback) {
            drawingReceivedCallback(DrawingRequest::deserialise(message));
        }
        break;
    case RequestType::ADD_NEW_COMPONENT:
        if (addComponentCallback) {
            addComponentCallback(ComponentInsert::deserialise(message).responseCode);
        }
        break;
    case RequestType::GET_NEXT_DRAWING_NUMBER:
        if (nextDrawingResponseCallback) {
            nextDrawingResponseCallback(NextDrawing::deserialise(message));
        }
        break;
    case RequestType::CREATE_DATABASE_BACKUP:
        if (backupResponseCallback) {
            backupResponseCallback(DatabaseBackup::deserialise(message).responseCode);
        }
        break;
    }
}

void DatabaseResponseHandler::setSearchResultsModel(DrawingSearchResultsModel *model) {
    resultsModel = model;
}

void DatabaseResponseHandler::setDrawingReceivedHandler(const std::function<void(DrawingRequest &)> &callback) {
    drawingReceivedCallback = callback;
}

void DatabaseResponseHandler::setDrawingInsertResponseHandler(const std::function<void(DrawingInsert::InsertResponseCode, unsigned)> &callback) {
    drawingInsertCallback = callback;
}

void DatabaseResponseHandler::setRepeatTokenResponseCallback(const std::function<void(const uint256 &token)> &callback) {
    repeatTokenCallback = callback;
}

void DatabaseResponseHandler::setAddComponentResponseCallback(const std::function<void(ComponentInsert::ComponentInsertResponse)> &callback) {
    addComponentCallback = callback;
}

void DatabaseResponseHandler::setBackupResponseCallback(const std::function<void(DatabaseBackup::BackupResponse)> &callback) {
    backupResponseCallback = callback;
}

void DatabaseResponseHandler::setNextDrawingNumberCallback(const std::function<void(const NextDrawing &)> &callback) {
    nextDrawingResponseCallback = callback;
}

RequestType DatabaseResponseHandler::getDeserialiseType(void *data) {
    return *((RequestType *) data);
}