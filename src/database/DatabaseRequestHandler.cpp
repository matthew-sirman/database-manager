//
// Created by matthew on 09/07/2020.
//

#include "../../include/database/DatabaseRequestHandler.h"

#include <utility>

DatabaseRequestHandler::DatabaseRequestHandler() : schema(0, 0, 0, 0, 0, 0, 0, 0, 0, 0) {
    pricingMap.insert({"running_m", MaterialPricingType::RUNNING_M });
    pricingMap.insert({ "square_m", MaterialPricingType::SQUARE_M });
    pricingMap.insert({ "sheet", MaterialPricingType::SHEET });
}

void DatabaseRequestHandler::onMessageReceived(Server &caller, const ClientHandle &clientHandle, void*&& message,
	unsigned int messageSize) {
	switch (getDeserialiseType(message)) {
        case RequestType::REPEAT_TOKEN_REQUEST:
            caller.sendRepeatToken(clientHandle, (unsigned) RequestType::REPEAT_TOKEN_REQUEST);
            break;
        case RequestType::USER_EMAIL_REQUEST:
            caller.sendEmailAddress(clientHandle, (unsigned) RequestType::USER_EMAIL_REQUEST);
            break;
        case RequestType::DRAWING_SEARCH_QUERY: {
            DatabaseSearchQuery &query = DatabaseSearchQuery::deserialise(std::move(message));
            std::vector<DrawingSummary> summaries = caller.databaseManager().executeSearchQuery(query);
            delete &query;

            DrawingSummaryCompressionSchema summaryCompressionSchema = compressionSchema(&caller.databaseManager());

            unsigned char *responseBuffer = (unsigned char *) alloca(sizeof(RequestType) +
                                                                     sizeof(DrawingSummaryCompressionSchema) +
                                                                     sizeof(unsigned) +
                                                                     summaries.size() *
                                                                     summaryCompressionSchema.maxCompressedSize());

            unsigned index = 0;

            *((RequestType *) responseBuffer) = RequestType::DRAWING_SEARCH_QUERY;
            index += sizeof(RequestType);

            memcpy(responseBuffer + index, &summaryCompressionSchema, sizeof(DrawingSummaryCompressionSchema));
            index += sizeof(DrawingSummaryCompressionSchema);

            *((unsigned *) (responseBuffer + index)) = summaries.size();
            index += sizeof(unsigned);

            for (const DrawingSummary &summary : summaries) {
                summaryCompressionSchema.compressSummary(summary, responseBuffer + index);
                index += summaryCompressionSchema.compressedSize(summary);
            }

            caller.addMessageToSendQueue(clientHandle, responseBuffer, index);

            break;
        }
        case RequestType::DRAWING_INSERT: {
            DrawingInsert &drawingInsert = DrawingInsert::deserialise(std::move(message));

            if (drawingInsert.drawingData.has_value()) {
                DrawingInsert response;

                response.responseEchoCode = drawingInsert.responseEchoCode;

                switch (caller.databaseManager().drawingExists(drawingInsert.drawingData->drawingNumber())) {
                    case DatabaseManager::DrawingExistsResponse::EXISTS:
                        if (drawingInsert.forcing()) {
                            if (caller.databaseManager().insertDrawing(drawingInsert)) {
                                response.insertResponseCode = DrawingInsert::SUCCESS;
                            } else {
                                response.insertResponseCode = DrawingInsert::FAILED;
                            }
                        } else {
                            response.insertResponseCode = DrawingInsert::DRAWING_EXISTS;
                        }
                        break;
                    case DatabaseManager::DrawingExistsResponse::NOT_EXISTS:
                        if (caller.databaseManager().insertDrawing(drawingInsert)) {
                            response.insertResponseCode = DrawingInsert::SUCCESS;
                        } else {
                            response.insertResponseCode = DrawingInsert::FAILED;
                        }
                        break;
                    case DatabaseManager::DrawingExistsResponse::R_ERROR:
                        response.insertResponseCode = DrawingInsert::FAILED;
                        break;
                }

                if (response.insertResponseCode == DrawingInsert::SUCCESS) {
                    setCompressionSchemaDirty();
                    caller.changelogMessage(clientHandle,
                                            "Added drawing " + drawingInsert.drawingData->drawingNumber());
                }

                delete &drawingInsert;

                unsigned responseSize = response.serialisedSize();
                void *responseBuffer = alloca(responseSize);
                response.serialise(responseBuffer);

                caller.addMessageToSendQueue(clientHandle, responseBuffer, responseSize);

                NextDrawing automatic, manual;
                automatic.drawingType = NextDrawing::DrawingType::AUTOMATIC;
                manual.drawingType = NextDrawing::DrawingType::MANUAL;

                automatic.drawingNumber = caller.databaseManager().nextAutomaticDrawingNumber();
                manual.drawingNumber = caller.databaseManager().nextManualDrawingNumber();

                unsigned autoBufferSize = automatic.serialisedSize();
                void *autoBuffer = alloca(autoBufferSize);
                automatic.serialise(autoBuffer);

                unsigned manualBufferSize = manual.serialisedSize();
                void *manualBuffer = alloca(manualBufferSize);
                manual.serialise(manualBuffer);
                caller.broadcastMessage(autoBuffer, autoBufferSize);
                caller.broadcastMessage(manualBuffer, manualBufferSize);
            }

            break;
        }
        case RequestType::SOURCE_PRODUCT_TABLE: {
            if (DrawingComponentManager<Product>::dirty()) {
                createSourceData<ProductData>(caller.databaseManager().sourceTable("products"));
            }

            caller.addMessageToSendQueue(clientHandle, DrawingComponentManager<Product>::rawSourceData(),
                                         DrawingComponentManager<Product>::rawSourceDataSize());

            break;
        }
        case RequestType::SOURCE_BACKING_STRIPS_TABLE:
        {
            if (DrawingComponentManager<BackingStrip>::dirty()) {
                if (DrawingComponentManager<Material>::dirty()) {
                    createSourceData<MaterialData>(caller.databaseManager().sourceMultipleTable("material_prices", "materials", "material_id"));
                }

                createSourceData<BackingStripData>(caller.databaseManager().sourceTable("backing_strips"));
            }

            caller.addMessageToSendQueue(clientHandle, DrawingComponentManager<BackingStrip>::rawSourceData(),
                                         DrawingComponentManager<BackingStrip>::rawSourceDataSize());

            break;
        }
        case RequestType::SOURCE_APERTURE_TABLE: {
            if (DrawingComponentManager<Aperture>::dirty()) {
                if (DrawingComponentManager<ApertureShape>::dirty()) {
                    createSourceData<ApertureShapeData>(caller.databaseManager().sourceTable("aperture_shapes"));
                }

                createSourceData<ApertureData>(caller.databaseManager().sourceTable("apertures"));
            }

            caller.addMessageToSendQueue(clientHandle, DrawingComponentManager<Aperture>::rawSourceData(),
                                         DrawingComponentManager<Aperture>::rawSourceDataSize());

            break;
        }
        case RequestType::SOURCE_APERTURE_SHAPE_TABLE: {
            if (DrawingComponentManager<ApertureShape>::dirty()) {
                createSourceData<ApertureShapeData>(caller.databaseManager().sourceTable("aperture_shapes"));
            }

            caller.addMessageToSendQueue(clientHandle, DrawingComponentManager<ApertureShape>::rawSourceData(),
                                         DrawingComponentManager<ApertureShape>::rawSourceDataSize());

            break;
        }
        case RequestType::SOURCE_MATERIAL_TABLE: {
            if (DrawingComponentManager<Material>::dirty()) {
                createSourceData<MaterialData>(caller.databaseManager().sourceMultipleTable("material_prices", "materials", "material_id"));
            }

            caller.addMessageToSendQueue(clientHandle, DrawingComponentManager<Material>::rawSourceData(),
                                         DrawingComponentManager<Material>::rawSourceDataSize());

            break;
        }
        case RequestType::SOURCE_EXTRA_PRICES_TABLE: {
            if (DrawingComponentManager<ExtraPrice>::dirty()) {
                createSourceData<ExtraPriceData>(caller.databaseManager().sourceTable("extra_prices"));
            }
            caller.addMessageToSendQueue(clientHandle, DrawingComponentManager<ExtraPrice>::rawSourceData(),
                                            DrawingComponentManager<ExtraPrice>::rawSourceDataSize());
            break;
        }
        case RequestType::SOURCE_LABOUR_TIMES_TABLE:
        {
            if (DrawingComponentManager<LabourTime>::dirty()) {
                createSourceData<LabourTimeData>(caller.databaseManager().sourceTable("labour_times"));
            }
            caller.addMessageToSendQueue(clientHandle, DrawingComponentManager<LabourTime>::rawSourceData(),
                DrawingComponentManager<LabourTime>::rawSourceDataSize());
                break;
        }
        case RequestType::SOURCE_POWDER_COATING_TABLE:
        {
            if (DrawingComponentManager<PowderCoatingPrice>::dirty()) {
                createSourceData<PowderCoatingPriceData>(caller.databaseManager().sourceTable("powder_coating_prices"));
            }
            caller.addMessageToSendQueue(clientHandle, DrawingComponentManager<PowderCoatingPrice>::rawSourceData(),
                DrawingComponentManager<PowderCoatingPrice>::rawSourceDataSize());
            break;
        }
        case RequestType::SOURCE_SIDE_IRON_PRICES_TABLE: {
            if (DrawingComponentManager<SideIronPrice>::dirty()) {
                createSourceData<SideIronPriceData>(caller.databaseManager().sourceTable("side_iron_prices"));
            }

            caller.addMessageToSendQueue(clientHandle, DrawingComponentManager<SideIronPrice>::rawSourceData(),
                DrawingComponentManager<SideIronPrice>::rawSourceDataSize());
            break;
        }
        case RequestType::SOURCE_SIDE_IRON_TABLE: {
            if (DrawingComponentManager<SideIron>::dirty()) {
                std::stringstream orderBy;
                orderBy << "CASE " << std::endl;
                orderBy << "WHEN drawing_number LIKE 'None' THEN 1 " << std::endl;
                orderBy << "WHEN drawing_number LIKE 'SCS1562%' THEN 2 " << std::endl;
                orderBy << "WHEN drawing_number LIKE 'SCS1565%' THEN 3 " << std::endl;
                orderBy << "WHEN drawing_number LIKE 'SCS1564%' THEN 4 " << std::endl;
                orderBy << "WHEN drawing_number LIKE 'SCS1567%' THEN 5 " << std::endl;
                orderBy << "WHEN drawing_number LIKE 'SCS1568%' THEN 6 " << std::endl;
                orderBy << "WHEN drawing_number LIKE 'SCS1334%' THEN 7 " << std::endl;
                orderBy << "WHEN drawing_number LIKE 'SCS1569%' THEN 8 " << std::endl;
                orderBy << "WHEN drawing_number LIKE 'SCS1335%' THEN 9 " << std::endl;
                orderBy << "ELSE 10 " << std::endl;
                orderBy << "END, length, drawing_number DESC" << std::endl;

                createSourceData<SideIronData>(caller.databaseManager().sourceTable("side_irons", orderBy.str()));
            }

            caller.addMessageToSendQueue(clientHandle, DrawingComponentManager<SideIron>::rawSourceData(),
                                         DrawingComponentManager<SideIron>::rawSourceDataSize());

            break;
        }
        case RequestType::SOURCE_MACHINE_TABLE: {
            if (DrawingComponentManager<Machine>::dirty()) {
                createSourceData<MachineData>(caller.databaseManager().sourceTable("machines",
                                                                                   "manufacturer<>'None', manufacturer, model"));
            }

            caller.addMessageToSendQueue(clientHandle, DrawingComponentManager<Machine>::rawSourceData(),
                                         DrawingComponentManager<Machine>::rawSourceDataSize());

            break;
        }
        case RequestType::SOURCE_MACHINE_DECK_TABLE: {
            if (DrawingComponentManager<MachineDeck>::dirty()) {
                createSourceData<MachineDeckData>(caller.databaseManager().sourceTable("machine_decks"));
            }

            caller.addMessageToSendQueue(clientHandle, DrawingComponentManager<MachineDeck>::rawSourceData(),
                                         DrawingComponentManager<MachineDeck>::rawSourceDataSize());

            break;
        }
        case RequestType::DRAWING_DETAILS: {
            DrawingRequest request = DrawingRequest::deserialise(std::move(message));

            Drawing *returnedDrawing = caller.databaseManager().executeDrawingQuery(request);
            if (returnedDrawing != nullptr) {
                request.drawingData = *returnedDrawing;
            } else {
                request.drawingData = Drawing();
                request.drawingData->setLoadWarning(Drawing::LOAD_FAILED);
            }

            unsigned bufferSize = request.serialisedSize();

            void *responseBuffer = malloc(bufferSize);
            request.serialise(responseBuffer);

            caller.addMessageToSendQueue(clientHandle, responseBuffer, bufferSize);


            delete returnedDrawing, responseBuffer;

            break;
        }
        case RequestType::ADD_NEW_COMPONENT: {
            ComponentInsert &insert = ComponentInsert::deserialise(std::move(message));

            ComponentInsert response;
            response.clearComponentData();

            response.responseCode = caller.databaseManager().insertComponent(insert)
                                    ? ComponentInsert::ComponentInsertResponse::SUCCESS
                                    : ComponentInsert::ComponentInsertResponse::FAILED;

            unsigned bufferSize = response.serialisedSize();
            void *responseBuffer = alloca(bufferSize);
            response.serialise(responseBuffer);

            caller.addMessageToSendQueue(clientHandle, responseBuffer, bufferSize);

            if (response.responseCode != ComponentInsert::ComponentInsertResponse::SUCCESS) {
                break;
            }

            void *sourceData;
            unsigned sourceDataBufferSize;

            switch (insert.getSourceTableCode()) {
                case RequestType::SOURCE_APERTURE_TABLE:
                {
                    if (DrawingComponentManager<ApertureShape>::dirty()) {
                        createSourceData<ApertureShapeData>(caller.databaseManager().sourceTable("aperture_shapes"));
                    }
                    createSourceData<ApertureData>(caller.databaseManager().sourceTable("apertures"));

                    sourceData = DrawingComponentManager<Aperture>::rawSourceData();
                    sourceDataBufferSize = DrawingComponentManager<Aperture>::rawSourceDataSize();

                    caller.changelogMessage(clientHandle, "Added a new aperture");
                    break;
                }
                case RequestType::SOURCE_BACKING_STRIPS_TABLE:
                {
                    createSourceData<BackingStripData>(caller.databaseManager().sourceTable("apertures"));

                    sourceData = DrawingComponentManager<BackingStrip>::rawSourceData();
                    sourceDataBufferSize = DrawingComponentManager<BackingStrip>::rawSourceDataSize();

                    caller.changelogMessage(clientHandle, "Added a new aperture");
                    break;
                }
                case RequestType::SOURCE_MACHINE_TABLE: {
                    createSourceData<MachineData>(caller.databaseManager().sourceTable("machines",
                                                                                       "manufacturer<>'None', manufacturer, model"));

                    sourceData = DrawingComponentManager<Machine>::rawSourceData();
                    sourceDataBufferSize = DrawingComponentManager<Machine>::rawSourceDataSize();

                    caller.changelogMessage(clientHandle, "Added a new machine");
                    break;
                }
                case RequestType::SOURCE_SIDE_IRON_TABLE: {
                   std::stringstream orderBy;
                    orderBy << "CASE " << std::endl;
                    orderBy << "WHEN drawing_number LIKE 'None' THEN 1 " << std::endl;
                    orderBy << "WHEN drawing_number LIKE 'SCS1562%' THEN 2 " << std::endl;
                    orderBy << "WHEN drawing_number LIKE 'SCS1565%' THEN 3 " << std::endl;
                    orderBy << "WHEN drawing_number LIKE 'SCS1564%' THEN 4 " << std::endl;
                    orderBy << "WHEN drawing_number LIKE 'SCS1567%' THEN 5 " << std::endl;
                    orderBy << "WHEN drawing_number LIKE 'SCS1568%' THEN 6 " << std::endl;
                    orderBy << "WHEN drawing_number LIKE 'SCS1334%' THEN 7 " << std::endl;
                    orderBy << "WHEN drawing_number LIKE 'SCS1569%' THEN 8 " << std::endl;
                    orderBy << "WHEN drawing_number LIKE 'SCS1335%' THEN 9 " << std::endl;
                    orderBy << "ELSE 10 " << std::endl;
                    orderBy << "END, length, drawing_number DESC" << std::endl;

                    createSourceData<SideIronData>(caller.databaseManager().sourceTable("side_irons", orderBy.str()));

                    sourceData = DrawingComponentManager<SideIron>::rawSourceData();
                    sourceDataBufferSize = DrawingComponentManager<SideIron>::rawSourceDataSize();

                    caller.changelogMessage(clientHandle, "Added a new side iron");
                    break;
                }
                case RequestType::SOURCE_MATERIAL_TABLE: {
                    createSourceData<MaterialData>(caller.databaseManager().sourceMultipleTable("material_prices", "materials", "material_id"));
                    sourceData = DrawingComponentManager<Material>::rawSourceData();
                    sourceDataBufferSize = DrawingComponentManager<Material>::rawSourceDataSize();

                    caller.changelogMessage(clientHandle, "Added a new material");
                    break;
                }
                case RequestType::SOURCE_EXTRA_PRICES_TABLE: {
                    createSourceData<ExtraPriceData>(caller.databaseManager().sourceTable("extra_prices"));
                    sourceData = DrawingComponentManager<ExtraPrice>::rawSourceData();
                    sourceDataBufferSize = DrawingComponentManager<ExtraPrice>::rawSourceDataSize();

                    caller.changelogMessage(clientHandle, "Added a new Extra Price");
                    break;
                }
                case RequestType::SOURCE_LABOUR_TIMES_TABLE:
                {
                    createSourceData<LabourTimeData>(caller.databaseManager().sourceTable("labour_times"));
                    sourceData = DrawingComponentManager<LabourTime>::rawSourceData();
                    sourceDataBufferSize = DrawingComponentManager<LabourTime>::rawSourceDataSize();
                    break;
                }
                case RequestType::SOURCE_POWDER_COATING_TABLE:
                {
                    createSourceData<PowderCoatingPriceData>(caller.databaseManager().sourceTable("powder_coating_prices"));
                    sourceData = DrawingComponentManager<PowderCoatingPrice>::rawSourceData();
                    sourceDataBufferSize = DrawingComponentManager<PowderCoatingPrice>::rawSourceDataSize();
                    break;
                }
                case RequestType::SOURCE_SIDE_IRON_PRICES_TABLE: {
                    createSourceData<SideIronPriceData>(caller.databaseManager().sourceMultipleTable("side_iron_prices", "side_iron_types", std::tuple<std::string, std::string>("type", "side_iron_type_id")));
                    sourceData = DrawingComponentManager<SideIronPrice>::rawSourceData();
                    sourceDataBufferSize = DrawingComponentManager<SideIronPrice>::rawSourceDataSize();

                    caller.changelogMessage(clientHandle, "Added a new material");
                    break;
                }
                default:
                    return;
            }

            caller.broadcastMessage(sourceData, sourceDataBufferSize);

            delete &insert;

            break;
        }
        case RequestType::GET_NEXT_DRAWING_NUMBER: {
            NextDrawing &next = NextDrawing::deserialise(std::move(message));

            switch (next.drawingType) {
                case NextDrawing::DrawingType::AUTOMATIC:
                    next.drawingNumber = caller.databaseManager().nextAutomaticDrawingNumber();
                    break;
                case NextDrawing::DrawingType::MANUAL:
                    next.drawingNumber = caller.databaseManager().nextManualDrawingNumber();
                    break;
            }

            unsigned bufferSize = next.serialisedSize();
            void *responseBuffer = alloca(bufferSize);
            next.serialise(responseBuffer);

            caller.addMessageToSendQueue(clientHandle, responseBuffer, bufferSize);

            delete &next;

            break;
        }
        case RequestType::CREATE_DATABASE_BACKUP: {
            DatabaseBackup &backup = DatabaseBackup::deserialise(std::move(message));

            std::filesystem::path backupFile = backupPath / backup.backupName;
            backupFile.replace_extension("sql");

            DatabaseBackup response;

            if (caller.databaseManager().createBackup(backupFile)) {
                response.responseCode = DatabaseBackup::BackupResponse::SUCCESS;
            } else {
                response.responseCode = DatabaseBackup::BackupResponse::FAILED;
            }

            response.backupName = std::string();

            unsigned bufferSize = response.serialisedSize();
            void *responseBuffer = alloca(bufferSize);
            response.serialise(responseBuffer);

            caller.addMessageToSendQueue(clientHandle, responseBuffer, bufferSize);

            delete &backup;

            break;
        }
    }
}

