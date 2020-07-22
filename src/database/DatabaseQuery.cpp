#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
//
// Created by matthew on 09/07/2020.
//

#include "../../include/database/DatabaseQuery.h"

DatabaseQuery::DatabaseQuery() = default;

void *DatabaseQuery::createBuffer(unsigned &size) const {
    size = serialisedSize();
    void *buffer = malloc(size);
    serialise(buffer);
    return buffer;
}

DatabaseSearchQuery::DatabaseSearchQuery() = default;

void DatabaseSearchQuery::serialise(void *target) const {
    unsigned char *buffer = (unsigned char *) target;
    *((RequestType *) buffer) = RequestType::DRAWING_SEARCH_QUERY;
    buffer += sizeof(RequestType);
    *((unsigned *) buffer) = getSearchParameters();
    buffer += sizeof(unsigned);

    if (drawingNumber.has_value()) {
        unsigned char sendDNumSize = MIN(255, drawingNumber->size());
        *buffer++ = sendDNumSize;
        memcpy(buffer, drawingNumber->c_str(), sendDNumSize);
        buffer += sendDNumSize;
    }
    if (width.has_value()) {
        width->serialise(buffer);
        buffer += width->serialisedSize();
    }
    if (length.has_value()) {
        length->serialise(buffer);
        buffer += length->serialisedSize();
    }
    if (productType.has_value()) {
        productType->serialise(buffer);
        buffer += productType->serialisedSize();
    }
    if (numberOfBars.has_value()) {
        *buffer++ = numberOfBars.value();
    }
    if (aperture.has_value()) {
        aperture->serialise(buffer);
        buffer += aperture->serialisedSize();
    }
    if (topThickness.has_value()) {
        topThickness->serialise(buffer);
        buffer += topThickness->serialisedSize();
    }
    if (bottomThickness.has_value()) {
        bottomThickness->serialise(buffer);
        buffer += bottomThickness->serialisedSize();
    }
    if (dateRange.has_value()) {
        dateRange->serialise(buffer);
        buffer += dateRange->serialisedSize();
    }
    if (sideIronType.has_value()) {
        *buffer++ = (unsigned char) sideIronType.value();
    }
    if (sideIronLength.has_value()) {
        *((unsigned short *) buffer) = sideIronLength.value();
        buffer += sizeof(unsigned short);
    }
    if (sidelapMode.has_value()) {
        *buffer++ = (unsigned char) sidelapMode.value();
    }
    if (overlapMode.has_value()) {
        *buffer++ = (unsigned char) overlapMode.value();
    }
    if (sidelapWidth.has_value()) {
        sidelapWidth->serialise(buffer);
        buffer += sidelapWidth->serialisedSize();
    }
    if (overlapWidth.has_value()) {
        overlapWidth->serialise(buffer);
        buffer += overlapWidth->serialisedSize();
    }
    if (sidelapAttachment.has_value()) {
        *buffer++ = (unsigned char) sidelapAttachment.value();
    }
    if (overlapAttachment.has_value()) {
        *buffer++ = (unsigned char) overlapAttachment.value();
    }
    if (machine.has_value()) {
        machine->serialise(buffer);
        buffer += machine->serialisedSize();
    }
    if (quantityOnDeck.has_value()) {
        *buffer++ = quantityOnDeck.value();
    }
    if (position.has_value()) {
        unsigned char sendPosSize = MIN(255, position->size());
        *buffer++ = sendPosSize;
        memcpy(buffer, position->c_str(), sendPosSize);
        buffer += sendPosSize;
    }
    if (machineDeck.has_value()) {
        machineDeck->serialise(buffer);
        buffer += machineDeck->serialisedSize();
    }
}

unsigned DatabaseSearchQuery::serialisedSize() const {
    unsigned size = sizeof(RequestType) + sizeof(unsigned);

    if (drawingNumber.has_value()) {
        size += sizeof(unsigned char) + drawingNumber->size();
    }
    if (width.has_value()) {
        size += width->serialisedSize();
    }
    if (length.has_value()) {
        size += length->serialisedSize();
    }
    if (productType.has_value()) {
        size += productType->serialisedSize();
    }
    if (numberOfBars.has_value()) {
        size += sizeof(unsigned char);
    }
    if (aperture.has_value()) {
        size += aperture->serialisedSize();
    }
    if (topThickness.has_value()) {
        size += topThickness->serialisedSize();
    }
    if (bottomThickness.has_value()) {
        size += bottomThickness->serialisedSize();
    }
    if (dateRange.has_value()) {
        size += dateRange->serialisedSize();
    }
    if (sideIronType.has_value()) {
        size += sizeof(unsigned char);
    }
    if (sideIronLength.has_value()) {
        size += sizeof(unsigned short);
    }
    if (sidelapMode.has_value()) {
        size += sizeof(unsigned char);
    }
    if (overlapMode.has_value()) {
        size += sizeof(unsigned char);
    }
    if (sidelapWidth.has_value()) {
        size += sidelapWidth->serialisedSize();
    }
    if (overlapWidth.has_value()) {
        size += overlapWidth->serialisedSize();
    }
    if (sidelapAttachment.has_value()) {
        size += sizeof(unsigned char);
    }
    if (overlapAttachment.has_value()) {
        size += sizeof(unsigned char);
    }
    if (machine.has_value()) {
        size += machine->serialisedSize();
    }
    if (quantityOnDeck.has_value()) {
        size += sizeof(unsigned char);
    }
    if (position.has_value()) {
        size += sizeof(unsigned char) + position->size();
    }
    if (machineDeck.has_value()) {
        size += machineDeck->serialisedSize();
    }

    return size;
}

