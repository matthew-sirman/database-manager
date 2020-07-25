#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
//
// Created by matthew on 06/07/2020.
//

#include "../../include/database/DatabaseManager.h"

DatabaseManager::DatabaseManager(const std::string &database, const std::string &user, const std::string &password,
    const std::string &host) : sess(host, 33060, user, password) {

}

/*void
DatabaseManager::connectToDatabase() {
    try {
        /*driver = get_driver_instance();
        if (conn != nullptr) {
            conn->close();
        }

        // conn = driver->connect("localhost", user, password);

        // conn->setSchema(database);

        sess = mysqlx::Session(host, 33060, user, password);
        db = sess.getSchema(database, true);

        mysqlx::Session testSess(host, 33060, user, password);
        mysqlx::Schema testSchema = testSess.getSchema(database);

        std::list<mysqlx::Schema> schemaList = sess.getSchemas();

        for (mysqlx::Schema schema : schemaList) {
            std::cout << schema.getName() << std::endl;
        }

    //} catch (sql::SQLException &e) {
    } catch (mysqlx::Error &e) {
        SQL_ERROR(e)
    }
}*/

void DatabaseManager::getCompressionSchemaDetails(unsigned &maxMatID, float &maxWidth, float &maxLength, float &maxLapSize, unsigned char &maxDrawingLength) {

    try {
        // unsigned maxMatID, maxThicknessHandle, maxApertureHandle;
        // float maxWidth, maxLength, maxLapSize;
        // unsigned char maxDrawingLength;

        maxMatID = sess.sql("SELECT MAX(mat_id) FROM scs_drawings.drawings").execute().fetchOne()[0];

        maxWidth = sess.sql("SELECT MAX(width) FROM scs_drawings.drawings").execute().fetchOne()[0];

        maxLength = sess.sql("SELECT MAX(length) FROM scs_drawings.drawings").execute().fetchOne()[0];

        // maxThicknessID = sess.sql("SELECT MAX(material_id) FROM scs_drawings.materials").execute().fetchOne()[0];

        maxLapSize = sess.sql("SELECT MAX(width) FROM (SELECT width FROM scs_drawings.sidelaps "
            "UNION SELECT width FROM scs_drawings.overlaps) AS laps").execute().fetchOne()[0];

        // maxApertureID = sess.sql("SELECT MAX(aperture_id) FROM scs_drawings.apertures").execute().fetchOne()[0];

        maxDrawingLength = sess.sql("SELECT MAX(LENGTH(drawing_number)) FROM scs_drawings.drawings").execute().fetchOne()[0].get<unsigned>();

        // return DrawingSummaryCompressionSchema(maxMatID, maxWidth, maxLength, maxThicknessID, maxLapSize,
        //                                        maxApertureID, maxDrawingLength);
    }
    //catch (sql::SQLException &e) {
    catch (mysqlx::Error &e) {
        SQL_ERROR(e)
    }
}

std::vector<DrawingSummary> DatabaseManager::executeSearchQuery(const DatabaseSearchQuery &query) {
    mysqlx::RowResult results;

    try {
        /*if (conn == nullptr) {
            ERROR_RAW("Attempted to execute query without connecting to database.")
        }

        sql::Statement *statement = conn->createStatement();
        sql::ResultSet *results = statement->executeQuery(query.toSQLQueryString());*/

        /*if (!sess) {
            ERROR_RAW("Attempted to create compression schema without connecting to database.")
        }*/

        results = sess.sql(query.toSQLQueryString()).execute();
    }
    //catch (sql::SQLException &e) {
    catch (mysqlx::Error &e) {
        SQL_ERROR_SAFE(e)
        return {};
    }

    std::vector<DrawingSummary> summaries = DatabaseSearchQuery::getQueryResultSummaries(results);

    return summaries;
}

