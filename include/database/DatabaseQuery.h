//
// Created by matthew on 09/07/2020.
//

#ifndef DATABASE_MANAGER_DATABASEQUERY_H
#define DATABASE_MANAGER_DATABASEQUERY_H

#include <mysqlx/xdevapi.h>

#include <string>
#include <cstring>
#include <optional>
#include <chrono>
#include <iomanip>
#include <vector>

#include <nlohmann/json.hpp>

#include "Drawing.h"

// Simple macro for returning the minumum of two values
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/// <summary>
/// ValueRange
/// A range of some type of value, such as an integer or a date
/// </summary>
/// <typeparam name="T">The type of the value range.</typeparam>
template<typename T>
struct ValueRange {
    /// The lower and upper bounds for this (inclusive) range
    T lowerBound, upperBound;

    /// <summary>
    /// Serialise the ValueRange object into the buffer
    /// </summary>
    /// <param name="buffer">The buffer to write this object into.</param>
    void serialise(void *buffer) const;

    /// <summary>
    /// Deserialise and construct a ValueRange from the buffer
    /// </summary>
    /// <param name="buffer">The buffer to read this object from.</param>
    /// <returns>A newly constructed ValueRange constructed from the buffer.</returns>
    static ValueRange<T> deserialise(void *buffer);

    /// <summary>
    /// Method to get the number of bytes this object will take up in the buffer
    /// </summary>
    /// <returns>The size in bytes this object will occupy in a buffer.</returns>
    unsigned serialisedSize() const;
};

// Serialise the ValueRange object into the buffer
template<typename T>
void ValueRange<T>::serialise(void *buffer) const {
    // Get the buffer as a T* for direct writing
    T *buff = (T *) buffer;
    // Write the first sizeof(T) bytes to be the value of the lowerBound
    // variable, and analogously for the second sizeof(T).
    *buff++ = lowerBound;
    *buff = upperBound;
}

// Deserialise and construct a ValueRange from the buffer
template<typename T>
ValueRange<T> ValueRange<T>::deserialise(void *buffer) {
    // Get the buffer as a T* for direct reading
    T *buff = (T *) buffer;
    // Create the range object to return
    ValueRange<T> range;
    // Read from the buffer into the lower and upper bounds
    range.lowerBound = *buff++;
    range.upperBound = *buff;

    // Return the constructed value range 
    return range;
}

// Method to get the number of bytes this object will take up in the buffer
template<typename T>
unsigned ValueRange<T>::serialisedSize() const {
    // Simply return twice the sizeof(T)
    return 2 * sizeof(T);
}

/// <summary>
/// DatabaseQuery
/// A base class for different types of queries to the database
/// </summary>
class DatabaseQuery {
public:
    /// <summary>
    /// Constructor for DatabaseQuery
    /// </summary>
    /// <returns>Constructs a new DatabaseQuery object</returns>
    DatabaseQuery();

    /// <summary>
    /// Serialse this object into the target buffer. 
    /// This is a pure virtual object, so any deriving classes must
    /// implement it.
    /// </summary>
    /// <param name="target">The target buffer to serialise this object into.</param>
    virtual void serialise(void *target) const = 0;

    /// <summary>
    /// Get the serialised size of this object.
    /// This size will be how many bytes this object will occupy in the buffer.
    /// This is also a pure virtual function, so any deriving classes must
    /// implement it.
    /// </summary>
    /// <returns>The size the object will occupy.</returns>
    virtual unsigned serialisedSize() const = 0;

    /// <summary>
    /// Create the buffer and serialise this object into it.
    /// </summary>
    /// <param name="size">The size this object will take, which will be written to the reference variable. 
    /// This avoids forcing the client to call serialisedSize() multiple times. </param>
    /// <returns>The newly created buffer.</returns>
    void *createBuffer(unsigned &size) const;
};

