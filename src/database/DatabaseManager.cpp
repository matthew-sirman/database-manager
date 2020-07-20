#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
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

        results = statement->executeQuery(
                "SELECT MAX(width) AS mx FROM (SELECT width FROM sidelaps UNION SELECT width FROM overlaps) AS all_lap_widths");
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

Drawing *DatabaseManager::executeDrawingQuery(const DrawingRequest &query) const {
    try {
        if (conn == nullptr) {
            ERROR_RAW("Attempted to execute query without connecting to database")
        }

        sql::Statement *statement = conn->createStatement();
        sql::ResultSet *results = nullptr;

        Drawing *drawing = new Drawing();

        std::stringstream queryString;

        queryString << "SELECT d.drawing_number AS drawing_number, d.product_id AS product_id, d.width AS width, "
                    << std::endl;
        queryString << "d.length AS length, d.tension_type AS tension_type, d.drawing_date AS drawing_date, "
                    << std::endl;
        queryString << "d.hyperlink AS hyperlink, d.notes AS notes, " << std::endl;
        queryString << "mt.machine_id AS machine_id, mt.quantity_on_deck AS quantity_on_deck, mt.position AS position, "
                    << std::endl;
        queryString << "mt.deck_id AS deck_id, " << std::endl;
        queryString << "mal.aperture_id AS aperture_id, mal.direction AS aperture_direction" << std::endl;
        queryString << "FROM drawings AS d " << std::endl;
        queryString << "INNER JOIN machine_templates AS mt ON d.template_id=mt.template_id" << std::endl;
        queryString << "INNER JOIN mat_aperture_link AS mal ON d.mat_id=mal.mat_id" << std::endl;
        queryString << "WHERE d.mat_id=" << query.matID << std::endl;

        results = statement->executeQuery(queryString.str());
        if (results->first()) {
            drawing->setDrawingNumber(results->getString("drawing_number"));
            drawing->setProduct(DrawingComponentManager<Product>::getComponentByID(results->getUInt("product_id")));
            drawing->setWidth((float) results->getDouble("width"));
            drawing->setLength((float) results->getDouble("length"));
            drawing->setTensionType((results->getString("tension_type") == "Side") ? Drawing::TensionType::SIDE
                                                                                   : Drawing::TensionType::END);
            drawing->setDate(Date::parse(results->getString("drawing_date")));
            drawing->setHyperlink(results->getString("hyperlink"));
            drawing->setNotes(results->getString("notes"));
            drawing->setMachineTemplate(
                    DrawingComponentManager<Machine>::getComponentByID(results->getUInt("machine_id")),
                    results->getUInt("quantity_on_deck"), results->getString("position"),
                    DrawingComponentManager<MachineDeck>::getComponentByID(results->getUInt("deck_id")));
            drawing->setAperture(DrawingComponentManager<Aperture>::getComponentByID(results->getUInt("aperture_id")));
            ApertureDirection apertureDirection;
            std::string apertureDirectionString = results->getString("aperture_direction");

            if (apertureDirectionString == "Longitudinal") {
                apertureDirection = ApertureDirection::LONGITUDINAL;
            } else if (apertureDirectionString == "Transverse") {
                apertureDirection = ApertureDirection::TRANSVERSE;
            } else {
                apertureDirection = ApertureDirection::NON_DIRECTIONAL;
            }
            drawing->setApertureDirection(apertureDirection);
        } else {
            ERROR_RAW_SAFE("Failed to load drawing with mat_id: " + std::to_string(query.matID))
            drawing->setLoadWarning(Drawing::LoadWarning::LOAD_FAILED);
            return drawing;
        }

        delete results;

        queryString.str(std::string());

        queryString << "SELECT material_thickness_id FROM thickness WHERE mat_id=" << query.matID << std::endl;

        results = statement->executeQuery(queryString.str());
        if (results->first()) {
            drawing->setMaterial(Drawing::MaterialLayer::TOP,
                                 DrawingComponentManager<Material>::getComponentByID(
                                         results->getUInt("material_thickness_id")));

            if (results->next()) {
                drawing->setMaterial(Drawing::MaterialLayer::BOTTOM,
                                     DrawingComponentManager<Material>::getComponentByID(
                                             results->getUInt("material_thickness_id")));
            }
        } else {
            ERROR_RAW_SAFE("Missing material for drawing: " + drawing->drawingNumber())
            drawing->setLoadWarning(Drawing::LoadWarning::MISSING_MATERIAL_DETECTED);
        }

        delete results;

        queryString.str(std::string());

        queryString << "SELECT bar_spacing, bar_width FROM bar_spacings WHERE mat_id=" << query.matID << std::endl;
        queryString << "ORDER BY bar_index ASC" << std::endl;

        std::vector<float> barSpacings, barWidths;

        results = statement->executeQuery(queryString.str());

        while (results->next()) {
            barSpacings.push_back((float) results->getDouble("bar_spacing"));
            barWidths.push_back((float) results->getDouble("bar_width"));
        }

        delete results;

        queryString.str(std::string());

        queryString << "SELECT side_iron_id, bar_width, inverted FROM mat_side_iron_link" << std::endl;
        queryString << "WHERE mat_id=" << query.matID << std::endl;
        queryString << "ORDER BY side_iron_index ASC" << std::endl;

        results = statement->executeQuery(queryString.str());
        if (results->first()) {
            barWidths.insert(barWidths.begin(), (float) results->getDouble("bar_width"));
            drawing->setSideIron(Drawing::Side::LEFT,
                                 DrawingComponentManager<SideIron>::getComponentByID(results->getUInt("side_iron_id")));
            drawing->setSideIronInverted(Drawing::LEFT, results->getBoolean("inverted"));

            if (results->next()) {
                barWidths.push_back((float) results->getUInt("bar_width"));
                drawing->setSideIron(Drawing::Side::RIGHT,
                                     DrawingComponentManager<SideIron>::getComponentByID(
                                             results->getUInt("side_iron_id")));
                drawing->setSideIronInverted(Drawing::RIGHT, results->getBoolean("inverted"));
            } else {
                ERROR_RAW_SAFE("Missing right side iron for drawing: " + drawing->drawingNumber())
                drawing->setLoadWarning(Drawing::LoadWarning::MISSING_SIDE_IRONS_DETECTED);
            }
        } else {
            ERROR_RAW_SAFE("Missing side irons for drawing: " + drawing->drawingNumber())
            drawing->setLoadWarning(Drawing::LoadWarning::MISSING_SIDE_IRONS_DETECTED);
        }

        barSpacings.push_back(drawing->width() - std::accumulate(barSpacings.begin(), barSpacings.end(), 0.0f));

        drawing->setBars(barSpacings, barWidths);

        delete results;

        queryString.str(std::string());

        queryString << "SELECT 'S' AS type, mat_side, width, attachment_type, material_id " << std::endl;
        queryString << "FROM sidelaps WHERE mat_id=" << query.matID << std::endl;
        queryString << "UNION SELECT 'O' AS type, mat_side, width, attachment_type, material_id " << std::endl;
        queryString << "FROM overlaps WHERE mat_id=" << query.matID << std::endl;

        results = statement->executeQuery(queryString.str());

        while (results->next()) {
            LapAttachment attachment;
            std::string attachmentString = results->getString("attachment_type");
            if (attachmentString == "Bonded") {
                attachment = LapAttachment::BONDED;
            } else if (attachmentString == "Integral") {
                attachment = LapAttachment::INTEGRAL;
            } else {
                ERROR_RAW_SAFE("Invalid drawing discovered (invalid lap attachment): " + drawing->drawingNumber())
                drawing->setLoadWarning(Drawing::LoadWarning::INVALID_LAPS_DETECTED);
                continue;
            }

            Drawing::Side side;
            std::string sideString = results->getString("mat_side");
            if (sideString == "Left") {
                side = Drawing::Side::LEFT;
            } else if (sideString == "Right") {
                side = Drawing::Side::RIGHT;
            } else {
                ERROR_RAW_SAFE("Invalid drawing discovered (invalid lap side): " + drawing->drawingNumber())
                drawing->setLoadWarning(Drawing::LoadWarning::INVALID_LAPS_DETECTED);
                continue;
            }

            if (results->getString("type") == "S") {
                drawing->setSidelap(side, Drawing::Lap((float) results->getDouble("width"), attachment,
                                                       DrawingComponentManager<Material>::getComponentByID(
                                                               results->getUInt("material_id"))));
            } else {
                drawing->setOverlap(side, Drawing::Lap((float) results->getDouble("width"), attachment,
                                                       DrawingComponentManager<Material>::getComponentByID(
                                                               results->getUInt("material_id"))));
            }
        }

        delete results;

        queryString.str(std::string());

        queryString << "SELECT hyperlink FROM punch_program_pdfs WHERE mat_id=" << query.matID << std::endl;

        results = statement->executeQuery(queryString.str());

        std::vector<std::string> pressPunchProgramPDFs;

        while (results->next()) {
            pressPunchProgramPDFs.push_back(results->getString("hyperlink"));
        }

        drawing->setPressDrawingHyperlinks(pressPunchProgramPDFs);

        delete results;

        delete statement;

        return drawing;
    } catch (sql::SQLException &e) {
        SQL_ERROR_SAFE(e);
        return nullptr;
    }
}

