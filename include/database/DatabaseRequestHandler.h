//
// Created by matthew on 09/07/2020.
//

#ifndef DATABASE_MANAGER_DATABASEREQUESTHANDLER_H
#define DATABASE_MANAGER_DATABASEREQUESTHANDLER_H

#include "DatabaseQuery.h"
#include "../networking/Server.h"

#include "../../packer.h"
#include <map>

/// <summary>\ingroup database
/// DatabaseRequestHandler
/// Inherits from ServerRequestHandler. This class is responsible for managing
/// requests from the client to access database resources. It acts as a "middle man"
/// and as such can filter out any invalid or unauthorised requests etc.
/// </summary>
class DatabaseRequestHandler : public ServerRequestHandler {
public:
	/// <summary>
	/// Constructor for DatabaseRequestHandler
	/// </summary>
	DatabaseRequestHandler();

	/// <summary>
	/// Message Received callback function. When the server receives a message from a client,
	/// it is passed to the ServerRequestHandler object set in the server. If this is that object
	/// then this function will be called.
	/// </summary>
	/// <param name="caller">A reference to the server object which called this function.</param>
	/// <param name="clientHandle">A handle to the client who sent the message so it is possible to formulate 
	/// a response for the server to return to them.</param>
	/// <param name="message">The message data itself, as a rvalue reference to indicate transfer
	/// of ownership..</param>
	/// <param name="messageSize">The length (in bytes) of the message data received.</param>
	void onMessageReceived(Server &caller, const ClientHandle &clientHandle, void*&& message, unsigned int messageSize) override;

	// The filepath to create backups under. Should be set in the server's meta file.
	std::filesystem::path backupPath;

private:
	/// <summary>
	/// A map for finding relevant prices for materials
	/// </summary>
	std::map<std::string, MaterialPricingType> pricingMap;

	/// <summary>
	/// Creates a compression schema from the database based upon the largest values in certain 
	/// fields.
	/// </summary>
	/// <param name="dbManager">The database manager object to request the compression schema details from</param>
	/// <returns>The newly constructed DrawingSummaryCompressionSchema object containing the compression schema information</returns>
	DrawingSummaryCompressionSchema compressionSchema(DatabaseManager *dbManager);

	/// <summary>
	/// Set the dirty flag for the compression schema. This simply indicates that the next time the compression schema
	/// is requested, it should first be updated. A reason for this may be a new drawing was inserted into the table,
	/// and as such the maximum mat_id will now be bigger.
	/// </summary>
	void setCompressionSchemaDirty();

	// The current compression schema object. It is not always the case that a new one must be created, so one
	// is stored for use if the dirty flag is not set.
	DrawingSummaryCompressionSchema schema;
	// The dirty flag for the compression schema. True if the compression schema needs to be updated the next time
	// the getter is called
	bool schemaDirty = true;

	/// <summary>
	/// TableSourceData
	/// An internal structure for storing (relatively) raw data about a row from a certain table
	/// </summary>
	struct TableSourceData {
		// Unsigned integers for the (database) id and (program) handle for this element
		unsigned id = 0, handle = 0;
	};

	/// <summary>
	/// ProductData
	/// Inherits from TableSourceData. This object is for storing data about a product read 
	/// from the database
	/// </summary>
	struct ProductData : TableSourceData {
		// Typedef for the ComponentType. This is used for accessing the appropriate DrawingComponentManager.
		typedef Product ComponentType;

		// The name of this product
		std::string name;
	};

	struct BackingStripData : TableSourceData {
		// Typedef for the ComponentType. This is used for accessing the appropriate DrawingComponentManager.
		typedef BackingStrip ComponentType;

		// The ID of the shape of this aperture
		unsigned materialID{};
	};

	/// <summary>
	/// ApertureData
	/// Inherits from TableSourceData. This object is for storing data about an aperture read 
	/// from the database
	/// </summary>
	struct ApertureData : TableSourceData {
		// Typedef for the ComponentType. This is used for accessing the appropriate DrawingComponentManager.
		typedef Aperture ComponentType;

		// The width and length of this aperture tool
		float width{}, length{};
		// The width and length of the base block on which this tool is mounted
		unsigned short baseWidth{}, baseLength{};
		// The ID of the shape of this aperture
		unsigned shapeID{};
		// The number of physical copies of this tool which exist
		unsigned short quantity{};

		bool isNibble;
		unsigned nibbleApertureId;
	};

	/// <summary>
	/// ApertureShapeData
	/// Inherits from TableSourceData. This object is for storing data about a product read 
	/// from the database
	/// </summary>
	struct ApertureShapeData : TableSourceData {
		// Typedef for the ComponentType. This is used for accessing the appropriate DrawingComponentManager.
		typedef ApertureShape ComponentType;