/// <summary>
/// DatabaseSearchQuery
/// Inherits from DatabaseQuery. This query type is for searching the database for
/// drawing summaries (instead of full drawing objects)
/// </summary>
class DatabaseSearchQuery : public DatabaseQuery {
public:
    /// <summary>
    /// Constructor for DatabaseSearchQuery
    /// </summary>
    /// <returns>A newly constructed DatabaseSearchQuery object.</returns>
    DatabaseSearchQuery();

    /// <summary>
    /// Serialise this object into the target buffer
    /// </summary>
    /// <param name="target">The buffer to write this serialises object into.</param>
    void serialise(void *target) const override;

    /// <summary>
    /// Get the serialised size of this object.
    /// This size will be how many bytes this object will occupy in the buffer.
    /// </summary>
    /// <returns>The size the object will occupy.</returns>
    unsigned serialisedSize() const override;

    /// <summary>
    /// Deserialise this object from the data buffer
    /// </summary>
    /// <param name="data">The buffer to read this object from</param>
    /// <returns>A newly constructed query object equivalent to the one the buffer was
    /// created with.</returns>
    static DatabaseSearchQuery &deserialise(void *data);

    /// <summary>
    /// Constructs an SQL query string from the search parameters defined in this
    /// object's attributes
    /// </summary>
    /// <returns>A string containing the SQL query</returns>
    std::string toSQLQueryString() const;

    /// <summary>
    /// Static function to return a list (vector) of summaries for each drawing
    /// which matched the query parameters
    /// </summary>
    /// <param name="resultSet">The rows from the database in their raw format.</param>
    /// <returns>A list of DrawingSummary objects for each matching drawing.</returns>
    static std::vector<DrawingSummary> getQueryResultSummaries(mysqlx::RowResult &resultSet);

    // Each parameter is nested inside an optional. This means that each value can
    // also take a "nullopt", which indicates that it should be omitted from the 
    // search.
    std::optional<std::string> drawingNumber;
    std::optional<ValueRange<unsigned>> width, length;
    std::optional<Product> productType;
    std::optional<unsigned char> numberOfBars;
    std::optional<Aperture> aperture;
    std::optional<Material> topThickness, bottomThickness;
    std::optional<ValueRange<Date>> dateRange;
    std::optional<SideIronType> sideIronType;
    std::optional<unsigned short> sideIronLength;
    std::optional<LapSetting> sidelapMode, overlapMode;
    std::optional<ValueRange<unsigned>> sidelapWidth, overlapWidth;
    std::optional<LapAttachment> sidelapAttachment, overlapAttachment;
    std::optional<Machine> machine;
    std::optional<std::string> manufacturer;
    std::optional<unsigned char> quantityOnDeck;
    std::optional<std::string> position;
    std::optional<MachineDeck> machineDeck;

private:
    /// <summary>
    /// SearchParamters flag based enum
    /// Each value is a power of two to make a flag system. Each flag indicates
    /// whether a particular value is used in the query or not. This is used for
    /// serialisation to determine which values to send in the message, and also
    /// to allow for reconstructing parameters as nullopts where necessary.
    /// </summary>
    enum class SearchParameters {
        DRAWING_NUMBER = 0x00000001,
        WIDTH = 0x00000002,
        LENGTH = 0x00000004,
        PRODUCT_TYPE = 0x00000008,
        NUMBER_OF_BARS = 0x00000010,
        APERTURE = 0x00000020,
        TOP_THICKNESS = 0x00000040,
        BOTTOM_THICKNESS = 0x00000080,
        DATE_RANGE = 0x00000100,
        SIDE_IRON_TYPE = 0x00000200,
        SIDE_IRON_LENGTH = 0x00000400,
        SIDELAP_MODE = 0x00000800,
        OVERLAP_MODE = 0x00001000,
        SIDELAP_WIDTH = 0x00002000,
        OVERLAP_WIDTH = 0x00004000,
        SIDELAP_ATTACHMENT = 0x00008000,
        OVERLAP_ATTACHMENT = 0x00010000,
        MACHINE = 0x00020000,
        MACHINE_MANUFACTURER = 0x00040000,
        QUANTITY_ON_DECK = 0x00080000,
        POSITION = 0x00100000,
        MACHINE_DECK = 0x00200000
    };