Drawing *DatabaseManager::executeDrawingQuery(const DrawingRequest &query) {
    //try {
        /*if (conn == nullptr) {
            ERROR_RAW("Attempted to execute query without connecting to database")
        }

        sql::Statement *statement = conn->createStatement();
        sql::ResultSet *results = nullptr;*/

        /*if (!sess) {
            ERROR_RAW("Attempted to create compression schema without connecting to database.")
        }*/

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
        queryString << "FROM scs_drawings.drawings AS d " << std::endl;
        queryString << "INNER JOIN scs_drawings.machine_templates AS mt ON d.template_id=mt.template_id" << std::endl;
        queryString << "INNER JOIN scs_drawings.mat_aperture_link AS mal ON d.mat_id=mal.mat_id" << std::endl;
        queryString << "WHERE d.mat_id=" << query.matID << std::endl;

        mysqlx::Row drawingResults = sess.sql(queryString.str()).execute().fetchOne();

        if (!drawingResults.isNull()) {
            drawing->setDrawingNumber(drawingResults[0].get<std::string>());
            drawing->setProduct(DrawingComponentManager<Product>::findComponentByID(drawingResults[1]));
            drawing->setWidth(drawingResults[2]);
            drawing->setLength(drawingResults[3]);
            drawing->setTensionType((drawingResults[4].get<std::string>() == "Side") ? Drawing::SIDE : Drawing::END);
            drawing->setDate(Date::parse(drawingResults[5].get<std::string>()));
            drawing->setHyperlink(drawingResults[6].get<std::string>());
            drawing->setNotes(drawingResults[7].get<std::string>());
            drawing->setMachine(DrawingComponentManager<Machine>::findComponentByID(drawingResults[8]));
            drawing->setQuantityOnDeck(drawingResults[9].get<int>());
            drawing->setMachinePosition(drawingResults[10].get<std::string>());
            drawing->setMachineDeck(DrawingComponentManager<MachineDeck>::findComponentByID(drawingResults[11]));

            std::vector<Aperture *> matchingApertures = DrawingComponentManager<Aperture>::allComponentsByID(drawingResults[12]);

            if (matchingApertures.size() == 1) {
                drawing->setAperture(*matchingApertures.front());
            } else {
                std::string apertureDirectionString = drawingResults[13].get<std::string>();

                if (apertureDirectionString == "Longitudinal") {
                    for (const Aperture *ap : matchingApertures) {
                        if (DrawingComponentManager<ApertureShape>::getComponentByHandle(ap->apertureShapeID).componentID() == 3) {
                            drawing->setAperture(*ap);
                        }
                    }
                } else if (apertureDirectionString == "Transverse") {
                    for (const Aperture *ap : matchingApertures) {
                        if (DrawingComponentManager<ApertureShape>::getComponentByHandle(ap->apertureShapeID).componentID() == 4) {
                            drawing->setAperture(*ap);
                        }
                    }
                }
            }
        }
        else {
            ERROR_RAW_SAFE("Failed to load drawing with mat_id: " + std::to_string(query.matID))
                drawing->setLoadWarning(Drawing::LoadWarning::LOAD_FAILED);
            return drawing;
        }

        mysqlx::RowResult materials = sess.getSchema("scs_drawings").getTable("thickness").select("material_thickness_id").where("mat_id=:matID").bind("matID", query.matID).execute();

        mysqlx::Row topMaterial = materials.fetchOne();
        if (!topMaterial.isNull()) {
            drawing->setMaterial(Drawing::MaterialLayer::TOP,
                DrawingComponentManager<Material>::findComponentByID(topMaterial[0]));

            mysqlx::Row bottomMaterial = materials.fetchOne();
            if (!bottomMaterial.isNull()) {
                drawing->setMaterial(Drawing::MaterialLayer::BOTTOM,
                    DrawingComponentManager<Material>::findComponentByID(bottomMaterial[0]));
            }
        }
        else {
            ERROR_RAW_SAFE("Missing material for drawing: " + drawing->drawingNumber())
                drawing->setLoadWarning(Drawing::LoadWarning::MISSING_MATERIAL_DETECTED);
        }

        mysqlx::RowResult bars = sess.getSchema("scs_drawings").getTable("bar_spacings").select("bar_spacing", "bar_width")
            .where("mat_id=:matID").orderBy("bar_index ASC").bind("matID", query.matID).execute();

        std::vector<float> barSpacings, barWidths;

        std::vector<mysqlx::Row> rows = bars.fetchAll();

        for (const mysqlx::Row &row : rows) {
            barSpacings.push_back(row[0]);
            barWidths.push_back(row[1]);
        }

        mysqlx::RowResult sideIrons = sess.getSchema("scs_drawings").getTable("mat_side_iron_link").select("side_iron_id", "bar_width", "inverted")
            .where("mat_id=:matID").orderBy("side_iron_index ASC").bind("matID", query.matID).execute();

        mysqlx::Row leftSideIron = sideIrons.fetchOne();

        if (!leftSideIron.isNull()) {
            barWidths.insert(barWidths.begin(), leftSideIron[1]);
            drawing->setSideIron(Drawing::Side::LEFT,
                DrawingComponentManager<SideIron>::findComponentByID(leftSideIron[0]));
            drawing->setSideIronInverted(Drawing::LEFT, leftSideIron[2].get<bool>());

            mysqlx::Row rightSideIron = sideIrons.fetchOne();
            if (!rightSideIron.isNull()) {
                barWidths.push_back(rightSideIron[1]);
                drawing->setSideIron(Drawing::Side::RIGHT,
                    DrawingComponentManager<SideIron>::findComponentByID(
                        rightSideIron[0]));
                drawing->setSideIronInverted(Drawing::RIGHT, rightSideIron[2].get<bool>());
            }
            else {
                ERROR_RAW_SAFE("Missing right side iron for drawing: " + drawing->drawingNumber())
                    drawing->setLoadWarning(Drawing::LoadWarning::MISSING_SIDE_IRONS_DETECTED);
            }
        }
        else {
            ERROR_RAW_SAFE("Missing side irons for drawing: " + drawing->drawingNumber())
                drawing->setLoadWarning(Drawing::LoadWarning::MISSING_SIDE_IRONS_DETECTED);
        }

        barSpacings.push_back(drawing->width() - std::accumulate(barSpacings.begin(), barSpacings.end(), 0.0f));

        drawing->setBars(barSpacings, barWidths);
        
        queryString.str(std::string());

        queryString << "SELECT 'S' AS type, mat_side, width, attachment_type, material_id " << std::endl;
        queryString << "FROM scs_drawings.sidelaps WHERE mat_id=" << query.matID << std::endl;
        queryString << "UNION SELECT 'O' AS type, mat_side, width, attachment_type, material_id " << std::endl;
        queryString << "FROM scs_drawings.overlaps WHERE mat_id=" << query.matID << std::endl;

        std::vector<mysqlx::Row> laps = sess.sql(queryString.str()).execute().fetchAll();

        for (const mysqlx::Row &lap : laps) {
            LapAttachment attachment;
            std::string attachmentString = lap[3].get<std::string>();
            if (attachmentString == "Bonded") {
                attachment = LapAttachment::BONDED;
            }
            else if (attachmentString == "Integral") {
                attachment = LapAttachment::INTEGRAL;
            }
            else {
                ERROR_RAW_SAFE("Invalid drawing discovered (invalid lap attachment): " + drawing->drawingNumber())
                    drawing->setLoadWarning(Drawing::LoadWarning::INVALID_LAPS_DETECTED);
                continue;
            }

            Drawing::Side side;
            std::string sideString = lap[1].get<std::string>();
            if (sideString == "Left") {
                side = Drawing::Side::LEFT;
            }
            else if (sideString == "Right") {
                side = Drawing::Side::RIGHT;
            }
            else {
                ERROR_RAW_SAFE("Invalid drawing discovered (invalid lap side): " + drawing->drawingNumber())
                    drawing->setLoadWarning(Drawing::LoadWarning::INVALID_LAPS_DETECTED);
                continue;
            }

            if (lap[0].get<std::string>() == "S") {
                drawing->setSidelap(side, Drawing::Lap(lap[2], attachment,
                    DrawingComponentManager<Material>::findComponentByID(lap[4])));
            }
            else {
                drawing->setOverlap(side, Drawing::Lap(lap[2], attachment,
                    DrawingComponentManager<Material>::findComponentByID(lap[4])));
            }
        }

        std::vector<mysqlx::Row> pressHyperlinkRows = sess.getSchema("scs_drawings").getTable("punch_program_pdfs").select("hyperlink")
            .where("mat_id=:matID").bind("matID", query.matID).execute().fetchAll();

        std::vector<std::string> pressHyperlinks;
        pressHyperlinks.reserve(pressHyperlinkRows.size());

        std::transform(pressHyperlinkRows.begin(), pressHyperlinkRows.end(), pressHyperlinks.begin(), [](const mysqlx::Row &row) { return row[0].get<std::string>(); });

        drawing->setPressDrawingHyperlinks(pressHyperlinks);

        return drawing;
    /*}
    //catch (sql::SQLException &e) {
    catch (mysqlx::Error &e) {
        SQL_ERROR_SAFE(e);
        return nullptr;
    }*/
}