DatabaseSearchQuery &DatabaseSearchQuery::deserialise(void *data) {
    DatabaseSearchQuery *query = new DatabaseSearchQuery();

    unsigned char *buffer = (unsigned char *) data + sizeof(RequestType);

    unsigned searchParameters = *((unsigned *) buffer);
    buffer += sizeof(unsigned);

    if (searchParameters & (unsigned) SearchParameters::DRAWING_NUMBER) {
        unsigned char dNumSize = *buffer++;
        query->drawingNumber = std::string((const char *) buffer, dNumSize);
        buffer += dNumSize;
    } else {
        query->drawingNumber = std::nullopt;
    }
    if (searchParameters & (unsigned) SearchParameters::WIDTH) {
        query->width = ValueRange<unsigned>::deserialise(buffer);
        buffer += query->width->serialisedSize();
    } else {
        query->width = std::nullopt;
    }
    if (searchParameters & (unsigned) SearchParameters::LENGTH) {
        query->length = ValueRange<unsigned>::deserialise(buffer);
        buffer += query->length->serialisedSize();
    } else {
        query->length = std::nullopt;
    }
    if (searchParameters & (unsigned) SearchParameters::PRODUCT_TYPE) {
        query->productType = DrawingComponentManager<Product>::getComponentByID(Product::deserialiseID(buffer));
        buffer += query->productType->serialisedSize();
    } else {
        query->productType = std::nullopt;
    }
    if (searchParameters & (unsigned) SearchParameters::NUMBER_OF_BARS) {
        query->numberOfBars = *buffer++;
    } else {
        query->numberOfBars = std::nullopt;
    }
    if (searchParameters & (unsigned) SearchParameters::APERTURE) {
        query->aperture = DrawingComponentManager<Aperture>::getComponentByID(Aperture::deserialiseID(buffer));
        buffer += query->aperture->serialisedSize();
    } else {
        query->aperture = std::nullopt;
    }
    if (searchParameters & (unsigned) SearchParameters::TOP_THICKNESS) {
        query->topThickness = DrawingComponentManager<Material>::getComponentByID(Material::deserialiseID(buffer));
        buffer += query->topThickness->serialisedSize();
    } else {
        query->topThickness = std::nullopt;
    }
    if (searchParameters & (unsigned) SearchParameters::BOTTOM_THICKNESS) {
        query->bottomThickness = DrawingComponentManager<Material>::getComponentByID(Material::deserialiseID(buffer));
        buffer += query->bottomThickness->serialisedSize();
    } else {
        query->bottomThickness = std::nullopt;
    }
    if (searchParameters & (unsigned) SearchParameters::DATE_RANGE) {
        query->dateRange = ValueRange<Date>::deserialise(buffer);
        buffer += query->dateRange->serialisedSize();
    } else {
        query->dateRange = std::nullopt;
    }
    if (searchParameters & (unsigned) SearchParameters::SIDE_IRON_TYPE) {
        query->sideIronType = (SideIronType) *buffer++;
    } else {
        query->sideIronType = std::nullopt;
    }
    if (searchParameters & (unsigned) SearchParameters::SIDE_IRON_LENGTH) {
        query->sideIronLength = *((unsigned short *) buffer);
        buffer += sizeof(unsigned short);
    } else {
        query->sideIronLength = std::nullopt;
    }
    if (searchParameters & (unsigned) SearchParameters::SIDELAP_MODE) {
        query->sidelapMode = (LapSetting) *buffer++;
    } else {
        query->sidelapMode = std::nullopt;
    }
    if (searchParameters & (unsigned) SearchParameters::OVERLAP_MODE) {
        query->overlapMode = (LapSetting) *buffer++;
    } else {
        query->overlapMode = std::nullopt;
    }
    if (searchParameters & (unsigned) SearchParameters::SIDELAP_WIDTH) {
        query->sidelapWidth = ValueRange<unsigned>::deserialise(buffer);
        buffer += query->sidelapWidth->serialisedSize();
    } else {
        query->sidelapWidth = std::nullopt;
    }
    if (searchParameters & (unsigned) SearchParameters::OVERLAP_WIDTH) {
        query->overlapWidth = ValueRange<unsigned>::deserialise(buffer);
        buffer += query->overlapWidth->serialisedSize();
    } else {
        query->overlapWidth = std::nullopt;
    }
    if (searchParameters & (unsigned) SearchParameters::SIDELAP_ATTACHMENT) {
        query->sidelapAttachment = (LapAttachment) *buffer++;
    } else {
        query->sidelapAttachment = std::nullopt;
    }
    if (searchParameters & (unsigned) SearchParameters::OVERLAP_ATTACHMENT) {
        query->overlapAttachment = (LapAttachment) *buffer++;
    } else {
        query->overlapAttachment = std::nullopt;
    }
    if (searchParameters & (unsigned) SearchParameters::MACHINE) {
        query->machine = DrawingComponentManager<Machine>::getComponentByID(Machine::deserialiseID(buffer));
        buffer += query->machine->serialisedSize();
    } else {
        query->machine = std::nullopt;
    }
    if (searchParameters & (unsigned) SearchParameters::QUANTITY_ON_DECK) {
        query->quantityOnDeck = *buffer++;
    } else {
        query->quantityOnDeck = std::nullopt;
    }
    if (searchParameters & (unsigned) SearchParameters::POSITION) {
        unsigned char posSize = *buffer++;
        query->position = std::string((const char *) buffer, posSize);
        buffer += posSize;
    } else {
        query->position = std::nullopt;
    }
    if (searchParameters & (unsigned) SearchParameters::MACHINE_DECK) {
        query->machineDeck = DrawingComponentManager<MachineDeck>::getComponentByID(MachineDeck::deserialiseID(buffer));
        buffer += query->machineDeck->serialisedSize();
    } else {
        query->machineDeck = std::nullopt;
    }

    return *query;
}

