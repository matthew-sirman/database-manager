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
    /// <param name="maxDrawingLength">A refernce to a variable to store the maximum drawing number length.
    /// Note: drawing numbers longer than 255 are disallowed by the database, so this value is used as a single byte.</param>
    void getCompressionSchemaDetails(unsigned &maxMatID, float &maxWidth, float &maxLength, float &maxLapSize, unsigned char &maxDrawingLength);

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

    std::string nextAutomaticDrawingNumber();

    std::string nextManualDrawingNumber();

    /// <summary>
    /// Closes the connection to the database.
    /// </summary>
    void closeConnection();

    void setErrorStream(std::ostream &stream = std::cerr);

private:
    // A session object for use during the lifetime of the DatabaseManager object.
    // This allows the manager to interact with the database.
    mysqlx::Session sess;
    std::string username, password;
    std::string database;

    std::ostream *errStream = &std::cerr;
};


#endif //DATABASE_MANAGER_DATABASEMANAGER_H