mysqlx::RowResult DatabaseManager::sourceTable(const std::string &tableName, const std::string &orderBy) {
    try {
        /*if (conn == nullptr) {
            ERROR_RAW("Attempted to execute query without connecting to database.")
        }*/

        /*if (!sess) {
            ERROR_RAW("Attempted to create compression schema without connecting to database.")
        }*/

        /*sql::Statement *statement = conn->createStatement();
        sql::ResultSet *results = statement->executeQuery("SELECT * FROM " + tableName + (orderBy.empty() ? "" : " ORDER BY " + orderBy));

        delete statement;*/

        return sess.sql("SELECT * FROM scs_drawings." + tableName + (orderBy.empty() ? "" : " ORDER BY " + orderBy)).execute();
    }
    // catch (sql::SQLException &e) {
    catch (mysqlx::Error &e) {
        SQL_ERROR_SAFE(e)
        return mysqlx::RowResult();
    }
}

bool DatabaseManager::insertDrawing(const DrawingInsert &insert) {
    try {
        /*if (conn == nullptr) {
            ERROR_RAW("Attempted to execute query without connecting to database.");
        }*/

        /*if (!sess) {
            ERROR_RAW("Attempted to create compression schema without connecting to database.")
        }*/

        if (!insert.drawingData.has_value()) {
            return false;
        }
        if (insert.drawingData->checkDrawingValidity() != Drawing::SUCCESS) {
            return false;
        }

        mysqlx::Row existingDrawing = sess.getSchema("scs_drawings").getTable("drawings").select("mat_id").where("drawing_number=:drawingNumber")
            .bind("drawingNumber", insert.drawingData->drawingNumber()).execute().fetchOne();

        if (!existingDrawing.isNull()) {
            if (!insert.forcing()) {
                return false;
            }
            sess.getSchema("scs_drawings").getTable("drawings").remove().where("mat_id=:matID").bind("matID", existingDrawing[0]).execute();
        }

        mysqlx::Row existingTemplate = sess.sql(insert.testMachineTemplateQuery()).execute().fetchOne();

        unsigned templateID;

        if (!existingTemplate.isNull()) {
            templateID = existingTemplate[0];
        }
        else {
            sess.sql(insert.machineTemplateInsertQuery()).execute();
            templateID = sess.sql("SELECT @@identity").execute().fetchOne()[0];
        }

        sess.sql(insert.drawingInsertQuery(templateID)).execute();
        unsigned matID = sess.sql("SELECT @@identity").execute().fetchOne()[0];

        sess.sql(insert.barSpacingInsertQuery(matID)).execute();
        sess.sql(insert.apertureInsertQuery(matID)).execute();
        sess.sql(insert.sideIronInsertQuery(matID)).execute();
        sess.sql(insert.thicknessInsertQuery(matID)).execute();

        std::string overlapInsert = insert.overlapsInsertQuery(matID), sidelapInsert = insert.sidelapsInsertQuery(matID);

        if (!overlapInsert.empty()) {
            sess.sql(overlapInsert).execute();
        }
        if (!sidelapInsert.empty()) {
            sess.sql(sidelapInsert).execute();
        }

        std::string punchProgramInsert = insert.punchProgramsInsertQuery(matID);

        if (!punchProgramInsert.empty()) {
            sess.sql(punchProgramInsert).execute();
        }

        return true;
    } 
    // catch (sql::SQLException &e) {
    catch (mysqlx::Error &e) {
        SQL_ERROR_SAFE(e);
        return false;
    }
}

DatabaseManager::DrawingExistsResponse DatabaseManager::drawingExists(const std::string &drawingNumber) {
    try {
        if (!sess.getSchema("scs_drawings").getTable("drawings").select("mat_id").where("drawing_number=:drawingNumber")
            .bind("drawingNumber", drawingNumber).execute().fetchOne().isNull()) {
            return EXISTS;
        }
        else {
            return NOT_EXISTS;
        }
    } 
    //catch (sql::SQLException &e) {
    catch (mysqlx::Error &e) {
        SQL_ERROR_SAFE(e);
        return R_ERROR;
    }
}

void DatabaseManager::closeConnection() {
    try {
        sess.close();
    }
    //catch (sql::SQLException &e) {
    catch (mysqlx::Error &e) {
        SQL_ERROR_SAFE(e)
    }
}

#pragma clang diagnostic pop