unsigned DatabaseSearchQuery::getSearchParameters() const {
    unsigned params = 0;

    if (drawingNumber.has_value()) {
        params |= (unsigned) SearchParameters::DRAWING_NUMBER;
    }
    if (width.has_value()) {
        params |= (unsigned) SearchParameters::WIDTH;
    }
    if (length.has_value()) {
        params |= (unsigned) SearchParameters::LENGTH;
    }
    if (productType.has_value()) {
        params |= (unsigned) SearchParameters::PRODUCT_TYPE;
    }
    if (numberOfBars.has_value()) {
        params |= (unsigned) SearchParameters::NUMBER_OF_BARS;
    }
    if (aperture.has_value()) {
        params |= (unsigned) SearchParameters::APERTURE;
    }
    if (topThickness.has_value()) {
        params |= (unsigned) SearchParameters::TOP_THICKNESS;
    }
    if (bottomThickness.has_value()) {
        params |= (unsigned) SearchParameters::BOTTOM_THICKNESS;
    }
    if (dateRange.has_value()) {
        params |= (unsigned) SearchParameters::DATE_RANGE;
    }
    if (sideIronType.has_value()) {
        params |= (unsigned) SearchParameters::SIDE_IRON_TYPE;
    }
    if (sideIronLength.has_value()) {
        params |= (unsigned) SearchParameters::SIDE_IRON_LENGTH;
    }
    if (sidelapMode.has_value()) {
        params |= (unsigned) SearchParameters::SIDELAP_MODE;
    }
    if (overlapMode.has_value()) {
        params |= (unsigned) SearchParameters::OVERLAP_MODE;
    }
    if (sidelapWidth.has_value()) {
        params |= (unsigned) SearchParameters::SIDELAP_WIDTH;
    }
    if (overlapWidth.has_value()) {
        params |= (unsigned) SearchParameters::OVERLAP_WIDTH;
    }
    if (sidelapAttachment.has_value()) {
        params |= (unsigned) SearchParameters::SIDELAP_ATTACHMENT;
    }
    if (overlapAttachment.has_value()) {
        params |= (unsigned) SearchParameters::OVERLAP_ATTACHMENT;
    }
    if (machine.has_value()) {
        params |= (unsigned) SearchParameters::MACHINE;
    }
    if (quantityOnDeck.has_value()) {
        params |= (unsigned) SearchParameters::QUANTITY_ON_DECK;
    }
    if (position.has_value()) {
        params |= (unsigned) SearchParameters::POSITION;
    }
    if (machineDeck.has_value()) {
        params |= (unsigned) SearchParameters::MACHINE_DECK;
    }

    return params;
}