RequestType DatabaseRequestHandler::getDeserialiseType(void *data) {
	return *((RequestType *)data);
}

DrawingSummaryCompressionSchema DatabaseRequestHandler::compressionSchema(DatabaseManager *dbManager) {
	if (schemaDirty) {
		if (!dbManager) {
			STD_ERROR("Database manager not set up. No connection to database.");
		}

		if (DrawingComponentManager<Aperture>::dirty()) {
			if (DrawingComponentManager<ApertureShape>::dirty()) {
				createSourceData<ApertureShapeData>(dbManager->sourceTable("aperture_shapes"));
			}

			createSourceData<ApertureData>(dbManager->sourceTable("apertures"));
		}

		if (DrawingComponentManager<Material>::dirty()) {
			createSourceData<MaterialData>(dbManager->sourceMultipleTable("material_prices", "materials", "material_id"));
		}

        if (DrawingComponentManager<SideIronPrice>::dirty()) {
            createSourceData<SideIronPriceData>(dbManager->sourceTable("side_iron_prices"));
        }

		unsigned maxMatID, maxThicknessHandle, maxApertureHandle;
		float maxWidth, maxLength, maxLapSize;
		unsigned char maxDrawingLength;
		unsigned char maxBarSpacingCount;
		float maxBarSpacing;
        unsigned char maxExtraApertureCount;


		dbManager->getCompressionSchemaDetails(maxMatID, maxWidth, maxLength, maxLapSize, maxBarSpacingCount, maxBarSpacing, maxDrawingLength, maxExtraApertureCount);
		maxThicknessHandle = DrawingComponentManager<Material>::maximumHandle();
		maxApertureHandle = DrawingComponentManager<Aperture>::maximumHandle();
		
		schema = DrawingSummaryCompressionSchema(maxMatID, maxWidth, maxLength, maxThicknessHandle, 
												 maxLapSize, maxApertureHandle, maxBarSpacingCount, 
												 maxBarSpacing, maxDrawingLength, maxExtraApertureCount);

		schemaDirty = false;
	}

	return schema;
}

