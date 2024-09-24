//
// Created by matthew on 09/07/2020.
//

#ifndef DATABASE_MANAGER_DATABASEQUERY_H
#define DATABASE_MANAGER_DATABASEQUERY_H

#include <mysqlx/xdevapi.h>

#include <chrono>
#include <cstring>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

#include "Drawing.h"
#include "Logger.h"
#include "RequestType.h"

// Simple macro for returning the minumum of two values
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/// <summary>
/// ValueRange
/// A range of some type of value, such as an integer or a date
/// </summary>
/// <typeparam name="T">The type of the value range.</typeparam>
template <typename T>
struct ValueRange {
  /// <summary>
  /// The lower bound for this (inclusive) range.
  /// </summary>
  T lowerBound;
  /// <summary>
  /// The upper bound for this (inclusive) range.
  /// </summary>
  T upperBound;

  /// <summary>
  /// Serialise the ValueRange object into the buffer
  /// </summary>
  /// <param name="buffer">The buffer to write this object into.</param>
  void serialise(void *buffer) const;

  /// <summary>
  /// Deserialise and construct a ValueRange from the buffer
  /// </summary>
  /// <param name="buffer">The buffer to read this object from.</param>
  /// <returns>A newly constructed ValueRange constructed from the
  /// buffer.</returns>
  static ValueRange<T> deserialise(void *buffer);

  /// <summary>
  /// Method to get the number of bytes this object will take up in the buffer
  /// </summary>
  /// <returns>The size in bytes this object will occupy in a buffer.</returns>
  unsigned serialisedSize() const;
};

// Serialise the ValueRange object into the buffer
template <typename T>
void ValueRange<T>::serialise(void *buffer) const {
  // Get the buffer as a T* for direct writing
  T *buff = (T *)buffer;
  // Write the first sizeof(T) bytes to be the value of the lowerBound
  // variable, and analogously for the second sizeof(T).
  *buff++ = lowerBound;
  *buff = upperBound;
}

// Deserialise and construct a ValueRange from the buffer
template <typename T>
ValueRange<T> ValueRange<T>::deserialise(void *buffer) {
  // Get the buffer as a T* for direct reading
  T *buff = (T *)buffer;
  // Create the range object to return
  ValueRange<T> range;
  // Read from the buffer into the lower and upper bounds
  range.lowerBound = *buff++;
  range.upperBound = *buff;

  // Return the constructed value range
  return range;
}

// Method to get the number of bytes this object will take up in the buffer
template <typename T>
unsigned ValueRange<T>::serialisedSize() const {
  // Simply return twice the sizeof(T)
  return 2 * sizeof(T);
}

/// <summary>
/// DatabaseQuery
/// A base class for different types of queries to the database
/// </summary>
class CORE_API DatabaseQuery {
 public:
  /// <summary>
  /// Constructor for DatabaseQuery
  /// </summary>
  DatabaseQuery();

  /// <summary>
  /// Serialse this object into the target buffer.
  /// This is a pure virtual object, so any deriving classes must
  /// implement it.
  /// </summary>
  /// <param name="target">The target buffer to serialise this object
  /// into.</param>
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
  /// <param name="size">The size this object will take, which will be written
  /// to the reference variable. This avoids forcing the client to call
  /// serialisedSize() multiple times. </param> <returns>The newly created
  /// buffer.</returns>
  void *createBuffer(unsigned &size) const;
};

///// <summary>
///// DatabasePriceQuery
///// inherits from DatabaseQuery.
///// This query type is for looking up prices of components.
///// </summary>
// class DatabasePriceQuery: public DatabaseQuery {
//
//     /// <summary>
//     /// Constructor for DatabasePriceQuery
//     /// </summary>
//     /// <returns>A newly constructed DatabasePriceQuery Object </returns>
//     DatabasePriceQuery();
//
//
//     void serialise(void* target) const;
//
//     unsigned serialisedSize() const;
//
//     static DatabasePriceQuery& deserialise(void *data);
// };

/// <summary>
/// DatabaseSearchQuery
/// Inherits from DatabaseQuery. This query type is for searching the database
/// for drawing summaries (instead of full drawing objects)
/// </summary>
class CORE_API DatabaseSearchQuery : public DatabaseQuery {
 public:
  /// <summary>
  /// Constructor for DatabaseSearchQuery
  /// </summary>
  DatabaseSearchQuery();

  /// <summary>
  /// Serialise this object into the target buffer
  /// </summary>
  /// <param name="target">The buffer to write this serialises object
  /// into.</param>
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
  /// <param name="data">The buffer to read this object from, as a rvalue
  /// reference to indicate transfer of ownership.</param> <returns>A newly
  /// constructed query object equivalent to the one the buffer was created
  /// with.</returns>
  static DatabaseSearchQuery &deserialise(void *&&data);

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
  /// <param name="resultSet">The rows from the database in their raw
  /// format.</param> 
  /// <param name="errStream">The stream to write errors to.</param>
  /// <returns>A list of DrawingSummary objects for each
  /// matching drawing.</returns>
  static std::vector<DrawingSummary> getQueryResultSummaries(
      mysqlx::RowResult resultSet, std::ostream *errStream = &std::cerr);

