//
// Created by matthew on 09/07/2020.
//

#include "../../include/database/DatabaseQuery.h"
// Default constructor for the DatabaseQuery object
DatabaseQuery::DatabaseQuery() = default;

// Helper function to create a buffer and serialise to it
void *DatabaseQuery::createBuffer(unsigned &size) const {
    // Get the size this object will occupy when serialised and write
    // to the reference variable
    size = serialisedSize();
    // Create a suitably sized buffer (on the heap for persistence)
    void *buffer = malloc(size);
    // Serialise this object into the buffer. Note that this will call the overridden
    // serialise function, as the base function in DatabaseQuery is pure virtual.
    serialise(buffer);
    // Return the created and filled buffer
    return buffer;
}

// Default constructor for the DatabaseSearchQuery object
DatabaseSearchQuery::DatabaseSearchQuery() = default;

// Serialisation function for writing the search query to a target buffer
void DatabaseSearchQuery::serialise(void *target) const {
    // First we cast the buffer to a byte pointer so we can perform pointer addition etc.
    unsigned char *buffer = (unsigned char *) target;
    // We first write the request type to the buffer. This is a search query, so we write
    // as such.
    *((RequestType *) buffer) = RequestType::DRAWING_SEARCH_QUERY;
    buffer += sizeof(RequestType);
    // Write the search parameters object into the buffer. This flags which
    // parameters are used by this query
    *((unsigned *) buffer) = getSearchParameters();
    buffer += sizeof(unsigned);

    if (drawingNumber.has_value()) {
        // If the drawingNumber parameter is specified,
        // we write the drawing number string preceded by its size to the
        // buffer.
        unsigned char sendDNumSize = MIN(255, drawingNumber->size());
        *buffer++ = sendDNumSize;
        memcpy(buffer, drawingNumber->c_str(), sendDNumSize);
        buffer += sendDNumSize;
    }
    if (width.has_value()) {
        // If the width range parameter is specified,
        // we write the it to the buffer.
        width->serialise(buffer);
        buffer += width->serialisedSize();
    }
    if (length.has_value()) {
        // If the length range parameter is specified,
        // we write it to the buffer.
        length->serialise(buffer);
        buffer += length->serialisedSize();
    }
    if (productType.has_value()) {
        // If the product type parameter is specified,
        // we write it to the buffer.
        productType->serialise(buffer);
        buffer += productType->serialisedSize();
    }
    if (numberOfBars.has_value()) {
        // If the number of bars parameter is specified,
        // we write it to the buffer.
        // Note this number is fixed to be below 255, so it is
        // written as a single byte.
        *buffer++ = numberOfBars.value();
    }
    if (aperture.has_value()) {
        // If the aperture parameter is specified,
        // we write it to the buffer.
        aperture->serialise(buffer);
        buffer += aperture->serialisedSize();
    }
    if (topThickness.has_value()) {
        // If the top layer thickness parameter is specified,
        // we write it to the buffer.
        topThickness->serialise(buffer);
        buffer += topThickness->serialisedSize();
    }
    if (bottomThickness.has_value()) {
        // If the bottom layer thickness parameter is specified,
        // we write it to the buffer.
        bottomThickness->serialise(buffer);
        buffer += bottomThickness->serialisedSize();
    }
    if (dateRange.has_value()) {
        // If the date range parameter is specified,
        // we write it to the buffer.
        dateRange->serialise(buffer);
        buffer += dateRange->serialisedSize();
    }
    if (sideIronType.has_value()) {
        // If the side iron type parameter is specified,
        // we write it to the buffer.
        // Note that there are (a lot) less than 255 side iron
        // typtes, so we cast to a single byte for serialising.
        *buffer++ = (unsigned char) sideIronType.value();
    }
    if (sideIronLength.has_value()) {
        // If the side iron length parameter is specified,
        // we write it to the buffer.
        // Note that side iron lengths are all less than 65535
        // so can be sent as a short.
        *((unsigned short *) buffer) = sideIronLength.value();
        buffer += sizeof(unsigned short);
    }
    if (sidelapMode.has_value()) {
        // If the sidelap mode parameter is specified,
        // we write it to the buffer.
        // Note that there are few and fixed modes this can take,
        // so again we limit to a single byte
        *buffer++ = (unsigned char) sidelapMode.value();
    }
    if (overlapMode.has_value()) {
        // If the overlap mode parameter is specified,
        // we write it to the buffer.
        // Note that there are few and fixed modes this can take,
        // so again we limit to a single byte
        *buffer++ = (unsigned char) overlapMode.value();
    }
    if (sidelapWidth.has_value()) {
        // If the sidelap width range is specified,
        // we write it to the buffer.
        sidelapWidth->serialise(buffer);
        buffer += sidelapWidth->serialisedSize();
    }
    if (overlapWidth.has_value()) {
        // If the overlap width range is specified,
        // we write it to the buffer.
        overlapWidth->serialise(buffer);
        buffer += overlapWidth->serialisedSize();
    }
    if (sidelapAttachment.has_value()) {
        // If the sidelap attachment type range is specified,
        // we write it to the buffer.
        // Note there are few and fixed attachment modes,
        // so again we write a single byte
        *buffer++ = (unsigned char) sidelapAttachment.value();
    }
    if (overlapAttachment.has_value()) {
        // If the overlap attachment type range is specified,
        // we write it to the buffer.
        // Note there are few and fixed attachment modes,
        // so again we write a single byte
        *buffer++ = (unsigned char) overlapAttachment.value();
    }
    if (machine.has_value()) {
        // If the machine is specified,
        // we write it to the buffer.
        machine->serialise(buffer);
        buffer += machine->serialisedSize();
    }
    if (manufacturer.has_value()) {
        unsigned char sendManufacturer = MIN(255, manufacturer->size());
        *buffer++ = sendManufacturer;
        memcpy(buffer, manufacturer->c_str(), sendManufacturer);
        buffer += sendManufacturer;
    }
    if (quantityOnDeck.has_value()) {
        // If the quantity on deck is specified,
        // we write it to the buffer.
        *buffer++ = quantityOnDeck.value();
    }
    if (position.has_value()) {
        // If the position string is specified,
        // we write it to the buffer.
        unsigned char sendPosSize = MIN(255, position->size());
        *buffer++ = sendPosSize;
        memcpy(buffer, position->c_str(), sendPosSize);
        buffer += sendPosSize;
    }
    if (machineDeck.has_value()) {
        // If the machine deck is specified,
        // we write it to the buffer.
        machineDeck->serialise(buffer);
        buffer += machineDeck->serialisedSize();
    }
}

// Calculates the total amount of space this object will occupy in a buffer
unsigned DatabaseSearchQuery::serialisedSize() const {
    // We initialise the size to be the size of the header, which consists of
    // the RequestType for this search, as well as an unsigned for the parameters specified in
    // this search query
    unsigned size = sizeof(RequestType) + sizeof(unsigned);

    if (drawingNumber.has_value()) {
        // If we have a drawing number parameter, increment the size by the space it will take.
        size += sizeof(unsigned char) + drawingNumber->size();
    }
    if (width.has_value()) {
        // If we have a width parameter, increment the size by the space it will take.
        size += width->serialisedSize();
    }
    if (length.has_value()) {
        // If we have a length parameter, increment the size by the space it will take.
        size += length->serialisedSize();
    }
    if (productType.has_value()) {
        // If we have a product type parameter, increment the size by the space it will take.
        size += productType->serialisedSize();
    }
    if (numberOfBars.has_value()) {
        // If we have a number of bars parameter, increment the size by the space it will take.
        size += sizeof(unsigned char);
    }
    if (aperture.has_value()) {
        // If we have an aperture parameter, increment the size by the space it will take.
        size += aperture->serialisedSize();
    }
    if (topThickness.has_value()) {
        // If we have a top thickness parameter, increment the size by the space it will take.
        size += topThickness->serialisedSize();
    }
    if (bottomThickness.has_value()) {
        // If we have a bottom thickness parameter, increment the size by the space it will take.
        size += bottomThickness->serialisedSize();
    }
    if (dateRange.has_value()) {
        // If we have a date range parameter, increment the size by the space it will take.
        size += dateRange->serialisedSize();
    }
    if (sideIronType.has_value()) {
        // If we have a side iron type parameter, increment the size by the space it will take.
        size += sizeof(unsigned char);
    }
    if (sideIronLength.has_value()) {
        // If we have a side iron length parameter, increment the size by the space it will take.
        size += sizeof(unsigned short);
    }
    if (sidelapMode.has_value()) {
        // If we have a sidelap mode parameter, increment the size by the space it will take.
        size += sizeof(unsigned char);
    }
    if (overlapMode.has_value()) {
        // If we have an overlap mode parameter, increment the size by the space it will take.
        size += sizeof(unsigned char);
    }
    if (sidelapWidth.has_value()) {
        // If we have a sidelap width parameter, increment the size by the space it will take.
        size += sidelapWidth->serialisedSize();
    }
    if (overlapWidth.has_value()) {
        // If we have an overlap width parameter, increment the size by the space it will take.
        size += overlapWidth->serialisedSize();
    }
    if (sidelapAttachment.has_value()) {
        // If we have a sidelap attachment parameter, increment the size by the space it will take.
        size += sizeof(unsigned char);
    }
    if (overlapAttachment.has_value()) {
        // If we have an overlap attachment parameter, increment the size by the space it will take.
        size += sizeof(unsigned char);
    }
    if (machine.has_value()) {
        // If we have a machine parameter, increment the size by the space it will take.
        size += machine->serialisedSize();
    }
    if (manufacturer.has_value()) {
        size += sizeof(unsigned char) + manufacturer->size();
    }
    if (quantityOnDeck.has_value()) {
        // If we have a quantity on deck parameter, increment the size by the space it will take.
        size += sizeof(unsigned char);
    }
    if (position.has_value()) {
        // If we have a position parameter, increment the size by the space it will take.
        size += sizeof(unsigned char) + position->size();
    }
    if (machineDeck.has_value()) {
        // If we have a machine deck parameter, increment the size by the space it will take.
        size += machineDeck->serialisedSize();
    }

    // Return the total size
    return size;
}