std::string DatabaseSearchQuery::toSQLQueryString() const {
    std::stringstream sql;
    sql << "SELECT d.mat_id AS mat_id, d.drawing_number AS drawing_number, d.width AS width, d.length AS length, "
           "mal.aperture_id AS aperture_id, "
           "(SELECT JSON_ARRAYAGG(t.material_thickness_id) FROM thickness AS t WHERE t.mat_id=d.mat_id) AS material_ids, "
           "d.tension_type AS tension_type, "
           "COALESCE((SELECT JSON_ARRAYAGG(JSON_ARRAY(l.type, l.width, l.mat_side)) "
           "FROM (SELECT mat_id, 'S' AS type, width, mat_side FROM sidelaps UNION "
           "SELECT mat_id, 'O' AS type, width, mat_side FROM overlaps) AS l "
           "WHERE l.mat_id=d.mat_id), JSON_ARRAY()) AS laps"
        << std::endl;

    sql << "FROM " << "drawings" << " AS d" << std::endl;
    sql << "INNER JOIN " << "mat_aperture_link" << " AS mal ON d.mat_id=mal.mat_id" << std::endl;

    std::vector<std::string> conditions;
    std::stringstream condition;

    if (drawingNumber.has_value()) {
        condition.str(std::string());
        condition << "d.drawing_number LIKE '" << drawingNumber.value() << "%'";
        conditions.push_back(condition.str());
    }
    if (width.has_value()) {
        condition.str(std::string());
        condition << "d.width BETWEEN " << width->lowerBound << " AND " << width->upperBound;
        conditions.push_back(condition.str());
    }
    if (length.has_value()) {
        condition.str(std::string());
        condition << "d.length BETWEEN " << length->lowerBound << " AND " << length->upperBound;
        conditions.push_back(condition.str());
    }
    if (productType.has_value()) {
        condition.str(std::string());
        condition << "d.product_id=" << productType->componentID;
        conditions.push_back(condition.str());
    }
    if (numberOfBars.has_value()) {
        condition.str(std::string());
        condition << "d.no_of_bars=" << (unsigned) numberOfBars.value();
        conditions.push_back(condition.str());
    }
    if (aperture.has_value()) {
        condition.str(std::string());
        condition << "mal.aperture_id=" << aperture->componentID;
        conditions.push_back(condition.str());
    }
    if (topThickness.has_value()) {
        condition.str(std::string());
        if (!bottomThickness.has_value()) {
            sql << "INNER JOIN " << "thickness" << " AS t ON d.mat_id=t.mat_id" << std::endl;
            condition << "t.material_thickness_id=" << topThickness->componentID;
        } else {
            condition << "d.mat_id IN (SELECT t1.mat_id AS mid FROM thickness AS t1" << std::endl;
            condition << "INNER JOIN thickness AS t2 ON t1.mat_id=t2.mat_id" << std::endl;
            condition << "WHERE t1.thickness_id < t2.thickness_id AND" << std::endl;
            condition << "t1.material_thickness_id=" << topThickness->componentID << " AND" << std::endl;
            condition << "t2.material_thickness_id=" << bottomThickness->componentID << ")";
        }
        conditions.push_back(condition.str());
    }
    if (dateRange.has_value()) {
        condition.str(std::string());
        condition << "d.drawing_date BETWEEN '" << dateRange->lowerBound.toMySQLDateString() << "' AND '"
                  << dateRange->upperBound.toMySQLDateString() << "'";
        conditions.push_back(condition.str());
    }
    if (sideIronType.has_value() || sideIronLength.has_value()) {
        sql << "INNER JOIN " << "mat_side_iron_link" << " AS msil ON d.mat_id=msil.mat_id" << std::endl;
        sql << "INNER JOIN " << "side_irons" << " AS si ON msil.side_iron_id=si.side_iron_id" << std::endl;
        if (sideIronType.has_value()) {
            condition.str(std::string());
            switch (sideIronType.value()) {
                case SideIronType::None:
                    condition << "msil.side_iron_id=1";
                    break;
                case SideIronType::A:
                    condition << "si.type=1";
                    break;
                case SideIronType::B:
                    condition << "si.type=2";
                    break;
                case SideIronType::C:
                    condition << "si.type=3";
                    break;
                case SideIronType::D:
                    condition << "si.type=4";
                    break;
                case SideIronType::E:
                    condition << "si.type=5";
                    break;
            }
            conditions.push_back(condition.str());
        }
        if (sideIronLength.has_value()) {
            condition.str(std::string());
            condition << "si.length=" << sideIronLength.value();
            conditions.push_back(condition.str());
        }
    }
    if (sidelapMode.has_value() || sidelapWidth.has_value() || sidelapAttachment.has_value()) {
        condition.str(std::string());
        condition << "d.mat_id IN (SELECT d_i.mat_id FROM drawings AS d_i" << std::endl;
        condition << "LEFT JOIN sidelaps AS s ON d_i.mat_id=s.mat_id" << std::endl;
        condition << "WHERE true" << std::endl;
        if (sidelapWidth.has_value()) {
            condition << "AND s.width BETWEEN " << sidelapWidth->lowerBound << " AND " << sidelapWidth->upperBound
                      << std::endl;
        }
        if (sidelapAttachment.has_value()) {
            condition << "AND attachment_type='";
            switch (sidelapAttachment.value()) {
                case LapAttachment::BONDED:
                    condition << "Bonded";
                    break;
                case LapAttachment::INTEGRAL:
                    condition << "Integral";
                    break;
            }
            condition << "'" << std::endl;
        }
        condition << "GROUP BY d_i.mat_id" << std::endl;
        if (sidelapMode.has_value()) {
            condition << "HAVING COUNT(s.mat_id)=";
            switch (sidelapMode.value()) {
                case LapSetting::HAS_NONE:
                    condition << 0;
                    break;
                case LapSetting::HAS_ONE:
                    condition << 1;
                    break;
                case LapSetting::HAS_BOTH:
                    condition << 2;
                    break;
            }
        }
        condition << ")" << std::endl;
        conditions.push_back(condition.str());
    }
    if (overlapMode.has_value() || overlapWidth.has_value() || overlapAttachment.has_value()) {
        condition.str(std::string());
        condition << "d.mat_id IN (SELECT d_i.mat_id FROM drawings AS d_i" << std::endl;
        condition << "LEFT JOIN overlaps AS o ON d_i.mat_id=o.mat_id" << std::endl;
        condition << "WHERE true" << std::endl;
        if (overlapWidth.has_value()) {
            condition << "AND o.width BETWEEN " << overlapWidth->lowerBound << " AND " << overlapWidth->upperBound
                      << std::endl;
        }
        if (overlapAttachment.has_value()) {
            condition << "AND attachment_type='";
            switch (overlapAttachment.value()) {
                case LapAttachment::BONDED:
                    condition << "Bonded";
                    break;
                case LapAttachment::INTEGRAL:
                    condition << "Integral";
                    break;
            }
            condition << "'" << std::endl;
        }
        condition << "GROUP BY d_i.mat_id" << std::endl;
        if (overlapMode.has_value()) {
            condition << "HAVING COUNT(o.mat_id)=";
            switch (overlapMode.value()) {
                case LapSetting::HAS_NONE:
                    condition << 0;
                    break;
                case LapSetting::HAS_ONE:
                    condition << 1;
                    break;
                case LapSetting::HAS_BOTH:
                    condition << 2;
                    break;
            }
        }
        condition << ")" << std::endl;
        conditions.push_back(condition.str());
    }
    if (machine.has_value() || quantityOnDeck.has_value() || position.has_value() || machineDeck.has_value()) {
        sql << "INNER JOIN machine_templates AS mt ON d.template_id=mt.template_id" << std::endl;

        if (machine.has_value()) {
            condition.str(std::string());
            condition << "mt.machine_id=" << machine->componentID;
            conditions.push_back(condition.str());
        }
        if (quantityOnDeck.has_value()) {
            condition.str(std::string());
            condition << "mt.quantity_on_deck=" << (unsigned) quantityOnDeck.value();
            conditions.push_back(condition.str());
        }
        if (position.has_value()) {
            condition.str(std::string());
            condition << "mt.position LIKE '" << position.value() << "%'";
            conditions.push_back(condition.str());
        }
        if (machineDeck.has_value()) {
            condition.str(std::string());
            condition << "mt.deck_id=" << machineDeck->componentID;
            conditions.push_back(condition.str());
        }
    }

    if (!conditions.empty()) {
        sql << "WHERE " << std::endl;
        for (unsigned i = 0; i < conditions.size() - 1; i++) {
            sql << conditions[i] << " AND" << std::endl;
        }
        sql << conditions.back() << std::endl;
    }

    sql << "GROUP BY d.mat_id, mal.aperture_id" << std::endl;
    sql << "ORDER BY IF(d.drawing_number REGEXP '^[A-Z][0-9]{2,}[A-Z]?$', "
           "CONCAT('0', d.drawing_number), d.drawing_number) DESC;";

    return sql.str();
}