  // Each parameter is nested inside an optional. This means that each value can
  // also take a "nullopt", which indicates that it should be omitted from the
  // search.

  /// <summary>
  /// Optional drawing number to search by.
  /// </summary>
  std::optional<std::string> drawingNumber;
  /// <summary>
  /// Optional width to search by.
  /// </summary>
  std::optional<ValueRange<unsigned>> width;
  /// <summary>
  /// optional length to search by.
  /// </summary>
  std::optional<ValueRange<unsigned>> length;
  /// <summary>
  /// Optional product to search by.
  /// </summary>
  std::optional<Product> productType;
  /// <summary>
  /// Optional number of bars to search by.
  /// </summary>
  std::optional<unsigned char> numberOfBars;
  /// <summary>
  /// Optional aperture to search by. Note this does not include extra
  /// apertures.
  /// </summary>
  std::optional<Aperture> aperture;
  /// <summary>
  /// Optional top thickness to search by.
  /// </summary>
  std::optional<Material> topThickness;
  /// <summary>
  /// Optional bottom thickness to search by.
  /// </summary>
  std::optional<Material> bottomThickness;
  /// <summary>
  /// Optional date range to search by.
  /// </summary>
  std::optional<ValueRange<Date>> dateRange;
  /// <summary>
  /// Optional side iron type to search by.
  /// </summary>
  std::optional<SideIronType> sideIronType;
  /// <summary>
  /// Optional side iron length to search by.
  /// </summary>
  std::optional<unsigned short> sideIronLength;
  /// <summary>
  /// Optional sidelap setting to search by.
  /// </summary>
  std::optional<LapSetting> sidelapMode;
  /// <summary>
  /// Optional overlap setting to search by.
  /// </summary>
  std::optional<LapSetting> overlapMode;
  /// <summary>
  /// Optional sidelap width range to search by.
  /// </summary>
  std::optional<ValueRange<unsigned>> sidelapWidth;
  /// <summary>
  /// Optional overlap width range to search by.
  /// </summary>
  std::optional<ValueRange<unsigned>> overlapWidth;
  /// <summary>
  /// Optional sidelap attachment type to search by.
  /// </summary>
  std::optional<LapAttachment> sidelapAttachment;
  /// <summary>
  /// Optional overlap attachment type to search by.
  /// </summary>
  std::optional<LapAttachment> overlapAttachment;
  /// <summary>
  /// Optional machine to search by.
  /// </summary>
  std::optional<Machine> machine;
  /// <summary>
  /// Optional machine manufacturer to search by.
  /// </summary>
  std::optional<std::string> manufacturer;
  /// <summary>
  /// Optional quantity on deck to search by.
  /// </summary>
  std::optional<unsigned char> quantityOnDeck;
  /// <summary>
  /// Optional position to search by.
  /// </summary>
  std::optional<std::string> position;
  /// <summary>
  /// Optional machine deck to search by.
  /// </summary>
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
/// Inherits from DatabaseQuery. This type of query requests all the information
/// about a single drawing in the database, as opposed to basic information
/// about many drawings. It functions by being constructed with a matID, the
/// object is then serialised and sent to the server, where the server will
/// populate the drawingData with what it found, and then return the object. The
/// data can then be read from the drawingData attribute.
/// </summary>
class CORE_API DrawingRequest : public DatabaseQuery {
 public:
  /// <summary>
  /// Simple static constructor for creating a request query, which contains
  /// no drawing information other than the matID of the desired drawing.
  /// </summary>
  /// <param name="matID">The database index of the desired drawing.</param>
  /// <param name="responseEchoCode">A code for the server to echo so the client
  /// can tell which request a given message is in response to, as the request
  /// model is asynchronous.</param> <returns>A (reference to a) DrawingRequest
  /// object with the matID and echo code specified.</returns>
  static DrawingRequest &makeRequest(unsigned matID, unsigned responseEchoCode);

  /// <summary>
  /// Serialise this object into the target buffer
  /// </summary>
  /// <param name="target">The buffer to write this serialises object
  /// into.</param>
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
  /// <param name="data">The buffer to read this object from, as a rvalue
  /// reference to indicate this object takes ownership of the buffer.</param>
  /// <returns>A newly constructed query object equivalent to the one the buffer
  /// was created with.</returns>
  static DrawingRequest &deserialise(void *&&data);

  /// <summary>
  /// The database index for the drawing this query is concerned with
  /// </summary>
  unsigned matID;

  /// <summary>
  /// The echo code for matching requests and response together, due to the
  /// asynchronous nature of the request system (i.e. a request is sent to the
  /// server, the client continues as usual, the server formulates a response
  /// and sends it back to the client, and at this point the client detects it
  /// has received a message and acts upon it. The client never "waits".)
  /// </summary>
  unsigned responseEchoCode;

