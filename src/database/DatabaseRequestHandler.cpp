#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
//
// Created by matthew on 09/07/2020.
//

#include "../../include/database/DatabaseRequestHandler.h"

#include <utility>

void DatabaseRequestHandler::onMessageReceived(Server &caller, const ClientHandle &clientHandle, void *message,
                                               unsigned int messageSize) {
    switch (getDeserialiseType(message)) {
        case RequestType::DRAWING_SEARCH_QUERY: {
            std::vector<DrawingSummary> summaries = caller.databaseManager().executeSearchQuery(
                    DatabaseSearchQuery::deserialise(message));

            DrawingSummaryCompressionSchema schema = caller.compressionSchema();

            unsigned char *responseBuffer = (unsigned char *) alloca(sizeof(RequestType) +
                                                                     sizeof(DrawingSummaryCompressionSchema) +
                                                                     sizeof(unsigned) +
                                                                     summaries.size() * schema.maxCompressedSize());

            unsigned index = 0;

            *((RequestType *) responseBuffer) = RequestType::DRAWING_SEARCH_QUERY;
            index += sizeof(RequestType);

            memcpy(responseBuffer + index, &schema, sizeof(DrawingSummaryCompressionSchema));
            index += sizeof(DrawingSummaryCompressionSchema);

            *((unsigned *) (responseBuffer + index)) = summaries.size();
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
                    case DatabaseManager::EXISTS:
                        if (drawingInsert.forcing()) {
                            if (caller.databaseManager().insertDrawing(drawingInsert)) {
                                response.insertResponseType = DrawingInsert::SUCCESS;
                            } else {
                                response.insertResponseType = DrawingInsert::FAILED;
                            }
                        } else {
                            response.insertResponseType = DrawingInsert::DRAWING_EXISTS;
                        }
                        break;
                    case DatabaseManager::NOT_EXISTS:
                        if (caller.databaseManager().insertDrawing(drawingInsert)) {
                            response.insertResponseType = DrawingInsert::SUCCESS;
                        } else {
                            response.insertResponseType = DrawingInsert::FAILED;
                        }
                        break;
                    case DatabaseManager::ERROR:
                        response.insertResponseType = DrawingInsert::FAILED;
                        break;
                }

                if (response.insertResponseType == DrawingInsert::SUCCESS) {
                    caller.setCompressionSchemaDirty();
                }

                unsigned responseSize = response.serialisedSize();
                void *responseBuffer = alloca(responseSize);
                response.serialise(responseBuffer);

                caller.addMessageToSendQueue(clientHandle, responseBuffer, responseSize);
            }

            break;
        }
        case RequestType::SOURCE_PRODUCT_TABLE: {
            if (DrawingComponentManager<Product>::dirty()) {
                sql::ResultSet *sourceData = caller.databaseManager().sourceTable("products");

                struct ProductData {
                    unsigned id{};
                    std::string name;
                };

                std::vector<ProductData> products;
                unsigned bufferSize = sizeof(RequestType) + sizeof(unsigned);

                while (sourceData->next()) {
                    ProductData data;
                    data.id = sourceData->getUInt("product_id");
                    data.name = sourceData->getString("product_name");

                    products.push_back(data);
                    bufferSize += sizeof(unsigned) + sizeof(unsigned char) + data.name.size();
                }

                void *sourceBuffer = malloc(bufferSize);

                unsigned char *buff = (unsigned char *) sourceBuffer;
                *((RequestType *) buff) = RequestType::SOURCE_PRODUCT_TABLE;
                buff += sizeof(RequestType);
                *((unsigned *) buff) = products.size();
                buff += sizeof(unsigned);

                for (const ProductData &product : products) {
                    *((unsigned *) buff) = product.id;
                    buff += sizeof(unsigned);
                    unsigned char nameSize = product.name.size();
                    *buff++ = nameSize;
                    memcpy(buff, product.name.c_str(), nameSize);
                    buff += nameSize;
                }

                DrawingComponentManager<Product>::sourceComponentTable(sourceBuffer, bufferSize);

                delete sourceData;
            }

            caller.addMessageToSendQueue(clientHandle, DrawingComponentManager<Product>::rawSourceData(),
                                         DrawingComponentManager<Product>::rawSourceDataSize());

            break;
        }
        case RequestType::SOURCE_APERTURE_TABLE: {
            if (DrawingComponentManager<Aperture>::dirty()) {
                sql::ResultSet *sourceData = caller.databaseManager().sourceTable("apertures");

PACK_START
                struct ApertureData {
                    unsigned id{};
                    unsigned short width{}, length{}, baseWidth{}, baseLength{};
                    unsigned shapeID{};
                    unsigned short quantity{};
                }
PACK_END

                std::vector<ApertureData> apertures;
                unsigned bufferSize = sizeof(RequestType) + sizeof(unsigned);

                while (sourceData->next()) {
                    ApertureData data;
                    data.id = sourceData->getUInt("aperture_id");
                    data.width = sourceData->getUInt("width");
                    data.length = sourceData->getUInt("length");
                    data.baseWidth = sourceData->getUInt("base_width");
                    data.baseLength = sourceData->getUInt("base_length");
                    data.shapeID = sourceData->getUInt("shape_id");
                    data.quantity = sourceData->getUInt("quantity");

                    apertures.push_back(data);
                    bufferSize += sizeof(ApertureData);
                }

                void *sourceBuffer = malloc(bufferSize);

                unsigned char *buff = (unsigned char *) sourceBuffer;
                *((RequestType *) buff) = RequestType::SOURCE_APERTURE_TABLE;
                buff += sizeof(RequestType);
                *((unsigned *) buff) = apertures.size();
                buff += sizeof(unsigned);

                for (const ApertureData &aperture : apertures) {
                    memcpy(buff, &aperture, sizeof(ApertureData));
                    buff += sizeof(ApertureData);
                }

                DrawingComponentManager<Aperture>::sourceComponentTable(sourceBuffer, bufferSize);

                delete sourceData;
            }

            caller.addMessageToSendQueue(clientHandle, DrawingComponentManager<Aperture>::rawSourceData(),
                                         DrawingComponentManager<Aperture>::rawSourceDataSize());

            break;
        }
        case RequestType::SOURCE_APERTURE_SHAPE_TABLE: {
            if (DrawingComponentManager<ApertureShape>::dirty()) {
                sql::ResultSet *sourceData = caller.databaseManager().sourceTable("aperture_shapes");

                struct ApertureShapeData {
                    unsigned id{};
                    std::string shape;
                };

                std::vector<ApertureShapeData> apertureShapes;
                unsigned bufferSize = sizeof(RequestType) + sizeof(unsigned);

                while (sourceData->next()) {
                    ApertureShapeData data;
                    data.id = sourceData->getUInt("aperture_shape_id");
                    data.shape = sourceData->getString("shape");

                    apertureShapes.push_back(data);
                    bufferSize += sizeof(unsigned) + sizeof(unsigned char) + data.shape.size();
                }

                void *sourceBuffer = malloc(bufferSize);

                unsigned char *buff = (unsigned char *) sourceBuffer;
                *((RequestType *) buff) = RequestType::SOURCE_APERTURE_SHAPE_TABLE;
                buff += sizeof(RequestType);
                *((unsigned *) buff) = apertureShapes.size();
                buff += sizeof(unsigned);

                for (const ApertureShapeData &apertureShape : apertureShapes) {
                    *((unsigned *) buff) = apertureShape.id;
                    buff += sizeof(unsigned);
                    unsigned char shapeSize = apertureShape.shape.size();
                    *buff++ = shapeSize;
                    memcpy(buff, apertureShape.shape.c_str(), shapeSize);
                    buff += shapeSize;
                }

                DrawingComponentManager<ApertureShape>::sourceComponentTable(sourceBuffer, bufferSize);

                delete sourceData;
            }

            caller.addMessageToSendQueue(clientHandle, DrawingComponentManager<ApertureShape>::rawSourceData(),
                                         DrawingComponentManager<ApertureShape>::rawSourceDataSize());

            break;
        }
        case RequestType::SOURCE_MATERIAL_TABLE: {
            if (DrawingComponentManager<Material>::dirty()) {
                sql::ResultSet *sourceData = caller.databaseManager().sourceTable("materials");

                struct MaterialData {
                    unsigned id{};
                    std::string name;
                    unsigned short hardness{}, thickness{};
                };

                std::vector<MaterialData> materials;
                unsigned bufferSize = sizeof(RequestType) + sizeof(unsigned);

                while (sourceData->next()) {
                    MaterialData data;
                    data.id = sourceData->getUInt("material_id");
                    data.name = sourceData->getString("material");
                    data.hardness = sourceData->getUInt("hardness");
                    data.thickness = sourceData->getUInt("thickness");

                    materials.push_back(data);
                    bufferSize +=
                            sizeof(unsigned) + sizeof(unsigned short) * 2 + sizeof(unsigned char) + data.name.size();
                }

                void *sourceBuffer = malloc(bufferSize);

                unsigned char *buff = (unsigned char *) sourceBuffer;
                *((RequestType *) buff) = RequestType::SOURCE_MATERIAL_TABLE;
                buff += sizeof(RequestType);
                *((unsigned *) buff) = materials.size();
                buff += sizeof(unsigned);

                for (const MaterialData &material : materials) {
                    *((unsigned *) buff) = material.id;
                    buff += sizeof(unsigned);
                    *((unsigned short *) buff) = material.hardness;
                    buff += sizeof(unsigned short);
                    *((unsigned short *) buff) = material.thickness;
                    buff += sizeof(unsigned short);
                    unsigned char nameSize = material.name.size();
                    *buff++ = nameSize;
                    memcpy(buff, material.name.c_str(), nameSize);
                    buff += nameSize;
                }

                DrawingComponentManager<Material>::sourceComponentTable(sourceBuffer, bufferSize);

                delete sourceData;
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

                sql::ResultSet *sourceData = caller.databaseManager().sourceTable("side_irons", orderBy.str());

                struct SideIronData {
                    unsigned id{};
                    unsigned char type{};
                    unsigned short length{};
                    std::string drawingNumber;
                    std::string hyperlink;
                };

                std::vector<SideIronData> sideIrons;
                unsigned bufferSize = sizeof(RequestType) + sizeof(unsigned);

                while (sourceData->next()) {
                    SideIronData data;
                    data.id = sourceData->getUInt("side_iron_id");
                    data.type = sourceData->getUInt("type");
                    data.length = sourceData->getUInt("length");
                    data.drawingNumber = sourceData->getString("drawing_number");
                    data.hyperlink = sourceData->getString("hyperlink");

                    sideIrons.push_back(data);
                    bufferSize +=
                            sizeof(unsigned) + sizeof(unsigned char) + sizeof(unsigned short) + sizeof(unsigned char) +
                            data.drawingNumber.size() + sizeof(unsigned char) + data.hyperlink.size();
                }

                void *sourceBuffer = malloc(bufferSize);

                unsigned char *buff = (unsigned char *) sourceBuffer;
                *((RequestType *) buff) = RequestType::SOURCE_SIDE_IRON_TABLE;
                buff += sizeof(RequestType);
                *((unsigned *) buff) = sideIrons.size();
                buff += sizeof(unsigned);

                for (const SideIronData &sideIron : sideIrons) {
                    *((unsigned *) buff) = sideIron.id;
                    buff += sizeof(unsigned);
                    *buff++ = sideIron.type;
                    *((unsigned short *) buff) = sideIron.length;
                    buff += sizeof(unsigned short);
                    unsigned char nameSize = sideIron.drawingNumber.size();
                    *buff++ = nameSize;
                    memcpy(buff, sideIron.drawingNumber.c_str(), nameSize);
                    buff += nameSize;
                    unsigned char hyperlinkSize = sideIron.hyperlink.size();
                    *buff++ = hyperlinkSize;
                    memcpy(buff, sideIron.hyperlink.c_str(), hyperlinkSize);
                    buff += hyperlinkSize;
                }

                DrawingComponentManager<SideIron>::sourceComponentTable(sourceBuffer, bufferSize);

                delete sourceData;
            }

            caller.addMessageToSendQueue(clientHandle, DrawingComponentManager<SideIron>::rawSourceData(),
                                         DrawingComponentManager<SideIron>::rawSourceDataSize());

            break;
        }
        case RequestType::SOURCE_MACHINE_TABLE: {
            if (DrawingComponentManager<Machine>::dirty()) {
                sql::ResultSet *sourceData = caller.databaseManager().sourceTable("machines",
                                                                                  "manufacturer<>'None', manufacturer, model");

                struct MachineData {
                    unsigned id{};
                    std::string manufacturer, model;
                };

                std::vector<MachineData> machines;
                unsigned bufferSize = sizeof(RequestType) + sizeof(unsigned);

                while (sourceData->next()) {
                    MachineData data;
                    data.id = sourceData->getUInt("machine_id");
                    data.manufacturer = sourceData->getString("manufacturer");
                    data.model = sourceData->getString("model");

                    machines.push_back(data);
                    bufferSize += sizeof(unsigned) + sizeof(unsigned char) + data.manufacturer.size() +
                                  sizeof(unsigned char) + data.model.size();
                }

                void *sourceBuffer = malloc(bufferSize);

                unsigned char *buff = (unsigned char *) sourceBuffer;
                *((RequestType *) buff) = RequestType::SOURCE_MACHINE_TABLE;
                buff += sizeof(RequestType);
                *((unsigned *) buff) = machines.size();
                buff += sizeof(unsigned);

                for (const MachineData &machine : machines) {
                    *((unsigned *) buff) = machine.id;
                    buff += sizeof(unsigned);

                    unsigned char manufacturerSize = machine.manufacturer.size();
                    *buff++ = manufacturerSize;
                    memcpy(buff, machine.manufacturer.c_str(), manufacturerSize);
                    buff += manufacturerSize;

                    unsigned char modelSize = machine.model.size();
                    *buff++ = modelSize;
                    memcpy(buff, machine.model.c_str(), modelSize);
                    buff += modelSize;
                }

                DrawingComponentManager<Machine>::sourceComponentTable(sourceBuffer, bufferSize);

                delete sourceData;
            }

            caller.addMessageToSendQueue(clientHandle, DrawingComponentManager<Machine>::rawSourceData(),
                                         DrawingComponentManager<Machine>::rawSourceDataSize());

            break;
        }
        case RequestType::SOURCE_MACHINE_DECK_TABLE: {
            if (DrawingComponentManager<MachineDeck>::dirty()) {
                sql::ResultSet *sourceData = caller.databaseManager().sourceTable("machine_decks");

                struct MachineDeckData {
                    unsigned id{};
                    std::string deck;
                };

                std::vector<MachineDeckData> machineDecks;
                unsigned bufferSize = sizeof(RequestType) + sizeof(unsigned);

                while (sourceData->next()) {
                    MachineDeckData data;
                    data.id = sourceData->getUInt("deck_id");
                    data.deck = sourceData->getString("deck");

                    machineDecks.push_back(data);
                    bufferSize += sizeof(unsigned) + sizeof(unsigned char) + data.deck.size();
                }

                void *sourceBuffer = malloc(bufferSize);

                unsigned char *buff = (unsigned char *) sourceBuffer;
                *((RequestType *) buff) = RequestType::SOURCE_MACHINE_DECK_TABLE;
                buff += sizeof(RequestType);
                *((unsigned *) buff) = machineDecks.size();
                buff += sizeof(unsigned);

                for (const MachineDeckData &machineDeck : machineDecks) {
                    *((unsigned *) buff) = machineDeck.id;
                    buff += sizeof(unsigned);

                    unsigned char deckSize = machineDeck.deck.size();
                    *buff++ = deckSize;
                    memcpy(buff, machineDeck.deck.c_str(), deckSize);
                    buff += deckSize;
                }

                DrawingComponentManager<MachineDeck>::sourceComponentTable(sourceBuffer, bufferSize);

                delete sourceData;
            }

            caller.addMessageToSendQueue(clientHandle, DrawingComponentManager<MachineDeck>::rawSourceData(),
                                         DrawingComponentManager<MachineDeck>::rawSourceDataSize());

            break;
        }
        case RequestType::DRAWING_DETAILS: {
            DrawingRequest request = DrawingRequest::deserialise(message);

            request.drawingData = *caller.databaseManager().executeDrawingQuery(request);

            unsigned bufferSize = request.serialisedSize();

            void *responseBuffer = alloca(bufferSize);
            request.serialise(responseBuffer);

            caller.addMessageToSendQueue(clientHandle, responseBuffer, bufferSize);

            break;
        }
    }
}

RequestType DatabaseRequestHandler::getDeserialiseType(void *data) {
    return *((RequestType *) data);
}

#pragma clang diagnostic pop