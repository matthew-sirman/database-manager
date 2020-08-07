#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
//
// Created by matthew on 09/07/2020.
//

#include "../../include/database/DatabaseRequestHandler.h"

#include <utility>

DatabaseRequestHandler::DatabaseRequestHandler() : schema(0, 0, 0, 0, 0, 0, 0, 0, 0) {

}

void DatabaseRequestHandler::onMessageReceived(Server &caller, const ClientHandle &clientHandle, void *message,
	unsigned int messageSize) {
	switch (getDeserialiseType(message)) {
		case RequestType::REPEAT_TOKEN_REQUEST:
			caller.sendRepeatToken(clientHandle, (unsigned)RequestType::REPEAT_TOKEN_REQUEST);
			break;
		case RequestType::DRAWING_SEARCH_QUERY: {
			std::vector<DrawingSummary> summaries = caller.databaseManager().executeSearchQuery(
				DatabaseSearchQuery::deserialise(message));

			DrawingSummaryCompressionSchema schema = compressionSchema(&caller.databaseManager());

			unsigned char *responseBuffer = (unsigned char *)alloca(sizeof(RequestType) +
				sizeof(DrawingSummaryCompressionSchema) +
				sizeof(unsigned) +
				summaries.size() * schema.maxCompressedSize());

			unsigned index = 0;

			*((RequestType *)responseBuffer) = RequestType::DRAWING_SEARCH_QUERY;
			index += sizeof(RequestType);

			memcpy(responseBuffer + index, &schema, sizeof(DrawingSummaryCompressionSchema));
			index += sizeof(DrawingSummaryCompressionSchema);

			*((unsigned *)(responseBuffer + index)) = summaries.size();
			index += sizeof(unsigned);

			for (const DrawingSummary &summary : summaries) {
				schema.compressSummary(summary, responseBuffer + index);
				index += schema.compressedSize(summary);
			}

			caller.addMessageToSendQueue(clientHandle, responseBuffer, index);

			break;
		}
		case RequestType::DRAWING_INSERT: {
			DrawingInsert drawingInsert = DrawingInsert::deserialise(message);

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
					caller.changelogMessage(clientHandle, "Added drawing " + drawingInsert.drawingData->drawingNumber());
				}

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
				createSourceData<MaterialData>(caller.databaseManager().sourceTable("materials"));
			}

			caller.addMessageToSendQueue(clientHandle, DrawingComponentManager<Material>::rawSourceData(),
				DrawingComponentManager<Material>::rawSourceDataSize());

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
			DrawingRequest request = DrawingRequest::deserialise(message);

			Drawing *returnedDrawing = caller.databaseManager().executeDrawingQuery(request);
			if (returnedDrawing != nullptr) {
				request.drawingData = *returnedDrawing;
			} else {
				request.drawingData = Drawing();
				request.drawingData->setLoadWarning(Drawing::LOAD_FAILED);
			}

			unsigned bufferSize = request.serialisedSize();

			void *responseBuffer = alloca(bufferSize);
			request.serialise(responseBuffer);

			caller.addMessageToSendQueue(clientHandle, responseBuffer, bufferSize);

			break;
		}
		case RequestType::ADD_NEW_COMPONENT: {
			ComponentInsert insert = ComponentInsert::deserialise(message);

			ComponentInsert response;
			response.clearComponentData();

			response.responseCode = caller.databaseManager().insertComponent(insert) ? ComponentInsert::ComponentInsertResponse::SUCCESS
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
				case RequestType::SOURCE_APERTURE_TABLE: {
					if (DrawingComponentManager<ApertureShape>::dirty()) {
						createSourceData<ApertureShapeData>(caller.databaseManager().sourceTable("aperture_shapes"));
					}
					createSourceData<ApertureData>(caller.databaseManager().sourceTable("apertures"));

					sourceData = DrawingComponentManager<Aperture>::rawSourceData();
					sourceDataBufferSize = DrawingComponentManager<Aperture>::rawSourceDataSize();

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
					createSourceData<MaterialData>(caller.databaseManager().sourceTable("materials"));
					sourceData = DrawingComponentManager<Material>::rawSourceData();
					sourceDataBufferSize = DrawingComponentManager<Material>::rawSourceDataSize();

					caller.changelogMessage(clientHandle, "Added a new material");
					break;
				}
				default:
					return;
			}

			caller.broadcastMessage(sourceData, sourceDataBufferSize);

			break;
		}
		case RequestType::GET_NEXT_DRAWING_NUMBER: {
			NextDrawing next = NextDrawing::deserialise(message);

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

			break;
		}
		case RequestType::CREATE_DATABASE_BACKUP: {
			DatabaseBackup backup = DatabaseBackup::deserialise(message);

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
			createSourceData<MaterialData>(dbManager->sourceTable("materials"));
		}

		unsigned maxMatID, maxThicknessHandle, maxApertureHandle;
		float maxWidth, maxLength, maxLapSize;
		unsigned char maxDrawingLength;
		unsigned char maxBarSpacingCount;
		float maxBarSpacing;


		dbManager->getCompressionSchemaDetails(maxMatID, maxWidth, maxLength, maxLapSize, maxBarSpacingCount, maxBarSpacing, maxDrawingLength);
		maxThicknessHandle = DrawingComponentManager<Material>::maximumHandle();
		maxApertureHandle = DrawingComponentManager<Aperture>::maximumHandle();
		
		schema = DrawingSummaryCompressionSchema(maxMatID, maxWidth, maxLength, maxThicknessHandle, 
												 maxLapSize, maxApertureHandle, maxBarSpacingCount, 
												 maxBarSpacing, maxDrawingLength);

		schemaDirty = false;
	}

	return schema;
}