  /// <summary>
  /// An optional value for the drawing data. If this is a request message,
  /// there is obviously no drawing to send. If it is a response, however,
  /// the drawingData should be set to a drawing matching the originally
  /// requested one.
  /// </summary>
  std::optional<Drawing> drawingData;
};

/// <summary>
/// DrawingInsert
/// Inherits from DatabaseQuery. This type of query is for inserting a new
/// drawing, or updating an existing drawing, in the database.
/// </summary>
class CORE_API DrawingInsert : public DatabaseQuery {
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
  /// should return a DRAWING_EXISTS code. This allows the client to
  /// appropriately respond, for example with another query set into forcing
  /// mode if they wish to update.
  /// </summary>
  enum InsertResponseCode {
    /// <summary>
    /// There was no response to the insert.
    /// </summary>
    NONE,
    /// <summary>
    /// The insert was successful.
    /// </summary>
    SUCCESS,
    /// <summary>
    /// The insert failed, most likely due to an sql error.
    /// </summary>
    FAILED,
    /// <summary>
    /// The drawing already exists.
    /// </summary>
    DRAWING_EXISTS
  };

  /// <summary>
  /// This indicates whether the Insert is trying to insert a new record, or
  /// update an existing one.
  /// </summary>
  enum InsertMode {
    /// <summary>
    /// The insert is trying to add a new record.
    /// </summary>
    ADD,
    /// <summary>
    /// The insert is trying to update an existing record.
    /// </summary>
    UPDATE
  };

  /// <summary>
  /// Serialise this object into the target buffer
  /// </summary>
  /// <param name="target">The buffer to write this serialises object
  /// into.</param>
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
  /// <param name="data">The buffer to read this object from, as a rvalue
  /// reference to indicate that this takes ownership of buffer.</param>
  /// <returns>A newly constructed query object equivalent to the one the buffer
  /// was created with.</returns>
  static DrawingInsert &deserialise(void *&&data);

  /// <summary>
  /// Sets the force of the inserts. True means that, if a drawing exists, the
  /// existing drawing will be deleted completely, and the new insert will take
  /// its drawing number. If False, The insertion will simply fail if the
  /// drawing number exists.
  /// </summary>
  /// <param name="val">True for forceful insertion, false for non-forceful
  /// insertion.</param>
  void setForce(bool val);

  /// <summary>
  /// Getter for forcing mode. See \sa setForce for more details.
  /// </summary>
  /// <returns>True for forceful insertion, False oterwise.</returns>

  bool forcing() const;
  /// <summary>
  /// Constructs an SQL query for inserting the drawing specified in the
  /// drawingData object in this query object.
  /// </summary>
  /// <param name="templateID">The templateID which is known only during the
  /// insert process, so must be passed from the database manager.</param>
  /// <returns>An SQL query string.</returns>
  std::string drawingInsertQuery(unsigned templateID) const;

  /// <summary>
  /// Constructs an SQL query for inserting the bar spacings and widths
  /// specified in the drawingData object in this query object.
  /// </summary>
  /// <param name="matID">The matID which is known only during the insert
  /// process, so must be passed from the database manager.</param> <returns>An
  /// SQL query string.</returns>
  std::string barSpacingInsertQuery(unsigned matID) const;

  /// <summary>
  /// Constructs an SQL query for inserting the machine template specified in
  /// the drawingData object in this query object.
  /// </summary>
  /// <returns>An SQL query string.</returns>
  std::string machineTemplateInsertQuery() const;

  /// <summary>
  /// Constructs an SQL query for testing if a matching machine template is
  /// already present in the database.
  /// </summary>
  /// <returns>An SQL query string.</returns>
  std::string testMachineTemplateQuery() const;

  /// <summary>
  /// Constructs an SQL query for inserting the aperture specified in the
  /// drawingData object in this query object.
  /// </summary>
  /// <param name="matID">The matID which is known only during the insert
  /// process, so must be passed from the database manager.</param> <returns>An
  /// SQL query string.</returns>
  std::string apertureInsertQuery(unsigned matID) const;

  /// <summary>
  /// Constructs an SQL query for inserting the backing strip specified in the
  /// drawingData object in this query object.
  /// </summary>
  /// <param name="matID">The matID which is known only during the insert
  /// process, so much be passed from the database manager.</param> <returns>An
  /// SQL query string.</returns>
  std::string backingStripInsertQuery(unsigned matID) const;

  /// <summary>
  /// Constructs an SQL query for inserting the side irons specified in the
  /// drawingData object in this query object.
  /// </summary>
  /// <param name="matID">The matID which is known only during the insert
  /// process, so must be passed from the database manager.</param> <returns>An
  /// SQL query string.</returns>
  std::string sideIronInsertQuery(unsigned matID) const;

  /// <summary>
  /// Constructs an SQL query for inserting the materials and thicknesses
  /// specified in the drawingData object in this query object.
  /// </summary>
  /// <param name="matID">The matID which is known only during the insert
  /// process, so must be passed from the database manager.</param> <returns>An
  /// SQL query string.</returns>
  std::string thicknessInsertQuery(unsigned matID) const;

  /// <summary>
  /// Constructs an SQL query for inserting the overlaps specified in the
  /// drawingData object in this query object.
  /// </summary>
  /// <param name="matID">The matID which is known only during the insert
  /// process, so must be passed from the database manager.</param> <returns>An
  /// SQL query string or an empty string if there are no overlaps to
  /// insert.</returns>
  std::string overlapsInsertQuery(unsigned matID) const;

