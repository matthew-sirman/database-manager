//
// Created by matthew on 09/07/2020.
//

#ifndef DATABASE_MANAGER_DATABASEQUERY_H
#define DATABASE_MANAGER_DATABASEQUERY_H

#include <cppconn/resultset.h>

#include <string>
#include <cstring>
#include <optional>
#include <chrono>
#include <iomanip>
#include <vector>

#include <nlohmann/json.hpp>

#include "Drawing.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

template<typename T>
struct ValueRange {
    T lowerBound, upperBound;

    void serialise(void *buffer) const;

    static ValueRange<T> deserialise(void *buffer);

    unsigned serialisedSize() const;
};

template<typename T>
void ValueRange<T>::serialise(void *buffer) const {
    T *buff = (T *) buffer;
    *buff++ = lowerBound;
    *buff = upperBound;
}

template<typename T>
ValueRange<T> ValueRange<T>::deserialise(void *buffer) {
    T *buff = (T *) buffer;
    ValueRange<T> range;
    range.lowerBound = *buff++;
    range.upperBound = *buff;

    return range;
}

template<typename T>
unsigned ValueRange<T>::serialisedSize() const {
    return 2 * sizeof(T);
}

struct Date {
    unsigned short year;
    unsigned char month, day;

    std::string toMySQLDateString() const;
} __attribute__((packed));

class DatabaseQuery {
public:
    DatabaseQuery();

    virtual void serialise(void *target) const = 0;

    virtual unsigned serialisedSize() const = 0;

    void *createBuffer(unsigned &size) const;
};

class DatabaseSearchQuery : public DatabaseQuery {
public:
    DatabaseSearchQuery();

    void serialise(void *target) const override;

    unsigned serialisedSize() const override;

    static DatabaseSearchQuery &deserialise(void *data);

    std::string toSQLQueryString() const;

    static std::vector<DrawingSummary> getQueryResultSummaries(sql::ResultSet *resultSet);

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
    std::optional<unsigned char> quantityOnDeck;
    std::optional<std::string> position;
    std::optional<MachineDeck> machineDeck;

private:
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
        QUANTITY_ON_DECK = 0x00040000,
        POSITION = 0x00080000,
        MACHINE_DECK = 0x00100000
    };

    const std::string drawingsTableName = "drawings";
    const char *searchColumns[2] = {"", ""};

    unsigned getSearchParameters() const;
};


#endif //DATABASE_MANAGER_DATABASEQUERY_H