std::vector<DrawingSummary> DatabaseSearchQuery::getQueryResultSummaries(const mysqlx::RowResult &resultSet) {
    std::vector<DrawingSummary> summaries;

    /*while (resultSet->next()) {
        DrawingSummary summary;
        summary.matID = resultSet->getUInt("mat_id");
        summary.drawingNumber = resultSet->getString("drawing_number");
        summary.setWidth((float) resultSet->getDouble("width"));
        summary.setLength((float) resultSet->getDouble("length"));
        summary.apertureID = resultSet->getUInt("aperture_id");

        nlohmann::json thicknessIDs = nlohmann::json::parse(resultSet->getString("material_ids").c_str()),
                lapSizes = nlohmann::json::parse(resultSet->getString("laps").c_str());

        unsigned int index = 0;
        memset(&summary.thicknessIDs, 0, sizeof(summary.thicknessIDs));
        for (unsigned i = 0; i < 4; i++) {
            summary.setLapSize(i, 0);
        }

        for (const nlohmann::json &it : thicknessIDs) {
            summary.thicknessIDs[index++] = it.get<unsigned>();
        }

        for (const nlohmann::json &it : lapSizes) {
            if (resultSet->getString("tension_type") == "Side") {
                index = (it[2] == "Right") + 2 * (it[0] == "O");
            } else {
                index = (it[2] == "Right") + 2 * (it[0] == "S");
            }
            summary.setLapSize(index, it[1].get<float>());
        }

        summaries.push_back(summary);
    }*/

    return summaries;
}