  /// <summary>
  /// Constructs an SQL query for inserting the sidelaps specified in the
  /// drawingData object in this query object.
  /// </summary>
  /// <param name="matID">The matID which is known only during the insert
  /// process, so must be passed from the database manager.</param> <returns>An
  /// SQL query string or an empty string if there are no sidelaps to
  /// insert.</returns>
  std::string sidelapsInsertQuery(unsigned matID) const;

  /// <summary>
  /// Constructs an SQL query for inserting the punch programs specified in the
  /// drawingData object in this query object.
  /// </summary>
  /// <param name="matID">The matID which is known only during the insert
  /// process, so must be passed from the database manager.</param> <returns>An
  /// SQL query string or an empty string if there are no punch programs to
  /// insert.</returns>
  std::string punchProgramsInsertQuery(unsigned matID) const;

  /// <summary>
  /// Constructs an SQL query for inserting the impact pads specified in the
  /// drawingData object in this query object.
  /// </summary>
  /// <param name="matID">The matID which is known only during the insert
  /// process, so must be passed from the database manager.</param> <returns>An
  /// SQL query string or an empty string if there are no impact pads to
  /// insert.</returns>
  std::string impactPadsInsertQuery(unsigned matID) const;

  /// <summary>
  /// Constructs an SQL Query for inserting dam bars specified in the
  /// drawingData object in this query object.
  /// </summary>
  /// <param name="matID">The matID which is known only during the insert
  /// process, so must be passed from the database manager.</param> <returns>An
  /// SQL query string</returns>
  std::string damBarInsertQuery(unsigned matID) const;

  /// <summary>
  /// Constructs an SQL Query for inserting blank spaces specified in the
  /// drawingData object in this query object.
  /// </summary>
  /// <param name="matID">The matID which is known only during the insert
  /// process, so must be passed from the database manager.</param> <returns>An
  /// SQL query string</returns>
  std::string blankSpaceInsertQuery(unsigned matID) const;

  /// <summary>
  /// Constructs an SQL Query for inserting extra apertures specified in the
  /// drawingData object in this query object.
  /// </summary>
  /// <param name="matID">The matID which is known only during the insert
  /// process, so must be passed from the database manager.</param> <returns>An
  /// SQL query string.</returns>
  std::string extraApertureInsertQuery(unsigned matID) const;

  /// <summary>
  /// Constructs an SQL query for inserting the centre holes specified in the
  /// drawingData object in this query object.
  /// </summary>
  /// <param name="matID">The matID which is known only during the insert
  /// process, so must be passed from the database manager.</param> <returns>An
  /// SQL query string or an empty string if there are no centre holes to
  /// insert.</returns>
  std::string centreHolesInsertQuery(unsigned matID) const;

  /// <summary>
  /// Constructs an SQL query for inserting the deflectors specified in the
  /// drawingData object in this query object.
  /// </summary>
  /// <param name="matID">The matID which is known only during the insert
  /// process, so must be passed from the database manager.</param> <returns>An
  /// SQL query string or an empty string if there are no deflectors to
  /// insert.</returns>
  std::string deflectorsInsertQuery(unsigned matID) const;

  /// <summary>
  /// Constructs an SQL query for inserting the divertors specified in the
  /// drawingData object in this query object.
  /// </summary>
  /// <param name="matID">The matID which is known only during the insert
  /// process, so must be passed from the database manager.</param> <returns>An
  /// SQL query string or an empty string if there are no divertors to
  /// insert.</returns>
  std::string divertorsInsertQuery(unsigned matID) const;

  /// <summary>
  /// The optional drawingData object. If this object is set, this is a
  /// request query, and the drawingData represents the drawing we wish to
  /// insert. If this is a response, this is set to nullopt (as the client
  /// already knows the drawing they tried to insert so there is no point in
  /// sending it back).
  /// </summary>
  std::optional<Drawing> drawingData;

  /// <summary>
  /// The response code for the insert query.
  /// See above for the meaning of each code.
  /// </summary>
  InsertResponseCode insertResponseCode = NONE;

  /// <summary>
  /// The response code for this drawing insertion, so the client can match up a
  /// response to a request. This is used to accommodate the asynchronous model
  /// used.
  /// </summary>
  unsigned responseEchoCode;

  /// <summary>
  /// The mode of the drawing insert. Defaults to InsertMode::ADD.
  /// </summary>
  InsertMode insertMode = InsertMode::ADD;

 private:
  // Boolean value containing whether or not this drawing insertion is in
  // forcing mode
  bool force = false;
};

/// <summary>
/// ComponentInsert
/// Inherits from DatabaseQuery. This type of query is for inserting a drawing
/// component into the database, for example to add a new aperture.
/// </summary>
class CORE_API ComponentInsert : public DatabaseQuery {
 public:
  /// <summary>
  /// An enum describing the change being made to a price in the Database.
  /// </summary>
  enum class PriceMode {
    /// <summary>
    /// Indicates the price is being newly added to the database.
    /// </summary>
    ADD,
    /// <summary>
    /// Indicates the price already exists and in being updated.
    /// </summary>
    UPDATE,
    /// <summary>
    /// Indicates the price exists and is being removed from the database.
    /// </summary>
    REMOVE
  };