// Reconstructs a DatabaseSearchQuery object from the data buffer
DatabaseSearchQuery &DatabaseSearchQuery::deserialise(void *data) {
    // First we create an empty query object on the heap for returning once
    // we have given it its values
    DatabaseSearchQuery *query = new DatabaseSearchQuery();

    // We cast the buffer to a byte buffer, and increment it by the sizeof 
    // a RequestType to skip over the initial header bytes which we are not
    // concerned with
    unsigned char *buffer = (unsigned char *) data + sizeof(RequestType);

    // Next we get the search parameters from the buffer
    unsigned searchParameters = *((unsigned *) buffer);
    buffer += sizeof(unsigned);

    // If we have a drawing number specified, read it from the buffer in the
    // same way it was written. Otherwise, set this field to a nullopt.
    if (searchParameters & (unsigned) SearchParameters::DRAWING_NUMBER) {
        unsigned char dNumSize = *buffer++;
        query->drawingNumber = std::string((const char *) buffer, dNumSize);
        buffer += dNumSize;
    } else {
        query->drawingNumber = std::nullopt;
    }
    // If we have a width range specified, read it from the buffer in the
    // same way it was written. Otherwise, set this field to a nullopt.
    if (searchParameters & (unsigned) SearchParameters::WIDTH) {
        query->width = ValueRange<unsigned>::deserialise(buffer);
        buffer += query->width->serialisedSize();
    } else {
        query->width = std::nullopt;
    }
    // If we have a length range specified, read it from the buffer in the
    // same way it was written. Otherwise, set this field to a nullopt.
    if (searchParameters & (unsigned) SearchParameters::LENGTH) {
        query->length = ValueRange<unsigned>::deserialise(buffer);
        buffer += query->length->serialisedSize();
    } else {
        query->length = std::nullopt;
    }
    // If we have a product type specified, read it from the buffer in the
    // same way it was written. Otherwise, set this field to a nullopt.
    if (searchParameters & (unsigned) SearchParameters::PRODUCT_TYPE) {
        query->productType = DrawingComponentManager<Product>::getComponentByHandle(Product::deserialise(buffer));
        buffer += query->productType->serialisedSize();
    } else {
        query->productType = std::nullopt;
    }
    // If we have a number of bars specified, read it from the buffer in the
    // same way it was written. Otherwise, set this field to a nullopt.
    if (searchParameters & (unsigned) SearchParameters::NUMBER_OF_BARS) {
        query->numberOfBars = *buffer++;
    } else {
        query->numberOfBars = std::nullopt;
    }
    // If we have an aperture specified, read it from the buffer in the
    // same way it was written. Otherwise, set this field to a nullopt.
    if (searchParameters & (unsigned) SearchParameters::APERTURE) {
        query->aperture = DrawingComponentManager<Aperture>::getComponentByHandle(Aperture::deserialise(buffer));
        buffer += query->aperture->serialisedSize();
    } else {
        query->aperture = std::nullopt;
    }
    // If we have a top thickness specified, read it from the buffer in the
    // same way it was written. Otherwise, set this field to a nullopt.
    if (searchParameters & (unsigned) SearchParameters::TOP_THICKNESS) {
        query->topThickness = DrawingComponentManager<Material>::getComponentByHandle(Material::deserialise(buffer));
        buffer += query->topThickness->serialisedSize();
    } else {
        query->topThickness = std::nullopt;
    }
    // If we have a bottom thickness specified, read it from the buffer in the
    // same way it was written. Otherwise, set this field to a nullopt.
    if (searchParameters & (unsigned) SearchParameters::BOTTOM_THICKNESS) {
        query->bottomThickness = DrawingComponentManager<Material>::getComponentByHandle(Material::deserialise(buffer));
        buffer += query->bottomThickness->serialisedSize();
    } else {
        query->bottomThickness = std::nullopt;
    }
    // If we have a date range specified, read it from the buffer in the
    // same way it was written. Otherwise, set this field to a nullopt.
    if (searchParameters & (unsigned) SearchParameters::DATE_RANGE) {
        query->dateRange = ValueRange<Date>::deserialise(buffer);
        buffer += query->dateRange->serialisedSize();
    } else {
        query->dateRange = std::nullopt;
    }
    // If we have a side iron type specified, read it from the buffer in the
    // same way it was written. Otherwise, set this field to a nullopt.
    if (searchParameters & (unsigned) SearchParameters::SIDE_IRON_TYPE) {
        query->sideIronType = (SideIronType) *buffer++;
    } else {
        query->sideIronType = std::nullopt;
    }
    // If we have a side iron length specified, read it from the buffer in the
    // same way it was written. Otherwise, set this field to a nullopt.
    if (searchParameters & (unsigned) SearchParameters::SIDE_IRON_LENGTH) {
        query->sideIronLength = *((unsigned short *) buffer);
        buffer += sizeof(unsigned short);
    } else {
        query->sideIronLength = std::nullopt;
    }
    // If we have a sidelap modes pecified, read it from the buffer in the
    // same way it was written. Otherwise, set this field to a nullopt.
    if (searchParameters & (unsigned) SearchParameters::SIDELAP_MODE) {
        query->sidelapMode = (LapSetting) *buffer++;
    } else {
        query->sidelapMode = std::nullopt;
    }
    // If we have an overlap mode specified, read it from the buffer in the
    // same way it was written. Otherwise, set this field to a nullopt.
    if (searchParameters & (unsigned) SearchParameters::OVERLAP_MODE) {
        query->overlapMode = (LapSetting) *buffer++;
    } else {
        query->overlapMode = std::nullopt;
    }
    // If we have a sidelap width specified, read it from the buffer in the
    // same way it was written. Otherwise, set this field to a nullopt.
    if (searchParameters & (unsigned) SearchParameters::SIDELAP_WIDTH) {
        query->sidelapWidth = ValueRange<unsigned>::deserialise(buffer);
        buffer += query->sidelapWidth->serialisedSize();
    } else {
        query->sidelapWidth = std::nullopt;
    }
    // If we have an overlap width specified, read it from the buffer in the
    // same way it was written. Otherwise, set this field to a nullopt.
    if (searchParameters & (unsigned) SearchParameters::OVERLAP_WIDTH) {
        query->overlapWidth = ValueRange<unsigned>::deserialise(buffer);
        buffer += query->overlapWidth->serialisedSize();
    } else {
        query->overlapWidth = std::nullopt;
    }
    // If we have a sidelap attachment specified, read it from the buffer in the
    // same way it was written. Otherwise, set this field to a nullopt.
    if (searchParameters & (unsigned) SearchParameters::SIDELAP_ATTACHMENT) {
        query->sidelapAttachment = (LapAttachment) *buffer++;
    } else {
        query->sidelapAttachment = std::nullopt;
    }
    // If we have an overlap attachment specified, read it from the buffer in the
    // same way it was written. Otherwise, set this field to a nullopt.
    if (searchParameters & (unsigned) SearchParameters::OVERLAP_ATTACHMENT) {
        query->overlapAttachment = (LapAttachment) *buffer++;
    } else {
        query->overlapAttachment = std::nullopt;
    }
    // If we have a machine specified, read it from the buffer in the
    // same way it was written. Otherwise, set this field to a nullopt.
    if (searchParameters & (unsigned) SearchParameters::MACHINE) {
        query->machine = DrawingComponentManager<Machine>::getComponentByHandle(Machine::deserialise(buffer));
        buffer += query->machine->serialisedSize();
    } else {
        query->machine = std::nullopt;
    }
    if (searchParameters & (unsigned) SearchParameters::MACHINE_MANUFACTURER) {
        unsigned char manufacturerSize = *buffer++;
        query->manufacturer = std::string((const char *) buffer, manufacturerSize);
        buffer += manufacturerSize;
    } else {
        query->manufacturer = std::nullopt;
    }
    // If we have a quantity on deck specified, read it from the buffer in the
    // same way it was written. Otherwise, set this field to a nullopt.
    if (searchParameters & (unsigned) SearchParameters::QUANTITY_ON_DECK) {
        query->quantityOnDeck = *buffer++;
    } else {
        query->quantityOnDeck = std::nullopt;
    }
    // If we have a position specified, read it from the buffer in the
    // same way it was written. Otherwise, set this field to a nullopt.
    if (searchParameters & (unsigned) SearchParameters::POSITION) {
        unsigned char posSize = *buffer++;
        query->position = std::string((const char *) buffer, posSize);
        buffer += posSize;
    } else {
        query->position = std::nullopt;
    }
    // If we have a machine deck specified, read it from the buffer in the
    // same way it was written. Otherwise, set this field to a nullopt.
    if (searchParameters & (unsigned) SearchParameters::MACHINE_DECK) {
        query->machineDeck = DrawingComponentManager<MachineDeck>::getComponentByHandle(MachineDeck::deserialise(buffer));
        buffer += query->machineDeck->serialisedSize();
    } else {
        query->machineDeck = std::nullopt;
    }

    // Return the constructed query
    return *query;
}

// Method to return a flag system containing whether the query contains each
// possible search parameter.
unsigned DatabaseSearchQuery::getSearchParameters() const {
    // Initialise the paramters as 0.
    unsigned params = 0;

    // For every parameter which has a value, we add its corresponding
    // flag to the params value using a bitwise or operation.
    if (drawingNumber.has_value()) {
        params |= (unsigned) SearchParameters::DRAWING_NUMBER;
    }
    if (width.has_value()) {
        params |= (unsigned) SearchParameters::WIDTH;
    }
    if (length.has_value()) {
        params |= (unsigned) SearchParameters::LENGTH;
    }
    if (productType.has_value()) {
        params |= (unsigned) SearchParameters::PRODUCT_TYPE;
    }
    if (numberOfBars.has_value()) {
        params |= (unsigned) SearchParameters::NUMBER_OF_BARS;
    }
    if (aperture.has_value()) {
        params |= (unsigned) SearchParameters::APERTURE;
    }
    if (topThickness.has_value()) {
        params |= (unsigned) SearchParameters::TOP_THICKNESS;
    }
    if (bottomThickness.has_value()) {
        params |= (unsigned) SearchParameters::BOTTOM_THICKNESS;
    }
    if (dateRange.has_value()) {
        params |= (unsigned) SearchParameters::DATE_RANGE;
    }
    if (sideIronType.has_value()) {
        params |= (unsigned) SearchParameters::SIDE_IRON_TYPE;
    }
    if (sideIronLength.has_value()) {
        params |= (unsigned) SearchParameters::SIDE_IRON_LENGTH;
    }
    if (sidelapMode.has_value()) {
        params |= (unsigned) SearchParameters::SIDELAP_MODE;
    }
    if (overlapMode.has_value()) {
        params |= (unsigned) SearchParameters::OVERLAP_MODE;
    }
    if (sidelapWidth.has_value()) {
        params |= (unsigned) SearchParameters::SIDELAP_WIDTH;
    }
    if (overlapWidth.has_value()) {
        params |= (unsigned) SearchParameters::OVERLAP_WIDTH;
    }
    if (sidelapAttachment.has_value()) {
        params |= (unsigned) SearchParameters::SIDELAP_ATTACHMENT;
    }
    if (overlapAttachment.has_value()) {
        params |= (unsigned) SearchParameters::OVERLAP_ATTACHMENT;
    }
    if (machine.has_value()) {
        params |= (unsigned) SearchParameters::MACHINE;
    }
    if (manufacturer.has_value()) {
        params |= (unsigned) SearchParameters::MACHINE_MANUFACTURER;
    }
    if (quantityOnDeck.has_value()) {
        params |= (unsigned) SearchParameters::QUANTITY_ON_DECK;
    }
    if (position.has_value()) {
        params |= (unsigned) SearchParameters::POSITION;
    }
    if (machineDeck.has_value()) {
        params |= (unsigned) SearchParameters::MACHINE_DECK;
    }

    // Finally, we return the paramters
    return params;
}

