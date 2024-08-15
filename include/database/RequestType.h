//
// Created by matthew on 11/07/2020.
//

#ifndef DATABASE_MANAGER_REQUESTTYPE_H
#define DATABASE_MANAGER_REQUESTTYPE_H

/// <summary>\ingroup database
/// All allowed request types for the network.
/// </summary>
enum class RequestType {
    REPEAT_TOKEN_REQUEST,
    USER_EMAIL_REQUEST,
    DRAWING_SEARCH_QUERY,
    DRAWING_INSERT,
    SOURCE_PRODUCT_TABLE,
    SOURCE_APERTURE_TABLE,
    SOURCE_APERTURE_SHAPE_TABLE,
    SOURCE_MATERIAL_TABLE,
    SOURCE_SIDE_IRON_TABLE,
    SOURCE_SIDE_IRON_PRICES_TABLE,
    SOURCE_MACHINE_TABLE,
    SOURCE_MACHINE_DECK_TABLE,
    SOURCE_EXTRA_PRICES_TABLE,
    SOURCE_BACKING_STRIPS_TABLE,
    DRAWING_DETAILS,
    ADD_NEW_COMPONENT,
    GET_NEXT_DRAWING_NUMBER,
    CREATE_DATABASE_BACKUP,
    SOURCE_LABOUR_TIMES_TABLE,
    SOURCE_POWDER_COATING_TABLE
};

/// <summary>\ingroup database
/// Enum with all ways of pricing materials.
/// </summary>
enum class MaterialPricingType {
    RUNNING_M,
    SQUARE_M,
    SHEET,

    TOTAL
};

/// <summary>\ingroup database
/// Enum with all extra prices.
/// </summary>
enum class ExtraPriceType {
    SIDE_IRON_NUTS,
    SIDE_IRON_SCREWS,
    TACKYBACK_GLUE,
    LABOUR,
    PRIMER
};

/// <summary>\ingroup database
/// Enum with all types of labour.
/// </summary>
enum class LabourType {
    CUTTING_AMOUNT,
    TIME_TO_PUNCH,
    TIME_TO_SHOD,
    TIME_TO_REBATE,
    BACKING_STRIPS,
    COVER_STRAPS,
    BONDED_OVERLAP,
    CUTTING_TO_SIZE,
    IMPACT_PADS,
    CENTRE_HOLES,
    DIVERTORS,
    DEFLECTORS,
    DAM_BARS,
    CUT_DOWN,
    ERR
};
#endif //DATABASE_MANAGER_REQUESTTYPE_H