  /// <summary>
  /// ApertureData
  /// Object representation of the aperture data needed to insert a new
  /// aperture. This is done to avoid creating a "true" Aperture object before
  /// one officially exists.
  /// </summary>
  struct ApertureData {
    /// <summary>
    /// The width of this aperture.
    /// </summary>
    float width;
    /// <summary>
    /// The length of this aperture.
    /// </summary>
    float length;
    /// <summary>
    /// The width of the base this aperture is on.
    /// </summary>
    unsigned baseWidth;
    /// <summary>
    /// The length of the base this aperture is on.
    /// </summary>
    unsigned baseLength;
    /// <summary>
    /// The quantity of aperture heads currently in storage.
    /// </summary>
    unsigned quantity;
    /// <summary>
    /// The component ID of the shape of this aperture.
    /// </summary>
    unsigned shapeID;
    /// <summary>
    /// The component ID of the aperture this is based off, it this is a nibble.
    /// </summary>
    std::optional<unsigned> nibbleApertureId;

    /// <summary>
    /// Getter for the size of this aperture when serialised.
    /// </summary>
    /// <returns>The serialised size of this aperture.</returns>
    inline constexpr unsigned serialisedSize() const {
      return (nibbleApertureId.has_value() ? sizeof(unsigned) : 0) +
             (sizeof(float) * 2 + sizeof(unsigned) * 4 + sizeof(bool));
    }
  };

  /// <summary>
  /// MachineData
  /// Object representation of the aperture data needed to insert a new machine.
  /// This is done to avoid creating a "true" Machine object before one
  /// officially exists.
  /// </summary>
  struct MachineData {
    /// <summary>
    /// The manufacturer of this machine.
    /// </summary>
    std::string manufacturer;
    /// <summary>
    /// The model of this machine.
    /// </summary>
    std::string model;

    /// <summary>
    /// Getter for the size of this machine when serialised.
    /// </summary>
    /// <returns>The serialised size of this machine.</returns>
    inline unsigned serialisedSize() const {
      return (unsigned)(sizeof(unsigned char) + manufacturer.size() +
             sizeof(unsigned char) + model.size());
    }
  };

  /// <summary>
  /// SideIronData
  /// Object representation of the aperture data needed to insert a new side
  /// iron. This is done to avoid creating a "true" SideIron object before one
  /// officially exists.
  /// </summary>
  struct SideIronData {
    /// <summary>
    /// The type of side iron this is.
    /// </summary>
    SideIronType type;
    /// <summary>
    /// The length of this side iron.
    /// </summary>
    unsigned length;
    /// <summary>
    /// The drawing number of this side iron.
    /// </summary>
    std::string drawingNumber;
    
    /// <summary>
    /// The hyperlink to the drawing of this side iron.
    /// </summary>
    std::filesystem::path hyperlink;

    /// <summary>
    /// True if this side iron is for extraflex mats, false otherwise.
    /// </summary>
    bool extraflex;
    /// <summary>
    /// The price of this side iron, if it exists.
    /// </summary>
    std::optional<float> price;
    /// <summary>
    /// The number of screws this side iron has, if it exists.
    /// </summary>
    std::optional<unsigned> screws;

    /// <summary>
    /// Getter for the size of this side iron when serialised.
    /// </summary>
    /// <returns>The serialised size of this side iron.</returns>
    inline unsigned serialisedSize() const {
      unsigned total = (unsigned)(sizeof(SideIronType) + sizeof(unsigned) +
                       sizeof(unsigned char) + drawingNumber.size() +
                       sizeof(unsigned char) +
                       hyperlink.generic_string().size()) + sizeof(bool);
      total += sizeof(bool);
      if (price.has_value()) total += sizeof(float);
      total += sizeof(bool);
      if (screws.has_value()) total += sizeof(unsigned);
      return total;
    }
  };

  /// <summary>
  /// The type used to send insert queries to the database.
  /// </summary>
  struct StrapData {
    /// <summary>
    /// stores the handle of the material this strap is based upon.
    /// </summary>
    unsigned materialHandle;
    /// <summary>
    /// True if this strap is a wear tile liner, false otherwise.
    /// </summary>
    bool isWTL;

    /// <summary>
    /// Gets the serialised size of a strap, for networking.
    /// </summary>
    /// <returns>The serialised size of this object.</returns>
    inline unsigned serialisedSize() const {
      return sizeof(unsigned) + sizeof(bool);
    };
  };

