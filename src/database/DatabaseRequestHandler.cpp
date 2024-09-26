//
// Created by matthew on 09/07/2020.
//

#include "../../include/database/DatabaseRequestHandler.h"

#include <utility>

DatabaseRequestHandler::DatabaseRequestHandler()
    : schema(0, 0, 0, 0, 0, 0, 0, 0, 0, 0) {
  pricingMap.insert({"running_m", MaterialPricingType::RUNNING_M});
  pricingMap.insert({"square_m", MaterialPricingType::SQUARE_M});
  pricingMap.insert({"sheet", MaterialPricingType::SHEET});
}

void DatabaseRequestHandler::onMessageReceived(Server &caller,
                                               const ClientHandle &clientHandle,
                                               void *&&message,
                                               unsigned int messageSize) {
  Logger::log("Message Type: " +
              std::to_string(static_cast<int>(getDeserialiseType(message))));
  switch (getDeserialiseType(message)) {
    case RequestType::REPEAT_TOKEN_REQUEST:
      caller.sendRepeatToken(clientHandle,
                             (unsigned)RequestType::REPEAT_TOKEN_REQUEST);
      break;
    case RequestType::USER_EMAIL_REQUEST:
      caller.sendEmailAddress(clientHandle,
                              (unsigned)RequestType::USER_EMAIL_REQUEST);
      break;
    case RequestType::DRAWING_SEARCH_QUERY: {
      DatabaseSearchQuery &query =
          DatabaseSearchQuery::deserialise(std::move(message));
      std::vector<DrawingSummary> summaries =
          caller.databaseManager().executeSearchQuery(query);
      delete &query;

      DrawingSummaryCompressionSchema summaryCompressionSchema =
          compressionSchema(&caller.databaseManager());

      unsigned char *responseBuffer = (unsigned char *)alloca(
          sizeof(RequestType) + sizeof(DrawingSummaryCompressionSchema) +
          sizeof(unsigned) +
          summaries.size() * summaryCompressionSchema.maxCompressedSize());

      unsigned index = 0;

      *((RequestType *)responseBuffer) = RequestType::DRAWING_SEARCH_QUERY;
      index += sizeof(RequestType);

      memcpy(responseBuffer + index, &summaryCompressionSchema,
             sizeof(DrawingSummaryCompressionSchema));
      index += sizeof(DrawingSummaryCompressionSchema);

      *((unsigned *)(responseBuffer + index)) = summaries.size();
      index += sizeof(unsigned);

      for (const DrawingSummary &summary : summaries) {
        summaryCompressionSchema.compressSummary(summary,
                                                 responseBuffer + index);
        index += summaryCompressionSchema.compressedSize(summary);
      }

      caller.addMessageToSendQueue(clientHandle, responseBuffer, index);

      break;
    }
    case RequestType::DRAWING_INSERT: {
      DrawingInsert &drawingInsert =
          DrawingInsert::deserialise(std::move(message));

      if (drawingInsert.drawingData.has_value()) {
        DrawingInsert response;

        response.responseEchoCode = drawingInsert.responseEchoCode;

        switch (caller.databaseManager().drawingExists(
            drawingInsert.drawingData->drawingNumber())) {
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
          caller.changelogMessage(
              clientHandle,
              "Added drawing " + drawingInsert.drawingData->drawingNumber());
        }

        delete &drawingInsert;

        unsigned responseSize = response.serialisedSize();
        void *responseBuffer = alloca(responseSize);
        response.serialise(responseBuffer);

        caller.addMessageToSendQueue(clientHandle, responseBuffer,
                                     responseSize);

        NextDrawing automatic, manual;
        automatic.drawingType = NextDrawing::DrawingType::AUTOMATIC;
        manual.drawingType = NextDrawing::DrawingType::MANUAL;

        automatic.drawingNumber =
            caller.databaseManager().nextAutomaticDrawingNumber();
        manual.drawingNumber =
            caller.databaseManager().nextManualDrawingNumber();

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
        createSourceData<ProductData>(
            caller.databaseManager().sourceTable("products"));
      }

      caller.addMessageToSendQueue(
          clientHandle, DrawingComponentManager<Product>::rawSourceData(),
          DrawingComponentManager<Product>::rawSourceDataSize());

      break;
    }
    case RequestType::SOURCE_BACKING_STRIPS_TABLE: {
      if (DrawingComponentManager<BackingStrip>::dirty()) {
        if (DrawingComponentManager<Material>::dirty()) {
          createSourceData<MaterialData>(
              caller.databaseManager().sourceMultipleTable(
                  "material_prices", "materials", "material_id"));
        }

        createSourceData<BackingStripData>(
            caller.databaseManager().sourceTable("backing_strips"));
      }

      caller.addMessageToSendQueue(
          clientHandle, DrawingComponentManager<BackingStrip>::rawSourceData(),
          DrawingComponentManager<BackingStrip>::rawSourceDataSize());

      break;
    }
    case RequestType::SOURCE_APERTURE_TABLE: {
      if (DrawingComponentManager<Aperture>::dirty()) {
        if (DrawingComponentManager<ApertureShape>::dirty()) {
          createSourceData<ApertureShapeData>(
              caller.databaseManager().sourceTable("aperture_shapes"));
        }

        createSourceData<ApertureData>(
            caller.databaseManager().sourceTable("apertures"));
      }

      caller.addMessageToSendQueue(
          clientHandle, DrawingComponentManager<Aperture>::rawSourceData(),
          DrawingComponentManager<Aperture>::rawSourceDataSize());

      break;
    }
    case RequestType::SOURCE_STRAPS_TABLE: {
      if (DrawingComponentManager<Strap>::dirty()) {
        if (DrawingComponentManager<Material>::dirty()) {
          createSourceData<MaterialData>(
              caller.databaseManager().sourceTable("materials"));
        }
        createSourceData<StrapData>(
            caller.databaseManager().sourceTable("straps"));
      }

      caller.addMessageToSendQueue(
          clientHandle, DrawingComponentManager<Strap>::rawSourceData(),
          DrawingComponentManager<Strap>::rawSourceDataSize());
      break;
    }
    case RequestType::SOURCE_APERTURE_SHAPE_TABLE: {
      if (DrawingComponentManager<ApertureShape>::dirty()) {
        createSourceData<ApertureShapeData>(
            caller.databaseManager().sourceTable("aperture_shapes"));
      }

      caller.addMessageToSendQueue(
          clientHandle, DrawingComponentManager<ApertureShape>::rawSourceData(),
          DrawingComponentManager<ApertureShape>::rawSourceDataSize());

      break;
    }
    case RequestType::SOURCE_MATERIAL_TABLE: {
      if (DrawingComponentManager<Material>::dirty()) {
        createSourceData<MaterialData>(
            caller.databaseManager().sourceMultipleTable(
                "material_prices", "materials", "material_id"));
      }

      caller.addMessageToSendQueue(
          clientHandle, DrawingComponentManager<Material>::rawSourceData(),
          DrawingComponentManager<Material>::rawSourceDataSize());

      break;
    }
    case RequestType::SOURCE_EXTRA_PRICES_TABLE: {
      if (DrawingComponentManager<ExtraPrice>::dirty()) {
        createSourceData<ExtraPriceData>(
            caller.databaseManager().sourceTable("extra_prices"));
      }
      caller.addMessageToSendQueue(
          clientHandle, DrawingComponentManager<ExtraPrice>::rawSourceData(),
          DrawingComponentManager<ExtraPrice>::rawSourceDataSize());
      break;
    }
    case RequestType::SOURCE_LABOUR_TIMES_TABLE: {
      if (DrawingComponentManager<LabourTime>::dirty()) {
        createSourceData<LabourTimeData>(
            caller.databaseManager().sourceTable("labour_times"));
      }
      caller.addMessageToSendQueue(
          clientHandle, DrawingComponentManager<LabourTime>::rawSourceData(),
          DrawingComponentManager<LabourTime>::rawSourceDataSize());
      break;
    }
    case RequestType::SOURCE_POWDER_COATING_TABLE: {
      if (DrawingComponentManager<PowderCoatingPrice>::dirty()) {
        createSourceData<PowderCoatingPriceData>(
            caller.databaseManager().sourceTable("powder_coating_prices"));
      }
      caller.addMessageToSendQueue(
          clientHandle,
          DrawingComponentManager<PowderCoatingPrice>::rawSourceData(),
          DrawingComponentManager<PowderCoatingPrice>::rawSourceDataSize());
      break;
    }
    case RequestType::SOURCE_SIDE_IRON_PRICES_TABLE: {
      if (DrawingComponentManager<SideIronPrice>::dirty()) {
        createSourceData<SideIronPriceData>(
            caller.databaseManager().sourceTable("side_iron_prices"));
      }

      caller.addMessageToSendQueue(
          clientHandle, DrawingComponentManager<SideIronPrice>::rawSourceData(),
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

        createSourceData<SideIronData>(
            caller.databaseManager().sourceTable("side_irons", orderBy.str()));
      }

      caller.addMessageToSendQueue(
          clientHandle, DrawingComponentManager<SideIron>::rawSourceData(),
          DrawingComponentManager<SideIron>::rawSourceDataSize());

      break;
    }
    case RequestType::SOURCE_MACHINE_TABLE: {
      if (DrawingComponentManager<Machine>::dirty()) {
        createSourceData<MachineData>(caller.databaseManager().sourceTable(
            "machines", "manufacturer<>'None', manufacturer, model"));
      }

      caller.addMessageToSendQueue(
          clientHandle, DrawingComponentManager<Machine>::rawSourceData(),
          DrawingComponentManager<Machine>::rawSourceDataSize());

      break;
    }
    case RequestType::SOURCE_MACHINE_DECK_TABLE: {
      if (DrawingComponentManager<MachineDeck>::dirty()) {
        createSourceData<MachineDeckData>(
            caller.databaseManager().sourceTable("machine_decks"));
      }

      caller.addMessageToSendQueue(
          clientHandle, DrawingComponentManager<MachineDeck>::rawSourceData(),
          DrawingComponentManager<MachineDeck>::rawSourceDataSize());

      break;
    }
    case RequestType::DRAWING_DETAILS: {
      DrawingRequest request = DrawingRequest::deserialise(std::move(message));

      Drawing *returnedDrawing =
          caller.databaseManager().executeDrawingQuery(request);
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
      ComponentInsert &insert =
          ComponentInsert::deserialise(std::move(message));

      ComponentInsert response;
      response.clearComponentData();

      response.responseCode =
          caller.databaseManager().insertComponent(insert)
              ? ComponentInsert::ComponentInsertResponse::SUCCESS
              : ComponentInsert::ComponentInsertResponse::FAILED;

      unsigned bufferSize = response.serialisedSize();
      void *responseBuffer = alloca(bufferSize);
      response.serialise(responseBuffer);

      caller.addMessageToSendQueue(clientHandle, responseBuffer, bufferSize);

      if (response.responseCode !=
          ComponentInsert::ComponentInsertResponse::SUCCESS) {
        break;
      }

      void *sourceData;
      unsigned sourceDataBufferSize;

      switch (insert.getSourceTableCode()) {
        case RequestType::SOURCE_APERTURE_TABLE: {
          if (DrawingComponentManager<ApertureShape>::dirty()) {
            createSourceData<ApertureShapeData>(
                caller.databaseManager().sourceTable("aperture_shapes"));
          }
          createSourceData<ApertureData>(
              caller.databaseManager().sourceTable("apertures"));

          sourceData = DrawingComponentManager<Aperture>::rawSourceData();
          sourceDataBufferSize =
              DrawingComponentManager<Aperture>::rawSourceDataSize();

          caller.changelogMessage(clientHandle, "Added a new aperture");
          break;
        }
        case RequestType::SOURCE_BACKING_STRIPS_TABLE: {
          createSourceData<BackingStripData>(
              caller.databaseManager().sourceTable("apertures"));

          sourceData = DrawingComponentManager<BackingStrip>::rawSourceData();
          sourceDataBufferSize =
              DrawingComponentManager<BackingStrip>::rawSourceDataSize();

          caller.changelogMessage(clientHandle, "Added a new aperture");
          break;
        }
        case RequestType::SOURCE_MACHINE_TABLE: {
          createSourceData<MachineData>(caller.databaseManager().sourceTable(
              "machines", "manufacturer<>'None', manufacturer, model"));

          sourceData = DrawingComponentManager<Machine>::rawSourceData();
          sourceDataBufferSize =
              DrawingComponentManager<Machine>::rawSourceDataSize();

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

          createSourceData<SideIronData>(caller.databaseManager().sourceTable(
              "side_irons", orderBy.str()));

          sourceData = DrawingComponentManager<SideIron>::rawSourceData();
          sourceDataBufferSize =
              DrawingComponentManager<SideIron>::rawSourceDataSize();

          caller.changelogMessage(clientHandle, "Added a new side iron");
          break;
        }
        case RequestType::SOURCE_MATERIAL_TABLE: {
          createSourceData<MaterialData>(
              caller.databaseManager().sourceMultipleTable(
                  "material_prices", "materials", "material_id"));
          sourceData = DrawingComponentManager<Material>::rawSourceData();
          sourceDataBufferSize =
              DrawingComponentManager<Material>::rawSourceDataSize();

          caller.changelogMessage(clientHandle, "Added a new material");
          break;
        }
        case RequestType::SOURCE_EXTRA_PRICES_TABLE: {
          createSourceData<ExtraPriceData>(
              caller.databaseManager().sourceTable("extra_prices"));
          sourceData = DrawingComponentManager<ExtraPrice>::rawSourceData();
          sourceDataBufferSize =
              DrawingComponentManager<ExtraPrice>::rawSourceDataSize();

          caller.changelogMessage(clientHandle, "Added a new Extra Price");
          break;
        }
        case RequestType::SOURCE_LABOUR_TIMES_TABLE: {
          createSourceData<LabourTimeData>(
              caller.databaseManager().sourceTable("labour_times"));
          sourceData = DrawingComponentManager<LabourTime>::rawSourceData();
          sourceDataBufferSize =
              DrawingComponentManager<LabourTime>::rawSourceDataSize();
          break;
        }
        case RequestType::SOURCE_POWDER_COATING_TABLE: {
          createSourceData<PowderCoatingPriceData>(
              caller.databaseManager().sourceTable("powder_coating_prices"));
          sourceData =
              DrawingComponentManager<PowderCoatingPrice>::rawSourceData();
          sourceDataBufferSize =
              DrawingComponentManager<PowderCoatingPrice>::rawSourceDataSize();
          break;
        }
        case RequestType::SOURCE_SIDE_IRON_PRICES_TABLE: {
          createSourceData<SideIronPriceData>(
              caller.databaseManager().sourceMultipleTable(
                  "side_iron_prices", "side_iron_types",
                  std::tuple<std::string, std::string>("type",
                                                       "side_iron_type_id")));
          sourceData = DrawingComponentManager<SideIronPrice>::rawSourceData();
          sourceDataBufferSize =
              DrawingComponentManager<SideIronPrice>::rawSourceDataSize();

          caller.changelogMessage(clientHandle, "Added a new material");
          break;
        }
        case RequestType::SOURCE_STRAPS_TABLE: {
          if (DrawingComponentManager<Material>::dirty()) {
            createSourceData<MaterialData>(
                caller.databaseManager().sourceTable("materials"));
          }
          createSourceData<StrapData>(
              caller.databaseManager().sourceTable("straps"));
          sourceData = DrawingComponentManager<Strap>::rawSourceData();
          sourceDataBufferSize =
              DrawingComponentManager<Strap>::rawSourceDataSize();

          caller.addMessageToSendQueue(
              clientHandle, DrawingComponentManager<Strap>::rawSourceData(),
              DrawingComponentManager<Strap>::rawSourceDataSize());
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
          next.drawingNumber =
              caller.databaseManager().nextAutomaticDrawingNumber();
          break;
        case NextDrawing::DrawingType::MANUAL:
          next.drawingNumber =
              caller.databaseManager().nextManualDrawingNumber();
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

DrawingSummaryCompressionSchema DatabaseRequestHandler::compressionSchema(
    DatabaseManager *dbManager) {
  if (schemaDirty) {
    if (!dbManager) {
      STD_ERROR("Database manager not set up. No connection to database.");
    }

    if (DrawingComponentManager<Aperture>::dirty()) {
      if (DrawingComponentManager<ApertureShape>::dirty()) {
        createSourceData<ApertureShapeData>(
            dbManager->sourceTable("aperture_shapes"));
      }

      createSourceData<ApertureData>(dbManager->sourceTable("apertures"));
    }

    if (DrawingComponentManager<Material>::dirty()) {
      createSourceData<MaterialData>(dbManager->sourceMultipleTable(
          "material_prices", "materials", "material_id"));
    }

    if (DrawingComponentManager<SideIronPrice>::dirty()) {
      createSourceData<SideIronPriceData>(
          dbManager->sourceTable("side_iron_prices"));
    }

    unsigned maxMatID, maxThicknessHandle, maxApertureHandle;
    float maxWidth, maxLength, maxLapSize;
    unsigned char maxDrawingLength;
    unsigned char maxBarSpacingCount;
    float maxBarSpacing;
    unsigned char maxExtraApertureCount;

    dbManager->getCompressionSchemaDetails(
        maxMatID, maxWidth, maxLength, maxLapSize, maxBarSpacingCount,
        maxBarSpacing, maxDrawingLength, maxExtraApertureCount);
    maxThicknessHandle = DrawingComponentManager<Material>::maximumHandle();
    maxApertureHandle = DrawingComponentManager<Aperture>::maximumHandle();

    schema = DrawingSummaryCompressionSchema(
        maxMatID, maxWidth, maxLength, maxThicknessHandle, maxLapSize,
        maxApertureHandle, maxBarSpacingCount, maxBarSpacing, maxDrawingLength,
        maxExtraApertureCount);

    schemaDirty = false;
  }

  return schema;
}

void DatabaseRequestHandler::setCompressionSchemaDirty() { schemaDirty = true; }

/// <summary>
/// Constructs DatabaseRequestHandler::ProductData from all rows recieved from
/// the database.
/// </summary>
/// <param name="productRow">The rows recieved from a database query.</param>
/// <param name="handle">The biggest handle of products currently, to increment
/// for each products.</param>
/// <param name="elements">A reference to a vector to
/// store the newly created elements in.</param>
/// <param name="sizeValue">A
/// reference to an unsigned integer to store the amount of bytes will be needed
/// to serialise the elements.</param>
template <>
void DatabaseRequestHandler::constructDataElements(
    mysqlx::RowResult &productRow, unsigned &handle,
    std::vector<DatabaseRequestHandler::ProductData> &elements,
    unsigned &sizeValue) const {
  for (mysqlx::Row row : productRow.fetchAll()) {
    if (!row.isNull()) {
      ProductData data;
      data.handle = handle++;
      data.id = row.get(0).get<unsigned int>();
      data.name = row.get(1).get<std::string>();
      sizeValue +=
          sizeof(unsigned) * 2 + sizeof(unsigned char) + data.name.size();

      elements.push_back(data);
    }
  }
}

/// <summary>
/// Serialises a singular DatabaseRequestHandler::ProductData to the provided
/// buffer, and increments the sizeValue.
/// </summary>
/// <param name="element">A reference to the element to serialise.</param>
/// <param name="buffer">The buffer to serialise the product to.</param>
template <>
void DatabaseRequestHandler::serialiseDataElement(
    const DatabaseRequestHandler::ProductData &element,
    unsigned char **buffer) const {
  unsigned char nameSize = element.name.size();
  *(*buffer)++ = nameSize;
  memcpy(*buffer, element.name.c_str(), nameSize);
  *buffer += nameSize;
}

template <>
RequestType DatabaseRequestHandler::getRequestType<
    DatabaseRequestHandler::ProductData>() const {
  return RequestType::SOURCE_PRODUCT_TABLE;
}

/// <summary>
/// Constructs DatabaseRequestHandler::ApertureData from all rows recieved from
/// the database.
/// </summary>
/// <param name="apertureRow">The rows recieved from a database query.</param>
/// <param name="handle">The biggest handle of Aperture currently, to increment
/// for each Aperture.</param>
/// <param name="elements">A reference to a vector to
/// store the newly created elements in.</param>
/// <param name="sizeValue">A
/// reference to an unsigned integer to store the amount of bytes will be needed
/// to serialise the elements.</param>
template <>
void DatabaseRequestHandler::constructDataElements(
    mysqlx::RowResult &apertureRow, unsigned &handle,
    std::vector<DatabaseRequestHandler::ApertureData> &elements,
    unsigned &sizeValue) const {
  for (mysqlx::Row row : apertureRow.fetchAll()) {
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
    data.shapeID =
        DrawingComponentManager<ApertureShape>::findComponentByID(row[6])
            .handle();
    data.isNibble = row[7].get<bool>();
    if (data.isNibble) {
      data.nibbleApertureId = row[8].get<unsigned>();
    }

    sizeValue += sizeof(unsigned) * 2 + sizeof(float) * 2 +
                 sizeof(unsigned short) * 3 + sizeof(unsigned) + sizeof(bool) +
                 (data.isNibble ? sizeof(unsigned) : 0);

    elements.push_back(data);
  }
}

/// <summary>
/// Serialises a singular DatabaseRequestHandler::ApertureData to the provided
/// buffer, and increments the sizeValue.
/// </summary>
/// <param name="element">A reference to the element to serialise.</param>
/// <param name="buffer">The buffer to serialise the aperture to.</param>
template <>
void DatabaseRequestHandler::serialiseDataElement(
    const DatabaseRequestHandler::ApertureData &element,
    unsigned char **buffer) const {
  *((float *)*buffer) = element.width;
  *buffer += sizeof(float);
  *((float *)*buffer) = element.length;
  *buffer += sizeof(float);
  *((unsigned short *)*buffer) = element.baseWidth;
  *buffer += sizeof(unsigned short);
  *((unsigned short *)*buffer) = element.baseLength;
  *buffer += sizeof(unsigned short);
  *((unsigned *)*buffer) = element.shapeID;
  *buffer += sizeof(unsigned);
  *((unsigned short *)*buffer) = element.quantity;
  *buffer += sizeof(unsigned short);
  *(*buffer)++ = element.isNibble;
  if (element.isNibble) {
    *((unsigned *)*buffer) = element.nibbleApertureId;
    *buffer += sizeof(unsigned);
  }
}

template <>
RequestType DatabaseRequestHandler::getRequestType<
    DatabaseRequestHandler::ApertureData>() const {
  return RequestType::SOURCE_APERTURE_TABLE;
}

/// <summary>
/// Constructs DatabaseRequestHandler::BackingStripData from all rows recieved
/// from the database.
/// </summary>
/// <param name="stripRow">The rows recieved from a database query.</param>
/// <param name="handle">The biggest handle of backing strip currently, to
/// increment for each backing strip.</param> <param name="elements">A reference
/// to a vector to store the newly created elements in.</param> <param
/// name="sizeValue">A reference to an unsigned integer to store the amount of
/// bytes will be needed to serialise the elements.</param>
template <>
void DatabaseRequestHandler::constructDataElements(
    mysqlx::RowResult &stripRow, unsigned &handle,
    std::vector<DatabaseRequestHandler::BackingStripData> &elements,
    unsigned &sizeValue) const {
  for (mysqlx::Row row : stripRow.fetchAll()) {
    if (row.isNull()) {
      std::cout << "Null" << std::endl;
      continue;
    }
    BackingStripData strip;
    strip.handle = handle++;
    strip.id = row[0];
    strip.materialID = row[1];

    sizeValue += sizeof(unsigned) * 3;

    elements.push_back(strip);
  }
}

/// <summary>
/// Serialises a singular DatabaseRequestHandler::BackingStripData to the
/// provided buffer, and increments the sizeValue.
/// </summary>
/// <param name="element">A reference to the element to serialise.</param>
/// <param name="buffer">The buffer to serialise the backing strip to.</param>
template <>
void DatabaseRequestHandler::serialiseDataElement(
    const DatabaseRequestHandler::BackingStripData &element,
    unsigned char **buffer) const {
  *((unsigned *)*buffer) = element.materialID;
  *buffer += sizeof(unsigned);
}

template <>
RequestType DatabaseRequestHandler::getRequestType<
    DatabaseRequestHandler::BackingStripData>() const {
  return RequestType::SOURCE_BACKING_STRIPS_TABLE;
}

/// <summary>
/// Constructs DatabaseRequestHandler::ExtraPriceData from all rows recieved
/// from the database.
/// </summary>
/// <param name="extraPriceRow">The rows recieved from a database query.</param>
/// <param name="handle">The biggest handle of extra prices currently, to
/// increment for each extra price.</param> <param name="elements">A reference
/// to a vector to store the newly created elements in.</param> <param
/// name="sizeValue">A reference to an unsigned integer to store the amount of
/// bytes will be needed to serialise the elements.</param>
template <>
void DatabaseRequestHandler::constructDataElements(
    mysqlx::RowResult &extraPriceRow, unsigned &handle,
    std::vector<DatabaseRequestHandler::ExtraPriceData> &elements,
    unsigned &sizeValue) const {
  ExtraPriceData data;
  for (mysqlx::Row row : extraPriceRow.fetchAll()) {
    if (row.isNull()) {
      std::cout << "Null" << std::endl;
      continue;
    }
    data = *(new ExtraPriceData);

    data.handle = handle++;
    data.id = row[0].get<unsigned int>();
    data.type = ExtraPrice::getType.at(row[1].get<std::string>());
    data.price = row[2].get<float>();
    if (row[3].isNull())
      data.amount = std::nullopt;
    else
      data.amount = std::make_optional(row[3].get<unsigned>());
    if (row[4].isNull())
      data.squareMetres = std::nullopt;
    else
      data.squareMetres = std::make_optional(row[4].get<float>());

    // if (row[1].get<std::string>() == "side_iron_nuts") {
    //   data.type = ExtraPriceType::SIDE_IRON_NUTS;
    //   data.amount = row[3].get<unsigned>();
    //   data.squareMetres = std::nullopt;
    //   sizeValue += sizeof(unsigned);
    // } else if (row[1].get<std::string>() == "side_iron_screws") {
    //   data.type = ExtraPriceType::SIDE_IRON_SCREWS;
    //   data.amount = row[3].get<unsigned>();
    //   data.squareMetres = std::nullopt;
    //   sizeValue += sizeof(unsigned);
    // } else if (row[1].get<std::string>() == "glue") {
    //   data.type = ExtraPriceType::TACKYBACK_GLUE;
    //   data.amount = std::nullopt;
    //   data.squareMetres = row[4].get<float>();
    //   sizeValue += sizeof(float);
    // } else if (row[1].get<std::string>() == "labour") {
    //   data.type = ExtraPriceType::LABOUR;
    //   data.amount = std::nullopt;
    //   data.squareMetres = std::nullopt;
    // } else if (row[1].get<std::string>() == "primer") {
    //   data.type = ExtraPriceType::PRIMER;
    //   data.amount = std::nullopt;
    //   data.squareMetres = row[4].get<float>();
    //   sizeValue += sizeof(float);
    // }
    // sizeValue += sizeof(ExtraPriceType) + sizeof(float) + sizeof(unsigned) *
    // 2;
    sizeValue += sizeof(ExtraPriceType) + sizeof(unsigned) + sizeof(float) +
                 (data.amount.has_value() ? sizeof(unsigned) : 0) +
                 (data.squareMetres.has_value() ? sizeof(float) : 0);

    elements.push_back(data);
  }
}

/// <summary>
/// Serialises a singular DatabaseRequestHandler::ExtraPriceData to the provided
/// buffer, and increments the sizeValue.
/// </summary>
/// <param name="element">A reference to the element to serialise.</param>
/// <param name="buffer">The buffer to serialise the extra price to.</param>
template <>
void DatabaseRequestHandler::serialiseDataElement(
    const DatabaseRequestHandler::ExtraPriceData &element,
    unsigned char **buffer) const {
  *((ExtraPriceType *)*buffer) = element.type;
  *buffer += sizeof(ExtraPriceType);

  *((float *)*buffer) = element.price;
  *buffer += sizeof(float);

  *(*buffer)++ = element.amount.has_value();
  if (element.amount.has_value()) {
    *((unsigned *)*buffer) = element.amount.value();
    *buffer += sizeof(unsigned);
  }
  *(*buffer)++ = element.squareMetres.has_value();
  if (element.squareMetres.has_value()) {
    *((float *)*buffer) = element.squareMetres.value();
    *buffer += sizeof(float);
  }
}

template <>
RequestType DatabaseRequestHandler::getRequestType<
    DatabaseRequestHandler::ExtraPriceData>() const {
  return RequestType::SOURCE_EXTRA_PRICES_TABLE;
}

/// <summary>
/// Constructs DatabaseRequestHandler::LabourTimeData from all rows recieved
/// from the database.
/// </summary>
/// <param name="labourTimeRow">The rows recieved from a database query.</param>
/// <param name="handle">The biggest handle of labour times currently, to
/// increment for each labour time.</param> <param name="elements">A reference
/// to a vector to store the newly created elements in.</param> <param
/// name="sizeValue">A reference to an unsigned integer to store the amount of
/// bytes will be needed to serialise the elements.</param>
template <>
void DatabaseRequestHandler::constructDataElements(
    mysqlx::RowResult &labourTimeRow, unsigned &handle,
    std::vector<DatabaseRequestHandler::LabourTimeData> &elements,
    unsigned &sizeValue) const {
  for (mysqlx::Row row : labourTimeRow.fetchAll()) {
    if (row.isNull()) {
      std::cout << "Null" << std::endl;
      continue;
    }
    // for (mysqlx::Row row : labourTimeRow) {
    LabourTimeData data;

    data.handle = handle++;
    data.id = row.get(0).get<unsigned>();
    data.job = row.get(1).get<std::string>();
    data.time = row[2].get<unsigned>();
    sizeValue +=
        sizeof(unsigned) * 3 + sizeof(data.job.size()) + data.job.size();

    elements.push_back(data);
  }
}

/// <summary>
/// Serialises a singular DatabaseRequestHandler::LabourTimeData to the provided
/// buffer, and increments the sizeValue.
/// </summary>
/// <param name="element">A reference to the element to serialise.</param>
/// <param name="buffer">The buffer to serialise the labour time to.</param>
template <>
void DatabaseRequestHandler::serialiseDataElement(
    const DatabaseRequestHandler::LabourTimeData &element,
    unsigned char **buffer) const {
  *((size_t *)*buffer) = element.job.size();
  *buffer += sizeof(size_t);
  std::memcpy(*buffer, element.job.c_str(), element.job.size());
  *buffer += element.job.size();

  *((unsigned *)*buffer) = element.time;
  *buffer += sizeof(unsigned);
}

template <>
RequestType DatabaseRequestHandler::getRequestType<
    DatabaseRequestHandler::LabourTimeData>() const {
  return RequestType::SOURCE_LABOUR_TIMES_TABLE;
}

/// <summary>
/// Constructs DatabaseRequestHandler::ApertureShapeData from all rows recieved
/// from the database.
/// </summary>
/// <param name="apertureShapeRow">The rows recieved from a database
/// query.</param> <param name="handle">The biggest handle of aperture shapes
/// currently, to increment for each aperture shape.</param> <param
/// name="elements">A reference to a vector to store the newly created elements
/// in.</param> <param name="sizeValue">A reference to an unsigned integer to
/// store the amount of bytes will be needed to serialise the elements.</param>
template <>
void DatabaseRequestHandler::constructDataElements(
    mysqlx::RowResult &apertureShapeRow, unsigned &handle,
    std::vector<DatabaseRequestHandler::ApertureShapeData> &elements,
    unsigned &sizeValue) const {
  for (mysqlx::Row row : apertureShapeRow.fetchAll()) {
    if (row.isNull()) {
      std::cout << "Null" << std::endl;
      continue;
    }
    if (!row[0].isNull() && !row[1].isNull()) {
      ApertureShapeData data;
      data.handle = handle++;
      data.id = row[0];
      data.shape = row[1].get<std::string>();
      sizeValue +=
          sizeof(unsigned) * 2 + sizeof(unsigned char) + data.shape.size();

      elements.push_back(data);
    }
  }
}

/// <summary>
/// Serialises a singular DatabaseRequestHandler::ApertureShapeData to the
/// provided buffer, and increments the sizeValue.
/// </summary>
/// <param name="element">A reference to the element to serialise.</param>
/// <param name="buffer">The buffer to serialise the aperture shape to.</param>
template <>
void DatabaseRequestHandler::serialiseDataElement(
    const DatabaseRequestHandler::ApertureShapeData &element,
    unsigned char **buffer) const {
  unsigned char shapeSize = element.shape.size();
  *(*buffer)++ = shapeSize;
  memcpy(*buffer, element.shape.c_str(), shapeSize);
  *buffer += shapeSize;
}

template <>
RequestType DatabaseRequestHandler::getRequestType<
    DatabaseRequestHandler::ApertureShapeData>() const {
  return RequestType::SOURCE_APERTURE_SHAPE_TABLE;
}

/// <summary>
/// Constructs DatabaseRequestHandler::StrapData from rows from the database.
/// </summary>
/// <param name="strapRow">The rows to construct the elements from.</param>
/// <param name="handle">A reference to the first handle to give the first
/// element, then be incremented.</param> <param name="elements">A reference to
/// a vector to put the newly constructed elements into.</param> <param
/// name="sizeValue">A reference to the size a buffer would need to be to store
/// all of these, to be incremented.</param>
template <>
void DatabaseRequestHandler::constructDataElements(
    mysqlx::RowResult &strapRow, unsigned &handle,
    std::vector<DatabaseRequestHandler::StrapData> &elements,
    unsigned &sizeValue) const {
  for (mysqlx::Row row : strapRow.fetchAll()) {
    if (row.isNull()) {
      std::cout << "Null" << std::endl;
      continue;
    }
    DatabaseRequestHandler::StrapData data;
    data.handle = handle++;
    data.id = row[0];
    data.materialHandle =
        DrawingComponentManager<Material>::findComponentByID(row[1]).handle();
    data.isWTL = row[2].get<bool>();
    elements.push_back(data);
    sizeValue += sizeof(bool) + sizeof(unsigned) * 3;
  }
}

/// <summary>
/// Serialises a single DatabaseRequestHandler::StrapData into the provided
/// buffer.
/// </summary>
/// <param name="element">The element to serialise to the buffer.</param>
/// <param name="buffer">The buffer to write the element to.</param>
template <>
void DatabaseRequestHandler::serialiseDataElement(
    const DatabaseRequestHandler::StrapData &element,
    unsigned char **buffer) const {
  *((unsigned *)*buffer) = element.materialHandle;
  *buffer += sizeof(unsigned);
  *(*buffer)++ = (bool)element.isWTL;
}

template <>
RequestType DatabaseRequestHandler::getRequestType<
    DatabaseRequestHandler::StrapData>() const {
  return RequestType::SOURCE_STRAPS_TABLE;
}

/// <summary>
/// Constructs DatabaseRequestHandler::MaterialData from all rows recieved from
/// the database.
/// </summary>
/// <param name="materialRow">The rows recieved from a database query.</param>
/// <param name="handle">The biggest handle of materials currently, to increment
/// for each material.</param>
/// <param name="elements">A reference to a vector to
/// store the newly created elements in.</param>
/// <param name="sizeValue">A
/// reference to an unsigned integer to store the amount of bytes will be needed
/// to serialise the elements.</param>
template <>
void DatabaseRequestHandler::constructDataElements(
    mysqlx::RowResult &materialRow, unsigned &handle,
    std::vector<DatabaseRequestHandler::MaterialData> &elements,
    unsigned &sizeValue) const {
  std::map<unsigned, DatabaseRequestHandler::MaterialData> material_ids;
  for (mysqlx::Row row : materialRow.fetchAll()) {
    if (row.isNull()) {
      std::cout << "Null" << std::endl;
      continue;
    }
    if (material_ids.find(row[6]) == material_ids.end()) {
      DatabaseRequestHandler::MaterialData data;
      data.handle = handle++;
      data.id = row[6];
      data.name = row[7].get<std::string>();
      data.hardness = row[8].get<unsigned>();
      data.thickness = row[9].get<unsigned>();
      sizeValue += sizeof(unsigned) * 2 + sizeof(unsigned short) * 2 +
                   sizeof(unsigned char) + data.name.size() +
                   sizeof(unsigned char);
      material_ids.insert({row[6], data});
      if (!row[2].isNull()) {
        sizeValue +=
            sizeof(unsigned) + sizeof(float) * 3 + sizeof(MaterialPricingType);
        material_ids[row[6]].materialPrices.push_back(
            {row[0], row[2], row[3], row[4],
             pricingMap.at(row[5].get<std::string>())});
      }
    } else {
      sizeValue +=
          sizeof(unsigned) + sizeof(float) * 3 + sizeof(MaterialPricingType);
      material_ids[row[6]].materialPrices.push_back(
          {row[0], row[2], row[3], row[4],
           pricingMap.at(row[5].get<std::string>())});
    }
  }
  for (const std::pair<unsigned, MaterialData> &key : material_ids) {
    elements.push_back(key.second);
  }
}

/// <summary>
/// Serialises a singular DatabaseRequestHandler::MaterialData to the provided
/// buffer, and increments the sizeValue.
/// </summary>
/// <param name="element">A reference to the element to serialise.</param>
/// <param name="buffer">The buffer to serialise the material to.</param>
template <>
void DatabaseRequestHandler::serialiseDataElement(
    const DatabaseRequestHandler::MaterialData &element,
    unsigned char **buffer) const {
  *((unsigned short *)*buffer) = element.hardness;
  *buffer += sizeof(unsigned short);

  *((unsigned short *)*buffer) = element.thickness;
  *buffer += sizeof(unsigned short);

  unsigned char nameSize = element.name.size();
  *(*buffer)++ = nameSize;
  memcpy(*buffer, element.name.c_str(), nameSize);
  *buffer += nameSize;

  unsigned char priceElements = element.materialPrices.size();
  *(*buffer)++ = priceElements;

  for (Material::MaterialPrice tuple : element.materialPrices) {
    *((unsigned *)*buffer) = std::get<0>(tuple);
    *buffer += sizeof(unsigned);
    *((float *)*buffer) = std::get<1>(tuple);
    *buffer += sizeof(float);
    *((float *)*buffer) = std::get<2>(tuple);
    *buffer += sizeof(float);
    *((float *)*buffer) = std::get<3>(tuple);
    *buffer += sizeof(float);
    *((MaterialPricingType *)*buffer) = std::get<4>(tuple);
    *buffer += sizeof(MaterialPricingType);
  }
}

template <>
RequestType DatabaseRequestHandler::getRequestType<
    DatabaseRequestHandler::MaterialData>() const {
  return RequestType::SOURCE_MATERIAL_TABLE;
}

/// <summary>
/// Constructs DatabaseRequestHandler::SideIronPriceData from all rows recieved
/// from the database.
/// </summary>
/// <param name="sideIronPriceRow">The rows recieved from a database
/// query.</param> <param name="handle">The biggest handle of side iron prices
/// currently, to increment for each side iron price.</param> <param
/// name="elements">A reference to a vector to store the newly created elements
/// in.</param> <param name="sizeValue">A reference to an unsigned integer to
/// store the amount of bytes will be needed to serialise the elements.</param>
template <>
void DatabaseRequestHandler::constructDataElements(
    mysqlx::RowResult &sideIronPriceRow, unsigned &handle,
    std::vector<DatabaseRequestHandler::SideIronPriceData> &elements,
    unsigned &sizeValue) const {
  // for (mysqlx::Row row : sideIronPriceRow) {
  for (mysqlx::Row row : sideIronPriceRow.fetchAll()) {
    if (row.isNull()) {
      std::cout << "Null" << std::endl;
      continue;
    }
    DatabaseRequestHandler::SideIronPriceData data;
    data.handle = handle++;
    if (row[0].isNull()) {
      continue;
    }
    data.id = row[0];
    if (row[1].isNull()) {
      continue;
    }
    data.type = (SideIronType)row[1].get<unsigned>();
    if (row[2].isNull()) {
      continue;
    }
    data.lowerLength = row[2].get<unsigned>();
    if (row[3].isNull()) {
      continue;
    }
    data.upperLength = row[3].get<unsigned>();
    if (row[4].isNull()) {
      continue;
    }
    data.extraflex = (row[4].get<unsigned>() - 1);
    if (row[5].isNull()) {
      continue;
    }
    data.price = row[5].get<float>();
    elements.push_back(data);
    sizeValue += sizeof(unsigned) * 4 + sizeof(SideIronType) + sizeof(bool) +
                 sizeof(float);
  }
};

/// <summary>
/// Serialises a singular DatabaseRequestHandler::SideIronPriceData to the
/// provided buffer, and increments the sizeValue.
/// </summary>
/// <param name="element">A reference to the element to serialise.</param>
/// <param name="buffer">The buffer to serialise the side iron price to.</param>
template <>
void DatabaseRequestHandler::serialiseDataElement(
    const DatabaseRequestHandler::SideIronPriceData &element,
    unsigned char **buffer) const {
  *((SideIronType *)*buffer) = element.type;
  *buffer += sizeof(SideIronType);

  *((unsigned *)*buffer) = element.lowerLength;
  *buffer += sizeof(unsigned);

  *((unsigned *)*buffer) = element.upperLength;
  *buffer += sizeof(unsigned);

  *(*buffer)++ = element.extraflex;

  *((float *)*buffer) = element.price;
  *buffer += sizeof(float);
}

template <>
RequestType DatabaseRequestHandler::getRequestType<
    DatabaseRequestHandler::SideIronPriceData>() const {
  return RequestType::SOURCE_SIDE_IRON_PRICES_TABLE;
}

/// <summary>
/// Constructs DatabaseRequestHandler::PowderCoatingPriceData from all rows
/// recieved from the database.
/// </summary>
/// <param name="powderCoatingRow">The rows recieved from a database
/// query.</param> <param name="handle">The biggest handle of powder coating
/// prices currently, to increment for each powder coating price.</param> <param
/// name="elements">A reference to a vector to store the newly created elements
/// in.</param> <param name="sizeValue">A reference to an unsigned integer to
/// store the amount of bytes will be needed to serialise the elements.</param>

template <>
void DatabaseRequestHandler::constructDataElements(
    mysqlx::RowResult &powderCoatingRow, unsigned &handle,
    std::vector<DatabaseRequestHandler::PowderCoatingPriceData> &elements,
    unsigned &sizeValue) const {
  // for (mysqlx::Row row : powderCoatingRow) {
  for (mysqlx::Row row : powderCoatingRow.fetchAll()) {
    if (row.isNull()) {
      std::cout << "Null" << std::endl;
      continue;
    }
    PowderCoatingPriceData data;
    data.handle = handle++;
    data.id = row[0];
    sizeValue += sizeof(unsigned) * 2;
    data.hookPrice = row[1].get<float>();
    sizeValue += sizeof(float);
    data.strapPrice = row[2].get<float>();
    sizeValue += sizeof(float);

    elements.push_back(data);
  }
}

/// <summary>
/// Serialises a singular DatabaseRequestHandler::PowderCoatingPriceData to the
/// provided buffer, and increments the sizeValue.
/// </summary>
/// <param name="element">A reference to the element to serialise.</param>
/// <param name="buffer">The buffer to serialise the powder coating price
/// to.</param>
template <>
void DatabaseRequestHandler::serialiseDataElement(
    const DatabaseRequestHandler::PowderCoatingPriceData &element,
    unsigned char **buffer) const {
  *((float *)*buffer) = element.hookPrice;
  *buffer += sizeof(float);
  *((float *)*buffer) = element.strapPrice;
  *buffer += sizeof(float);
}

template <>
RequestType DatabaseRequestHandler::getRequestType<
    DatabaseRequestHandler::PowderCoatingPriceData>() const {
  return RequestType::SOURCE_POWDER_COATING_TABLE;
}

/// <summary>
/// Constructs DatabaseRequestHandler::SideIronData from all rows recieved from
/// the database.
/// </summary>
/// <param name="sideIronRow">The rows recieved from a database query.</param>
/// <param name="handle">The biggest handle of side irons currently, to
/// increment for each side iron.</param> <param name="elements">A reference to
/// a vector to store the newly created elements in.</param> <param
/// name="sizeValue">A reference to an unsigned integer to store the amount of
/// bytes will be needed to serialise the elements.</param>
template <>
void DatabaseRequestHandler::constructDataElements(
    mysqlx::RowResult &sideIronRow, unsigned &handle,
    std::vector<DatabaseRequestHandler::SideIronData> &elements,
    unsigned &sizeValue) const {
  for (mysqlx::Row row : sideIronRow.fetchAll()) {
    if (row.isNull()) {
      std::cout << "Null" << std::endl;
      continue;
    }
    SideIronData data;
    data.handle = handle++;
    data.id = row[0];
    if (!row[1].isNull()) {
      data.type = row[1].get<unsigned>();
    } else {
      data.type = 0;
    }
    if (!row[2].isNull()) {
      data.length = row[2].get<unsigned>();
    } else {
      data.length = 0;
    }
    data.drawingNumber = row[3].get<std::string>();
    data.hyperlink = row[4].get<std::string>();

    data.extraflex = row[7].get<bool>();

    if (!row[5].isNull()) {
      data.price = std::make_optional(row[5].get<float>());
    } else {
      data.price = std::nullopt;
    }
    if (!row[6].isNull()) {
      data.screws = std::make_optional(row[6].get<unsigned>());
    } else {
      data.screws = std::nullopt;
    }

    sizeValue += sizeof(unsigned) * 2 + sizeof(unsigned char) +
                 sizeof(unsigned short) + sizeof(unsigned char) +
                 data.drawingNumber.size() + sizeof(unsigned char) +
                 data.hyperlink.size() + sizeof(bool) * 3 +
                 (data.price.has_value() ? sizeof(float) : 0) +
                 (data.screws.has_value() ? sizeof(unsigned) : 0);

    elements.push_back(data);
  }
}

/// <summary>
/// Serialises a singular DatabaseRequestHandler::SideIronData to the provided
/// buffer, and increments the sizeValue.
/// </summary>
/// <param name="element">A reference to the element to serialise.</param>
/// <param name="buffer">The buffer to serialise the side iron to.</param>
template <>
void DatabaseRequestHandler::serialiseDataElement(
    const DatabaseRequestHandler::SideIronData &element,
    unsigned char **buffer) const {
  *(*buffer)++ = element.type;
  *((unsigned short *)*buffer) = element.length;
  *buffer += sizeof(unsigned short);
  unsigned char nameSize = element.drawingNumber.size();
  *(*buffer)++ = nameSize;
  memcpy(*buffer, element.drawingNumber.c_str(), nameSize);
  *buffer += nameSize;
  unsigned char hyperlinkSize = element.hyperlink.size();
  *(*buffer)++ = hyperlinkSize;
  memcpy(*buffer, element.hyperlink.c_str(), hyperlinkSize);
  *buffer += hyperlinkSize;
  *(*buffer)++ = element.extraflex;
  *(*buffer)++ = element.price.has_value();
  if (element.price.has_value()) {
    *((float *)*buffer) = *element.price;
    *buffer += sizeof(float);
  }
  *(*buffer)++ = element.screws.has_value();
  if (element.screws.has_value()) {
    *((unsigned *)*buffer) = *element.screws;
    *buffer += sizeof(unsigned);
  }
}

template <>
RequestType DatabaseRequestHandler::getRequestType<
    DatabaseRequestHandler::SideIronData>() const {
  return RequestType::SOURCE_SIDE_IRON_TABLE;
}

/// <summary>
/// Constructs DatabaseRequestHandler::MachineData from all rows recieved from
/// the database.
/// </summary>
/// <param name="machineRow">The rows recieved from a database query.</param>
/// <param name="handle">The biggest handle of machines currently, to increment
/// for each machine.</param>
/// <param name="elements">A reference to a vector to
/// store the newly created elements in.</param>
/// <param name="sizeValue">A
/// reference to an unsigned integer to store the amount of bytes will be needed
/// to serialise the elements.</param>
template <>
void DatabaseRequestHandler::constructDataElements(
    mysqlx::RowResult &machineRow, unsigned &handle,
    std::vector<DatabaseRequestHandler::MachineData> &elements,
    unsigned &sizeValue) const {
  for (mysqlx::Row row : machineRow.fetchAll()) {
    if (row.isNull()) {
      std::cout << "Null" << std::endl;
      continue;
    }
    MachineData data;
    data.handle = handle++;
    data.id = row[0];
    data.manufacturer = row[1].get<std::string>();
    data.model = row[2].get<std::string>();
    sizeValue += sizeof(unsigned) * 2 + sizeof(unsigned char) +
                 data.manufacturer.size() + sizeof(unsigned char) +
                 data.model.size();

    elements.push_back(data);
  }
}

/// <summary>
/// Serialises a singular DatabaseRequestHandler::MachineData to the provided
/// buffer, and increments the sizeValue.
/// </summary>
/// <param name="element">A reference to the element to serialise.</param>
/// <param name="buffer">The buffer to serialise the machine to.</param>
template <>
void DatabaseRequestHandler::serialiseDataElement(
    const DatabaseRequestHandler::MachineData &element,
    unsigned char **buffer) const {
  unsigned char manufacturerSize = element.manufacturer.size();
  *(*buffer)++ = manufacturerSize;
  memcpy(*buffer, element.manufacturer.c_str(), manufacturerSize);
  *buffer += manufacturerSize;

  unsigned char modelSize = element.model.size();
  *(*buffer)++ = modelSize;
  memcpy(*buffer, element.model.c_str(), modelSize);
  *buffer += modelSize;
}

template <>
RequestType DatabaseRequestHandler::getRequestType<
    DatabaseRequestHandler::MachineData>() const {
  return RequestType::SOURCE_MACHINE_TABLE;
}

/// <summary>
/// Constructs DatabaseRequestHandler::MachineDeckData from all rows recieved
/// from the database.
/// </summary>
/// <param name="machineDeckRow">The rows recieved from a database
/// query.</param> <param name="handle">The biggest handle of machine decks
/// currently, to increment for each machine deck.</param> <param
/// name="elements">A reference to a vector to store the newly created elements
/// in.</param> <param name="sizeValue">A reference to an unsigned integer to
/// store the amount of bytes will be needed to serialise the elements.</param>

template <>
void DatabaseRequestHandler::constructDataElements(
    mysqlx::RowResult &machineDeckRow, unsigned &handle,
    std::vector<DatabaseRequestHandler::MachineDeckData> &elements,
    unsigned &sizeValue) const {
  for (mysqlx::Row row : machineDeckRow.fetchAll()) {
    if (row.isNull()) {
      std::cout << "Null" << std::endl;
      continue;
    }
    MachineDeckData data;
    data.handle = handle++;
    data.id = row[0];
    data.deck = row[1].get<std::string>();
    sizeValue +=
        sizeof(unsigned) * 2 + sizeof(unsigned char) + data.deck.size();

    elements.push_back(data);
  }
}

/// <summary>
/// Serialises a singular DatabaseRequestHandler::MachineDeckData to the
/// provided buffer, and increments the sizeValue.
/// </summary>
/// <param name="element">A reference to the element to serialise.</param>
/// <param name="buffer">The buffer to serialise the machine deck to.</param>
template <>
void DatabaseRequestHandler::serialiseDataElement(
    const DatabaseRequestHandler::MachineDeckData &element,
    unsigned char **buffer) const {
  unsigned char deckSize = element.deck.size();
  *(*buffer)++ = deckSize;
  memcpy(*buffer, element.deck.c_str(), deckSize);
  *buffer += deckSize;
}

template <>
RequestType DatabaseRequestHandler::getRequestType<
    DatabaseRequestHandler::MachineDeckData>() const {
  return RequestType::SOURCE_MACHINE_DECK_TABLE;
}