//
// Created by matthew on 06/07/2020.
//

#ifndef DATABASE_MANAGER_DATABASEMANAGER_H
#define DATABASE_MANAGER_DATABASEMANAGER_H

// #include <jdbc/mysql_connection.h>

// #include <jdbc/cppconn/driver.h>
// #include <jdbc/cppconn/exception.h>
// #include <jdbc/cppconn/resultset.h>
// #include <jdbc/cppconn/statement.h>
// #include <jdbc/cppconn/prepared_statement.h>

#include <mysqlx/xdevapi.h>

#include <vector>

#include "DatabaseQuery.h"
#include "Drawing.h"

#include "../../guard.h"

class DatabaseManager {
public:
    enum DrawingExistsResponse {
        EXISTS,
        NOT_EXISTS,
        R_ERROR
    };

    DatabaseManager(const std::string &database, const std::string &user, const std::string &password,
        const std::string &host = "localhost");

    void getCompressionSchemaDetails(unsigned &maxMatID, float &maxWidth, float &maxLength, float &maxLapSize, unsigned char &maxDrawingLength);

    std::vector<DrawingSummary> executeSearchQuery(const DatabaseSearchQuery &query);

    Drawing *executeDrawingQuery(const DrawingRequest &query);

    mysqlx::RowResult sourceTable(const std::string &tableName, const std::string &orderBy = std::string());

    bool insertDrawing(const DrawingInsert &insert);

    DrawingExistsResponse drawingExists(const std::string &drawingNumber);

    void closeConnection();

private:
    // sql::Driver *driver = nullptr;
    // sql::Connection *conn = nullptr;
    mysqlx::Session sess;
    // mysqlx::Schema db;
};


#endif //DATABASE_MANAGER_DATABASEMANAGER_H