  /// <summary>
  /// SideIronPriceData
  /// An object that can find a side iron matching the description stored and
  /// modify its price.
  /// </summary>
  struct SideIronPriceData {
    /// <summary>
    /// The type of side iron this pricing data appies to.
    /// </summary>
    SideIronType type;
    /// <summary>
    /// The lowest length a side iron can be and still be priced the same.
    /// </summary>
    unsigned lowerLength;
    /// <summary>
    /// The highest length a side iron can be and still be priced the same.
    /// </summary>
    unsigned upperLength;
    /// <summary>
    /// The price of all side irons that fit into these restrictions.
    /// </summary>
    float price;
    /// <summary>
    /// True if the side iron is meant for extraflex mats, false otherwise.
    /// </summary>
    bool extraflex;
    /// <summary>
    /// The mode this component insert is targetted to be.
    /// </summary>
    PriceMode priceMode;
    /// <summary>
    /// The component ID of this side iron price, if exists.
    /// </summary>
    std::optional<unsigned> componentID = std::nullopt;

    /// <summary>
    /// Getter for the size of this side iron price when serialised.
    /// </summary>
    /// <returns>The serialised size of this side iron price.</returns>
    inline unsigned serialisedSize() const {
      return sizeof(SideIronType) + sizeof(bool) * 2 + sizeof(float) +
             sizeof(unsigned) * 3 + sizeof(PriceMode) +
             (componentID.has_value() ? sizeof(unsigned) : 0);
    }
  };

  /// <summary>
  /// SpecificSideIronPriceData
  /// An object that uses a side iron's id to directly modify pricing data about
  /// it, specifically how many screws it requires.
  /// </summary>
  struct SpecificSideIronPriceData {
    /// <summary>
    /// The mode this component insert is targetted to be.
    /// </summary>
    PriceMode priceMode;
    /// <summary>
    /// The component ID of the side iron this price is for.
    /// </summary>
    unsigned sideIronComponentID;
    /// <summary>
    /// The price of this side iron.
    /// </summary>
    float price;
    /// <summary>
    /// The number of screws this side iron requires to be attached.
    /// </summary>
    unsigned screws;

    /// <summary>
    /// Getter for the size of this specific side iron price when serialised.
    /// </summary>
    /// <returns>The serialised size of this specific side iron price.</returns>
    inline unsigned serialisedSize() const {
      return sizeof(unsigned) * 2 + sizeof(float) + sizeof(priceMode);
    }
  };

  /// <summary>
  /// MaterialData
  /// Object representation of the aperture data needed to insert a new
  /// material. This is done to avoid creating a "true" Material object before
  /// one officially exists.
  /// </summary>
  struct MaterialData {
    /// <summary>
    /// The name of this material.
    /// </summary>
    std::string materialName;
    /// <summary>
    /// The hardness of this material.
    /// </summary>
    unsigned hardness;
    /// <summary>
    /// The thickness of this material.
    /// </summary>
    unsigned thickness;

    /// <summary>
    /// Getter for the size of this material when serialised.
    /// </summary>
    /// <returns>The serialised size of this material.</returns>
    inline unsigned serialisedSize() const {
      return sizeof(unsigned char) + materialName.size() + sizeof(unsigned) * 2;
    }
  };

  /// <summary>
  /// MaterialPriceData
  /// Object to modify the price attached to a specific type of material,
  /// needing to know the size the materials are ordered in, their size and the
  /// price.
  /// </summary>
  struct MaterialPriceData {
    /// <summary>
    /// The component ID of this price.
    /// </summary>
    unsigned material_price_id;
    /// <summary>
    /// The component ID of the material this price is for.
    /// </summary>
    unsigned material_id;
    /// <summary>
    /// The width of the source of this material, e.g. the width of the roll.
    /// </summary>
    float width;
    /// <summary>
    /// For all price modes but MaterialPricingType::SHEET, this is 0. For
    /// sheets, this is the length of the sheet.
    /// </summary>
    float length;
    /// <summary>
    /// The price of this material.
    /// </summary>
    float price;
    /// <summary>
    /// How the material is priced.
    /// </summary>
    MaterialPricingType pricingType;
    /// <summary>
    /// The mode this component insert is targetted to be.
    /// </summary>
    PriceMode priceMode;

    /// <summary>
    /// Getter for the size of this material price when serialised.
    /// </summary>
    /// <returns>The serialised size of this material price.</returns>
    inline unsigned serialisedSize() const {
      return sizeof(unsigned) * 2 + sizeof(float) * 3 +
             sizeof(MaterialPricingType) + sizeof(PriceMode);
    }
  };

  /// <summary>
  /// Allows modification of "extra" prices, prices not directly attributed to
  /// any individual object, such as screws or tackyback glue.
  /// </summary>
  struct ExtraPriceData {
    /// <summary>
    /// The component ID of this extra price.
    /// </summary>
    unsigned priceId;
    /// <summary>
    /// The type of this extra price.
    /// </summary>
    ExtraPriceType type;
    /// <summary>
    /// The price of this extra price.
    /// </summary>
    float price;
    /// <summary>
    /// The amount of square metres aquired per price.
    /// </summary>
    std::optional<float> squareMetres;
    /// <summary>
    /// The amount aquired per price.
    /// </summary>
    std::optional<unsigned> amount;
    /// <summary>
    /// Default constructor for a new ExtraPriceData.
    /// </summary>
    ExtraPriceData(){};