// Method for constructing an SQL query from the specified parameters
std::string DatabaseSearchQuery::toSQLQueryString() const {
    // String stream for writing the query into
    std::stringstream sql;

    // First, we specify every value we wish to select. This includes two aggregated fields, namely for the thicknesses
    // and the laps, which are returned as JSON arrays. Otherwise, every selection is a standard SQL selection.
    sql << "SELECT d.mat_id AS mat_id, d.drawing_number AS drawing_number, d.width AS width, d.length AS length, "
           "mal.aperture_id AS aperture_id, mal.direction AS direction, "
           "(SELECT JSON_ARRAYAGG(t.material_thickness_id) FROM {0}.thickness AS t WHERE t.mat_id=d.mat_id) AS material_ids, "
           "d.tension_type AS tension_type, "
           "COALESCE((SELECT JSON_ARRAYAGG(JSON_ARRAY(l.type, l.width, l.mat_side)) "
           "FROM (SELECT mat_id, 'S' AS type, width, mat_side FROM {0}.sidelaps UNION "
           "SELECT mat_id, 'O' AS type, width, mat_side FROM {0}.overlaps) AS l "
           "WHERE l.mat_id=d.mat_id), JSON_ARRAY()) AS laps, "
           "COALESCE((SELECT JSON_ARRAYAGG(bs.bar_spacing) FROM {0}.bar_spacings AS bs WHERE bs.mat_id=d.mat_id), "
           "JSON_ARRAY()) AS spacings"
        << std::endl;

    // We then specify the primary table to select from, and begin joining other required tables.
    sql << "FROM {0}.drawings" << " AS d" << std::endl;
    sql << "INNER JOIN {0}.mat_aperture_link" << " AS mal ON d.mat_id=mal.mat_id" << std::endl;

    // Next we declare a vector of strings for the conditions. We do this because, for example,
    // we need to know if there are no conditions, because in this case we do not put the "WHERE"
    // clause at all in the query.
    std::vector<std::string> conditions;
    std::stringstream condition;

    // For each of these simpler queries, we simply check if the parameter is present, 
    // and if it is, we create a conditional string matching the field in the database
    // to whatever was passed into the query.
    if (drawingNumber.has_value()) {
        condition.str(std::string());
        condition << "d.drawing_number LIKE '" << drawingNumber.value() << "%'";
        conditions.push_back(condition.str());
    }
    if (width.has_value()) {
        condition.str(std::string());
        condition << "d.width BETWEEN " << width->lowerBound << " AND " << width->upperBound;
        conditions.push_back(condition.str());
    }
    if (length.has_value()) {
        condition.str(std::string());
        condition << "d.length BETWEEN " << length->lowerBound << " AND " << length->upperBound;
        conditions.push_back(condition.str());
    }
    if (productType.has_value()) {
        condition.str(std::string());
        condition << "d.product_id=" << productType->componentID();
        conditions.push_back(condition.str());
    }
    if (numberOfBars.has_value()) {
        condition.str(std::string());
        condition << "d.no_of_bars=" << (unsigned) numberOfBars.value();
        conditions.push_back(condition.str());
    }
    // The aperture query is marginally more complex, as we may need to specify which direction it faces,
    // but otherwise is very similar.
    if (aperture.has_value()) {
        condition.str(std::string());
        condition << "mal.aperture_id=" << aperture->componentID();
        conditions.push_back(condition.str());
        
        // Find out the component ID of the aperture shape, so if it is a longitudinal or transverse aperture,
        // we impose this added restriction.
        switch (DrawingComponentManager<ApertureShape>::getComponentByHandle(aperture->apertureShapeID).componentID()) {
            case 3:
                condition.str(std::string());
                condition << "mal.direction='Longitudinal'";
                conditions.push_back(condition.str());
                break;
            case 4:
                condition.str(std::string());
                condition << "mal.direction='Transverse'";
                conditions.push_back(condition.str());
                break;
            default:
                break;
        }
    }
    if (topThickness.has_value()) {
        condition.str(std::string());
        // If we just are searching for a top layer thickness, the search is the same as the above conditions:
        // a condition is simply added to match the thickness ID
        if (!bottomThickness.has_value()) {
            sql << "INNER JOIN {0}.thickness" << " AS t ON d.mat_id=t.mat_id" << std::endl;
            condition << "t.material_thickness_id=" << topThickness->componentID();
        } else {
            // If the search is for multiple layers, it is more complex. We have to join the table to itself on
            // the same mat_id. Then, we impose the restriction that t1's ID is less than t2. This avoids getting
            // duplicate search results. Finally, we check that the top layer and bottom layer both match with
            // what was searched. Note: As this is a nested query, it is evaluated once and returns a set of mat_id's
            // which match the thickness restrictions. This could potentially be slow, however the subquery itself
            // is called a minimal amount of times.
            condition << "d.mat_id IN (SELECT t1.mat_id AS mid FROM {0}.thickness AS t1" << std::endl;
            condition << "INNER JOIN {0}.thickness AS t2 ON t1.mat_id=t2.mat_id" << std::endl;
            condition << "WHERE t1.thickness_id < t2.thickness_id AND" << std::endl;
            condition << "t1.material_thickness_id=" << topThickness->componentID() << " AND" << std::endl;
            condition << "t2.material_thickness_id=" << bottomThickness->componentID() << ")";
        }
        conditions.push_back(condition.str());
    }
    if (dateRange.has_value()) {
        condition.str(std::string());
        condition << "d.drawing_date BETWEEN '" << dateRange->lowerBound.toMySQLDateString() << "' AND '"
                  << dateRange->upperBound.toMySQLDateString() << "'";
        conditions.push_back(condition.str());
    }
    if (sideIronType.has_value() || sideIronLength.has_value()) {
        // For the side irons, we link to the side iron table itself and check if the length and type align with what was
        // searched.
        sql << "INNER JOIN {0}.mat_side_iron_link" << " AS msil ON d.mat_id=msil.mat_id" << std::endl;
        sql << "INNER JOIN {0}.side_irons" << " AS si ON msil.side_iron_id=si.side_iron_id" << std::endl;
        if (sideIronType.has_value()) {
            condition.str(std::string());
            switch (sideIronType.value()) {
                case SideIronType::None:
                    condition << "msil.side_iron_id=1";
                    break;
                case SideIronType::A:
                    condition << "si.type=1";
                    break;
                case SideIronType::B:
                    condition << "si.type=2";
                    break;
                case SideIronType::C:
                    condition << "si.type=3";
                    break;
                case SideIronType::D:
                    condition << "si.type=4";
                    break;
                case SideIronType::E:
                    condition << "si.type=5";
                    break;
            }
            conditions.push_back(condition.str());
        }
        if (sideIronLength.has_value()) {
            condition.str(std::string());
            condition << "si.length=" << sideIronLength.value();
            conditions.push_back(condition.str());
        }
    }
    if (sidelapMode.has_value() || sidelapWidth.has_value() || sidelapAttachment.has_value()) {
        condition.str(std::string());
        // The sidelaps work by nesting a subquery which returns a set of all mat_ids which match the
        // criteria imposed by the sidelap parameters.
        condition << "d.mat_id IN (SELECT d_i.mat_id FROM {0}.drawings AS d_i" << std::endl;
        condition << "LEFT JOIN {0}.sidelaps AS s ON d_i.mat_id=s.mat_id" << std::endl;
        condition << "WHERE true" << std::endl;
        // First we impose that the width is in the correct range
        if (sidelapWidth.has_value()) {
            condition << "AND s.width BETWEEN " << sidelapWidth->lowerBound << " AND " << sidelapWidth->upperBound
                      << std::endl;
        }
        // Next we impose that the sidelaps have the correct attachment type
        if (sidelapAttachment.has_value()) {
            condition << "AND attachment_type='";
            switch (sidelapAttachment.value()) {
                case LapAttachment::BONDED:
                    condition << "Bonded";
                    break;
                case LapAttachment::INTEGRAL:
                    condition << "Integral";
                    break;
            }
            condition << "'" << std::endl;
        }
        condition << "GROUP BY d_i.mat_id" << std::endl;
        // And finally, if we require that there are a specific number of laps matching these criteria,
        // we add a HAVING clause.
        if (sidelapMode.has_value()) {
            condition << "HAVING COUNT(s.mat_id)=";
            switch (sidelapMode.value()) {
                case LapSetting::HAS_NONE:
                    condition << 0;
                    break;
                case LapSetting::HAS_ONE:
                    condition << 1;
                    break;
                case LapSetting::HAS_BOTH:
                    condition << 2;
                    break;
            }
        }
        condition << ")" << std::endl;
        conditions.push_back(condition.str());
    }
    if (overlapMode.has_value() || overlapWidth.has_value() || overlapAttachment.has_value()) {
        condition.str(std::string());
        // The overlaps work by nesting a subquery which returns a set of all mat_ids which match the
        // criteria imposed by the overlap parameters.
        condition << "d.mat_id IN (SELECT d_i.mat_id FROM {0}.drawings AS d_i" << std::endl;
        condition << "LEFT JOIN {0}.overlaps AS o ON d_i.mat_id=o.mat_id" << std::endl;
        condition << "WHERE true" << std::endl;
        // First we impose that the width is in the correct range
        if (overlapWidth.has_value()) {
            condition << "AND o.width BETWEEN " << overlapWidth->lowerBound << " AND " << overlapWidth->upperBound
                      << std::endl;
        }
        // Next we impose that the overlaps have the correct attachment type
        if (overlapAttachment.has_value()) {
            condition << "AND attachment_type='";
            switch (overlapAttachment.value()) {
                case LapAttachment::BONDED:
                    condition << "Bonded";
                    break;
                case LapAttachment::INTEGRAL:
                    condition << "Integral";
                    break;
            }
            condition << "'" << std::endl;
        }
        condition << "GROUP BY d_i.mat_id" << std::endl;
        // And finally, if we require that there are a specific number of laps matching these criteria,
        // we add a HAVING clause.
        if (overlapMode.has_value()) {
            condition << "HAVING COUNT(o.mat_id)=";
            switch (overlapMode.value()) {
                case LapSetting::HAS_NONE:
                    condition << 0;
                    break;
                case LapSetting::HAS_ONE:
                    condition << 1;
                    break;
                case LapSetting::HAS_BOTH:
                    condition << 2;
                    break;
            }
        }
        condition << ")" << std::endl;
        conditions.push_back(condition.str());
    }
    if (machine.has_value() || manufacturer.has_value() || quantityOnDeck.has_value() || position.has_value() || machineDeck.has_value()) {
        // If the query requests that any restrictions concerning the machine template should be imposed,
        // we join to the machine templates table.
        sql << "INNER JOIN {0}.machine_templates AS mt ON d.template_id=mt.template_id" << std::endl;

        // For each of the restrictions, we simply check if the parameter matches, if they are included in
        // the query parameters.
        if (machine.has_value()) {
            condition.str(std::string());
            condition << "mt.machine_id=" << machine->componentID();
            conditions.push_back(condition.str());
        }
        if (manufacturer.has_value()) {
            sql << "INNER JOIN {0}.machines AS m ON mt.machine_id=m.machine_id" << std::endl;
            condition.str(std::string());
            condition << "m.manufacturer='" << manufacturer.value() << "'";
            conditions.push_back(condition.str());
        }
        if (quantityOnDeck.has_value()) {
            condition.str(std::string());
            condition << "mt.quantity_on_deck=" << (unsigned) quantityOnDeck.value();
            conditions.push_back(condition.str());
        }
        if (position.has_value()) {
            condition.str(std::string());
            condition << "mt.position LIKE '" << position.value() << "%'";
            conditions.push_back(condition.str());
        }
        if (machineDeck.has_value()) {
            condition.str(std::string());
            condition << "mt.deck_id=" << machineDeck->componentID();
            conditions.push_back(condition.str());
        }
    }

    // If we have at least a single condition, we append the "where" clause to the 
    // query string, and add each condition, delimited by the word AND.
    if (!conditions.empty()) {
        sql << "WHERE " << std::endl;
        for (unsigned i = 0; i < conditions.size() - 1; i++) {
            sql << conditions[i] << " AND" << std::endl;
        }
        sql << conditions.back() << std::endl;
    }

    // Finally, we group the results by mat_id and aperture_id to aggregate them, and then order the results
    // by the drawing number order. This ordering is such that all drawings with a single letter (e.g A34) appear
    // before double letter drawings (e.g. EA34) and then the drawings are order lexicographically in descending order
    // so more recent drawings appear higher up the search list.
    sql << "GROUP BY d.mat_id, mal.aperture_id" << std::endl;
    sql << "ORDER BY IF(d.drawing_number REGEXP '^[A-Z][0-9]{{2,}}[A-Z]?$', "
           "CONCAT('0', d.drawing_number), d.drawing_number) DESC;";

    return sql.str();
}

