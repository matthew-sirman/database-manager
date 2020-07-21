//
// Created by matthew on 06/07/2020.
//

#ifndef DATABASE_MANAGER_DATABASEMANAGER_H
#define DATABASE_MANAGER_DATABASEMANAGER_H

#include <mysql_connection.h>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

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

    DatabaseManager();

    void connectToDatabase(const std::string &database, const std::string &user, const std::string &password,
                                  const std::string &host = "localhost");

    DrawingSummaryCompressionSchema createCompressionSchema();

    std::vector<DrawingSummary> executeSearchQuery(const DatabaseSearchQuery &query) const;

    Drawing *executeDrawingQuery(const DrawingRequest &query) const;

    sql::ResultSet *sourceTable(const std::string &tableName, const std::string &orderBy = std::string());

    bool insertDrawing(const DrawingInsert &insert);

    DrawingExistsResponse drawingExists(const std::string &drawingNumber) const;

    void execute(const std::string &sqlQuery);

    void closeConnection();

private:
    sql::Driver *driver = nullptr;
    sql::Connection *conn = nullptr;
};


#endif //DATABASE_MANAGER_DATABASEMANAGER_H