DrawingRequest &DrawingRequest::makeRequest(unsigned matID, unsigned responseEchoCode) {
    DrawingRequest *request = new DrawingRequest();
    request->matID = matID;
    request->responseEchoCode = responseEchoCode;
    request->drawingData = std::nullopt;
    return *request;
}

void DrawingRequest::serialise(void *target) const {
    unsigned char *buffer = (unsigned char *) target;
    *((RequestType *) buffer) = RequestType::DRAWING_DETAILS;
    buffer += sizeof(RequestType);

    *((unsigned *) buffer) = matID;
    buffer += sizeof(unsigned);

    *((unsigned *) buffer) = responseEchoCode;
    buffer += sizeof(unsigned);

    bool hasDrawingData = drawingData.has_value();
    *buffer++ = hasDrawingData;

    if (hasDrawingData) {
        DrawingSerialiser::serialise(drawingData.value(), buffer);
    }
}

unsigned int DrawingRequest::serialisedSize() const {
    return sizeof(RequestType) + sizeof(unsigned) + sizeof(unsigned) + sizeof(unsigned char) +
           (drawingData.has_value() ? DrawingSerialiser::serialisedSize(drawingData.value()) : 0);
}

DrawingRequest &DrawingRequest::deserialise(void *data) {
    DrawingRequest *drawingRequest = new DrawingRequest();

    unsigned char *buffer = (unsigned char *) data + sizeof(RequestType);
    drawingRequest->matID = *((unsigned *) buffer);
    buffer += sizeof(unsigned);
    drawingRequest->responseEchoCode = *((unsigned *) buffer);
    buffer += sizeof(unsigned);

    bool hasDrawingData = *buffer++;

    if (hasDrawingData) {
        drawingRequest->drawingData = DrawingSerialiser::deserialise(buffer);
    } else {
        drawingRequest->drawingData = std::nullopt;
    }

    return *drawingRequest;
}

void DrawingInsert::serialise(void *target) const {
    unsigned char *buffer = (unsigned char *) target;
    *((RequestType *) buffer) = RequestType::DRAWING_INSERT;
    buffer += sizeof(RequestType);

    *((InsertResponseType *) buffer) = insertResponseType;
    buffer += sizeof(InsertResponseType);

    *((unsigned *) buffer) = responseEchoCode;
    buffer += sizeof(unsigned);

    *buffer++ = force;

    bool hasDrawingData = drawingData.has_value();
    *buffer++ = hasDrawingData;

    if (hasDrawingData) {
        DrawingSerialiser::serialise(drawingData.value(), buffer);
    }
}

unsigned int DrawingInsert::serialisedSize() const {
    return sizeof(RequestType) + sizeof(InsertResponseType) + sizeof(unsigned) + sizeof(unsigned char) +
           sizeof(unsigned char) +
           (drawingData.has_value() ? DrawingSerialiser::serialisedSize(drawingData.value()) : 0);
}

DrawingInsert &DrawingInsert::deserialise(void *data) {
    DrawingInsert *drawingInsert = new DrawingInsert();

    unsigned char *buffer = (unsigned char *) data + sizeof(RequestType);
    drawingInsert->insertResponseType = *((InsertResponseType *) buffer);
    buffer += sizeof(InsertResponseType);
    drawingInsert->responseEchoCode = *((unsigned *) buffer);
    buffer += sizeof(unsigned);

    drawingInsert->force = *buffer++;

    bool hasDrawingData = *buffer++;

    if (hasDrawingData) {
        drawingInsert->drawingData = DrawingSerialiser::deserialise(buffer);
    } else {
        drawingInsert->drawingData = std::nullopt;
    }

    return *drawingInsert;
}

