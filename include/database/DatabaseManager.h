//
// Created by matthew on 06/07/2020.
//

#ifndef DATABASE_MANAGER_DATABASEMANAGER_H
#define DATABASE_MANAGER_DATABASEMANAGER_H

#include <mysqlx/xdevapi.h>

#include <vector>

#include "DatabaseQuery.h"
#include "Drawing.h"

#include "../util/format.h"

#include "../../guard.h"

/// <summary>
/// DatabaseManager
/// A class for managing connections to the MySQL database underlying the application.
/// Also contains methods for some specific actions such as reading values for a 
/// summary compression schema, and reading/writing drawings to the database.
/// </summary>
class DatabaseManager {
public:
    /// <summary>
    /// DrawingExistsResponse
    /// A simple enum for a response from a check if a drawing exists.
    /// Either returns EXISTS if the drawing is found, NOT_EXISTS if the
    /// drawing is not found or R_ERROR if there was an error in the process.
    /// </summary>
    enum class DrawingExistsResponse {
        EXISTS,
        NOT_EXISTS,
        R_ERROR
    };

    /// <summary>
    /// Constructor for DatabaseManager
    /// Note: the values called with are cached for possible reconnection to the database.
    /// </summary>
    /// <param name="database">The name of the database to connect to.</param>
    /// <param name="user">The username for the connection.</param>
    /// <param name="password">The password for the user specified by the username.</param>
    /// <param name="host">The server to connect to, where the database is located.</param>
    /// <returns></returns>
    DatabaseManager(const std::string &database, const std::string &user, const std::string &password,
        const std::string &host = "localhost");

    /// <summary>
    /// Reads details for a compression schema from the database, such as the maximum mat_id.
    /// Writes the results to the reference parameters passed in.
    /// </summary>
    /// <param name="maxMatID">A reference to a variable to store the maximum mat_id.</param>
    /// <param name="maxWidth">A reference to a variable to store the maximum width.</param>
    /// <param name="maxLength">A reference to a variable to store the maximum length.</param>
    /// <param name="maxLapSize">A reference to a variable to store the maxmimum lap size</param>
    /// <param name="maxBarSpacingCount">A reference to a variable to store the maximum number of bar spacings on any mat</param>
    /// <param name="maxBarSpacing">A reference to a variable to store the maximum bar spacing</param>
    /// <param name="maxDrawingLength">A refernce to a variable to store the maximum drawing number length.
    /// Note: drawing numbers longer than 255 are disallowed by the database, so this value is used as a single byte.</param>
    void getCompressionSchemaDetails(unsigned &maxMatID, float &maxWidth, float &maxLength, float &maxLapSize, 
                                     unsigned char &maxBarSpacingCount, float &maxBarSpacing, unsigned char &maxDrawingLength);

    /// <summary>
    /// Executes a search query based on a DatabaseSearchQuery object parameterisation.
    /// </summary>
    /// <param name="query">A query object containing the parameters for the search.</param>
    /// <returns>A list (vector) of summaries for each drawing in the database which matches the search query.</returns>
    std::vector<DrawingSummary> executeSearchQuery(const DatabaseSearchQuery &query);

    /// <summary>
    /// Executes a data retrieval query for a specific drawing request.
    /// </summary>
    /// <param name="query">A request object containing the identifier for a specific drawing.</param>
    /// <returns>The full details about the requested drawing from the database.</returns>
    Drawing *executeDrawingQuery(const DrawingRequest &query);

    /// <summary>
    /// Sources all columns and all rows for a particular table.
    /// </summary>
    /// <param name="tableName">The string name of the table we wish to source.</param>
    /// <param name="orderBy">An optional string to order the results from the query.</param>
    /// <returns>The rows from the table.</returns>
    mysqlx::RowResult sourceTable(const std::string &tableName, const std::string &orderBy = std::string());

    /// <summary>
    /// Inserts a drawing into the database based upon the passed in DrawingInsert object, which contains
    /// a Drawing object.
    /// </summary>
    /// <param name="insert">An object contianing metadata about this drawing as well as the drawing data itself.</param>
    /// <returns>A boolean indicating the success of inputting the drawing. Returns false if the drawing itself fails 
    /// a validity check, if the drawing data is missing, if the drawing is already in the database and the DrawingInsert
    /// object doesn't indicate forcing mode or if there was a MySQL error. </returns>
    bool insertDrawing(const DrawingInsert &insert);

    /// <summary>
    /// Checks whether a given drawing already exists in the database.
    /// </summary>
    /// <param name="drawingNumber">A string containing the drawing number to check for presence.</param>
    /// <returns>A DrawingExistsResponse enum value which will either state whether or not the drawing exists,
    /// or returin an error code if there was an error. </returns>
    DrawingExistsResponse drawingExists(const std::string &drawingNumber);

    /// <summary>
    /// Adds a new component to the database, for example a new aperture.
    /// </summary>
    /// <param name="insert">An object containing the information required to construct a MySQL query string
    /// to add the component to the database.</param>
    /// <returns>A boolean indicating whether the insertion was successful. Returns true if successful, or false if there
    /// was an error during the transaction.</returns>
    bool insertComponent(const ComponentInsert &insert);

    /// <summary>
    /// Creates a backup of the drawings database
    /// </summary>
    /// <param name="backupLocation">The file to create the backup in.</param>
    /// <returns>A boolean indicating whether the backup creation was successful.</returns>
    bool createBackup(const std::filesystem::path &backupLocation);

    /// <summary>
    /// Retrieves the current "newest" automatic drawing number from the database and calculates the proceeding
    /// drawing number.
    /// </summary>
    /// <returns>The calculated next automatic drawing number based on the current database state</returns>
    std::string nextAutomaticDrawingNumber();

    /// <summary>
    /// Retrieves the current "newest" manual drawing number from the database and calculates the proceeding
    /// drawing number.
    /// </summary>
    /// <returns>The calculated next manual drawing number based on the current database state</returns>
    std::string nextManualDrawingNumber();

    /// <summary>
    /// Closes the connection to the database.
    /// </summary>
    void closeConnection();

    /// <summary>
    /// Tests if the connection is broken
    /// <returns>Returns true if the connection is OK otherwise false</returns>
    /// </summary>
    bool testConnection();

    /// <summary>
    /// A getter for the current connection status
    /// </summary>
    /// <returns>Whether or not the server is currently connected to the database</returns>
    bool connected() const;

    /// <summary>
    /// Sets the output error stream to an arbitrary ostream
    /// </summary>
    /// <param name="stream">The ostream to write to. Defaults to stdcerr</param>
    void setErrorStream(std::ostream &stream = std::cerr);

private:
    // A session object for use during the lifetime of the DatabaseManager object.
    // This allows the manager to interact with the database.
    mysqlx::Session sess;
    // The cached username, password and database name for reconnecting if necessary.
    std::string username, password;
    std::string database;

    // A pointer to the output error stream to write to. Defaults to stdcerr
    std::ostream *errStream = &std::cerr;

    // Flag indicating whether or not the database is currently connected.
    bool isConnected;

    // Logs an error, e, to the errStream. If safe is set to false, the error will terminate the program.
    void logError(const mysqlx::Error &e, unsigned lineNumber = -1, bool safe = true);

    // Returns a string representation of the current time
    std::string timestamp() const;
};


#endif //DATABASE_MANAGER_DATABASEMANAGER_H