// Translates a list of query result rows into a list of DrawingSummary objects
std::vector<DrawingSummary> DatabaseSearchQuery::getQueryResultSummaries(mysqlx::RowResult resultSet) {
    // Declare the target array
    std::vector<DrawingSummary> summaries;

    // Loop through each row
    for (const mysqlx::Row &row : resultSet) {
        // Create a summary object 
        DrawingSummary summary;
        // Set the basic parameters from the data source
        summary.matID = row[0];
        summary.drawingNumber = row[1].get<std::string>();
        summary.setWidth(row[2]);
        summary.setLength(row[3]);

        // It may be the case that this mat uses a bidirectional aperture, in which case we return
        // all matching apertures and choose the correct one, based upon the aperture direction
        std::vector<Aperture *> matchingApertures = DrawingComponentManager<Aperture>::allComponentsByID(row[4]);

        if (matchingApertures.size() == 1) {
            // If there is just one matching aperture, we simply use this.
            summary.apertureHandle = matchingApertures.front()->handle();
        } else {
            // If there are multiple matching apertures, we start by getting the direction string 
            std::string apertureDirectionString = row[5].get<std::string>();

            // By default we set the aperture handle to 0. This indicates that there was no matching aperture
            // and as such there was an error with this drawing. If there is a correct aperture found, then
            // the handle will be overwritten with the correct value.
            summary.apertureHandle = 0;

            // Depending on which direction the aperture faces, we loop through the aperture list to find whichever has
            // the correct "shape". Note that these matchingApertures lists should have at maximum a length of 2, so 
            // this operation should not be slow.
            if (apertureDirectionString == "Longitudinal") {
                for (const Aperture *ap : matchingApertures) {
                    if (DrawingComponentManager<ApertureShape>::getComponentByHandle(ap->apertureShapeID).componentID() == 3) {
                        summary.apertureHandle = ap->handle();
                        break;
                    }
                }
            } else if (apertureDirectionString == "Transverse") {
                for (const Aperture *ap : matchingApertures) {
                    if (DrawingComponentManager<ApertureShape>::getComponentByHandle(ap->apertureShapeID).componentID() == 4) {
                        summary.apertureHandle = ap->handle();
                        break;
                    }
                }
            }
        }

        // We nullify the thickness handles and lap sizes to clear any residual memory which might
        // be accidentally written
        memset(&summary.thicknessHandles, 0, sizeof(summary.thicknessHandles));
        for (unsigned i = 0; i < 4; i++) {
            summary.setLapSize(i, 0);
        }

        // We then loop through each returned element in the thicknesses returned and set the corresponding thickness
        // handle in the summary object
        for (unsigned thicknessSlot = 0; thicknessSlot < row[6].elementCount(); thicknessSlot++) {
            summary.thicknessHandles[thicknessSlot] = DrawingComponentManager<Material>::findComponentByID(row[6][thicknessSlot]).handle();
        }

        // Then we deal with any laps there may be in the drawing
        for (const mysqlx::Value &lap : row[8]) {
            // If we encounter a "damaged" lap, we just skip it to avoid a possible
            // error
            if (lap[2].isNull()) {
                continue;
            }

            // We calculate the index this lap belongs to. This depends on the side of the mat, the tension type
            // and whether it is a sidelap or overlap. Note that a boolean equivalence will return 1 if equal and
            // 0 if inequal, so if lap[2] == "Right", then the index will be 1 to start with, otherwise 0.
            // This means we can easily calculate the index associated with this lap value.
            unsigned index = (lap[2].get<std::string>() == "Right");
            if (row[7].get<std::string>() == "Side") {
                index += 2 * (lap[0].get<std::string>() == "O");
            }
            else {
                index += 2 * (lap[0].get<std::string>() == "S");
            }
            // Finally, after computing the index, we simply write the size to the summary in the correct slot.
            summary.setLapSize(index, lap[1].get<double>());
        }

        for (double spacing : row[9]) {
            summary.addSpacing(spacing);
        }

        // Once we have finished creating this summary, we add it to the list
        summaries.push_back(summary);
    }

    // Finally once all summaries are created, we return the final list
    return summaries;
}

// Helper method for creating a DrawningRequest object for a specific matID
DrawingRequest &DrawingRequest::makeRequest(unsigned matID, unsigned responseEchoCode) {
    // First we create the request object for returning
    DrawingRequest *request = new DrawingRequest();
    // Next we set the matID and response code
    request->matID = matID;
    request->responseEchoCode = responseEchoCode;
    // We set the drawing data to be a nullopt because this is the request; we do not currently
    // know what the drawing data contains
    request->drawingData = std::nullopt;
    // Finally we return this request object
    return *request;
}

// Serialises the drawing request into the target buffer
void DrawingRequest::serialise(void *target) const {
    // First we convert the target buffer into a buffer of bytes
    unsigned char *buffer = (unsigned char *) target;
    // We set the start of the buffer to contain that this is a DRAWING_DETAILS
    // request type
    *((RequestType *) buffer) = RequestType::DRAWING_DETAILS;
    buffer += sizeof(RequestType);

    // Next we set the matID this request is for
    *((unsigned *) buffer) = matID;
    buffer += sizeof(unsigned);

    // Then we set the response echo code
    *((unsigned *) buffer) = responseEchoCode;
    buffer += sizeof(unsigned);

    // We set a simple boolean which determines whether or not the request contains
    // a drawing or not (i.e. if it is a request or a response)
    bool hasDrawingData = drawingData.has_value();
    *buffer++ = hasDrawingData;

    // If we do have a drawing to write into the buffer, we serialise it using the 
    // DrawingSerialiser into the buffer
    if (hasDrawingData) {
        DrawingSerialiser::serialise(drawingData.value(), buffer);
    }
}

// Helper function to return the amount of space this object will occupy in a buffer
unsigned int DrawingRequest::serialisedSize() const {
    // Returns the size of the header info, the sizes of the matID and response code, the size of the byte used to
    // determine whether there is a drawing, and then if there is a drawing, the size this drawing will occupy.
    return sizeof(RequestType) + sizeof(unsigned) + sizeof(unsigned) + sizeof(unsigned char) +
           (drawingData.has_value() ? DrawingSerialiser::serialisedSize(drawingData.value()) : 0);
}

// Deserialses a DrawingRequest from a data buffer
DrawingRequest &DrawingRequest::deserialise(void *data) {
    // First we construct the request object to return
    DrawingRequest *drawingRequest = new DrawingRequest();

    // We again cast the buffer into a byte buffer, and skip over the initial
    // bytes because at this point we do not care about the request type (we
    // already know this is a drawing request)
    unsigned char *buffer = (unsigned char *) data + sizeof(RequestType);
    // We read the matID
    drawingRequest->matID = *((unsigned *) buffer);
    buffer += sizeof(unsigned);
    // We read the response code
    drawingRequest->responseEchoCode = *((unsigned *) buffer);
    buffer += sizeof(unsigned);

    // Next we read the following byte to determine whether there is a drawing.
    bool hasDrawingData = *buffer++;

    if (hasDrawingData) {
        // If there is a drawing in this message, we deserialise it into the drawingData
        // optional value
        drawingRequest->drawingData = DrawingSerialiser::deserialise(buffer);
    } else {
        // Otherwise we set the drawingData to a nullopt, indicating that it is not present
        drawingRequest->drawingData = std::nullopt;
    }

    return *drawingRequest;
}

// Serialises a DrawingInsert query into the target buffer
void DrawingInsert::serialise(void *target) const {
    // First we cast the target buffer to a byte buffer
    unsigned char *buffer = (unsigned char *) target;
    // We begin by writing in that this is a DRAWING_INSERT request type
    *((RequestType *) buffer) = RequestType::DRAWING_INSERT;
    buffer += sizeof(RequestType);

    // Next we write in the insert response code. This will be NONE
    // if this is a request, otherwise it will hold the server's response
    // to the query
    *((InsertResponseCode *) buffer) = insertResponseCode;
    buffer += sizeof(InsertResponseCode);

    // Next we write in a response code for this query to allow the client
    // to use the asynchronous request-response model
    *((unsigned *) buffer) = responseEchoCode;
    buffer += sizeof(unsigned);

    // We write in whether or not this request is in forcing mode
    *buffer++ = force;

    // We set a simple boolean which determines whether or not the request contains
    // a drawing or not (i.e. if it is a request or a response)
    bool hasDrawingData = drawingData.has_value();
    *buffer++ = hasDrawingData;

    // If we do have a drawing to write into the buffer, we serialise it using the 
    // DrawingSerialiser into the buffer
    if (hasDrawingData) {
        DrawingSerialiser::serialise(drawingData.value(), buffer);
    }
}

// Helper function to return the amount of space this object will occupy in a buffer
unsigned int DrawingInsert::serialisedSize() const {
    // Returns the size of the header info, the sizes of the insert response and echo codes, 
    // the size of the byte used to determine if it is in forcing mode, the size of the byte used to
    // determine whether there is a drawing, and then if there is a drawing, the size this drawing will occupy.
    return sizeof(RequestType) + sizeof(InsertResponseCode) + sizeof(unsigned) + sizeof(unsigned char) +
           sizeof(unsigned char) +
           (drawingData.has_value() ? DrawingSerialiser::serialisedSize(drawingData.value()) : 0);
}

// Deserialses a DrawingInsert from a data buffer
DrawingInsert &DrawingInsert::deserialise(void *data) {
    // First we construct the insert object to return
    DrawingInsert *drawingInsert = new DrawingInsert();

    // Next we cast the data buffer to a byte buffer and increment by the size of
    // a request type, as we already know what the request type is
    unsigned char *buffer = (unsigned char *) data + sizeof(RequestType);
    // Next we read in the insert response code
    drawingInsert->insertResponseCode = *((InsertResponseCode *) buffer);
    buffer += sizeof(InsertResponseCode);
    // Then we read the response echo code
    drawingInsert->responseEchoCode = *((unsigned *) buffer);
    buffer += sizeof(unsigned);

    // Then we read if the insertion was in forcing mode or not
    drawingInsert->force = *buffer++;

    // Then we read if the insertion has an attached drawing
    bool hasDrawingData = *buffer++;

    if (hasDrawingData) {
        // If there is an attached drawing, we deserialise it into the drawing data object
        drawingInsert->drawingData = DrawingSerialiser::deserialise(buffer);
    } else {
        // Otherwise we set the data to be a nullopt
        drawingInsert->drawingData = std::nullopt;
    }

    // Finally we return the reconstruced insert object
    return *drawingInsert;
}