void DrawingInsert::setForce(bool val) {
    force = val;
}

bool DrawingInsert::forcing() const {
    return force;
}

std::string DrawingInsert::drawingInsertQuery(unsigned templateID) const {
    std::stringstream insert;

    insert << "INSERT INTO drawings" << std::endl;
    insert << "(drawing_number, product_id, template_id, width, length, tension_type, drawing_date, no_of_bars, "
              "hyperlink, notes)" << std::endl;
    insert << "VALUES" << std::endl;

    insert << "('" << drawingData->drawingNumber() << "', " << drawingData->product().componentID << ", " <<
           templateID << ", " << drawingData->width() << ", " << drawingData->length() << ", ";

    switch (drawingData->tensionType()) {
        case Drawing::SIDE:
            insert << "'Side'";
            break;
        case Drawing::END:
            insert << "'End'";
            break;
    }

    insert << ", '" << drawingData->date().toMySQLDateString() << "', " << drawingData->numberOfBars() << ", '" <<
           drawingData->hyperlink() << "', '" << drawingData->notes() << "')" << std::endl;

    return insert.str();
}

std::string DrawingInsert::barSpacingInsertQuery(unsigned matID) const {
    if (drawingData->numberOfBars() == 0) {
        return std::string();
    }

    std::stringstream insert;

    insert << "INSERT INTO bar_spacings" << std::endl;
    insert << "(mat_id, bar_spacing, bar_width, bar_index)" << std::endl;
    insert << "VALUES" << std::endl;

    for (unsigned i = 0; i < drawingData->numberOfBars(); i++) {
        insert << "(" << matID << ", " << drawingData->barSpacing(i) << ", " << drawingData->barWidth(i + 1) <<
               ", " << i << ")";
        if (i != drawingData->numberOfBars() - 1) {
            insert << ", ";
        }
        insert << std::endl;
    }

    return insert.str();
}

std::string DrawingInsert::machineTemplateInsertQuery() const {
    std::stringstream insert;

    insert << "INSERT INTO machine_templates" << std::endl;
    insert << "(machine_id, quantity_on_deck, position, deck_id)" << std::endl;
    insert << "VALUES" << std::endl;

    Drawing::MachineTemplate t = drawingData->machineTemplate();

    insert << "(" << t.machine().componentID << ", " << t.quantityOnDeck << ", '" <<
           t.position << "', " << t.deck().componentID << ")" << std::endl;

    return insert.str();
}

std::string DrawingInsert::testMachineTemplateQuery() const {
    std::stringstream query;

    Drawing::MachineTemplate t = drawingData->machineTemplate();

    query << "SELECT template_id FROM machine_templates" << std::endl;
    query << "WHERE machine_id=" << t.machine().componentID << " AND " << std::endl;
    query << "quantity_on_deck=" << t.quantityOnDeck << " AND" << std::endl;
    query << "position='" << t.position << "' AND" << std::endl;
    query << "deck_id=" << t.deck().componentID << std::endl;

    return query.str();
}

std::string DrawingInsert::apertureInsertQuery(unsigned matID) const {
    std::stringstream insert;

    insert << "INSERT INTO mat_aperture_link" << std::endl;
    insert << "(mat_id, aperture_id, direction)" << std::endl;
    insert << "VALUES" << std::endl;

    insert << "(" << matID << ", " << drawingData->aperture().componentID << ", '";

    switch (drawingData->apertureDirection()) {
        case ApertureDirection::NON_DIRECTIONAL:
            insert << "Nondirectional";
            break;
        case ApertureDirection::LONGITUDINAL:
            insert << "Longitudinal";
            break;
        case ApertureDirection::TRANSVERSE:
            insert << "Transverse";
            break;
    }

    insert << "')" << std::endl;

    return insert.str();
}

std::string DrawingInsert::sideIronInsertQuery(unsigned matID) const {
    std::stringstream insert;

    insert << "INSERT INTO mat_side_iron_link" << std::endl;
    insert << "(mat_id, side_iron_id, bar_width, inverted, side_iron_index)" << std::endl;
    insert << "VALUES" << std::endl;

    insert << "(" << matID << ", " << drawingData->sideIron(Drawing::LEFT).componentID << ", " << drawingData->leftBar() << ", " <<
           drawingData->sideIronInverted(Drawing::LEFT) << ", 0), " << std::endl;
    insert << "(" << matID << ", " << drawingData->sideIron(Drawing::RIGHT).componentID << ", " << drawingData->rightBar() << ", " <<
           drawingData->sideIronInverted(Drawing::RIGHT) << ", 1)" << std::endl;

    return insert.str();
}