void DatabaseRequestHandler::setCompressionSchemaDirty() {
	schemaDirty = true;
}

template<>
void DatabaseRequestHandler::constructDataElement(
	const mysqlx::Row &productRow, unsigned &handle,
	std::vector<DatabaseRequestHandler::ProductData> &elements, unsigned &sizeValue
) const {
	ProductData data;
	data.handle = handle++;
	data.id = productRow[0];
	data.name = productRow[1].get<std::string>();
	sizeValue += sizeof(unsigned) * 2 + sizeof(unsigned char) + data.name.size();

	elements.push_back(data);
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
void DatabaseRequestHandler::constructDataElement(
	const mysqlx::Row &apertureRow, unsigned &handle,
	std::vector<DatabaseRequestHandler::ApertureData> &elements, unsigned &sizeValue
) const {
	if (apertureRow[6].isNull()) {
		ERROR_RAW_SAFE("Aperture with missing shape ID detected.", std::cerr);
		return;
	}

	if (apertureRow[6].get<unsigned>() == DrawingComponentManager<ApertureShape>::findComponentByID(5).handle()) {
		ApertureData slData, stData;
		slData.handle = handle++;
		slData.id = apertureRow[0];
		slData.width = apertureRow[1].get<float>();
		slData.length = apertureRow[2].get<float>();
		slData.baseWidth = apertureRow[3].get<unsigned>();
		slData.baseLength = apertureRow[4].get<unsigned>();
		slData.quantity = apertureRow[5].get<unsigned>();
		slData.shapeID = DrawingComponentManager<ApertureShape>::findComponentByID(3).handle();

		stData.handle = handle++;
		stData.id = apertureRow[0];
		stData.width = apertureRow[1].get<float>();
		stData.length = apertureRow[2].get<float>();
		stData.baseWidth = apertureRow[3].get<unsigned>();
		stData.baseLength= apertureRow[4].get<unsigned>();
		stData.quantity = apertureRow[5].get<unsigned>();
		stData.shapeID = DrawingComponentManager<ApertureShape>::findComponentByID(4).handle();

		sizeValue += (sizeof(unsigned) * 2 + sizeof(float) * 2 + sizeof(unsigned short) * 3 + sizeof(unsigned)) * 2;

		elements.push_back(slData);
		elements.push_back(stData);
	}
	else {
		ApertureData data;
		data.handle = handle++;
		data.id = apertureRow[0];
		data.width = apertureRow[1].isNull() ? 0 : apertureRow[1].get<float>();
		data.length = apertureRow[2].isNull() ? 0 : apertureRow[2].get<float>();
		data.baseWidth = apertureRow[3].isNull() ? 0 : apertureRow[3].get<unsigned>();
		data.baseLength = apertureRow[4].isNull() ? 0 : apertureRow[4].get<unsigned>();
		data.quantity = apertureRow[5].isNull() ? 0 : apertureRow[5].get<unsigned>();
		data.shapeID = DrawingComponentManager<ApertureShape>::findComponentByID(apertureRow[6]).handle();

		sizeValue += sizeof(unsigned) * 2 + sizeof(float) * 2 + sizeof(unsigned short) * 3 + sizeof(unsigned);

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

	sizeValue += sizeof(float) * 2 + sizeof(unsigned short) * 3 + sizeof(unsigned);
}

template<>
RequestType DatabaseRequestHandler::getRequestType<DatabaseRequestHandler::ApertureData>() const {
	return RequestType::SOURCE_APERTURE_TABLE;
}

template<>
void DatabaseRequestHandler::constructDataElement(
	const mysqlx::Row &apertureShapeRow, unsigned &handle,
	std::vector<DatabaseRequestHandler::ApertureShapeData> &elements, unsigned &sizeValue
) const {
	ApertureShapeData data;
	data.handle = handle++;
	data.id = apertureShapeRow[0];
	data.shape = apertureShapeRow[1].get<std::string>();
	sizeValue += sizeof(unsigned) * 2 + sizeof(unsigned char) + data.shape.size();

	elements.push_back(data);
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
void DatabaseRequestHandler::constructDataElement(
	const mysqlx::Row &materialRow, unsigned &handle,
	std::vector<DatabaseRequestHandler::MaterialData> &elements, unsigned &sizeValue
) const {
	MaterialData data;
	data.handle = handle++;
	data.id = materialRow[0];
	data.name = materialRow[1].get<std::string>();
	data.hardness = materialRow[2].get<unsigned>();
	data.thickness = materialRow[3].get<unsigned>();
	sizeValue += sizeof(unsigned) * 2 + sizeof(unsigned short) * 2 + sizeof(unsigned char) + data.name.size();

	elements.push_back(data);
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

	sizeValue += sizeof(unsigned short) * 2 + sizeof(unsigned char) + nameSize;
}

template<>
RequestType DatabaseRequestHandler::getRequestType<DatabaseRequestHandler::MaterialData>() const {
	return RequestType::SOURCE_MATERIAL_TABLE;
}

template<>
void DatabaseRequestHandler::constructDataElement(
	const mysqlx::Row &sideIronRow, unsigned &handle,
	std::vector<DatabaseRequestHandler::SideIronData> &elements, unsigned &sizeValue
) const {
	SideIronData data;
	data.handle = handle++;
	data.id = sideIronRow[0];
	if (!sideIronRow[1].isNull()) {
		data.type = sideIronRow[1].get<unsigned>();
	}
	else {
		data.type = 0;
	}
	if (!sideIronRow[2].isNull()) {
		data.length = sideIronRow[2].get<unsigned>();
	}
	else {
		data.length = 0;
	}
	data.drawingNumber = sideIronRow[3].get<std::string>();
	data.hyperlink = sideIronRow[4].get<std::string>();
	sizeValue += sizeof(unsigned) * 2 + sizeof(unsigned char) + sizeof(unsigned short) + sizeof(unsigned char) +
		data.drawingNumber.size() + sizeof(unsigned char) + data.hyperlink.size();

	elements.push_back(data);
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

	sizeValue += sizeof(unsigned char) + sizeof(unsigned short) + sizeof(unsigned char) + nameSize + sizeof(unsigned char) + hyperlinkSize;
}

template<>
RequestType DatabaseRequestHandler::getRequestType<DatabaseRequestHandler::SideIronData>() const {
	return RequestType::SOURCE_SIDE_IRON_TABLE;
}

template<>
void DatabaseRequestHandler::constructDataElement(
	const mysqlx::Row &machineRow, unsigned &handle,
	std::vector<DatabaseRequestHandler::MachineData> &elements, unsigned &sizeValue
) const {
	MachineData data;
	data.handle = handle++;
	data.id = machineRow[0];
	data.manufacturer = machineRow[1].get<std::string>();
	data.model = machineRow[2].get<std::string>();
	sizeValue += sizeof(unsigned) * 2 + sizeof(unsigned char) + data.manufacturer.size() +
		sizeof(unsigned char) + data.model.size();

	elements.push_back(data);
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
void DatabaseRequestHandler::constructDataElement(
	const mysqlx::Row &machineDeckRow, unsigned &handle,
	std::vector<DatabaseRequestHandler::MachineDeckData> &elements, unsigned &sizeValue
) const {
	MachineDeckData data;
	data.handle = handle++;
	data.id = machineDeckRow[0];
	data.deck = machineDeckRow[1].get<std::string>();
	sizeValue += sizeof(unsigned) * 2 + sizeof(unsigned char) + data.deck.size();

	elements.push_back(data);
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

#pragma clang diagnostic pop