    /// <summary>
    /// Looks at each parameter in the query. If the parameter is present, 
    /// its matching flag gets added to a flag object (represented by an unsigned
    /// integer)
    /// </summary>
    /// <returns>The constructed flag object indicating which paramters are used
    /// in the search.</returns>
    unsigned getSearchParameters() const;
};

/// <summary>
/// DrawingRequest
/// Inherits from DatabaseQuery. This type of query requests all the information about
/// a single drawing in the database, as opposed to basic information about many
/// drawings.
/// </summary>
class DrawingRequest : public DatabaseQuery {
public:
    /// <summary>
    /// Simple static constructor for creating a request query, which contains
    /// no drawing information other than the matID of the desired drawing.
    /// </summary>
    /// <param name="matID">The database index of the desired drawing.</param>
    /// <param name="responseEchoCode">A code for the server to echo so the client can tell which
    /// request a given message is in response to, as the request model is asynchronous.</param>
    /// <returns>A (reference to a) DrawingRequest object with the matID and echo code specified.</returns>
    static DrawingRequest &makeRequest(unsigned matID, unsigned responseEchoCode);

    /// <summary>
    /// Serialise this object into the target buffer
    /// </summary>
    /// <param name="target">The buffer to write this serialises object into.</param>
    void serialise(void *target) const override;

    /// <summary>
    /// Get the serialised size of this object.
    /// This size will be how many bytes this object will occupy in the buffer.
    /// </summary>
    /// <returns>The size the object will occupy.</returns>
    unsigned int serialisedSize() const override;

    /// <summary>
    /// Deserialise this object from the data buffer
    /// </summary>
    /// <param name="data">The buffer to read this object from</param>
    /// <returns>A newly constructed query object equivalent to the one the buffer was
    /// created with.</returns>
    static DrawingRequest &deserialise(void *data);

    // The database index for the drawing this query is concerned with
    unsigned matID;

    // The echo code for matching requests and response together, due to the
    // asynchronous nature of the request system (i.e. a request is sent to the
    // server, the client continues as usual, the server formulates a response and
    // sends it back to the client, and at this point the client detects it has 
    // received a message and acts upon it. The client never "waits".)
    unsigned responseEchoCode;

    // An optional value for the drawing data. If this is a request message,
    // there is obviously no drawing to send. If it is a response, however,
    // the drawingData should be set to a drawing matching the originally
    // requested one.
    std::optional<Drawing> drawingData;
};

/// <summary>
/// DrawingInsert
/// Inherits from DatabaseQuery. This type of query is for inserting a new drawing,
/// or updating an existing drawing, in the database.
/// </summary>
class DrawingInsert : public DatabaseQuery {
public:
    /// <summary>
    /// InsertResponseCode enum
    /// This is an enum for different response types to a certain insert
    /// request. If the DrawingInsert is a request itself, it should
    /// contain no response code, so it uses the NONE code.
    /// If the insertion was successful, a respose DrawingInsert should
    /// be sent with the SUCCESS code set, indicating to the client 
    /// that the insertion was successful.
    /// If the insertion failed for some (possibly unknown) reason, 
    /// such as a MySQL error, the insert returns that the insertion FAILED.
    /// Finally, if the insertion was attempted on a drawing which already existed
    /// in the database, and the insert wasn't in "forcing" mode, the response
    /// should return a DRAWING_EXISTS code. This allows the client to appropriately
    /// respond, for example with another query set into forcing mode if they wish
    /// to update.
    /// </summary>
    enum InsertResponseCode {
        NONE,
        SUCCESS,
        FAILED,
        DRAWING_EXISTS
    };