void DatabaseRequestHandler::setCompressionSchemaDirty() {
	schemaDirty = true;
}

template<>
void DatabaseRequestHandler::constructDataElements(
	mysqlx::RowResult &productRow, unsigned &handle,
	std::vector<DatabaseRequestHandler::ProductData> &elements, unsigned &sizeValue
) const {
    for (mysqlx::Row row : productRow) {
        if (!row.isNull()) {
            ProductData data;
            data.handle = handle++;
            data.id = row.get(0).get<unsigned int>();
            data.name = row.get(1).get<std::string>();
            sizeValue += sizeof(unsigned) * 2 + sizeof(unsigned char) + data.name.size();

            elements.push_back(data);
        }
    }
}

template<>
void DatabaseRequestHandler::serialiseDataElement(const DatabaseRequestHandler::ProductData &element, void *buffer, unsigned &sizeValue) const {
	unsigned char *buff = (unsigned char *)buffer;

	unsigned char nameSize = element.name.size();
	*buff++ = nameSize;
	memcpy(buff, element.name.c_str(), nameSize);

	sizeValue += sizeof(unsigned char) + nameSize;
}

template<>
RequestType DatabaseRequestHandler::getRequestType<DatabaseRequestHandler::ProductData>() const {
	return RequestType::SOURCE_PRODUCT_TABLE;
}

