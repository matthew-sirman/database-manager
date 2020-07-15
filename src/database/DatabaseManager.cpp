//
// Created by matthew on 06/07/2020.
//

#include "../../include/database/DatabaseManager.h"

DatabaseManager::DatabaseManager() = default;

void
DatabaseManager::connectToDatabase(const std::string &database, const std::string &user, const std::string &password,
                                   const std::string &host) {
    try {
        driver = get_driver_instance();
        if (conn != nullptr) {
            conn->close();
        }

        conn = driver->connect(host, user, password);

        conn->setSchema(database);

    } catch (sql::SQLException &e) {
        SQL_ERROR(e)
    }
}

DrawingSummaryCompressionSchema DatabaseManager::createCompressionSchema() {

    try {
        if (conn == nullptr) {
            ERROR_RAW("Attempted to create compression schema without connecting to database.")
        }
        sql::Statement *statement = conn->createStatement();
        sql::ResultSet *results;

        unsigned maxMatID, maxThicknessID, maxApertureID;
        float maxWidth, maxLength, maxLapSize;
        unsigned char maxDrawingLength;

        results = statement->executeQuery("SELECT MAX(mat_id) AS mx FROM drawings");
        results->first();
        maxMatID = results->getUInt("mx");
        delete results;

        results = statement->executeQuery("SELECT MAX(width) AS mx FROM drawings");
        results->first();
        maxWidth = (float) results->getDouble("mx");
        delete results;

        results = statement->executeQuery("SELECT MAX(length) AS mx FROM drawings");
        results->first();
        maxLength = (float) results->getDouble("mx");
        delete results;

        results = statement->executeQuery("SELECT MAX(material_id) AS mx FROM materials");
        results->first();
        maxThicknessID = results->getUInt("mx");
        delete results;

        results = statement->executeQuery("SELECT MAX(width) AS mx FROM (SELECT width FROM sidelaps UNION SELECT width FROM overlaps) AS all_lap_widths");
        results->first();
        maxLapSize = (float) results->getDouble("mx");
        delete results;

        results = statement->executeQuery("SELECT MAX(aperture_id) AS mx FROM apertures");
        results->first();
        maxApertureID = results->getUInt("mx");
        delete results;

        results = statement->executeQuery("SELECT MAX(LENGTH(drawing_number)) AS mx FROM drawings");
        results->first();
        maxDrawingLength = results->getUInt("mx");
        delete results;

        delete statement;

        return DrawingSummaryCompressionSchema(maxMatID, maxWidth, maxLength, maxThicknessID, maxLapSize,
                                               maxApertureID, maxDrawingLength);
    } catch (sql::SQLException &e) {
        SQL_ERROR(e)
    }
}

std::vector<DrawingSummary> DatabaseManager::executeSearchQuery(const DatabaseSearchQuery &query) const {
    try {
        if (conn == nullptr) {
            ERROR_RAW("Attempted to execute query without connecting to database.")
        }

        sql::Statement *statement = conn->createStatement();
        sql::ResultSet *results = statement->executeQuery(query.toSQLQueryString());

        if (!results) {
            ERROR_RAW("Uncaught MySQL Error. Returned result set was null");
        }

        std::vector<DrawingSummary> summaries = DatabaseSearchQuery::getQueryResultSummaries(results);

        delete statement;
        delete results;

        return summaries;
    } catch (sql::SQLException &e) {
        SQL_ERROR_SAFE(e)
        return {};
    }
}

sql::ResultSet *DatabaseManager::sourceTable(const std::string &tableName) {
    try {
        if (conn == nullptr) {
            ERROR_RAW("Attempted to execute query without connecting to database.")
        }

        sql::Statement *statement = conn->createStatement();
        sql::ResultSet *results = statement->executeQuery("SELECT * FROM " + tableName);

        delete statement;

        return results;
    } catch (sql::SQLException &e) {
        SQL_ERROR_SAFE(e)
        return nullptr;
    }
}

void DatabaseManager::execute(const std::string &sqlQuery) {
    try {
        if (conn == nullptr) {
            ERROR_RAW("Attempted to execute query without connecting to database.")
        }

        sql::Statement *statement = conn->createStatement();
        statement->execute(sqlQuery);

        delete statement;

    } catch (sql::SQLException &e) {
        SQL_ERROR_SAFE(e)
    }
}

void DatabaseManager::closeConnection() {
    try {
        if (conn != nullptr) {
            conn->close();
            delete conn;
        }
    } catch (sql::SQLException &e) {
        SQL_ERROR_SAFE(e)
    }
}