		// A string containing the shape name of this shape (e.g. SQ or DIA)
		std::string shape;
	};

	/// <summary>
	/// MaterialData
	/// Inherits from TableSourceData. This object is for storing data about an aperture shape
	/// read from the database
	/// </summary>
	struct MaterialData : TableSourceData {
		// Typedef for the ComponentType. This is used for accessing the appropriate DrawingComponentManager.
		typedef Material ComponentType;

		// The name of this material (e.g. Rubber Screen Cloth)
		std::string name;
		// The hardness and thickness of this material
		unsigned short hardness{}, thickness{};

		std::vector<Material::MaterialPrice> materialPrices;
	};
	/// <summary>
	/// ExtraPriceData
	/// Inhertis from TableSourceData. This object is for storing data about extra prices
	/// read from the database
	/// </summary>
	struct ExtraPriceData : TableSourceData {
		// Typedef for the ComponentType. This is used for accessing the appropriate DrawingComponentManager.
		typedef ExtraPrice ComponentType;

		ExtraPriceType type;
		float price;
		std::optional<float> squareMetres;
		std::optional<unsigned> amount;
	};
	/// <summary>
	/// LabourTimeData
	/// Inhertis from TableSourceData. This object is for storing data about labour time
	/// read from the database
	/// </summary>
	struct LabourTimeData : TableSourceData {
		typedef LabourTime ComponentType;

		std::string job;
		unsigned time;
	};

	/// <summary>
	/// SideIronPriceData 
	/// Inhertis from TableSourceData. This object is for storing data about side irons
	/// read from the database
	/// </summary>
	struct SideIronPriceData : TableSourceData {
		typedef SideIronPrice ComponentType;
		SideIronType type;
		unsigned lowerLength, upperLength;
		float price;
		bool extraflex;
	};

	/// <summary>
	/// PowderCoatingPriceData
	/// Inhertis from TableSourceData. This object is for storing data about powder coating
	/// read from the database
	/// </summary>
	struct PowderCoatingPriceData : TableSourceData {
		typedef PowderCoatingPrice ComponentType;

		float hookPrice, strapPrice;
	};

	/// <summary>
	/// SideIronData
	/// Inherits from TableSourceData. This object is for storing data about a side iron read 
	/// from the database
	/// </summary>
	struct SideIronData : TableSourceData {
		// Typedef for the ComponentType. This is used for accessing the appropriate DrawingComponentManager.
		typedef SideIron ComponentType;

		// The type of this side iron (e.g. A, B, C, D, E)
		unsigned char type{};
		// The length of this side iron
		unsigned short length{};
		// The associated drawing number for this side iron
		std::string drawingNumber;
		// The hyperlink to the drawing for this side iron
		std::string hyperlink;

		std::optional<float> price = std::nullopt;
		std::optional<unsigned> screws = std::nullopt;
	};

	/// <summary>
	/// MachineData
	/// Inherits from TableSourceData. This object is for storing data about a machine read 
	/// from the database
	/// </summary>
	struct MachineData : TableSourceData {
		// Typedef for the ComponentType. This is used for accessing the appropriate DrawingComponentManager.
		typedef Machine ComponentType;

		// The names of both the manufacturer and model of this machine
		std::string manufacturer, model;
	};

	/// <summary>
	/// MachineDeckData
	/// Inherits from TableSourceData. This object is for storing data about a machine deck read 
	/// from the database
	/// </summary>
	struct MachineDeckData : TableSourceData {
		// Typedef for the ComponentType. This is used for accessing the appropriate DrawingComponentManager.
		typedef MachineDeck ComponentType;

		// The name of the deck (e.g. All, Top, Middle, etc.)
		std::string deck;
	};

	/// <summary>
	/// Gets the request type from the start of the data buffer
	/// </summary>
	/// <param name="data">The data buffer to read the request type from.</param>
	/// <returns>The request type from the buffer.</returns>
	static RequestType getDeserialiseType(void *data);

	/// <summary>
	/// Creates source data for the specified type for the DrawingComponentManager.
	/// This method creates a data buffer to be sent into the DrawingComponentManager of the matching
	/// type, such that the elements can be reconstructed in a format usable by the program.
	/// </summary>
	/// <typeparam name="T">The type for which to create the source data.</typeparam>
	/// <param name="sourceRows">The row sources from the correct MySQL table.</param>
	template<typename T>
	void createSourceData(mysqlx::RowResult sourceRows) const;