    /// <summary>
    /// Serialise this object into the target buffer
    /// </summary>
    /// <param name="target">The buffer to write this serialises object into.</param>
    void serialise(void *target) const override;

    /// <summary>
    /// Get the serialised size of this object.
    /// This size will be how many bytes this object will occupy in the buffer.
    /// </summary>
    /// <returns>The size the object will occupy.</returns>
    unsigned int serialisedSize() const override;

    /// <summary>
    /// Deserialise this object from the data buffer
    /// </summary>
    /// <param name="data">The buffer to read this object from</param>
    /// <returns>A newly constructed query object equivalent to the one the buffer was
    /// created with.</returns>
    static DrawingInsert &deserialise(void *data);

    /// <summary>
    /// Set the forcing mode of this insert query
    /// The forcing mode indicates what to do if the drawing is already present in the database.
    /// A forcing insert will erase the original drawing and update with a new one.
    /// A non-forcing insert will return a response flag indicating that a drawing was found,
    /// leaving it up to the client to respond appropriately.
    /// </summary>
    /// <param name="val">The mode to set the query to. True for forcing, false for non-forcing.</param>
    void setForce(bool val);

    /// <summary>
    /// Getter for the forcing mode.
    /// The forcing mode indicates what to do if the drawing is already present in the database.
    /// A forcing insert will erase the original drawing and update with a new one.
    /// A non-forcing insert will return a response flag indicating that a drawing was found,
    /// leaving it up to the client to respond appropriately.
    /// </summary>
    /// <returns>The mode this query is set to. True for forcing, false for non-forcing.</returns>
    bool forcing() const;

    /// <summary>
    /// Constructs an SQL query for inserting the drawing specified in the drawingData
    /// object in this query object.
    /// </summary>
    /// <param name="templateID">The templateID which is known only during the insert process, so must be passed
    /// from the database manager.</param>
    /// <returns>An SQL query string.</returns>
    std::string drawingInsertQuery(unsigned templateID) const;

    /// <summary>
    /// Constructs an SQL query for inserting the bar spacings and widths specified in the drawingData
    /// object in this query object.
    /// </summary>
    /// <param name="matID">The matID which is known only during the insert process, so must be passed
    /// from the database manager.</param>
    /// <returns>An SQL query string.</returns>
    std::string barSpacingInsertQuery(unsigned matID) const;

    /// <summary>
    /// Constructs an SQL query for inserting the machine template specified in the drawingData
    /// object in this query object.
    /// </summary>
    /// <returns>An SQL query string.</returns>
    std::string machineTemplateInsertQuery() const;

    /// <summary>
    /// Constructs an SQL query for testing if a matching machine template is already
    /// present in the database.
    /// </summary>
    /// <returns>An SQL query string.</returns>
    std::string testMachineTemplateQuery() const;

    /// <summary>
    /// Constructs an SQL query for inserting the aperture specified in the drawingData
    /// object in this query object.
    /// </summary>
    /// <param name="matID">The matID which is known only during the insert process, so must be passed
    /// from the database manager.</param>
    /// <returns>An SQL query string.</returns>
    std::string apertureInsertQuery(unsigned matID) const;

    /// <summary>
    /// Constructs an SQL query for inserting the side irons specified in the drawingData
    /// object in this query object.
    /// </summary>
    /// <param name="matID">The matID which is known only during the insert process, so must be passed
    /// from the database manager.</param>
    /// <returns>An SQL query string.</returns>
    std::string sideIronInsertQuery(unsigned matID) const;

    /// <summary>
    /// Constructs an SQL query for inserting the materials and thicknesses specified in the drawingData
    /// object in this query object.
    /// </summary>
    /// <param name="matID">The matID which is known only during the insert process, so must be passed
    /// from the database manager.</param>
    /// <returns>An SQL query string.</returns>
    std::string thicknessInsertQuery(unsigned matID) const;