template<>
void DatabaseRequestHandler::constructDataElements(
	mysqlx::RowResult &apertureRow, unsigned &handle,
	std::vector<DatabaseRequestHandler::ApertureData> &elements, unsigned &sizeValue
) const {
    for (mysqlx::Row row : apertureRow) {
        if (row[6].isNull()) {
            ERROR_RAW_SAFE("Aperture with missing shape ID detected.", std::cerr);
            return;
        }
        ApertureData data;
        data.handle = handle++;
        data.id = row[0];
        data.width = row[1].isNull() ? 0 : row[1].get<float>();
        data.length = row[2].isNull() ? 0 : row[2].get<float>();
        data.baseWidth = row[3].isNull() ? 0 : row[3].get<unsigned>();
        data.baseLength = row[4].isNull() ? 0 : row[4].get<unsigned>();
        data.quantity = row[5].isNull() ? 0 : row[5].get<unsigned>();
        data.shapeID = DrawingComponentManager<ApertureShape>::findComponentByID(row[6]).handle();
        data.isNibble = row[7].get<bool>();
        if (data.isNibble) {
            data.nibbleApertureId = row[8].get<unsigned>();
        }

        sizeValue += sizeof(unsigned) * 2 + sizeof(float) * 2 + sizeof(unsigned short) * 3 + sizeof(unsigned) + sizeof(bool) + (data.isNibble ? sizeof(unsigned) : 0);

        elements.push_back(data);
    }
}

