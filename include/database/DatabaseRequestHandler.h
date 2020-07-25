//
// Created by matthew on 09/07/2020.
//

#ifndef DATABASE_MANAGER_DATABASEREQUESTHANDLER_H
#define DATABASE_MANAGER_DATABASEREQUESTHANDLER_H

#include "DatabaseQuery.h"
#include "../networking/Server.h"

#include "../../packer.h"

class DatabaseRequestHandler : public ServerRequestHandler {
public:
	DatabaseRequestHandler();

	void onMessageReceived(Server &caller, const ClientHandle &clientHandle, void *message, unsigned int messageSize) override;

private:
	DrawingSummaryCompressionSchema compressionSchema(DatabaseManager *dbManager);

	void setCompressionSchemaDirty();

	DrawingSummaryCompressionSchema schema;
	bool schemaDirty = true;

	struct TableSourceData {
		unsigned id = 0, handle = 0;
	};

	struct ProductData : TableSourceData {
		typedef Product ComponentType;

		std::string name;
	};

	struct ApertureData : TableSourceData {
		typedef Aperture ComponentType;

		float width{}, length{};
		unsigned short baseWidth{}, baseLength{};
		unsigned shapeID{};
		unsigned short quantity{};
	};

	struct ApertureShapeData : TableSourceData {
		typedef ApertureShape ComponentType;

		std::string shape;
	};

	struct MaterialData : TableSourceData {
		typedef Material ComponentType;

		std::string name;
		unsigned short hardness{}, thickness{};
	};

	struct SideIronData : TableSourceData {
		typedef SideIron ComponentType;

		unsigned char type{};
		unsigned short length{};
		std::string drawingNumber;
		std::string hyperlink;
	};

	struct MachineData : TableSourceData {
		typedef Machine ComponentType;

		std::string manufacturer, model;
	};

	struct MachineDeckData : TableSourceData {
		typedef MachineDeck ComponentType;

		std::string deck;
	};

	static RequestType getDeserialiseType(void *data);

	template<typename T>
	void createSourceData(mysqlx::RowResult &sourceRows) const;

	template<typename T>
	void constructDataElement(const mysqlx::Row &sourceRow, unsigned &handle, std::vector<T> &elements, unsigned &sizeValue) const;

	template<typename T>
	void serialiseDataElement(const T &element, void *buffer, unsigned &sizeValue) const;

	template<typename T>
	constexpr RequestType getRequestType() const;
};

template<typename T>
void DatabaseRequestHandler::createSourceData(mysqlx::RowResult &sourceRows) const {
	static_assert(std::is_base_of<TableSourceData, T>::value, "Create Source Data can only be called with templates deriving TableSourceData.");

	std::vector<T> elements;
	unsigned bufferSize = sizeof(RequestType) + sizeof(unsigned);

	unsigned handle = 1;

	for (const mysqlx::Row &row : sourceRows) {
		constructDataElement<T>(row, handle, elements, bufferSize);
	}

	void *sourceBuffer = malloc(bufferSize);

	unsigned char *buff = (unsigned char *)sourceBuffer;
	*((RequestType *)buff) = getRequestType<T>();
	buff += sizeof(RequestType);
	*((unsigned *)buff) = elements.size();
	buff += sizeof(unsigned);

	for (const T &element : elements) {
		*((unsigned *)buff) = element.handle;
		buff += sizeof(unsigned);
		*((unsigned *)buff) = element.id;
		buff += sizeof(unsigned);

		unsigned elementSize = 0;
		serialiseDataElement<T>(element, buff, elementSize);
		buff += elementSize;
	}

	DrawingComponentManager<T::ComponentType>::sourceComponentTable(sourceBuffer, bufferSize);
}


#endif //DATABASE_MANAGER_DATABASEREQUESTHANDLER_H