sql::ResultSet *DatabaseManager::sourceTable(const std::string &tableName, const std::string &orderBy) {
    try {
        if (conn == nullptr) {
            ERROR_RAW("Attempted to execute query without connecting to database.")
        }

        sql::Statement *statement = conn->createStatement();
        sql::ResultSet *results = statement->executeQuery("SELECT * FROM " + tableName + (orderBy.empty() ? "" : " ORDER BY " + orderBy));

        delete statement;

        return results;
    } catch (sql::SQLException &e) {
        SQL_ERROR_SAFE(e)
        return nullptr;
    }
}

bool DatabaseManager::insertDrawing(const DrawingInsert &insert) {
    try {
        if (conn == nullptr) {
            ERROR_RAW("Attempted to execute query without connecting to database.");
        }

        if (!insert.drawingData.has_value()) {
            return false;
        }
        if (insert.drawingData->checkDrawingValidity() != Drawing::SUCCESS) {
            return false;
        }

        sql::Statement *statement = conn->createStatement();

        sql::ResultSet *existingMatIDResults = statement->executeQuery(
                "SELECT mat_id FROM drawings WHERE drawing_number='" + insert.drawingData->drawingNumber() + "'");

        if (existingMatIDResults->first()) {
            if (!insert.forcing()) {
                delete existingMatIDResults;
                return false;
            }
            unsigned existingMatID = existingMatIDResults->getUInt("mat_id");
            statement->execute("DELETE FROM drawings WHERE mat_id=" + to_str(existingMatID));
        }
        delete existingMatIDResults;

        sql::ResultSet *existingTemplateResults = statement->executeQuery(insert.testMachineTemplateQuery());

        unsigned templateID;

        if (existingTemplateResults->first()) {
            templateID = existingTemplateResults->getUInt("template_id");
        } else {
            statement->execute(insert.machineTemplateInsertQuery());
            sql::ResultSet *templateIDResults = statement->executeQuery("SELECT @@identity AS id");
            if (templateIDResults->first()) {
                templateID = templateIDResults->getUInt("id");
            } else {
                delete templateIDResults;
                delete existingTemplateResults;
                return false;
            }
            delete templateIDResults;
        }

        delete existingTemplateResults;

        statement->execute(insert.drawingInsertQuery(templateID));
        sql::ResultSet *matIDResults = statement->executeQuery("SELECT @@identity AS id");

        unsigned matID;
        if (matIDResults->first()) {
            matID = matIDResults->getUInt("id");
        } else {
            delete matIDResults;
            return false;
        }
        delete matIDResults;

        statement->execute(insert.barSpacingInsertQuery(matID));
        statement->execute(insert.apertureInsertQuery(matID));
        statement->execute(insert.sideIronInsertQuery(matID));
        statement->execute(insert.thicknessInsertQuery(matID));

        std::string overlapInsert = insert.overlapsInsertQuery(matID), sidelapInsert = insert.sidelapsInsertQuery(
                matID);

        if (!overlapInsert.empty()) {
            statement->execute(overlapInsert);
        }
        if (!sidelapInsert.empty()) {
            statement->execute(sidelapInsert);
        }

        std::string punchProgramInsert = insert.punchProgramsInsertQuery(matID);

        if (!punchProgramInsert.empty()) {
            statement->execute(punchProgramInsert);
        }

        delete statement;

        return true;
    } catch (sql::SQLException &e) {
        SQL_ERROR_SAFE(e);
        return false;
    }
}

DatabaseManager::DrawingExistsResponse DatabaseManager::drawingExists(const std::string &drawingNumber) const {
    try {
        if (conn == nullptr) {
            ERROR_RAW("Attempted to execute query without connecting to database.");
        }

        sql::Statement *statement = conn->createStatement();
        sql::ResultSet *results = statement->executeQuery(
                "SELECT mat_id FROM drawings WHERE drawing_number='" + drawingNumber + "'");

        delete statement;

        if (results->first()) {
            delete results;
            return EXISTS;
        } else {
            delete results;
            return NOT_EXISTS;
        }
    } catch (sql::SQLException &e) {
        SQL_ERROR_SAFE(e);
        return ERROR;
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

#pragma clang diagnostic pop