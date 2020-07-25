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
            drawingInsertCallback(response.insertResponseType, response.responseEchoCode);
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
    }
}

void DatabaseResponseHandler::setSearchResultsModel(DrawingSearchResultsModel *model) {
    resultsModel = model;
}

void DatabaseResponseHandler::setDrawingReceivedHandler(const std::function<void(DrawingRequest &)> &callback) {
    drawingReceivedCallback = callback;
}

void DatabaseResponseHandler::setDrawingInsertResponseHandler(const std::function<void(DrawingInsert::InsertResponseType, unsigned)> &callback) {
    drawingInsertCallback = callback;
}

void DatabaseResponseHandler::setRepeatTokenResponseCallback(const std::function<void(const uint256 &token)> &callback) {
    repeatTokenCallback = callback;
}

RequestType DatabaseResponseHandler::getDeserialiseType(void *data) {
    return *((RequestType *) data);
}