// Setter for forcing mode
void DrawingInsert::setForce(bool val) {
    force = val;
}

// Getter for forcing mode
bool DrawingInsert::forcing() const {
    return force;
}

// Creates a MySQL query string for inserting the data into the drawings table
std::string DrawingInsert::drawingInsertQuery(unsigned templateID) const {
    std::stringstream insert;

    // First we specify the columns we are inserting
    insert << "INSERT INTO {0}.drawings" << std::endl;
    insert << "(drawing_number, product_id, template_id, width, length, tension_type, drawing_date, no_of_bars, "
              "rebated, backing_strips, hyperlink, notes)" << std::endl;
    insert << "VALUES" << std::endl;

    // Next we add the simple values from the drawingData object
    insert << "('" << drawingData->drawingNumber() << "', " << drawingData->product().componentID() << ", " <<
           templateID << ", " << drawingData->width() << ", " << drawingData->length() << ", ";

    // The tension type needs to be converted from an enum type to a string, so we use
    // a simple switch statement
    switch (drawingData->tensionType()) {
        case Drawing::SIDE:
            insert << "'Side'";
            break;
        case Drawing::END:
            insert << "'End'";
            break;
    }

    // Finally we add the rest of the details
    insert << ", '" << drawingData->date().toMySQLDateString() << "', " << drawingData->numberOfBars() << ", " <<
        drawingData->rebated() << ", " << drawingData->hasBackingStrips() << ", '" << drawingData->hyperlink().generic_string() <<
        "', '" << drawingData->notes() << "')" << std::endl;

    // Returns the constructed query string
    return insert.str();
}

// Creates a MySQL query string for inserting the bars into the bar_spacings table
std::string DrawingInsert::barSpacingInsertQuery(unsigned matID) const {
    // First we check if there are no bars. If there are none, we have nothing to enter
    // so we just return an empty string.
    if (drawingData->numberOfBars() == 0) {
        return std::string();
    }

    std::stringstream insert;

    // Specify the columns to insert
    insert << "INSERT INTO {0}.bar_spacings" << std::endl;
    insert << "(mat_id, bar_spacing, bar_width, bar_index)" << std::endl;
    insert << "VALUES" << std::endl;

    // Loop through each bar and add it to the insert query
    for (unsigned i = 0; i < drawingData->numberOfBars(); i++) {
        insert << "(" << matID << ", " << drawingData->barSpacing(i) << ", " << drawingData->barWidth(i + 1) <<
               ", " << i << ")";
        if (i != drawingData->numberOfBars() - 1) {
            insert << ", ";
        }
        insert << std::endl;
    }

    // Return the constructed MySQL string
    return insert.str();
}

// Creates a MySQL query string for inserting the machine data into the machine_templates table
std::string DrawingInsert::machineTemplateInsertQuery() const {
    std::stringstream insert;

    // First specify the columns to insert
    insert << "INSERT INTO {0}.machine_templates" << std::endl;
    insert << "(machine_id, quantity_on_deck, position, deck_id)" << std::endl;
    insert << "VALUES" << std::endl;

    // Get the machine template from the drawing object
    Drawing::MachineTemplate t = drawingData->machineTemplate();

    // Add the details to the query string
    insert << "(" << t.machine().componentID() << ", " << t.quantityOnDeck << ", '" <<
           t.position << "', " << t.deck().componentID() << ")" << std::endl;

    // Return the constructed MySQL string
    return insert.str();
}

// Creates a MySQL query string for testing if the machine template already exists
std::string DrawingInsert::testMachineTemplateQuery() const {
    std::stringstream query;

    // Get the machine template from the drawing object
    Drawing::MachineTemplate t = drawingData->machineTemplate();

    // Construct a simple search query in the machine templates table. This
    // query returns the template_id so if a match is found, we know which
    // ID to use from this point.
    query << "SELECT template_id FROM {0}.machine_templates" << std::endl;
    query << "WHERE machine_id=" << t.machine().componentID() << " AND " << std::endl;
    query << "quantity_on_deck=" << t.quantityOnDeck << " AND" << std::endl;
    query << "position='" << t.position << "' AND" << std::endl;
    query << "deck_id=" << t.deck().componentID() << std::endl;

    // Return the constructed MySQL string
    return query.str();
}

// Creates a MySQL query string for inserting the aperture into the mat_aperture_link table
std::string DrawingInsert::apertureInsertQuery(unsigned matID) const {
    std::stringstream insert;

    // First specify the columns to insert
    insert << "INSERT INTO {0}.mat_aperture_link" << std::endl;
    insert << "(mat_id, aperture_id, direction)" << std::endl;
    insert << "VALUES" << std::endl;

    // Add the basic data to the query
    insert << "(" << matID << ", " << drawingData->aperture().componentID() << ", '";

    // Depending on what direction the aperture was in, we insert a direction parameter
    switch (DrawingComponentManager<ApertureShape>::getComponentByHandle(drawingData->aperture().apertureShapeID).componentID()) {
        case 3:
            insert << "Longitudinal";
            break;
        case 4:
            insert << "Transverse";
            break;
        default:
            insert << "Nondirectional";
            break;
    }

    insert << "')" << std::endl;

    // Return the constructed MySQL string
    return insert.str();
}

// Creates a MySQL query string for inserting the side irons into the mat_side_iron_link table
std::string DrawingInsert::sideIronInsertQuery(unsigned matID) const {
    std::stringstream insert;

    // First specify the columns to insert
    insert << "INSERT INTO {0}.mat_side_iron_link" << std::endl;
    insert << "(mat_id, side_iron_id, bar_width, inverted, side_iron_index)" << std::endl;
    insert << "VALUES" << std::endl;

    // Then add the data to insert for both of the side iron side. We specify which is which by the final index parameter.
    insert << "(" << matID << ", " << drawingData->sideIron(Drawing::LEFT).componentID() << ", " << drawingData->leftMargin() << ", " <<
           drawingData->sideIronInverted(Drawing::LEFT) << ", 0), " << std::endl;
    insert << "(" << matID << ", " << drawingData->sideIron(Drawing::RIGHT).componentID() << ", " << drawingData->rightMargin() << ", " <<
           drawingData->sideIronInverted(Drawing::RIGHT) << ", 1)" << std::endl;

    // Return the constructed MySQL string
    return insert.str();
}

// Creates a MySQL query string for inserting the materials into the thickness table
std::string DrawingInsert::thicknessInsertQuery(unsigned matID) const {
    std::stringstream insert;

    // First specify the columns to insert
    insert << "INSERT INTO {0}.thickness" << std::endl;
    insert << "(mat_id, material_thickness_id)" << std::endl;
    insert << "VALUES" << std::endl;

    // Next we add the top material. Every mat should have this.
    insert << "(" << matID << ", " << drawingData->material(Drawing::TOP)->componentID() << ")";

    // Next we add the option bottom material, if it exists. Not every mat will have this
    if (drawingData->material(Drawing::BOTTOM).has_value()) {
        insert << ", " << std::endl << "(" << matID << ", " << drawingData->material(Drawing::BOTTOM)->componentID()
               << ")";
    }

    insert << std::endl;

    // Return the constructed MySQL string
    return insert.str();
}

// Creates a MySQL query string for inserting the overlaps into the overlaps table
std::string DrawingInsert::overlapsInsertQuery(unsigned matID) const {
    // First we check if there are no overlaps. If there are none, we have nothing to enter
    // so we just return an empty string.
    if (!drawingData->hasOverlaps()) {
        return std::string();
    }

    std::stringstream insert;

    // First specify the columns to insert
    insert << "INSERT INTO {0}.overlaps" << std::endl;
    insert << "(mat_id, width, mat_side, attachment_type, material_id)" << std::endl;
    insert << "VALUES" << std::endl;

    // We get the left and right overlaps from the drawing data. If they do not exist, these will be 
    // nullopts.
    std::optional<Drawing::Lap> left = drawingData->overlap(Drawing::LEFT), right = drawingData->overlap(
            Drawing::RIGHT);

    if (left.has_value()) {
        // If the left lap has a value, then we add the basic info.
        insert << "(" << matID << ", " << left->width << ", 'Left', ";

        // Here we have an enum type, so we translate it to a string
        switch (left->attachmentType) {
            case LapAttachment::INTEGRAL:
                insert << "'Integral', " << drawingData->material(Drawing::TOP)->componentID() << ")";
                break;
            case LapAttachment::BONDED:
                insert << "'Bonded', " << left->material().componentID() << ")";
                break;
        }

        // If we also have a right lap, we need to add a comma to delimit them
        if (right.has_value()) {
            insert << ", ";
        }
        insert << std::endl;
    }

    if (right.has_value()) {
        // If the right lap has a value, then we add the basic info.
        insert << "(" << matID << ", " << right->width << ", 'Right', ";

        // Here we again have an enum type, so we translate it to a string
        switch (right->attachmentType) {
            case LapAttachment::INTEGRAL:
                insert << "'Integral', " << drawingData->material(Drawing::TOP)->componentID() << ")";
                break;
            case LapAttachment::BONDED:
                insert << "'Bonded', " << right->material().componentID() << ")";
                break;
        }
        insert << std::endl;
    }

    // Return the constructed MySQL string
    return insert.str();
}

// Creates a MySQL query string for inserting the sidelaps into the sidelaps table
std::string DrawingInsert::sidelapsInsertQuery(unsigned matID) const {
    // First we check if there are no sidelaps. If there are none, we have nothing to enter
    // so we just return an empty string.
    if (!drawingData->hasSidelaps()) {
        return std::string();
    }

    std::stringstream insert;

    // First specify the columns to insert
    insert << "INSERT INTO {0}.sidelaps" << std::endl;
    insert << "(mat_id, width, mat_side, attachment_type, material_id)" << std::endl;
    insert << "VALUES" << std::endl;

    // We get the left and right sidelaps from the drawing data. If they do not exist, these will be 
    // nullopts.
    std::optional<Drawing::Lap> left = drawingData->sidelap(Drawing::LEFT), right = drawingData->sidelap(
            Drawing::RIGHT);

    if (left.has_value()) {
        // If the left lap has a value, then we add the basic info.
        insert << "(" << matID << ", " << left->width << ", 'Left', ";

        // Here we have an enum type, so we translate it to a string
        switch (left->attachmentType) {
            case LapAttachment::INTEGRAL:
                insert << "'Integral', " << drawingData->material(Drawing::TOP)->componentID() << ")";
                break;
            case LapAttachment::BONDED:
                insert << "'Bonded', " << left->material().componentID() << ")";
                break;
        }

        // If we also have a right lap, we need to add a comma to delimit them
        if (right.has_value()) {
            insert << ", ";
        }
        insert << std::endl;
    }

    if (right.has_value()) {
        // If the right lap has a value, then we add the basic info.
        insert << "(" << matID << ", " << right->width << ", 'Right', ";

        // Here we again have an enum type, so we translate it to a string
        switch (right->attachmentType) {
            case LapAttachment::INTEGRAL:
                insert << "'Integral', " << drawingData->material(Drawing::TOP)->componentID() << ")";
                break;
            case LapAttachment::BONDED:
                insert << "'Bonded', " << right->material().componentID() << ")";
                break;
        }
        insert << std::endl;
    }

    // Return the constructed MySQL string
    return insert.str();
}