    /// <summary>
    /// Constructs an SQL query for inserting the overlaps specified in the drawingData
    /// object in this query object.
    /// </summary>
    /// <param name="matID">The matID which is known only during the insert process, so must be passed
    /// from the database manager.</param>
    /// <returns>An SQL query string or an empty string if there are no overlaps to insert.</returns>
    std::string overlapsInsertQuery(unsigned matID) const;

    /// <summary>
    /// Constructs an SQL query for inserting the sidelaps specified in the drawingData
    /// object in this query object.
    /// </summary>
    /// <param name="matID">The matID which is known only during the insert process, so must be passed
    /// from the database manager.</param>
    /// <returns>An SQL query string or an empty string if there are no sidelaps to insert.</returns>
    std::string sidelapsInsertQuery(unsigned matID) const;

    /// <summary>
    /// Constructs an SQL query for inserting the punch programs specified in the drawingData
    /// object in this query object.
    /// </summary>
    /// <param name="matID">The matID which is known only during the insert process, so must be passed
    /// from the database manager.</param>
    /// <returns>An SQL query string or an empty string if there are no punch programs to insert.</returns>
    std::string punchProgramsInsertQuery(unsigned matID) const;

    /// <summary>
    /// Constructs an SQL query for inserting the impact pads specified in the drawingData 
    /// object in this query object.
    /// </summary>
    /// <param name="matID">The matID which is known only during the insert process, so must be passed
    /// from the database manager.</param>
    /// <returns>An SQL query string or an empty string if there are no impact pads to insert.</returns>
    std::string impactPadsInsertQuery(unsigned matID) const;

    std::string damBarInsertQuery(unsigned matID) const;

    std::string blankSpaceInsertQuery(unsigned matID) const;

    /// <summary>
    /// Constructs an SQL query for inserting the centre holes specified in the drawingData 
    /// object in this query object.
    /// </summary>
    /// <param name="matID">The matID which is known only during the insert process, so must be passed
    /// from the database manager.</param>
    /// <returns>An SQL query string or an empty string if there are no centre holes to insert.</returns>
    std::string centreHolesInsertQuery(unsigned matID) const;

    /// <summary>
    /// Constructs an SQL query for inserting the deflectors specified in the drawingData 
    /// object in this query object.
    /// </summary>
    /// <param name="matID">The matID which is known only during the insert process, so must be passed
    /// from the database manager.</param>
    /// <returns>An SQL query string or an empty string if there are no deflectors to insert.</returns>
    std::string deflectorsInsertQuery(unsigned matID) const;

    /// <summary>
    /// Constructs an SQL query for inserting the divertors specified in the drawingData 
    /// object in this query object.
    /// </summary>
    /// <param name="matID">The matID which is known only during the insert process, so must be passed
    /// from the database manager.</param>
    /// <returns>An SQL query string or an empty string if there are no divertors to insert.</returns>
    std::string divertorsInsertQuery(unsigned matID) const;

    // The optional drawingData object. If this object is set, this is a
    // request query, and the drawingData represents the drawing we wish to insert.
    // If this is a response, this is set to nullopt (as the client already knows the
    // drawing they tried to insert so there is no point in sending it back).
    std::optional<Drawing> drawingData;

    // The response code for the insert query.
    // See above for the meaning of each code.
    InsertResponseCode insertResponseCode = NONE;

    // The response code for this drawing insertion, so the client can match up a response
    // to a request. This is used to accommodate the asynchronous model used.
    unsigned responseEchoCode;

private:
    // Boolean value containing whether or not this drawing insertion is in forcing mode
    bool force = false;
};

/// <summary>
/// ComponentInsert
/// Inherits from DatabaseQuery. This type of query is for inserting a drawing component
/// into the database, for example to add a new aperture.
/// </summary>
class ComponentInsert : public DatabaseQuery {
public:
    /// <summary>
    /// ApertureData
    /// Object representation of the aperture data needed to insert a new aperture. This is done
    /// to avoid creating a "true" Aperture object before one officially exists.
    /// </summary>
    struct ApertureData {
        float width, length;
        unsigned baseWidth, baseLength;
        unsigned quantity;
        unsigned shapeID;