    /// <summary>
    /// Constructs a new ExtraPriceData from all components.
    /// </summary>
    /// <param name="id">The id of the new ExtraPriceData.</param>
    /// <param name="type">The type of extra price it is.</param>
    /// <param name="price">The current price.</param>
    /// <param name="squareMetres">The amount of square metre coverage, or
    /// std::nullopt.</param> <param name="amount">The amount, or
    /// std::nullopt.</param>
    ExtraPriceData(unsigned id, ExtraPriceType type, float price,
                   std::optional<float> squareMetres,
                   std::optional<unsigned> amount) {
      priceId = id;
      this->type = type;
      this->price = price;
      this->squareMetres = squareMetres;
      this->amount = amount;
      }

    /// <summary>
    /// Getter for the size of this extra price when serialised.
    /// </summary>
    /// <returns>The serialised size of this extra price.</returns>
    inline unsigned serialisedSize() const {
      return sizeof(unsigned) + sizeof(ExtraPriceType) + sizeof(float) +
             sizeof(bool) * 2 + (squareMetres.has_value() ? sizeof(float) : 0) +
             (amount.has_value() ? sizeof(unsigned) : 0);
    };

  };

  /// <summary>
  /// Allows modification of the data reflecting costs and time commitment of
  /// labour.
  /// </summary>
  struct LabourTimeData {
    /// <summary>
    /// The component ID of this labour time.
    /// </summary>
    unsigned labourId;
    /// <summary>
    /// The name of the job this time relates to.
    /// </summary>
    std::string job;
    /// <summary>
    /// The amount of time taken to complete this job.
    /// </summary>
    unsigned time;

    /// <summary>
    /// Default constructor for an empty LabourTimeData.
    /// </summary>
    LabourTimeData(){};

    /// <summary>
    /// Constructs a full LabourTimeData ready to be used by the server.
    /// </summary>
    /// <param name="id"></param>
    /// <param name="job"></param>
    /// <param name="time"></param>
    LabourTimeData(unsigned int id, std::string job, unsigned int time) {
      labourId = id;
      this->job = job;
      this->time = time;
    };

    /// <summary>
    /// Getter for the size of this labour time when serialised.
    /// </summary>
    /// <returns>The serialised size of this labour time.</returns>
    inline unsigned serialisedSize() const {
      return sizeof(unsigned) * 2 + job.size() + sizeof(size_t);
    };
  };
  /// <summary>
  /// Allows modification of the prices relating to powder coating.
  /// </summary>
  struct PowderCoatingPriceData {
    /// <summary>
    /// The component ID of this powder coating price.
    /// </summary>
    unsigned priceID;
    /// <summary>
    /// The price to coat the hooks.
    /// </summary>
    float hookPrice;
    /// <summary>
    /// The price to coat the straps.
    /// </summary>
    float strapPrice;

    /// <summary>
    /// Default constructor of an empty powder coating price data.
    /// </summary>
    PowderCoatingPriceData(){};

    /// <summary>
    /// Constructor of a full powder coating price data that is ready for the
    /// server.
    /// </summary>
    /// <param name="id"></param>
    /// <param name="hookPrice"></param>
    /// <param name="strapPrice"></param>
    PowderCoatingPriceData(unsigned id, float hookPrice, float strapPrice) {
      priceID = id;
      this->hookPrice = hookPrice;
      this->strapPrice = strapPrice;
    }

    /// <summary>
    /// Getter for the size of this powder coating price when serialised.
    /// </summary>
    /// <returns>The serialised size of this powde coating price.</returns>
    inline unsigned serialisedSize() const {
      return sizeof(unsigned) + sizeof(float) * 2;
    }
  };

  /// <summary>
  /// ComponentInsertResponse
  /// Represents a response code from the server which depends on whether the
  /// component was successfully added to the database or not. The code "None"
  /// is used in the request.
  /// </summary>
  enum class ComponentInsertResponse {
    /// <summary>
    /// There was no response to the components insertion.
    /// </summary>
    NONE,
    /// <summary>
    /// The insertion was a success.
    /// </summary>
    SUCCESS,
    /// <summary>
    /// The insertion failed.
    /// </summary>
    FAILED
  };

  /// <summary>
  /// Constructor for ComponentInsert
  /// </summary>
  ComponentInsert();

  /// <summary>
  /// Serialise this object into the target buffer
  /// </summary>
  /// <param name="target">The buffer to write this serialises object
  /// into.</param>
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
  /// <param name="data">The buffer to read this object from, as a rvalue to
  /// indicate transfer of ownership.</param> <returns>A newly constructed query
  /// object equivalent to the one the buffer was created with.</returns>
  static ComponentInsert &deserialise(void *&&data);

  /// <summary>
  /// Sets the data for the component to be added to the database
  /// </summary>
  /// <typeparam name="T">The type of component data to add (e.g.
  /// ApertureData)</typeparam> <param name="data">The component value itself to
  /// add to the database</param>
  template <typename T>
  void setComponentData(T &&data);

  /// <summary>
  /// Clears the component data for this object
  /// </summary>
  void clearComponentData();

  /// <summary>
  /// Creates an SQL query string for inserting this object into the database.
  /// This will depend on the type of object specified to add.
  /// </summary>
  /// <returns>The SQL query string used to add the new component to the
  /// database.</returns>
  std::string toSQLQueryString() const;