// Creates a MySQL query string for inserting the punch program PDF hyperlinks into the punch_program_pdfs table
std::string DrawingInsert::punchProgramsInsertQuery(unsigned matID) const {
    std::vector<std::filesystem::path> punchPDFs = drawingData->pressDrawingHyperlinks();

    // First we check if there are no PDFs. If there are none, we have nothing to enter
    // so we just return an empty string.
    if (punchPDFs.empty()) {
        return std::string();
    }

    std::stringstream insert;

    // First specify the columns to insert
    insert << "INSERT INTO {0}.punch_program_pdfs" << std::endl;
    insert << "(mat_id, hyperlink)" << std::endl;
    insert << "VALUES" << std::endl;

    // Next we loop over each PDF and append it to the values we add in the query.
    for (std::vector<std::filesystem::path>::const_iterator it = punchPDFs.begin(); it != punchPDFs.end(); it++) {
        std::string fileString = it->generic_string();
        size_t pos = 0;
        while ((pos = fileString.find('\'', pos)) < fileString.size()) {
            fileString.insert(fileString.begin() + pos, '\\');
            pos += 2;
        }
        insert << "(" << matID << ", '" << fileString << "')";
        // If this is not the last PDF, we also add a comma to delimit.
        if (it != punchPDFs.end() - 1) {
            insert << ", ";
        }
        insert << std::endl;
    }

    // Return the constructed MySQL string
    return insert.str();
}

std::string DrawingInsert::impactPadsInsertQuery(unsigned matID) const {
    std::vector<Drawing::ImpactPad> impactPads = drawingData->impactPads();

    if (impactPads.empty()) {
        return std::string();
    }

    std::stringstream insert;

    insert << "INSERT INTO {0}.impact_pads" << std::endl;
    insert << "(mat_id, material_id, aperture_id, aperture_direction, width, length, x_coord, y_coord)" << std::endl;
    insert << "VALUES" << std::endl;

    for (std::vector<Drawing::ImpactPad>::const_iterator it = impactPads.begin(); it != impactPads.end(); it++) {
        Drawing::ImpactPad pad = *it;

        Aperture &ap = pad.aperture();

        insert << "(" << matID << ", " << pad.material().componentID() << ", " << ap.componentID() << ", '";

        switch (DrawingComponentManager<ApertureShape>::getComponentByHandle(ap.apertureShapeID).componentID()) {
            case 3:
                insert << "Longitudinal";
                break;
            case 4:
                insert << "Transverse";
                break;
            default:
                insert << "Nondirectional";
                break;
        }

        insert << "', " << pad.width << ", " << pad.length << ", " << pad.pos.x << ", " << pad.pos.y << ")";

        if (it != impactPads.end() - 1) {
            insert << ", ";
        }
        insert << std::endl;
    }

    return insert.str();
}

std::string DrawingInsert::damBarInsertQuery(unsigned matID) const {
    std::vector<Drawing::DamBar> damBars = drawingData->damBars();

    if (damBars.empty()) {
        return std::string();
    }

    std::stringstream insert;

    insert << "INSERT INTO {0}.impact_pads" << std::endl;
    insert << "(mat_id, width, length, thickness, x_coord, y_coord)" << std::endl;
    insert << "VALUES" << std::endl;

    for (std::vector<Drawing::DamBar>::const_iterator it = damBars.begin(); it != damBars.end(); it++) {
        Drawing::DamBar bar = *it;


        insert << "', " << bar.width << ", " << bar.length << ", " << bar.thickness << ", " << bar.pos.x << ", " << bar.pos.y << ")";

        if (it != damBars.end() - 1) {
            insert << ", ";
        }
        insert << std::endl;
    }

    return insert.str();
}

std::string DrawingInsert::blankSpaceInsertQuery(unsigned matID) const {
    std::vector<Drawing::BlankSpace> blankSpaces = drawingData->blankSpaces();

    if (blankSpaces.empty()) {
        return std::string();
    }

    std::stringstream insert;

    insert << "INSERT INTO {0}.blank_spaces" << std::endl;
    insert << "(mat_id, width, length, x_coord, y_coord)" << std::endl;
    insert << "VALUES" << std::endl;

    for (std::vector<Drawing::BlankSpace>::const_iterator it = blankSpaces.begin(); it != blankSpaces.end(); it++) {
        Drawing::BlankSpace space = *it;

        insert << "(" << matID << ", " << space.width << ", " << space.length << ", " << space.pos.x << ", " << space.pos.y << ")";

        if (it != blankSpaces.end() - 1) {
            insert << ", ";
        }
        insert << std::endl;
    }

    return insert.str();
}

std::string DrawingInsert::extraApertureInsertQuery(unsigned matID) const {
    std::vector<Drawing::ExtraAperture> extraApertures = drawingData->extraApertures();

    if (extraApertures.empty()) {
        return std::string();
    }

    std::stringstream insert;

    insert << "INSERT INTO {0}.extra_apertures" << std::endl;
    insert << "(mat_id, width, length, x_coord, y_coord, aperture_id)" << std::endl;
    insert << "VALUES" << std::endl;

    for (std::vector<Drawing::ExtraAperture>::const_iterator it = extraApertures.begin(); it != extraApertures.end(); it++) {
        Drawing::ExtraAperture aperture = *it;

        insert << "(" << matID << ", " << aperture.width << ", " << aperture.length << ", " << aperture.pos.x << ", " << aperture.pos.y << ", " << aperture.apertureID <<")";

        if (it != extraApertures.end() - 1) {
            insert << ", ";
        }
        insert << std::endl;
    }

    return insert.str();
}

std::string DrawingInsert::centreHolesInsertQuery(unsigned matID) const {
    std::vector<Drawing::CentreHole> centreHoles = drawingData->centreHoles();

    if (centreHoles.empty()) {
        return std::string();
    }

    std::stringstream insert;

    insert << "INSERT INTO {0}.centre_holes" << std::endl;
    insert << "(mat_id, x_coord, y_coord, shape_width, shape_length, rounded)" << std::endl;
    insert << "VALUES" << std::endl;

    for (std::vector<Drawing::CentreHole>::const_iterator it = centreHoles.begin(); it != centreHoles.end(); it++) {
        Drawing::CentreHole hole = *it;

        insert << "(" << matID << ", " << hole.pos.x << ", " << hole.pos.y << ", " << hole.centreHoleShape.width << "," <<
            hole.centreHoleShape.length << ", " << hole.centreHoleShape.rounded << ")";

        if (it != centreHoles.end() - 1) {
            insert << ", ";
        }
        insert << std::endl;
    }

    return insert.str();
}

std::string DrawingInsert::deflectorsInsertQuery(unsigned matID) const {
    std::vector<Drawing::Deflector> deflectors = drawingData->deflectors();

    if (deflectors.empty()) {
        return std::string();
    }

    std::stringstream insert;

    insert << "INSERT INTO {0}.deflectors" << std::endl;
    insert << "(mat_id, material_id, size, x_coord, y_coord)" << std::endl;
    insert << "VALUES" << std::endl;

    for (std::vector<Drawing::Deflector>::const_iterator it = deflectors.begin(); it != deflectors.end(); it++) {
        Drawing::Deflector deflector = *it;

        insert << "(" << matID << ", " << deflector.material().componentID() << ", " << deflector.size << ", " << deflector.pos.x <<
            ", " << deflector.pos.y << ")";

        if (it != deflectors.end() - 1) {
            insert << ", ";
        }
        insert << std::endl;
    }

    return insert.str();
}

std::string DrawingInsert::divertorsInsertQuery(unsigned matID) const {
    std::vector<Drawing::Divertor> divertors = drawingData->divertors();

    if (divertors.empty()) {
        return std::string();
    }

    std::stringstream insert;

    insert << "INSERT INTO {0}.divertors" << std::endl;
    insert << "(mat_id, material_id, width, length, mat_side, y_coord)" << std::endl;
    insert << "VALUES" << std::endl;

    for (std::vector<Drawing::Divertor>::const_iterator it = divertors.begin(); it != divertors.end(); it++) {
        Drawing::Divertor divertor = *it;

        insert << "(" << matID << ", " << divertor.material().componentID() << ", " << divertor.width << ", " <<
            divertor.length << ", '";

        switch (divertor.side) {
            case Drawing::LEFT:
                insert << "Left";
                break;
            case Drawing::RIGHT:
                insert << "Right";
                break;
            default:
                break;
        }

        insert << "', " << divertor.verticalPosition << ")";

        if (it != divertors.end() - 1) {
            insert << ", ";
        }
        insert << std::endl;
    }

    return insert.str();
}

ComponentInsert::ComponentInsert() {
    apertureData = std::nullopt;
    machineData = std::nullopt;
    sideIronData = std::nullopt;
    materialData = std::nullopt;
    materialPriceData = std::nullopt;
    extraPriceData = std::nullopt;
}