template<>
void DatabaseRequestHandler::serialiseDataElement(const DatabaseRequestHandler::ApertureData &element, void *buffer, unsigned &sizeValue) const {
	unsigned char *buff = (unsigned char *)buffer;

	*((float *)buff) = element.width;
	buff += sizeof(float);
	*((float *)buff) = element.length;
	buff += sizeof(float);
	*((unsigned short *)buff) = element.baseWidth;
	buff += sizeof(unsigned short);
	*((unsigned short *)buff) = element.baseLength;
	buff += sizeof(unsigned short);
	*((unsigned *)buff) = element.shapeID;
	buff += sizeof(unsigned);
	*((unsigned short *)buff) = element.quantity;
	buff += sizeof(unsigned short);
    *buff++ = element.isNibble;
    if (element.isNibble) {
        *((unsigned*)buff) = element.nibbleApertureId;
    }

	sizeValue += sizeof(float) * 2 + sizeof(unsigned short) * 3 + sizeof(unsigned) + sizeof(bool) + (element.isNibble ? sizeof(unsigned) : 0);
}

template<>
RequestType DatabaseRequestHandler::getRequestType<DatabaseRequestHandler::ApertureData>() const {
	return RequestType::SOURCE_APERTURE_TABLE;
}

template<>
void DatabaseRequestHandler::constructDataElements(
    mysqlx::RowResult& stripRow, unsigned& handle,
    std::vector<DatabaseRequestHandler::BackingStripData>& elements, unsigned& sizeValue
) const {
    for (mysqlx::Row row : stripRow) {
        BackingStripData strip;
        strip.handle = handle++;
        strip.id = row[0];
        strip.materialID = row[1];

        sizeValue += sizeof(unsigned) * 3;

        elements.push_back(strip);
    }
}

template<>
void DatabaseRequestHandler::serialiseDataElement(const DatabaseRequestHandler::BackingStripData& element, void* buffer, unsigned& sizeValue) const {
    unsigned char* buff = (unsigned char*)buffer;

    *((unsigned*)buff) = element.materialID;
    buff += sizeof(unsigned);

    sizeValue += sizeof(unsigned);
}

template<>
RequestType DatabaseRequestHandler::getRequestType<DatabaseRequestHandler::BackingStripData>() const {
    return RequestType::SOURCE_BACKING_STRIPS_TABLE;
}

template<>
void DatabaseRequestHandler::constructDataElements(
    mysqlx::RowResult& extraPriceRow, unsigned& handle, std::vector<DatabaseRequestHandler::ExtraPriceData>& elements, unsigned& sizeValue
) const {
    ExtraPriceData data;
    for (mysqlx::Row row : extraPriceRow) {
        data = *(new ExtraPriceData);

        data.handle = handle++;
        data.id = row.get(0).get<unsigned int>();

        if (row[1].get<std::string>() == "side_iron_nuts") {
            data.type = ExtraPriceType::SIDE_IRON_NUTS;
            data.amount = row[3].get<unsigned>();
            data.squareMetres = std::nullopt;
            sizeValue += sizeof(unsigned);
        }
        else if (row[1].get<std::string>() == "side_iron_screws") {
            data.type = ExtraPriceType::SIDE_IRON_SCREWS;
            data.amount = row[3].get<unsigned>();
            data.squareMetres = std::nullopt;
            sizeValue += sizeof(unsigned);
        }
        else if (row[1].get<std::string>() == "glue") {
            data.type = ExtraPriceType::TACKYBACK_GLUE;
            data.amount = std::nullopt;
            data.squareMetres = row[4].get<float>();
            sizeValue += sizeof(float);
        }
        else if (row[1].get<std::string>() == "labour") {
            data.type = ExtraPriceType::LABOUR;
            data.amount = std::nullopt;
            data.squareMetres= std::nullopt;
        }
        else if (row[1].get<std::string>() == "primer") {
            data.type = ExtraPriceType::PRIMER;
            data.amount = std::nullopt;
            data.squareMetres = row[4].get<float>();
            sizeValue += sizeof(float);
        }
        data.price = row[2];
        sizeValue += sizeof(ExtraPriceType) + sizeof(float) + sizeof(unsigned) * 2;

        elements.push_back(data);
    }
}