        inline constexpr unsigned serialisedSize() const {
            return sizeof(float) * 2 + sizeof(unsigned) * 4;
        }
    };

    /// <summary>
    /// MachineData
    /// Object representation of the aperture data needed to insert a new machine. This is done
    /// to avoid creating a "true" Machine object before one officially exists.
    /// </summary>
    struct MachineData {
        std::string manufacturer, model;

        inline unsigned serialisedSize() const {
            return sizeof(unsigned char) + manufacturer.size() + sizeof(unsigned char) + model.size();
        }
    };

    /// <summary>
    /// SideIronData
    /// Object representation of the aperture data needed to insert a new side iron. This is done
    /// to avoid creating a "true" SideIron object before one officially exists.
    /// </summary>
    struct SideIronData {
        SideIronType type;
        unsigned length;
        std::string drawingNumber;
        std::filesystem::path hyperlink;

        inline unsigned serialisedSize() const {
            return sizeof(SideIronType) + sizeof(unsigned) + sizeof(unsigned char) + 
                drawingNumber.size() + sizeof(unsigned char) + hyperlink.generic_string().size();
        }
    };

    /// <summary>
    /// MaterialData
    /// Object representation of the aperture data needed to insert a new material. This is done
    /// to avoid creating a "true" Material object before one officially exists.
    /// </summary>
    struct MaterialData {
        std::string materialName;
        unsigned hardness, thickness;

        inline unsigned serialisedSize() const {
            return sizeof(unsigned char) + materialName.size() + sizeof(unsigned) * 2;
        }
    };

    /// <summary>
    /// ComponentInsertResponse
    /// Represents a response code from the server which depends on whether the component was successfully
    /// added to the database or not. The code "None" is used in the request.
    /// </summary>
    enum class ComponentInsertResponse {
        NONE,
        SUCCESS,
        FAILED
    };

    /// <summary>
    /// Constructor for ComponentInsert
    /// </summary>
    /// <returns></returns>
    ComponentInsert();

    /// <summary>
    /// Serialise this object into the target buffer
    /// </summary>
    /// <param name="target">The buffer to write this serialises object into.</param>
    void serialise(void *target) const override;

    /// <summary>
    /// Get the serialised size of this object.
    /// This size will be how many bytes this object will occupy in the buffer.
    /// </summary>
    /// <returns>The size the object will occupy.</returns>
    unsigned int serialisedSize() const override;

    /// <summary>
    /// Deserialise this object from the data buffer
    /// </summary>
    /// <param name="data">The buffer to read this object from</param>
    /// <returns>A newly constructed query object equivalent to the one the buffer was
    /// created with.</returns>
    static ComponentInsert &deserialise(void *data);

    /// <summary>
    /// Sets the data for the component to be added to the database
    /// </summary>
    /// <typeparam name="T">The type of component data to add (e.g. ApertureData)</typeparam>
    /// <param name="data">The component value itself to add to the database</param>
    template<typename T>
    void setComponentData(const T &data);

    /// <summary>
    /// Clears the component data for this object
    /// </summary>
    void clearComponentData();

    /// <summary>
    /// Creates an SQL query string for inserting this object into the database. This will depend
    /// on the type of object specified to add.
    /// </summary>
    /// <returns>The SQL query string used to add the new component to the database.</returns>
    std::string toSQLQueryString() const;

    /// <summary>
    /// Getter for the appropriate request code
    /// </summary>
    /// <returns>The appropriate request type code depending on what sort of component this object
    /// inserted.</returns>
    RequestType getSourceTableCode() const;

