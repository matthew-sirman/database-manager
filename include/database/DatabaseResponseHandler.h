//
// Created by matthew on 11/07/2020.
//

#ifndef DATABASE_MANAGER_DATABASERESPONSEHANDLER_H
#define DATABASE_MANAGER_DATABASERESPONSEHANDLER_H

#include "DatabaseQuery.h"
#include "DrawingSearchResultsModel.h"
#include "../networking/Client.h"

class DatabaseResponseHandler : public ClientResponseHandler {
public:
    void onMessageReceived(void *message, unsigned int messageSize) override;

    void setSearchResultsModel(DrawingSearchResultsModel *model);

    void setDrawingReceivedHandler(const std::function<void(DrawingRequest &)> &callback);

    void setDrawingInsertResponseHandler(const std::function<void(DrawingInsert::InsertResponseType, unsigned)> &callback);

    void setRepeatTokenResponseCallback(const std::function<void(const uint256 &token)> &callback);
private:
    static RequestType getDeserialiseType(void *data);

    DrawingSearchResultsModel *resultsModel = nullptr;

    std::function<void(DrawingRequest &)> drawingReceivedCallback = nullptr;

    std::function<void(DrawingInsert::InsertResponseType, unsigned)> drawingInsertCallback = nullptr;

    std::function<void(const uint256 &token)> repeatTokenCallback = nullptr;
};


#endif //DATABASE_MANAGER_DATABASERESPONSEHANDLER_H