	/// <summary>
	/// Constructs one or more data elements of a given type from a single MySQL row.
	/// This function circumvents the problem with having non bijective mappings between the database
	/// entries for a given component table, and the "true" entries, i.e. with bidirectional apertures
	/// where one database entry maps to two different "true" entries.
	/// This function is specialised for each matching type, as there is no "general" way to construct
	/// each component.
	/// </summary>
	/// <typeparam name="T">The type for which to create the data element(s).</typeparam>
	/// <param name="sourceRow">The row source to use to construct the element(s).</param>
	/// <param name="handle">The current handle to use. This will be incremented by this method for every element it adds.</param>
	/// <param name="elements">The current vector of elements. This is appended to within the method.</param>
	/// <param name="sizeValue">The amount of space the element(s) added will occupy in a buffer, so we know how big of a buffer to 
	/// construct.</param>
	template<typename T>
	void constructDataElements(mysqlx::RowResult &sourceRow, unsigned &handle, std::vector<T> &elements, unsigned &sizeValue) const;

	/// <summary>
	/// Writes a single element to the buffer.
	/// This function is specialised for each matching type, as there is no "general" way to serialise
	/// each component.
	/// </summary>
	/// <typeparam name="T">The type of the element to write to the buffer.</typeparam>
	/// <param name="element">The element object itself to write.</param>
	/// <param name="buffer">The buffer to write to.</param>
	/// <param name="sizeValue">A reference value to increment with the amount of space this element occupies.</param>
	template<typename T>
	void serialiseDataElement(const T &element, void *buffer, unsigned &sizeValue) const;

	/// <summary>
	/// Gets the request type header to attach to the response buffer.
	/// This function is specialised for each matching type to return the correct header value
	/// </summary>
	/// <typeparam name="T">The type of the element to return the header.</typeparam>
	/// <returns>The correct RequestType for the type T.</returns>
	template<typename T>
	constexpr RequestType getRequestType() const;
};

// Creates all the source data for a given set of rows
template<typename T>
void DatabaseRequestHandler::createSourceData(mysqlx::RowResult sourceRows) const {
	// We statically assert that this is being used on a valid type. If there is an attempt to call this function on a type which does not derive
	// from TableSourceData, there will be a compiler error.
	static_assert(std::is_base_of<TableSourceData, T>::value, "Create Source Data can only be called with templates deriving TableSourceData.");

	// Note: The calls to the other template methods within this method are all assumed to exist. If there is an attempt to call one which
	// does not exist, a linker error will occur, meaning that the missing method must be implemented.

	// Declare a list of the data elements for this type
	std::vector<T> elements;
	// Create a value for the buffer size. We will increment this with each element
	// to find the total required buffer size.
	unsigned bufferSize = sizeof(RequestType) + sizeof(unsigned);

	// Initialise the handle value to 1. This will be incremented for each element. Handle 0 is reserved as a "null" option
	// by the DrawingComponentManager.
	unsigned handle = 1;

	// We loop through each row in the source
		// For each, we construct the data element(s). This is added tot he elements list from inside this function.
	constructDataElements<T>(sourceRows, handle, elements, bufferSize);

	// Next we create the source data buffer with the size we have calculated.
	void *sourceBuffer = malloc(bufferSize);

	// We cast it to a byte buffer
	unsigned char *buff = (unsigned char *)sourceBuffer;
	// First we write the request type to the beginning of the buffer
	*((RequestType *)buff) = getRequestType<T>();
	buff += sizeof(RequestType);
	// Next we write the number of elements, so the recipient knows how many elements to expect
	*((unsigned *)buff) = elements.size();
	buff += sizeof(unsigned);

	// Then we loop over each element in the list
	for (const T &element : elements) {
		// We begin by writing the (program) handle to the buffer
		*((unsigned *)buff) = element.handle;
		buff += sizeof(unsigned);
		// Then we write the component (database) ID to the buffer
		*((unsigned *)buff) = element.id;
		buff += sizeof(unsigned);

		// Next we serialise the data element to the buffer and increment the buffer by the amount of
		// space the element occupied.
		unsigned elementSize = 0;
		serialiseDataElement<T>(element, buff, elementSize);
		buff += elementSize;
	}

	// Finally, once the buffer has been constructed, we send the buffer to the DrawingComponentManager of the correct type.
	// This is where we use the T::ComponentType which will be the "true" type for each data type above. (e.g. ProductData -> Product)
	DrawingComponentManager<typename T::ComponentType>::sourceComponentTable(std::move(sourceBuffer), bufferSize + 4);
}


#endif //DATABASE_MANAGER_DATABASEREQUESTHANDLER_H