    // The response code for the object so the client can determine whether the request was successful.
    ComponentInsertResponse responseCode = ComponentInsertResponse::NONE;

private:
    /// <summary>
    /// InsertType
    /// Flag variable to simply determine which type of component the query intended to add.
    /// </summary>
    enum class InsertType {
        NONE,
        APERTURE,
        MACHINE,
        SIDE_IRON,
        MATERIAL
    };

    // The realisation of the insert type variable. Defaults to None
    InsertType insertType = InsertType::NONE;

    // Holds 4 different optionals for each different potential type. At any point, only one of these
    // should contain a value, and if multiple do, then the insertType variable dictates which is correct
    std::optional<ApertureData> apertureData;
    std::optional<MachineData> machineData;
    std::optional<SideIronData> sideIronData;
    std::optional<MaterialData> materialData;

};

/// <summary>
/// DatabaseBackup
/// An object representing a user request to create a backup of the database in the form of a MySQL dump.
/// The server's DatabaseManager is responsible for creating this backup, and the location to write to is determined
/// in the server's meta file.
/// </summary>
class DatabaseBackup : DatabaseQuery {
public:
    /// <summary>
    /// BackupResponse
    /// Response code for whether the backup request was successful.
    /// </summary>
    enum class BackupResponse {
        NONE,
        SUCCESS,
        FAILED
    };

    /// <summary>
    /// Default constructor
    /// </summary>
    DatabaseBackup() = default;

    /// <summary>
    /// Serialise this object into the target buffer
    /// </summary>
    /// <param name="target">The buffer to write this serialises object into.</param>
    void serialise(void *target) const override;

    /// <summary>
    /// Get the serialised size of this object.
    /// This size will be how many bytes this object will occupy in the buffer.
    /// </summary>
    /// <returns>The size the object will occupy.</returns>
    unsigned int serialisedSize() const override;

    /// <summary>
    /// Deserialise this object from the data buffer
    /// </summary>
    /// <param name="data">The buffer to read this object from</param>
    /// <returns>A newly constructed query object equivalent to the one the buffer was
    /// created with.</returns>
    static DatabaseBackup &deserialise(void *data);

    // The response code for this object. Defaults to None indicating that the object 
    // represents a reques.
    BackupResponse responseCode = BackupResponse::NONE;

    // The string name to save the backup under
    std::string backupName;

private:
};

/// <summary>
/// NextDrawing
/// A request object to obtain the name of the next drawing that should be added. This can either be
/// the next automatic drawing or the next manual drawing based on the current database state.
/// </summary>
class NextDrawing : DatabaseQuery {
public:
    /// <summary>
    /// DrawingType
    /// Simply determines whether the concerned drawing number query should return the next
    /// automatic or manual drawing number.
    /// </summary>
    enum class DrawingType {
        AUTOMATIC,
        MANUAL
    };

    /// <summary>
    /// Default constructor
    /// </summary>
    NextDrawing() = default;

    /// <summary>
    /// Serialise this object into the target buffer
    /// </summary>
    /// <param name="target">The buffer to write this serialises object into.</param>
    void serialise(void *target) const override;

    /// <summary>
    /// Get the serialised size of this object.
    /// This size will be how many bytes this object will occupy in the buffer.
    /// </summary>
    /// <returns>The size the object will occupy.</returns>
    unsigned int serialisedSize() const override;

    /// <summary>
    /// Deserialise this object from the data buffer
    /// </summary>
    /// <param name="data">The buffer to read this object from</param>
    /// <returns>A newly constructed query object equivalent to the one the buffer was
    /// created with.</returns>
    static NextDrawing &deserialise(void *data);
    
    // Realisation of the DrawingType. Defaults to automatic, but should probably be set explictitly.
    DrawingType drawingType = DrawingType::AUTOMATIC;

    // The string variable containing the drawing number. This is used only in the response
    // part of the query.
    std::optional<std::string> drawingNumber = std::nullopt;

private:
};

#endif //DATABASE_MANAGER_DATABASEQUERY_H
