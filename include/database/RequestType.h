//
// Created by matthew on 11/07/2020.
//

#ifndef DATABASE_MANAGER_REQUESTTYPE_H
#define DATABASE_MANAGER_REQUESTTYPE_H

 ///\file

/// <summary>
/// All allowed request types for the network.
/// </summary>
enum class RequestType {
    REPEAT_TOKEN_REQUEST, ///< A request for a repeat token to log in faster. These are valid until the server restarts.
    USER_EMAIL_REQUEST,///< Requests the users email address.

    /// <summary>
    /// A query to search for drawings, using drawing summaries. A DatabaseSearchQuery is sent across the network,
    /// and a set of DrawingSummary 's are returned to the client.
    /// </summary>
    DRAWING_SEARCH_QUERY,
    /// <summary>
    /// A query to insert a new drawing and relevant components to the table. If a previous drawing is being updated,
    /// the old drawing is wiped and reinserted.
    /// </summary>
    DRAWING_INSERT,
    /// <summary>
    /// Requests the server to update its products, which leads to a broadcast of the newly updated products table.
    /// </summary>
    SOURCE_PRODUCT_TABLE,
    /// <summary>
    /// Requests the server to update its apertures, which leads to a broadcast of the newly updated apertures table.
    /// </summary>
    SOURCE_APERTURE_TABLE,
    /// <summary>
    /// Requests the server to update its aperture shapes, which leads to a broadcast of the newly updated aperture shapes table.
    /// </summary>
    SOURCE_APERTURE_SHAPE_TABLE,
    /// <summary>
    /// Requests the server to update its materials, which leads to a broadcast of the newly updated materials table.
    /// </summary>
    SOURCE_MATERIAL_TABLE,
    /// <summary>
    /// Requests the server to update its side irons, which leads to a broadcast of the newly updated side irons table.
    /// </summary>
    SOURCE_SIDE_IRON_TABLE,
    /// <summary>
    /// Requests the server to update its side iron prices, which leads to a broadcast of the newly updated side iron prices table.
    /// </summary>
    SOURCE_SIDE_IRON_PRICES_TABLE,
    /// <summary>
    /// Requests the server to update its machines, which leads to a broadcast of the newly updated machines table.
    /// </summary>
    SOURCE_MACHINE_TABLE,
    /// <summary>
    /// Requests the server to update its machine decks, which leads to a broadcast of the newly updated machine decks table.
    /// </summary>
    SOURCE_MACHINE_DECK_TABLE,
    /// <summary>
    /// Requests the server to update its extra prices, which leads to a broadcast of the newly updated extra prices table.
    /// </summary>
    SOURCE_EXTRA_PRICES_TABLE,
    /// <summary>
    /// Requests the server to update its backing strips, which leads to a broadcast of the newly updated backing strips table.
    /// </summary>
    SOURCE_BACKING_STRIPS_TABLE,
    /// <summary>
    /// Requests the server to update its labour times, which leads to a broadcast of the newly updated side iron labour times table.
    /// </summary>
    SOURCE_LABOUR_TIMES_TABLE,
    /// <summary>
    /// Requests the server to update its powder coating prices, which leads to a broadcast of the newly updated powder coating prices table.
    /// </summary>
    SOURCE_POWDER_COATING_TABLE,
    /// <summary>
    /// Requests the details of a specific drawing. Sends a DrawingRequest across to the server, and the server sends the requested drawing
    /// back to the requesting client.
    /// </summary>
    DRAWING_DETAILS,
    /// <summary>
    /// Reuqest to add a new component to the database. The client sends a ComponentInsert to the server, which tries to fufill the request,
    /// then updates its table and broadcasts the newly updated table.
    /// </summary>
    ADD_NEW_COMPONENT,
    /// <summary>
    /// Requests a the next avaliable drawing number. The client sends a NextDrawing to the server, then the server responds with the next
    /// drawing number. Note this simply gets the highest drawing number and increments it, it does \a not fill any gaps from deleted drawings.
    /// </summary>
    GET_NEXT_DRAWING_NUMBER,
    /// <summary>
    /// Requests that the server makes the database dump into a sql file.
    /// </summary>
    CREATE_DATABASE_BACKUP,
    /// <summary>
    /// Requests the server to update its straps, which leads to a broadcast of the newly updated straps table.
    /// </summary>
    SOURCE_STRAPS_TABLE
};