template<>
void DatabaseRequestHandler::serialiseDataElement(const DatabaseRequestHandler::ExtraPriceData& element, void* buffer, unsigned& sizeValue) const {
    unsigned char* buff = (unsigned char*)buffer;

    *((ExtraPriceType*)buff) = element.type;
    buff += sizeof(ExtraPriceType);

    *((float*)buff) = element.price;
    buff += sizeof(float);

    if (element.amount.has_value()) {
        *((unsigned*)buff) = element.amount.value();
        buff += sizeof(unsigned);
        sizeValue += sizeof(unsigned);
    }
    if (element.squareMetres.has_value()) {
        *((float*)buff) = element.squareMetres.value();
        buff += sizeof(float);
        sizeValue += sizeof(unsigned);
    }


    sizeValue += sizeof(ExtraPriceType) + sizeof(float);

}

template<>
RequestType DatabaseRequestHandler::getRequestType<DatabaseRequestHandler::ExtraPriceData>() const {
    return RequestType::SOURCE_EXTRA_PRICES_TABLE;
}

template<>
void DatabaseRequestHandler::constructDataElements(mysqlx::RowResult& labourTimeRow, unsigned& handle,
                                                   std::vector<DatabaseRequestHandler::LabourTimeData>& elements, unsigned& sizeValue) const {

    for (mysqlx::Row row : labourTimeRow) {
        LabourTimeData data;

        data.handle = handle++;
        data.id = row.get(0).get<unsigned>();
        data.job = row.get(1).get<std::string>();
        data.time = row[2].get<unsigned>();
        sizeValue += sizeof(unsigned) * 3 + sizeof(data.job.size()) + data.job.size();

        elements.push_back(data);
    }
}

template<>
void DatabaseRequestHandler::serialiseDataElement(const DatabaseRequestHandler::LabourTimeData& element, void* buffer, unsigned& sizeValue) const {
    unsigned char* buff = (unsigned char*)buffer;

    *((size_t*)buff) = element.job.size();
    buff += sizeof(size_t);
    std::memcpy(buff, element.job.c_str(), element.job.size());
    buff += element.job.size();

    *((unsigned*)buff) = element.time;
    buff += sizeof(unsigned);

    sizeValue += sizeof(unsigned) + sizeof(size_t) + element.job.size();
}

template<>
RequestType DatabaseRequestHandler::getRequestType<DatabaseRequestHandler::LabourTimeData>() const {
    return RequestType::SOURCE_LABOUR_TIMES_TABLE;
}

template<>
void DatabaseRequestHandler::constructDataElements(
	mysqlx::RowResult &apertureShapeRow, unsigned &handle,
	std::vector<DatabaseRequestHandler::ApertureShapeData> &elements, unsigned &sizeValue
) const {
    for (mysqlx::Row row : apertureShapeRow) {
        if (!row[0].isNull() && !row[1].isNull()) {
            ApertureShapeData data;
            data.handle = handle++;
            data.id = row[0];
            data.shape = row[1].get<std::string>();
            sizeValue += sizeof(unsigned) * 2 + sizeof(unsigned char) + data.shape.size();

            elements.push_back(data);
        }
    }
}

template<>
void DatabaseRequestHandler::serialiseDataElement(const DatabaseRequestHandler::ApertureShapeData &element, void *buffer, unsigned &sizeValue) const {
	unsigned char *buff = (unsigned char *)buffer;

	unsigned char shapeSize = element.shape.size();
	*buff++ = shapeSize;
	memcpy(buff, element.shape.c_str(), shapeSize);

	sizeValue += sizeof(unsigned char) + shapeSize;
}

template<>
RequestType DatabaseRequestHandler::getRequestType<DatabaseRequestHandler::ApertureShapeData>() const {
	return RequestType::SOURCE_APERTURE_SHAPE_TABLE;
}

template<>
void DatabaseRequestHandler::constructDataElements(
	mysqlx::RowResult &materialRow, unsigned &handle,
	std::vector<DatabaseRequestHandler::MaterialData> &elements, unsigned &sizeValue
) const {
    std::map<unsigned, DatabaseRequestHandler::MaterialData> material_ids;
    for (mysqlx::Row row : materialRow) {
        if (material_ids.find(row[6]) == material_ids.end()) {
            DatabaseRequestHandler::MaterialData data;
            data.handle = handle++;
            data.id = row[6];
            data.name = row[7].get<std::string>();
            data.hardness = row[8].get<unsigned>();
            data.thickness = row[9].get<unsigned>();
            sizeValue += sizeof(unsigned) * 2 + sizeof(unsigned short) * 2 + sizeof(unsigned char) + data.name.size() + sizeof(unsigned char);
            material_ids.insert({ row[6], data });
            if (!row[2].isNull()) {
                sizeValue += sizeof(unsigned) + sizeof(float) * 3 + sizeof(MaterialPricingType);
                material_ids[row[6]].materialPrices.push_back({ row[0], row[2], row[3], row[4], pricingMap.at(row[5].get<std::string>()) });
            }
        }
        else {
            sizeValue += sizeof(unsigned) +  sizeof(float) * 3 + sizeof(MaterialPricingType);
            material_ids[row[6]].materialPrices.push_back({ row[0], row[2], row[3], row[4], pricingMap.at(row[5].get<std::string>()) });
        }
    }
    for (const std::pair<unsigned, MaterialData>& key : material_ids) {
        elements.push_back(key.second);
    }
}

