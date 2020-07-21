//
// Created by matthew on 09/07/2020.
//

#ifndef DATABASE_MANAGER_DATABASEREQUESTHANDLER_H
#define DATABASE_MANAGER_DATABASEREQUESTHANDLER_H

#include "DatabaseQuery.h"
#include "../networking/Server.h"

#include "../../packer.h"

class DatabaseRequestHandler : public ServerRequestHandler {
public:
    void onMessageReceived(Server &caller, const ClientHandle &clientHandle, void *message, unsigned int messageSize) override;
private:
    static RequestType getDeserialiseType(void *data);
};


#endif //DATABASE_MANAGER_DATABASEREQUESTHANDLER_H