void ComponentInsert::serialise(void *target) const {
    unsigned char *buff = (unsigned char *)target;

    *((RequestType *)buff) = RequestType::ADD_NEW_COMPONENT;
    buff += sizeof(RequestType);

    *((InsertType *)buff) = insertType;
    buff += sizeof(InsertType);

    *((ComponentInsertResponse *)buff) = responseCode;
    buff += sizeof(ComponentInsertResponse);

    switch (insertType) {
        case InsertType::NONE:
            break;
        case InsertType::APERTURE:
            *((float *)buff) = apertureData->width;
            buff += sizeof(float);
            *((float *)buff) = apertureData->length;
            buff += sizeof(float);
            *((unsigned *)buff) = apertureData->baseWidth;
            buff += sizeof(unsigned);
            *((unsigned *)buff) = apertureData->baseLength;
            buff += sizeof(unsigned);
            *((unsigned *)buff) = apertureData->quantity;
            buff += sizeof(unsigned);
            *((unsigned *)buff) = apertureData->shapeID;
            buff += sizeof(unsigned);
            break;
        case InsertType::MACHINE: {
            unsigned char manufacturerSize = machineData->manufacturer.size(),
                modelSize = machineData->model.size();
            *buff++ = manufacturerSize;
            memcpy(buff, machineData->manufacturer.c_str(), manufacturerSize);
            buff += manufacturerSize;
            *buff++ = modelSize;
            memcpy(buff, machineData->model.c_str(), modelSize);
            buff += modelSize;
            break;
        }
        case InsertType::SIDE_IRON: {
            *((SideIronType*)buff) = sideIronData->type;
            buff += sizeof(SideIronType);
            *((unsigned*)buff) = sideIronData->length;
            buff += sizeof(unsigned);
            unsigned char drawingNumberSize = sideIronData->drawingNumber.size(),
                hyperlinkSize = sideIronData->hyperlink.generic_string().size();
            *buff++ = drawingNumberSize;
            memcpy(buff, sideIronData->drawingNumber.c_str(), drawingNumberSize);
            buff += drawingNumberSize;
            *buff++ = hyperlinkSize;
            memcpy(buff, sideIronData->hyperlink.generic_string().c_str(), hyperlinkSize);
            buff += hyperlinkSize;
            break;
        }
        case InsertType::SIDE_IRON_PRICE: {
            *((SideIronType*)buff) = sideIronPriceData->type;
            buff += sizeof(SideIronType);

            *((bool*)buff) = sideIronPriceData->extraflex;
            buff += sizeof(bool);

            *((float*)buff) = sideIronPriceData->length;
            buff += sizeof(float);

            *((float*)buff) = sideIronPriceData->price;
            buff += sizeof(float);

            *((PriceMode*)buff) = sideIronPriceData->priceMode;
            buff += sizeof(PriceMode);

            *((unsigned*)buff) = sideIronPriceData->screws;
            buff += sizeof(unsigned);

            *((unsigned*)buff) = sideIronPriceData->sideIronPriceId;
            buff += sizeof(unsigned);

            break;
        }
        case InsertType::MATERIAL: {
            unsigned char nameSize = materialData->materialName.size();
            *buff++ = nameSize;
            memcpy(buff, materialData->materialName.c_str(), nameSize);
            buff += nameSize;
            *((unsigned *)buff) = materialData->hardness;
            buff += sizeof(unsigned);
            *((unsigned *)buff) = materialData->thickness;
            buff += sizeof(unsigned);
            break;
        }
        case InsertType::MATERIAL_PRICE: {
            *((unsigned *)buff) = materialPriceData->material_id;;
            buff += sizeof(unsigned);
            *((float*)buff) = materialPriceData->length;
            buff += sizeof(float);
            *((float*)buff) = materialPriceData->width;
            buff += sizeof(float);
            *((float*)buff) = materialPriceData->price;
            buff += sizeof(float);
            *((MaterialPricingType*)buff) = materialPriceData->pricingType;
            buff += sizeof(MaterialPricingType);
            *((PriceMode*)buff) = materialPriceData->priceMode;
            buff += sizeof(PriceMode);
            *((float*)buff) = materialPriceData->oldWidth;
            buff += sizeof(float);
            *((float*)buff) = materialPriceData->oldLength;
            buff += sizeof(float);

            break;
        }
        case (InsertType::EXTRA_PRICE): {
            *((unsigned*)buff) = extraPriceData->priceId;
            buff += sizeof(unsigned);
            *((ExtraPriceType*)buff) = extraPriceData->type;
            buff += sizeof(ExtraPriceType);
            *((float*)buff) = extraPriceData->price;
            buff += sizeof(float);
            switch (extraPriceData->type) {
                case (ExtraPriceType::SIDE_IRON_NUTS):
                    *((unsigned*)buff) = extraPriceData->amount;
                    buff += sizeof(unsigned);
                    break;
                case (ExtraPriceType::SIDE_IRON_SCREWS):
                    *((unsigned*)buff) = extraPriceData->amount;
                    buff += sizeof(unsigned);
                    break;
                case (ExtraPriceType::TACKYBACK_GLUE):
                    *((float*)buff) = extraPriceData->squareMetres;
                    buff += sizeof(float);
                    break;
                case (ExtraPriceType::LABOUR):
                    break;
                default:
                    break;
            }
            break;
        }
    }
}

unsigned int ComponentInsert::serialisedSize() const {
    switch (insertType) {
        case InsertType::APERTURE:
            return sizeof(RequestType) + sizeof(InsertType) + sizeof(ComponentInsertResponse) + apertureData->serialisedSize();
        case InsertType::MACHINE:
            return sizeof(RequestType) + sizeof(InsertType) + sizeof(ComponentInsertResponse) + machineData->serialisedSize();
        case InsertType::SIDE_IRON:
            return sizeof(RequestType) + sizeof(InsertType) + sizeof(ComponentInsertResponse) + sideIronData->serialisedSize();
        case InsertType::SIDE_IRON_PRICE:
            return sizeof(RequestType) + sizeof(InsertType) + sizeof(ComponentInsertResponse) + sideIronPriceData->serialisedSize();
        case InsertType::MATERIAL:
            return sizeof(RequestType) + sizeof(InsertType) + sizeof(ComponentInsertResponse) + materialData->serialisedSize();
        case InsertType::MATERIAL_PRICE:
            return sizeof(RequestType) + sizeof(InsertType) + sizeof(ComponentInsertResponse) + materialPriceData->serialisedSize();
        case InsertType::EXTRA_PRICE:
            return sizeof(RequestType) + sizeof(InsertType) + sizeof(ComponentInsertResponse) + extraPriceData->serialisedSize();
        case InsertType::NONE:
        default:
            return sizeof(RequestType) + sizeof(InsertType) + sizeof(ComponentInsertResponse);
    }
}

ComponentInsert &ComponentInsert::deserialise(void *data) {
    ComponentInsert *insert = new ComponentInsert();

    unsigned char *buff = (unsigned char *)data + sizeof(RequestType);

    insert->insertType = *((InsertType *)buff);
    buff += sizeof(InsertType);

    insert->responseCode = *((ComponentInsertResponse *)buff);
    buff += sizeof(ComponentInsertResponse);

    switch (insert->insertType) {
        case InsertType::NONE:
            break;
        case InsertType::APERTURE: {
            ApertureData data;
            data.width = *((float *)buff);
            buff += sizeof(float);
            data.length = *((float *)buff);
            buff += sizeof(float);
            data.baseWidth = *((unsigned *)buff);
            buff += sizeof(unsigned);
            data.baseLength = *((unsigned *)buff);
            buff += sizeof(unsigned);
            data.quantity = *((unsigned *)buff);
            buff += sizeof(unsigned);
            data.shapeID = *((unsigned *)buff);
            buff += sizeof(unsigned);

            insert->apertureData = data;
            break;
        }
        case InsertType::MACHINE: {
            MachineData data;
            unsigned char manufacturerSize = *buff++;
            data.manufacturer = std::string((const char *)buff, manufacturerSize);
            buff += manufacturerSize;
            unsigned char modelSize = *buff++;
            data.model = std::string((const char *)buff, modelSize);
            buff += modelSize;

            insert->machineData = data;
            break;
        }
        case InsertType::SIDE_IRON: {
            SideIronData data;
            data.type = *((SideIronType*)buff);
            buff += sizeof(SideIronType);
            data.length = *((unsigned*)buff);
            buff += sizeof(unsigned);
            unsigned char drawingNumberSize = *buff++;
            data.drawingNumber = std::string((const char*)buff, drawingNumberSize);
            buff += drawingNumberSize;
            unsigned char hyperlinkSize = *buff++;
            data.hyperlink = std::string((const char*)buff, hyperlinkSize);
            buff += hyperlinkSize;

            insert->sideIronData = data;
            break;
        }
        case InsertType::SIDE_IRON_PRICE: {
            SideIronPriceData data;

            data.type = *((SideIronType*)buff);
            buff += sizeof(SideIronType);

            data.extraflex = *((bool*)buff);
            buff += sizeof(bool);

            data.length = *((float*)buff);
            buff += sizeof(float);

            data.price = *((float*)buff);
            buff += sizeof(float);

            data.priceMode = *((PriceMode*)buff);
            buff += sizeof(PriceMode);

            data.screws = *((unsigned*)buff);
            buff += sizeof(unsigned);

            data.sideIronPriceId = *((unsigned*)buff);
            buff += sizeof(unsigned);

            insert->sideIronPriceData = data;
            break;
        }
        case InsertType::MATERIAL: {
            MaterialData data;
            unsigned char nameSize = *buff++;
            data.materialName = std::string((const char *)buff, nameSize);
            buff += nameSize;
            data.hardness = *((unsigned *)buff);
            buff += sizeof(unsigned);
            data.thickness = *((unsigned *)buff);
            buff += sizeof(unsigned);

            insert->materialData = data;
            break;
        }
        case InsertType::MATERIAL_PRICE: {
            MaterialPriceData data;
            data.material_id = *((unsigned*)buff);
            buff += sizeof(unsigned);
            data.length = *((float*)buff);
            buff += sizeof(float);
            data.width = *((float*)buff);
            buff += sizeof(float);
            data.price = *((float*)buff);
            buff += sizeof(float);
            data.pricingType = *((MaterialPricingType*)buff);
            buff += sizeof(MaterialPricingType);
            data.priceMode = *((PriceMode*)buff);
            buff += sizeof(PriceMode);
            data.oldWidth = *((float*)buff);
            buff += sizeof(float);
            data.oldLength = *((float*)buff);
            buff += sizeof(float);

            insert->materialPriceData = data;
            break;
        }
        case InsertType::EXTRA_PRICE :
            ExtraPriceData data;

            data.priceId = *((unsigned*)buff);
            buff += sizeof(unsigned);

            data.type = *((ExtraPriceType*)buff);
            buff += sizeof(ExtraPriceType);

            data.price = *((float*)buff);
            buff += sizeof(float);

            switch (data.type) {
                case ExtraPriceType::SIDE_IRON_NUTS:
                    data.amount = *((unsigned*)buff);
                    buff += sizeof(unsigned);
                    break;
                case ExtraPriceType::SIDE_IRON_SCREWS:
                    data.amount = *((unsigned*)buff);
                    buff += sizeof(unsigned);
                    break;
                case ExtraPriceType::TACKYBACK_GLUE:
                    data.squareMetres = *((float*)buff);
                    buff += sizeof(float);
                    break;
                case ExtraPriceType::LABOUR:
                    break;
                default:
                    break;
            }
            insert->extraPriceData = data;
            break;
    }

    return *insert;
}

template<>
void ComponentInsert::setComponentData(const ApertureData &data) {
    insertType = InsertType::APERTURE;
    responseCode = ComponentInsertResponse::NONE;

    apertureData = data;
}

template<>
void ComponentInsert::setComponentData(const MachineData &data) {
    insertType = InsertType::MACHINE;
    responseCode = ComponentInsertResponse::NONE;

    machineData = data;
}

template<>
void ComponentInsert::setComponentData(const SideIronData &data) {
    insertType = InsertType::SIDE_IRON;
    responseCode = ComponentInsertResponse::NONE;

    sideIronData = data;
}

template<>
void ComponentInsert::setComponentData(const SideIronPriceData& data) {
    insertType = InsertType::SIDE_IRON_PRICE;

    responseCode = ComponentInsertResponse::NONE;

    sideIronPriceData = data;
}

template<>
void ComponentInsert::setComponentData(const MaterialData& data) {
    insertType = InsertType::MATERIAL;
    responseCode = ComponentInsertResponse::NONE;

    materialData = data;
}

template<>
void ComponentInsert::setComponentData(const MaterialPriceData& data) {
    insertType = InsertType::MATERIAL_PRICE;
    responseCode = ComponentInsertResponse::NONE;

    materialPriceData = data;
}