template<>
void DatabaseRequestHandler::serialiseDataElement(const DatabaseRequestHandler::MaterialData &element, void *buffer, unsigned &sizeValue) const {
	unsigned char *buff = (unsigned char *)buffer;

	*((unsigned short *)buff) = element.hardness;
	buff += sizeof(unsigned short);

	*((unsigned short *)buff) = element.thickness;
	buff += sizeof(unsigned short);

	unsigned char nameSize = element.name.size();
	*buff++ = nameSize;
	memcpy(buff, element.name.c_str(), nameSize);
    buff += nameSize;

    unsigned char priceElements = element.materialPrices.size();
    *buff++ = priceElements;

    for (Material::MaterialPrice tuple : element.materialPrices) {
        *((unsigned*)buff) = std::get<0>(tuple);
        buff += sizeof(unsigned);
        *((float*)buff) = std::get<1>(tuple);
        buff += sizeof(float);
        *((float*)buff) = std::get<2>(tuple);
        buff += sizeof(float);
        *((float*)buff) = std::get<3>(tuple);
        buff += sizeof(float);
        *((MaterialPricingType*)buff) = std::get<4>(tuple);
        buff += sizeof(MaterialPricingType);
    }
    

	sizeValue += sizeof(unsigned short) * 2 + sizeof(unsigned char) + nameSize + sizeof(unsigned char) + priceElements * (sizeof(unsigned) + sizeof(float) * 3 + sizeof(MaterialPricingType));
}


template<>
RequestType DatabaseRequestHandler::getRequestType<DatabaseRequestHandler::MaterialData>() const {
	return RequestType::SOURCE_MATERIAL_TABLE;
}

template<>
void DatabaseRequestHandler::constructDataElements(mysqlx::RowResult& sideIronPriceRow, unsigned& handle,
    std::vector<DatabaseRequestHandler::SideIronPriceData>& elements, unsigned& sizeValue
) const {
    for (mysqlx::Row row : sideIronPriceRow) {
        if (row.isNull())
            continue;
        DatabaseRequestHandler::SideIronPriceData data;
        data.handle = handle++;
        if (row[0].isNull()) {
            std::cout << "row 0" << std::endl;
            continue;
        }
        data.id = row[0];
        if (row[1].isNull()) {
            std::cout << "row 0" << std::endl;
            continue;
        }
        data.type = (SideIronType)row[1].get<unsigned>();
        if (row[2].isNull()) {
            std::cout << "row 1" << std::endl;
            continue;
        }
        data.lowerLength = row[2].get<unsigned>();
        if (row[3].isNull()) {
            std::cout << "row 2" << std::endl;
            continue;
        }
        data.upperLength = row[3].get<unsigned>();
        if (row[4].isNull()) {
            std::cout << "row 3" << std::endl;
            continue;
        }
        data.extraflex = (row[4].get<unsigned>()-1);
        if (row[5].isNull()) {
            std::cout << "row 4" << std::endl;
            continue;
        }
        data.price = row[5].get<float>();
        elements.push_back(data);
        sizeValue += sizeof(unsigned) * 4 + sizeof(SideIronType) + sizeof(bool) + sizeof(float);
    }
};

template<>
void DatabaseRequestHandler::serialiseDataElement(const DatabaseRequestHandler::SideIronPriceData& element, void* buffer, unsigned& sizeValue) const {
    unsigned char* buff = (unsigned char*)buffer;

    *((SideIronType*)buff) = element.type;
    buff += sizeof(SideIronType);

    *((unsigned*)buff) = element.lowerLength;
    buff += sizeof(unsigned);

    *((unsigned*)buff) = element.upperLength;
    buff += sizeof(unsigned);

    *buff++ = element.extraflex;

    *((float*)buff) = element.price;
    buff += sizeof(float);

    sizeValue += sizeof(unsigned) * 2 + sizeof(float) + sizeof(SideIronType) + sizeof(bool);
}

template<>
void DatabaseRequestHandler::constructDataElements(
    mysqlx::RowResult& powderCoatingRow, unsigned& handle,
    std::vector<DatabaseRequestHandler::PowderCoatingPriceData>& elements, unsigned& sizeValue
) const {
    for (mysqlx::Row row : powderCoatingRow) {
        PowderCoatingPriceData data;
        data.handle = handle++;
        data.id = row[0];
        sizeValue += sizeof(unsigned) * 2;
        if (!row[1].isNull()) {
            data.hookPrice = row[1].get<float>();
            sizeValue += sizeof(float);
        }
        if (!row[2].isNull()) {
            data.strapPrice = row[2].get<float>();
            sizeValue += sizeof(float);
        }

        elements.push_back(data);
    }
}


template<>
void DatabaseRequestHandler::serialiseDataElement(const DatabaseRequestHandler::PowderCoatingPriceData& element, void* buffer, unsigned& sizeValue) const {
    unsigned char* buff = (unsigned char*)buffer;

    *((float*)buff) = element.hookPrice;
    buff += sizeof(float);
    *((float*)buff) = element.strapPrice;
    buff += sizeof(float);
    sizeValue += sizeof(float) * 2;

}

template<>
RequestType DatabaseRequestHandler::getRequestType<DatabaseRequestHandler::PowderCoatingPriceData>() const {
    return RequestType::SOURCE_POWDER_COATING_TABLE;
}

template<>
RequestType DatabaseRequestHandler::getRequestType<DatabaseRequestHandler::SideIronPriceData>() const {
    return RequestType::SOURCE_SIDE_IRON_PRICES_TABLE;
}