/// <summary>
/// Enum with all ways of pricing materials.
/// </summary>
enum class MaterialPricingType {
    /// <summary>
    /// Indicates the price is per running metre. The price is for the amount of material of 1 metre of the longer side of the roll,
    /// and the full width of the roll.
    /// </summary>
    RUNNING_M,
    /// <summary>
    /// Indicates the price is per square metre. The price is simply based upon th area of used.
    /// </summary>
    SQUARE_M,
    /// <summary>
    /// Indicates the price is per sheet. The price is fixed for a sheet of material.
    /// </summary>
    SHEET
};

/// <summary>
/// Enum with all extra prices.
/// </summary>
enum class ExtraPriceType {
    /// <summary>
    /// Indicates the price is for side iron nuts. The price is per nut.
    /// </summary>
    SIDE_IRON_NUTS,
    /// <summary>
    /// Indicates the price is for side iron screws. The price is per screw.
    /// </summary>
    SIDE_IRON_SCREWS,
    /// <summary>
    /// Indicates the price is for tackyback glue. The price is per square metre of coverage.
    /// </summary>
    TACKYBACK_GLUE,
    /// <summary>
    /// Indicates the price is for labour. The price is per hour of labour.
    /// </summary>
    LABOUR,
    /// <summary>
    /// Indicates the price is for primer. The price is per square metre of coverage.
    /// </summary>
    PRIMER,
    /// <summary>
    /// Indicates the price is for shot blasting. This item is per unit.
    /// </summary>
    SHOT_BLASTING
};

/// <summary>
/// Enum with all types of labour.
/// </summary>
enum class LabourType {
    /// <summary>
    /// Relates to labour costs of cutting side irons down to size.
    /// </summary>
    CUTTING_AMOUNT,
    /// <summary>
    /// Relates to the amount of time taken to punch a mat on the autopress.
    /// </summary>
    TIME_TO_PUNCH,
    /// <summary>
    /// Relates to the amount of time taken to shod a mat.
    /// </summary>
    TIME_TO_SHOD,
    /// <summary>
    /// Relates to the amount of time taken to rebate a mat.
    /// </summary>
    TIME_TO_REBATE,
    /// <summary>
    /// Relates to the time taken to apply the backing strips to a mat.
    /// </summary>
    BACKING_STRIPS,
    /// <summary>
    /// Relates to the time taken to cover the backing straps on a mat.
    /// </summary>
    COVER_STRAPS,
    /// <summary>
    /// Relates to the time taken to bond overlaps onto a mat.
    /// </summary>
    BONDED_OVERLAP,
    /// <summary>
    /// Relates to the time taken to cut down a roll of material to the correct size.
    /// </summary>
    CUTTING_TO_SIZE,
    /// <summary>
    /// Relates to the time taken to apply an impact pad to a mat.
    /// </summary>
    IMPACT_PADS,
    /// <summary>
    /// Relates to the time taken to cut centre holes out of a mat.
    /// </summary>
    CENTRE_HOLES,
    /// <summary>
    /// Relates to the time taken to add divertors to the mat.
    /// </summary>
    DIVERTORS,
    /// <summary>
    /// Relates to the time taken to add deflectors to the mat.
    /// </summary>
    DEFLECTORS,
    /// <summary>
    /// Relates to the time taken to add dam bars to the mat.
    /// </summary>
    DAM_BARS,
    /// <summary>
    /// Relates to the time taken to cut down.
    /// </summary>
    CUT_DOWN,
    /// <summary>
    /// An error has occured.
    /// </summary>
    ERR
};
#endif //DATABASE_MANAGER_REQUESTTYPE_H