template<>
void ComponentInsert::setComponentData(const ExtraPriceData& data) {
    insertType = InsertType::EXTRA_PRICE;
    responseCode = ComponentInsertResponse::NONE;

    extraPriceData = data;
}

std::string ComponentInsert::toSQLQueryString() const {
    std::stringstream insert;

    switch (insertType) {
        case InsertType::APERTURE:
            insert << "INSERT INTO {0}.apertures (width, length, base_width, base_length, quantity, shape_id)" << std::endl;
            insert << "VALUES" << std::endl;
            insert << "(" << apertureData->width << ", " << apertureData->length << ", " << apertureData->baseWidth << ", "
                << apertureData->baseLength << ", " << apertureData->quantity << ", " << apertureData->shapeID << ")" << std::endl;
            break;
        case InsertType::MACHINE:
            insert << "INSERT INTO {0}.machines (manufacturer, model)" << std::endl;
            insert << "VALUES" << std::endl;
            insert << "('" << machineData->manufacturer << "', '" << machineData->model << "')" << std::endl;
            break;
        case InsertType::SIDE_IRON:
            insert << "INSERT INTO {0}.side_irons (type, length, drawing_number, hyperlink)" << std::endl;
            insert << "VALUES" << std::endl;
            insert << "(" << (unsigned)sideIronData->type << ", " << sideIronData->length << ", '" << sideIronData->drawingNumber << "', '"
                << sideIronData->hyperlink.generic_string() << "')" << std::endl;
            break;
        case InsertType::SIDE_IRON_PRICE:
            switch (sideIronPriceData->priceMode) {
                case PriceMode::ADD:
                    insert << "INSERT INTO {0}.side_iron_prices (type, length, price, screws, product_id)" << std::endl;
                    insert << "VALUES" << std::endl;
                    insert << "(" << (unsigned)sideIronPriceData->type << ", " << sideIronPriceData->length << ", " << sideIronPriceData->price << ", " << sideIronPriceData->screws << ", " << (int)sideIronPriceData->extraflex + 1 << ")" << std::endl;
                    break;
                case PriceMode::UPDATE:
                    insert << "UPDATE {0}.side_iron_prices SET" << std::endl;
                    insert << "length = " << sideIronPriceData->length << ", price = " << sideIronPriceData->price << ", product_id="  << (int)sideIronPriceData->extraflex + 1 << std::endl;
                    insert << "WHERE side_iron_price_id = " << sideIronPriceData->sideIronPriceId << std::endl;
                    break;
                case PriceMode::REMOVE:
                    insert << "DELETE FROM {0}.side_iron_prices" << std::endl;
                    insert << "WHERE side_iron_price_id = " << sideIronPriceData->sideIronPriceId << std::endl;
                    break;
                default:
                    break;
            }
            break;
        case InsertType::MATERIAL:
            insert << "INSERT INTO {0}.materials (material, hardness, thickness)" << std::endl;
            insert << "VALUES" << std::endl;
            insert << "('" << materialData->materialName << "', " << materialData->hardness << ", " << materialData->thickness << ")" << std::endl;
            break;
        case InsertType::MATERIAL_PRICE:
            switch (materialPriceData->priceMode) {
                case (PriceMode::ADD): 
                    insert << "INSERT INTO {0}.material_prices (material_id, width, length, price, pricing)" << std::endl;
                    insert << "VALUES" << std::endl;
                    if (materialPriceData->pricingType == MaterialPricingType::RUNNING_M) {
                        insert << "(" << materialPriceData->material_id << ", " << materialPriceData->width << ", " << materialPriceData->length << ", " << materialPriceData->price << ", 'running_m')" << std::endl;
                    }
                    else if (materialPriceData->pricingType == MaterialPricingType::SQUARE_M) {
                        insert << "(" << materialPriceData->material_id << ", " << materialPriceData->width << ", " << materialPriceData->length << ", " << materialPriceData->price << ", 'square_m')" << std::endl;
                    }
                    else if (materialPriceData->pricingType == MaterialPricingType::SHEET) {
                        insert << "(" << materialPriceData->material_id << ", " << materialPriceData->width << ", " << materialPriceData->length << ", " << materialPriceData->price << ", 'sheet')" << std::endl;
                    }
                    break;

                case (PriceMode::UPDATE):
                    if (materialPriceData->pricingType == MaterialPricingType::RUNNING_M) {
                        insert << "UPDATE {0}.material_prices SET" << std::endl;
                        insert << "width = " << materialPriceData->width << ", length = " << materialPriceData->length << ", price = " << materialPriceData->price << ", pricing = " << "'running_m'" << std::endl;
                        insert << "WHERE " << std::endl;
                        insert << "material_id = " << materialPriceData->material_id << " AND width = " << materialPriceData->oldWidth << " AND length = " << materialPriceData->oldLength;
                    }
                    else if (materialPriceData->pricingType == MaterialPricingType::SQUARE_M) {
                        insert << "UPDATE {0}.material_prices SET" << std::endl;
                        insert << "width = " << materialPriceData->width << ", length = " << materialPriceData->length << ", price = " << materialPriceData->price << ", pricing = " << "'square_m'" << std::endl;
                        insert << "WHERE " << std::endl;
                        insert << "material_id = " << materialPriceData->material_id << " AND width = " << materialPriceData->oldWidth << " AND length = " << materialPriceData->oldLength;
                    }
                    else if (materialPriceData->pricingType == MaterialPricingType::SHEET) {
                        insert << "UPDATE {0}.material_prices SET" << std::endl;
                        insert << "width = " << materialPriceData->width << ", length = " << materialPriceData->length << ", price = " << materialPriceData->price << ", pricing = " << "'sheet'" << std::endl;
                        insert << "WHERE " << std::endl;
                        insert << "material_id = " << materialPriceData->material_id << " AND width = " << materialPriceData->oldWidth << " AND length = " << materialPriceData->oldLength;
                    }
                    break;
                case (PriceMode::REMOVE):
                    insert << "DELETE FROM {0}.material_prices" << std::endl;
                    insert << "WHERE material_id = " << materialPriceData->material_id << " AND width = " << materialPriceData->width << " AND length = " << materialPriceData->length;
                    break;
                default:
                    break;
            }
            break;
        case InsertType::EXTRA_PRICE:
            switch (extraPriceData->type) {
                case ExtraPriceType::SIDE_IRON_NUTS:
                    insert << "UPDATE {0}.extra_prices SET" << std::endl;
                    insert << "price = " << extraPriceData->price << ", amount = " << extraPriceData->amount << " WHERE" << std::endl;
                    insert << "price_id = " << extraPriceData->priceId << " AND type = 'side_iron_nuts'" << std::endl;
                    break;
                case ExtraPriceType::SIDE_IRON_SCREWS:
                    insert << "UPDATE {0}.extra_prices SET" << std::endl;
                    insert << "price = " << extraPriceData->price << ", amount = " << extraPriceData->amount << " WHERE" << std::endl;
                    insert << "price_id = " << extraPriceData->priceId << " AND type = 'side_iron_screws'" << std::endl;
                    break;
                case ExtraPriceType::TACKYBACK_GLUE:
                    insert << "UPDATE {0}.extra_prices SET" << std::endl;
                    insert << "price = " << extraPriceData->price << ", square_metres = " << extraPriceData->squareMetres << " WHERE" << std::endl;
                    insert << "price_id = " << extraPriceData->priceId << " AND type = 'glue'" << std::endl;
                    break;
                case ExtraPriceType::LABOUR:
                    insert << "UPDATE {0}.extra_prices SET" << std::endl;
                    insert << "price = " << extraPriceData->price << " WHERE" << std::endl;
                    insert << "price_id = " << extraPriceData->priceId << " AND type = 'labour'" << std::endl;
                    break;
            }
            break;
        default:
            break;
    }

    return insert.str();
}

RequestType ComponentInsert::getSourceTableCode() const {
    switch (insertType) {
        case InsertType::APERTURE:
            return RequestType::SOURCE_APERTURE_TABLE;
        case InsertType::MACHINE:
            return RequestType::SOURCE_MACHINE_TABLE;
        case InsertType::SIDE_IRON:
            return RequestType::SOURCE_SIDE_IRON_TABLE;
        case InsertType::SIDE_IRON_PRICE:
            return RequestType::SOURCE_SIDE_IRON_PRICES_TABLE;
        case InsertType::MATERIAL:
            return RequestType::SOURCE_MATERIAL_TABLE;
        case InsertType::MATERIAL_PRICE:
            return RequestType::SOURCE_MATERIAL_TABLE;
        case InsertType::EXTRA_PRICE:
            return RequestType::SOURCE_EXTRA_PRICES_TABLE;
        default:
            return (RequestType)(-1);
    }
}

void ComponentInsert::clearComponentData() {
    insertType = InsertType::NONE;
}

void DatabaseBackup::serialise(void *target) const {
    unsigned char *buff = (unsigned char *)target;

    *((RequestType *)buff) = RequestType::CREATE_DATABASE_BACKUP;
    buff += sizeof(RequestType);

    *((BackupResponse *)buff) = responseCode;
    buff += sizeof(BackupResponse);

    unsigned char backupNameSize = MIN(255, backupName.size());
    *buff++ = backupNameSize;
    memcpy(buff, backupName.c_str(), backupNameSize);
}

unsigned int DatabaseBackup::serialisedSize() const {
    return sizeof(RequestType) + sizeof(BackupResponse) + sizeof(unsigned char) + backupName.size();
}

DatabaseBackup &DatabaseBackup::deserialise(void *data) {
    DatabaseBackup *backup = new DatabaseBackup();

    unsigned char *buff = (unsigned char *)data + sizeof(RequestType);

    backup->responseCode = *((BackupResponse *)buff);
    buff += sizeof(BackupResponse);

    unsigned char backupNameSize = *buff++;
    backup->backupName = std::string((const char *)buff, backupNameSize);

    return *backup;
}

void NextDrawing::serialise(void *target) const {
    unsigned char *buff = (unsigned char *) target;

    *((RequestType *) buff) = RequestType::GET_NEXT_DRAWING_NUMBER;
    buff += sizeof(RequestType);

    *((DrawingType *) buff) = drawingType;
    buff += sizeof(DrawingType);

    *buff++ = drawingNumber.has_value();

    if (drawingNumber.has_value()) {
        unsigned char drawingNumberSize = MIN(255, drawingNumber->size());
        *buff++ = drawingNumberSize;
        memcpy(buff, drawingNumber->c_str(), drawingNumberSize);
    }
}

unsigned int NextDrawing::serialisedSize() const {
    return sizeof(RequestType) + sizeof(DrawingType) + sizeof(unsigned char) + (drawingNumber.has_value() ? sizeof(unsigned char) + drawingNumber->size() : 0);
}

NextDrawing &NextDrawing::deserialise(void *data) {
    NextDrawing *next = new NextDrawing();

    unsigned char *buff = (unsigned char *) data + sizeof(RequestType);

    next->drawingType = *((DrawingType *) buff);
    buff += sizeof(DrawingType);

    bool hasDrawingNumber = *buff++;

    if (hasDrawingNumber) {
        unsigned char drawingNumberSize = *buff++;
        next->drawingNumber = std::string((const char *) buff, drawingNumberSize);
    } else {
        next->drawingNumber = std::nullopt;
    }

    return *next;
}