template<>
void DatabaseRequestHandler::constructDataElements(
	mysqlx::RowResult &sideIronRow, unsigned &handle,
	std::vector<DatabaseRequestHandler::SideIronData> &elements, unsigned &sizeValue
) const {
    for (mysqlx::Row row : sideIronRow) {
        SideIronData data;
        data.handle = handle++;
        data.id = row[0];
        if (!row[1].isNull()) {
            data.type = row[1].get<unsigned>();
        }
        else {
            data.type = 0;
        }
        if (!row[2].isNull()) {
            data.length = row[2].get<unsigned>();
        }
        else {
            data.length = 0;
        }
        data.drawingNumber = row[3].get<std::string>();
        data.hyperlink = row[4].get<std::string>();

        if (!row[5].isNull()) {
            data.price = std::make_optional(row[5].get<float>());
        }
        else {
            data.price = std::nullopt;
        }
        if (!row[6].isNull()) {
            data.screws = std::make_optional(row[6].get<unsigned>());
        }
        else {
            data.screws = std::nullopt;
        }

        sizeValue += sizeof(unsigned) * 2 + sizeof(unsigned char) + sizeof(unsigned short) + sizeof(unsigned char) +
            data.drawingNumber.size() + sizeof(unsigned char) + data.hyperlink.size() + sizeof(bool) * 2 + 
            (data.price.has_value() ? sizeof(float) : 0) + (data.screws.has_value() ? sizeof(unsigned) : 0);

        elements.push_back(data);
    }
}

template<>
void DatabaseRequestHandler::serialiseDataElement(const DatabaseRequestHandler::SideIronData &element, void *buffer, unsigned &sizeValue) const {
	unsigned char *buff = (unsigned char *)buffer;

	*buff++ = element.type;
	*((unsigned short *)buff) = element.length;
	buff += sizeof(unsigned short);
	unsigned char nameSize = element.drawingNumber.size();
	*buff++ = nameSize;
	memcpy(buff, element.drawingNumber.c_str(), nameSize);
	buff += nameSize;
	unsigned char hyperlinkSize = element.hyperlink.size();
    *buff++ = hyperlinkSize;
	memcpy(buff, element.hyperlink.c_str(), hyperlinkSize);
    buff += hyperlinkSize;
    *buff++ = element.price.has_value();
    if (element.price.has_value()) {
        *((float*)buff) = *element.price;
        buff += sizeof(float);
    }
    *buff++ = element.screws.has_value();
    if (element.screws.has_value()) {
        *((unsigned*)buff) = *element.screws;
        buff += sizeof(unsigned);
    }

	sizeValue += sizeof(unsigned char) + sizeof(unsigned short) + sizeof(unsigned char) + nameSize + sizeof(unsigned char) + hyperlinkSize
        + sizeof(bool) * 2 + (element.price.has_value() ? sizeof(float) : 0) + (element.screws.has_value() ? sizeof(unsigned) : 0);
}

template<>
RequestType DatabaseRequestHandler::getRequestType<DatabaseRequestHandler::SideIronData>() const {
	return RequestType::SOURCE_SIDE_IRON_TABLE;
}

template<>
void DatabaseRequestHandler::constructDataElements(
    mysqlx::RowResult& machineRow, unsigned& handle,
    std::vector<DatabaseRequestHandler::MachineData>& elements, unsigned& sizeValue
) const {
    for (mysqlx::Row row : machineRow) {
        MachineData data;
        data.handle = handle++;
        data.id = row[0];
        data.manufacturer = row[1].get<std::string>();
        data.model = row[2].get<std::string>();
        sizeValue += sizeof(unsigned) * 2 + sizeof(unsigned char) + data.manufacturer.size() +
            sizeof(unsigned char) + data.model.size();

        elements.push_back(data);
    }
}

template<>
void DatabaseRequestHandler::serialiseDataElement(const DatabaseRequestHandler::MachineData &element, void *buffer, unsigned &sizeValue) const {
	unsigned char *buff = (unsigned char *)buffer;

	unsigned char manufacturerSize = element.manufacturer.size();
	*buff++ = manufacturerSize;
	memcpy(buff, element.manufacturer.c_str(), manufacturerSize);
	buff += manufacturerSize;

	unsigned char modelSize = element.model.size();
	*buff++ = modelSize;
	memcpy(buff, element.model.c_str(), modelSize);

	sizeValue += sizeof(unsigned char) + manufacturerSize + sizeof(unsigned char) + modelSize;
}

template<>
RequestType DatabaseRequestHandler::getRequestType<DatabaseRequestHandler::MachineData>() const {
	return RequestType::SOURCE_MACHINE_TABLE;
}

template<>
void DatabaseRequestHandler::constructDataElements(
    mysqlx::RowResult& machineDeckRow, unsigned& handle,
    std::vector<DatabaseRequestHandler::MachineDeckData>& elements, unsigned& sizeValue
) const {
    for (mysqlx::Row row : machineDeckRow) {
        MachineDeckData data;
        data.handle = handle++;
        data.id = row[0];
        data.deck = row[1].get<std::string>();
        sizeValue += sizeof(unsigned) * 2 + sizeof(unsigned char) + data.deck.size();

        elements.push_back(data);
    }
}

template<>
void DatabaseRequestHandler::serialiseDataElement(const DatabaseRequestHandler::MachineDeckData &element, void *buffer, unsigned &sizeValue) const {
	unsigned char *buff = (unsigned char *)buffer;

	unsigned char deckSize = element.deck.size();
	*buff++ = deckSize;
	memcpy(buff, element.deck.c_str(), deckSize);

	sizeValue += sizeof(unsigned char) + deckSize;
}

template<>
RequestType DatabaseRequestHandler::getRequestType<DatabaseRequestHandler::MachineDeckData>() const {
	return RequestType::SOURCE_MACHINE_DECK_TABLE;
}