  /// <summary>
  /// Getter for the appropriate request code
  /// </summary>
  /// <returns>The appropriate request type code depending on what sort of
  /// component this object inserted.</returns>
  RequestType getSourceTableCode() const;

  /// <summary>
  /// The response code for the object so the client can determine whether the
  /// request was successful.
  /// </summary>
  ComponentInsertResponse responseCode = ComponentInsertResponse::NONE;

 private:
  /// <summary>
  /// InsertType
  /// Flag variable to simply determine which type of component the query
  /// intended to add.
  /// </summary>
  enum class InsertType {
    NONE,
    APERTURE,
    MACHINE,
    SIDE_IRON,
    MATERIAL,
    MATERIAL_PRICE,
    SIDE_IRON_PRICE,
    EXTRA_PRICE,
    LABOUR_TIMES,
    SPECIFIC_SIDE_IRON_PRICE,
    POWDER_COATING,
    STRAP
  };

  // The realisation of the insert type variable. Defaults to None
  InsertType insertType = InsertType::NONE;

  // Holds 4 different optionals for each different potential type. At any
  // point, only one of these should contain a value, and if multiple do, then
  // the insertType variable dictates which is correct
  std::optional<ApertureData> apertureData;
  std::optional<MachineData> machineData;
  std::optional<SideIronData> sideIronData;
  std::optional<SideIronPriceData> sideIronPriceData;
  std::optional<SpecificSideIronPriceData> specificSideIronPriceData;
  std::optional<MaterialData> materialData;
  std::optional<MaterialPriceData> materialPriceData;
  std::optional<StrapData> strapData;
  std::optional<ExtraPriceData> extraPriceData;
  std::optional<LabourTimeData> labourTimeData;
  std::optional<PowderCoatingPriceData> powderCoatingPriceData;
};

/// <summary>
/// DatabaseBackup
/// An object representing a user request to create a backup of the database in
/// the form of a MySQL dump. The server's DatabaseManager is responsible for
/// creating this backup, and the location to write to is determined in the
/// server's meta file.
/// </summary>
class CORE_API DatabaseBackup : DatabaseQuery {
 public:
  /// <summary>
  /// BackupResponse
  /// Response code for whether the backup request was successful.
  /// </summary>
  enum class BackupResponse {
    /// <summary>
    /// There was no response to a backup request.
    /// </summary>
    NONE,
    /// <summary>
    /// The backup was successful.
    /// </summary>
    SUCCESS,
    /// <summary>
    /// The backup failed.
    /// </summary>
    FAILED
  };

  /// <summary>
  /// Default constructor
  /// </summary>
  DatabaseBackup() = default;

  /// <summary>
  /// Serialise this object into the target buffer
  /// </summary>
  /// <param name="target">The buffer to write this serialises object
  /// into.</param>
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
  /// <param name="data">The buffer to read this object from, as a rvalue to
  /// indicate transfer of ownership.</param> <returns>A newly constructed query
  /// object equivalent to the one the buffer was created with.</returns>
  static DatabaseBackup &deserialise(void *&&data);

  /// <summary>
  /// The response code for this object. Defaults to None indicating that the
  /// object represents a reques.
  /// </summary>
  BackupResponse responseCode = BackupResponse::NONE;

  /// <summary>
  /// The string name to save the backup under
  /// </summary>
  std::string backupName;

 private:
};

/// <summary>
/// NextDrawing
/// A request object to obtain the name of the next drawing that should be
/// added. This can either be the next automatic drawing or the next manual
/// drawing based on the current database state.
/// </summary>
class CORE_API NextDrawing : DatabaseQuery {
 public:
  /// <summary>
  /// DrawingType
  /// Simply determines whether the concerned drawing number query should return
  /// the next automatic or manual drawing number.
  /// </summary>
  enum class DrawingType {
    /// <summary>
    /// This drawing is for automatic presses.
    /// </summary>
    AUTOMATIC,
    /// <summary>
    /// This drawing is for manual presses.
    /// </summary>
    MANUAL
  };

  /// <summary>
  /// Default constructor
  /// </summary>
  NextDrawing() = default;

  /// <summary>
  /// Serialise this object into the target buffer
  /// </summary>
  /// <param name="target">The buffer to write this serialises object
  /// into.</param>
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
  /// <param name="data">The buffer to read this object from, as a rvalue to
  /// indicate transfer of ownership.</param> <returns>A newly constructed query
  /// object equivalent to the one the buffer was created with.</returns>
  static NextDrawing &deserialise(void *&&data);

  /// <summary>
  /// Realisation of the DrawingType. Defaults to automatic, but should probably
  /// be set explictitly.
  /// </summary>
  DrawingType drawingType = DrawingType::AUTOMATIC;

  /// <summary>
  /// The string variable containing the drawing number. This is used only in
  /// the response part of the query.
  /// </summary>
  std::optional<std::string> drawingNumber = std::nullopt;

 private:
};

#endif  // DATABASE_MANAGER_DATABASEQUERY_H
