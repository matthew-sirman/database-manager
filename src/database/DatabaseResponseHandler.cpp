//
// Created by matthew on 11/07/2020.
//

#include "../../include/database/DatabaseResponseHandler.h"

void DatabaseResponseHandler::onMessageReceived(void *message, unsigned int messageSize) {
    switch (getDeserialiseType(message)) {
        case RequestType::DRAWING_SEARCH_QUERY:
            if (resultsModel) {
                resultsModel->sourceDataFromBuffer(message);
            }
            break;
        case RequestType::DRAWING_ADD:
            break;
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

RequestType DatabaseResponseHandler::getDeserialiseType(void *data) {
    return *((RequestType *) data);
}