std::string DrawingInsert::thicknessInsertQuery(unsigned matID) const {
    std::stringstream insert;

    insert << "INSERT INTO thickness" << std::endl;
    insert << "(mat_id, material_thickness_id)" << std::endl;
    insert << "VALUES" << std::endl;

    insert << "(" << matID << ", " << drawingData->material(Drawing::TOP)->componentID << ")";

    if (drawingData->material(Drawing::BOTTOM).has_value()) {
        insert << ", " << std::endl << "(" << matID << ", " << drawingData->material(Drawing::BOTTOM)->componentID
               << ")";
    }

    insert << std::endl;

    return insert.str();
}

std::string DrawingInsert::overlapsInsertQuery(unsigned matID) const {
    if (!drawingData->hasOverlaps()) {
        return std::string();
    }

    std::stringstream insert;

    insert << "INSERT INTO overlaps" << std::endl;
    insert << "(mat_id, width, mat_side, attachment_type, material_id)" << std::endl;
    insert << "VALUES" << std::endl;

    std::optional<Drawing::Lap> left = drawingData->overlap(Drawing::LEFT), right = drawingData->overlap(
            Drawing::RIGHT);

    if (left.has_value()) {
        insert << "(" << matID << ", " << left->width << ", 'Left', ";

        switch (left->attachmentType) {
            case LapAttachment::INTEGRAL:
                insert << "'Integral', " << drawingData->material(Drawing::TOP)->componentID << ")";
                break;
            case LapAttachment::BONDED:
                insert << "'Bonded', " << left->material().componentID << ")";
                break;
        }

        if (right.has_value()) {
            insert << ", ";
        }
        insert << std::endl;
    }

    if (right.has_value()) {
        insert << "(" << matID << ", " << right->width << ", 'Right', ";

        switch (right->attachmentType) {
            case LapAttachment::INTEGRAL:
                insert << "'Integral', " << drawingData->material(Drawing::TOP)->componentID << ")";
                break;
            case LapAttachment::BONDED:
                insert << "'Bonded', " << right->material().componentID << ")";
                break;
        }
        insert << std::endl;
    }

    return insert.str();
}

std::string DrawingInsert::sidelapsInsertQuery(unsigned matID) const {
    if (!drawingData->hasSidelaps()) {
        return std::string();
    }

    std::stringstream insert;

    insert << "INSERT INTO sidelaps" << std::endl;
    insert << "(mat_id, width, mat_side, attachment_type, material_id)" << std::endl;
    insert << "VALUES" << std::endl;

    std::optional<Drawing::Lap> left = drawingData->sidelap(Drawing::LEFT), right = drawingData->sidelap(
            Drawing::RIGHT);

    if (left.has_value()) {
        insert << "(" << matID << ", " << left->width << ", 'Left', ";

        switch (left->attachmentType) {
            case LapAttachment::INTEGRAL:
                insert << "'Integral', " << drawingData->material(Drawing::TOP)->componentID << ")";
                break;
            case LapAttachment::BONDED:
                insert << "'Bonded', " << left->material().componentID << ")";
                break;
        }

        if (right.has_value()) {
            insert << ", ";
        }
        insert << std::endl;
    }

    if (right.has_value()) {
        insert << "(" << matID << ", " << right->width << ", 'Right', ";

        switch (right->attachmentType) {
            case LapAttachment::INTEGRAL:
                insert << "'Integral', " << drawingData->material(Drawing::TOP)->componentID << ")";
                break;
            case LapAttachment::BONDED:
                insert << "'Bonded', " << right->material().componentID << ")";
                break;
        }
        insert << std::endl;
    }

    return insert.str();
}

std::string DrawingInsert::punchProgramsInsertQuery(unsigned matID) const {
    std::vector<std::string> punchPDFs = drawingData->pressDrawingHyperlinks();

    if (punchPDFs.empty()) {
        return std::string();
    }

    std::stringstream insert;

    insert << "INSERT INTO punch_program_pdfs" << std::endl;
    insert << "(mat_id, hyperlink)" << std::endl;
    insert << "VALUES" << std::endl;

    for (std::vector<std::string>::const_iterator it = punchPDFs.begin(); it != punchPDFs.end(); it++) {
        insert << "(" << matID << ", '" << *it << "')";
        if (it != punchPDFs.end() - 1) {
            insert << ", ";
        }
        insert << std::endl;
    }

    return insert.str();
}

#pragma clang diagnostic pop
