#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
//
// Created by matthew on 06/07/2020.
//

#include "../../include/database/Drawing.h"

void writeAtBitOffset(void *value, size_t valueByteLength, void *target, size_t bitOffset) {
    size_t startIndex = bitOffset / 8;
    byte offset = bitOffset % 8;
    byte writeByte;

    byte *valueBytes = (byte *) value, *targetBytes = (byte *) target + startIndex;

    if (offset == 0) {
        memcpy(targetBytes, value, valueByteLength);
        return;
    }

    writeByte = *targetBytes;

    for (unsigned i = 0; i < valueByteLength; i++) {
        writeByte |= (byte) (valueBytes[i] << offset);
        *targetBytes++ = writeByte;
        writeByte = valueBytes[i] >> (8u - offset);
    }

    *targetBytes = writeByte;
}

void readFromBitOffset(void *data, size_t bitOffset, void *target, size_t bitReadSize) {
    size_t startIndex = bitOffset / 8, fullReadLength = bitReadSize / 8;
    byte offset = bitOffset % 8, finalByteSize = bitReadSize % 8;
    byte readByte;

    byte *dataBytes = (byte *) data, *targetBytes = (byte *) target;

    if (offset == 0) {
        memcpy(target, dataBytes + startIndex, fullReadLength);

        if (finalByteSize != 0) {
            targetBytes[fullReadLength] = dataBytes[startIndex + fullReadLength] & (0xFFu >> (8u - finalByteSize));
        }

        return;
    }

    for (unsigned i = 0; i < fullReadLength; i++) {
        readByte = dataBytes[startIndex + i] >> offset;
        readByte |= (byte) (dataBytes[startIndex + i + 1] << (8u - offset));
        *targetBytes++ = readByte;
    }

    if (finalByteSize != 0) {
        readByte = dataBytes[startIndex + fullReadLength] >> offset;
        readByte |= (byte) (dataBytes[startIndex + fullReadLength + 1] << (8u - offset));
        *targetBytes = readByte & (0xFFu >> (8u - finalByteSize));
    }
}

Date::Date(unsigned year, unsigned month, unsigned day) {
    this->year = year;
    this->month = month;
    this->day = day;
}

std::string Date::toMySQLDateString() const {
    std::stringstream ss;
    ss << year << "-" << (unsigned) month << "-" << (unsigned) day << " 00:00:00";
    return ss.str();
}

Date Date::parse(std::time_t rawDate) {
    std::tm *timePoint = std::localtime(&rawDate);
    return { (unsigned)(timePoint->tm_year + 1900), (unsigned)(timePoint->tm_mon + 1), (unsigned)timePoint->tm_mday };
}

Drawing::Drawing() = default;

Drawing::Drawing(const Drawing &drawing) {
    this->__drawingNumber = drawing.__drawingNumber;
    this->__date = drawing.__date;
    this->__width = drawing.__width;
    this->__length = drawing.__length;
    this->__hyperlink = drawing.__hyperlink;
    this->__notes = drawing.__notes;
    this->__machineTemplate = drawing.__machineTemplate;
    this->productHandle = drawing.productHandle;
    this->apertureHandle = drawing.apertureHandle;
    this->__tensionType = drawing.__tensionType;
    this->__pressDrawingHyperlinks = drawing.__pressDrawingHyperlinks;
    this->barSpacings = drawing.barSpacings;
    this->barWidths = drawing.barWidths;
    this->sideIronHandles[0] = drawing.sideIronHandles[0];
    this->sideIronHandles[1] = drawing.sideIronHandles[1];
    this->sideIronsInverted[0] = drawing.sideIronsInverted[0];
    this->sideIronsInverted[1] = drawing.sideIronsInverted[1];
    this->sidelaps[0] = drawing.sidelaps[0];
    this->sidelaps[1] = drawing.sidelaps[1];
    this->overlaps[0] = drawing.overlaps[0];
    this->overlaps[1] = drawing.overlaps[1];
    this->topLayerThicknessHandle = drawing.topLayerThicknessHandle;
    this->bottomLayerThicknessHandle = drawing.bottomLayerThicknessHandle;
    this->loadWarnings = drawing.loadWarnings;
}

void Drawing::setAsDefault() {
    this->__drawingNumber = "";
    this->__date = { 2000, 1, 1 };
    this->__width = 0;
    this->__length = 0;
    this->__hyperlink = "";
    this->__notes = "";
    this->__machineTemplate = MachineTemplate();
    this->productHandle = 0;
    this->apertureHandle = 0;
    this->__tensionType = TensionType::SIDE;
    this->__pressDrawingHyperlinks = std::vector<std::string>();
    this->barSpacings = { 0 };
    this->barWidths = { 0, 0 };
    this->sideIronHandles[0] = DrawingComponentManager<SideIron>::findComponentByID(1).handle();
    this->sideIronHandles[1] = DrawingComponentManager<SideIron>::findComponentByID(1).handle();
    this->sideIronsInverted[0] = false;
    this->sideIronsInverted[1] = false;
    this->sidelaps[0] = std::nullopt;
    this->sidelaps[1] = std::nullopt;
    this->overlaps[0] = std::nullopt;
    this->overlaps[1] = std::nullopt;
    this->topLayerThicknessHandle = 0;
    this->bottomLayerThicknessHandle = std::nullopt;
    this->loadWarnings = 0;
}

std::string Drawing::drawingNumber() const {
    return __drawingNumber;
}

void Drawing::setDrawingNumber(const std::string &newDrawingNumber) {
    __drawingNumber = newDrawingNumber;
    invokeUpdateCallbacks();
}

Date Drawing::date() const {
    return __date;
}

void Drawing::setDate(Date newDate) {
    __date = newDate;
    invokeUpdateCallbacks();
}

float Drawing::width() const {
    return __width;
}

void Drawing::setWidth(float newWidth) {
    __width = newWidth;
    invokeUpdateCallbacks();
}

float Drawing::length() const {
    return __length;
}

void Drawing::setLength(float newLength) {
    __length = newLength;
    invokeUpdateCallbacks();
}

std::string Drawing::hyperlink() const {
    return __hyperlink;
}

void Drawing::setHyperlink(const std::string &newHyperlink) {
    __hyperlink = newHyperlink;
    invokeUpdateCallbacks();
}

std::string Drawing::notes() const {
    return __notes;
}

void Drawing::setNotes(const std::string &newNotes) {
    __notes = newNotes;
    invokeUpdateCallbacks();
}

Drawing::MachineTemplate Drawing::machineTemplate() const {
    return __machineTemplate;
}

void Drawing::setMachineTemplate(const Machine &machine, unsigned int quantityOnDeck, const std::string &position,
                                 const MachineDeck &deck) {
    __machineTemplate.machineHandle = machine.handle();
    __machineTemplate.quantityOnDeck = quantityOnDeck;
    __machineTemplate.position = position;
    __machineTemplate.deckHandle = deck.handle();
    invokeUpdateCallbacks();
}

void Drawing::setMachine(const Machine &machine) {
    __machineTemplate.machineHandle = machine.handle();
    invokeUpdateCallbacks();
}

void Drawing::setQuantityOnDeck(unsigned quantityOnDeck) {
    __machineTemplate.quantityOnDeck = quantityOnDeck;
    invokeUpdateCallbacks();
}

void Drawing::setMachinePosition(const std::string &position) {
    __machineTemplate.position = position;
    invokeUpdateCallbacks();
}

void Drawing::setMachineDeck(const MachineDeck &deck) {
    __machineTemplate.deckHandle = deck.handle();
    invokeUpdateCallbacks();
}

Product Drawing::product() const {
    return DrawingComponentManager<Product>::getComponentByHandle(productHandle);
}

void Drawing::setProduct(const Product &prod) {
    productHandle = prod.handle();
    invokeUpdateCallbacks();
}

Aperture Drawing::aperture() const {
    return DrawingComponentManager<Aperture>::getComponentByHandle(apertureHandle);
}

void Drawing::setAperture(const Aperture &ap) {
    apertureHandle = ap.handle();
    invokeUpdateCallbacks();
}

Drawing::TensionType Drawing::tensionType() const {
    return __tensionType;
}

void Drawing::setTensionType(Drawing::TensionType newTensionType) {
    __tensionType = newTensionType;
    invokeUpdateCallbacks();
}

std::optional<Material> Drawing::material(Drawing::MaterialLayer layer) const {
    switch (layer) {
        case TOP:
            return DrawingComponentManager<Material>::getComponentByHandle(topLayerThicknessHandle);
        case BOTTOM:
            if (bottomLayerThicknessHandle.has_value()) {
                return DrawingComponentManager<Material>::getComponentByHandle(bottomLayerThicknessHandle.value());
            } else {
                return std::nullopt;
            }
    }
    return std::nullopt;
}

void Drawing::setMaterial(Drawing::MaterialLayer layer, const Material &mat) {
    switch (layer) {
        case TOP:
            topLayerThicknessHandle = mat.handle();
            break;
        case BOTTOM:
            bottomLayerThicknessHandle = mat.handle();
            break;
    }
    invokeUpdateCallbacks();
}

void Drawing::removeBottomLayer() {
    bottomLayerThicknessHandle = std::nullopt;
    invokeUpdateCallbacks();
}

unsigned Drawing::numberOfBars() const {
    return MAX(0, barWidths.size() - 2);
}

void Drawing::setBars(const std::vector<float> &spacings, const std::vector<float> &widths) {
    barSpacings = spacings;
    barWidths = widths;
    invokeUpdateCallbacks();
}

float Drawing::barSpacing(unsigned int index) const {
    return barSpacings[index];
}

float Drawing::barWidth(unsigned int index) const {
    return barWidths[index];
}

float Drawing::leftBar() const {
    return barWidths.front();
}

float Drawing::rightBar() const {
    return barWidths.back();
}

std::vector<float> Drawing::allBarSpacings() const {
    return barSpacings;
}

std::vector<float> Drawing::allBarWidths() const {
    return barWidths;
}

SideIron Drawing::sideIron(Drawing::Side side) const {
    switch (side) {
        case LEFT:
            return DrawingComponentManager<SideIron>::getComponentByHandle(sideIronHandles[0]);
        case RIGHT:
            return DrawingComponentManager<SideIron>::getComponentByHandle(sideIronHandles[1]);
    }
    ERROR_RAW("Invalid Side Iron side requested. The side must be either Left or Right", std::cerr);
}

bool Drawing::sideIronInverted(Drawing::Side side) const {
    switch (side) {
        case LEFT:
            return sideIronsInverted[0];
        case RIGHT:
            return sideIronsInverted[1];
    }
    ERROR_RAW("Invalid Side Iron side requested. The side must be either Left or Right", std::cerr);
}

void Drawing::setSideIron(Drawing::Side side, const SideIron &sideIron) {
    switch (side) {
        case LEFT:
            sideIronHandles[0] = sideIron.handle();
            break;
        case RIGHT:
            sideIronHandles[1] = sideIron.handle();
            break;
    }
    invokeUpdateCallbacks();
}

void Drawing::setSideIronInverted(Drawing::Side side, bool inverted) {
    switch (side) {
        case LEFT:
            sideIronsInverted[0] = inverted;
            break;
        case RIGHT:
            sideIronsInverted[1] = inverted;
            break;
    }
    invokeUpdateCallbacks();
}

void Drawing::removeSideIron(Drawing::Side side) {
    switch (side) {
        case LEFT:
            sideIronHandles[0] = DrawingComponentManager<SideIron>::findComponentByID(1).handle();
            sideIronsInverted[0] = false;
            break;
        case RIGHT:
            sideIronHandles[1] = DrawingComponentManager<SideIron>::findComponentByID(1).handle();
            sideIronsInverted[1] = false;
            break;
    }
    invokeUpdateCallbacks();
}

std::optional<Drawing::Lap> Drawing::sidelap(Drawing::Side side) const {
    switch (side) {
        case LEFT:
            return sidelaps[0];
        case RIGHT:
            return sidelaps[1];
    }
    return std::nullopt;
}

void Drawing::setSidelap(Drawing::Side side, const Drawing::Lap &lap) {
    switch (side) {
        case LEFT:
            sidelaps[0] = lap;
            break;
        case RIGHT:
            sidelaps[1] = lap;
            break;
    }
    invokeUpdateCallbacks();
}

void Drawing::removeSidelap(Drawing::Side side) {
    switch (side) {
        case LEFT:
            sidelaps[0] = std::nullopt;
            break;
        case RIGHT:
            sidelaps[1] = std::nullopt;
            break;
    }
    invokeUpdateCallbacks();
}

std::optional<Drawing::Lap> Drawing::overlap(Drawing::Side side) const {
    switch (side) {
        case LEFT:
            return overlaps[0];
        case RIGHT:
            return overlaps[1];
    }
    return std::nullopt;
}

void Drawing::setOverlap(Drawing::Side side, const Drawing::Lap &lap) {
    switch (side) {
        case LEFT:
            overlaps[0] = lap;
            break;
        case RIGHT:
            overlaps[1] = lap;
            break;
    }
    invokeUpdateCallbacks();
}

void Drawing::removeOverlap(Drawing::Side side) {
    switch (side) {
        case LEFT:
            overlaps[0] = std::nullopt;
            break;
        case RIGHT:
            overlaps[1] = std::nullopt;
            break;
    }
    invokeUpdateCallbacks();
}

std::vector<std::string> Drawing::pressDrawingHyperlinks() const {
    return __pressDrawingHyperlinks;
}

void Drawing::setPressDrawingHyperlinks(const std::vector<std::string> &hyperlinks) {
    __pressDrawingHyperlinks = hyperlinks;
    invokeUpdateCallbacks();
}

bool Drawing::hasSidelaps() const {
    return sidelaps[0].has_value() || sidelaps[1].has_value();
}

bool Drawing::hasOverlaps() const {
    return overlaps[0].has_value() || overlaps[1].has_value();
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCSimplifyInspection"

Drawing::BuildWarning Drawing::checkDrawingValidity(unsigned exclusions) const {
    if (!std::regex_match(__drawingNumber, std::regex(drawingNumberRegexPattern)) && !(exclusions & INVALID_DRAWING_NUMBER)) {
        return INVALID_DRAWING_NUMBER;
    }
    if (!DrawingComponentManager<Product>::validComponentHandle(productHandle) && !(exclusions & INVALID_PRODUCT)) {
        return INVALID_PRODUCT;
    }
    if (__width <= 0 && !(exclusions & INVALID_WIDTH)) {
        return INVALID_WIDTH;
    }
    if (__length <= 0 && !(exclusions & INVALID_LENGTH)) {
        return INVALID_LENGTH;
    }
    if (!DrawingComponentManager<Material>::validComponentHandle(topLayerThicknessHandle) && !(exclusions & INVALID_TOP_MATERIAL)) {
        return INVALID_TOP_MATERIAL;
    }
    if (bottomLayerThicknessHandle.has_value()) {
        if (!DrawingComponentManager<Material>::validComponentHandle(bottomLayerThicknessHandle.value()) && 
            !(exclusions & INVALID_BOTTOM_MATERIAL)) {
            return INVALID_BOTTOM_MATERIAL;
        }
    }
    if (!DrawingComponentManager<Aperture>::validComponentHandle(apertureHandle) && !(exclusions & INVALID_APERTURE)) {
        return INVALID_APERTURE;
    }
    if (std::accumulate(barSpacings.begin(), barSpacings.end(), 0.0f) != __width && !(exclusions & INVALID_BAR_SPACINGS)) {
        return INVALID_BAR_SPACINGS;
    }
    if (std::find(barWidths.begin(), barWidths.end(), 0.0f) != barWidths.end() && !(exclusions & INVALID_BAR_WIDTHS)) {
        return INVALID_BAR_WIDTHS;
    }
    if (!DrawingComponentManager<SideIron>::validComponentHandle(sideIronHandles[0]) && !(exclusions & INVALID_SIDE_IRONS)) {
        return INVALID_SIDE_IRONS;
    }
    if (!DrawingComponentManager<SideIron>::validComponentHandle(sideIronHandles[1]) && !(exclusions & INVALID_SIDE_IRONS)) {
        return INVALID_SIDE_IRONS;
    }
    if (!DrawingComponentManager<Machine>::validComponentHandle(__machineTemplate.machineHandle) && !(exclusions & INVALID_MACHINE)) {
        return INVALID_MACHINE;
    }
    if (!std::regex_match(__machineTemplate.position, std::regex(positionRegexPattern)) && !(exclusions & INVALID_MACHINE_POSITION)) {
        return INVALID_MACHINE_POSITION;
    }
    if (!DrawingComponentManager<MachineDeck>::validComponentHandle(__machineTemplate.deckHandle) && !(exclusions & INVALID_MACHINE_DECK)) {
        return INVALID_MACHINE_DECK;
    }
    if (__hyperlink.empty() && !(exclusions & INVALID_HYPERLINK)) {
        return INVALID_HYPERLINK;
    }

    return SUCCESS;
}

#pragma clang diagnostic pop

void Drawing::setLoadWarning(Drawing::LoadWarning warning) {
    loadWarnings |= warning;
}

bool Drawing::loadWarning(Drawing::LoadWarning warning) const {
    return loadWarnings & warning;
}

void Drawing::addUpdateCallback(const std::function<void()> &callback) {
    updateCallbacks.push_back(callback);
}

void Drawing::invokeUpdateCallbacks() const {
    for (const std::function<void()> &callback : updateCallbacks) {
        if (callback) {
            callback();
        }
    }
}

void DrawingSerialiser::serialise(const Drawing &drawing, void *target) {
    unsigned char *buffer = (unsigned char *) target;

    // Drawing Number
    unsigned char drawingNumberSize = drawing.__drawingNumber.size();
    *buffer++ = drawingNumberSize;
    memcpy(buffer, drawing.__drawingNumber.c_str(), drawingNumberSize);
    buffer += drawingNumberSize;

    // Date
    memcpy(buffer, &drawing.__date, sizeof(Date));
    buffer += sizeof(Date);

    // Width
    *((float *) buffer) = drawing.__width;
    buffer += sizeof(float);

    // Length
    *((float *) buffer) = drawing.__length;
    buffer += sizeof(float);

    // Hyperlink
    unsigned char hyperlinkSize = drawing.__hyperlink.size();
    *buffer++ = hyperlinkSize;
    memcpy(buffer, drawing.__hyperlink.c_str(), hyperlinkSize);
    buffer += hyperlinkSize;

    // Notes
    unsigned char notesSize = drawing.__notes.size();
    *buffer++ = notesSize;
    memcpy(buffer, drawing.__notes.c_str(), notesSize);
    buffer += notesSize;

    // Machine Template: Machine ID, Quantity on Deck, Position string (<256 chars), Deck ID
    *((unsigned *) buffer) = drawing.__machineTemplate.machine().handle();
    buffer += sizeof(unsigned);
    *((unsigned *) buffer) = drawing.__machineTemplate.quantityOnDeck;
    buffer += sizeof(unsigned);
    unsigned char machinePositionSize = drawing.__machineTemplate.position.size();
    *buffer++ = machinePositionSize;
    memcpy(buffer, drawing.__machineTemplate.position.c_str(), machinePositionSize);
    buffer += machinePositionSize;
    *((unsigned *) buffer) = drawing.__machineTemplate.deck().handle();
    buffer += sizeof(unsigned);

    // Product ID
    *((unsigned *) buffer) = drawing.productHandle;
    buffer += sizeof(unsigned);

    // Aperture ID
    *((unsigned *) buffer) = drawing.apertureHandle;
    buffer += sizeof(unsigned);

    // Tension Type
    *buffer++ = (unsigned char) drawing.__tensionType;

    // Press Drawing Links
    *buffer++ = drawing.__pressDrawingHyperlinks.size();
    for (const std::string &pdl : drawing.__pressDrawingHyperlinks) {
        unsigned char pdlSize = pdl.size();
        *buffer++ = pdlSize;
        memcpy(buffer, pdl.c_str(), pdlSize);
        buffer += pdlSize;
    }

    // Bar spacings
    *buffer++ = drawing.barSpacings.size();
    for (float b : drawing.barSpacings) {
        *((float *) buffer) = b;
        buffer += sizeof(float);
    }

    // Bar widths
    *buffer++ = drawing.barWidths.size();
    for (float b : drawing.barWidths) {
        *((float *) buffer) = b;
        buffer += sizeof(float);
    }

    // Side Irons
    *((unsigned *) buffer) = drawing.sideIronHandles[0];
    buffer += sizeof(unsigned);
    *buffer++ = drawing.sideIronsInverted[0];
    *((unsigned *) buffer) = drawing.sideIronHandles[1];
    buffer += sizeof(unsigned);
    *buffer++ = drawing.sideIronsInverted[1];

    // A byte for flags: UNUSED, UNUSED, UNUSED, HAS_BOTTOM_LAYER, OL_R, OL_L, SL_R, SL_L
    enum Flags {
        SIDELAP_L = 0x01,
        SIDELAP_R = 0x02,
        OVERLAP_L = 0x04,
        OVERLAP_R = 0x08,
        HAS_BOTTOM_LAYER = 0x10
    };

    *buffer = 0x00;

    if (drawing.sidelaps[0].has_value()) {
        *buffer |= SIDELAP_L;
    }
    if (drawing.sidelaps[1].has_value()) {
        *buffer |= SIDELAP_R;
    }
    if (drawing.overlaps[0].has_value()) {
        *buffer |= OVERLAP_L;
    }
    if (drawing.overlaps[1].has_value()) {
        *buffer |= OVERLAP_R;
    }
    if (drawing.bottomLayerThicknessHandle.has_value()) {
        *buffer |= HAS_BOTTOM_LAYER;
    }

    buffer++;

    // Lap: Attachment Type, Width, Material ID
    // Left sidelap
    if (drawing.sidelaps[0].has_value()) {
        *buffer++ = (unsigned char) drawing.sidelaps[0]->attachmentType;
        *((float *) buffer) = drawing.sidelaps[0]->width;
        buffer += sizeof(float);
        *((unsigned *) buffer) = drawing.sidelaps[0]->material().handle();
        buffer += sizeof(unsigned);
    }
    // Right sidelap
    if (drawing.sidelaps[1].has_value()) {
        *buffer++ = (unsigned char) drawing.sidelaps[1]->attachmentType;
        *((float *) buffer) = drawing.sidelaps[1]->width;
        buffer += sizeof(float);
        *((unsigned *) buffer) = drawing.sidelaps[1]->material().handle();
        buffer += sizeof(unsigned);
    }
    // Left overlap
    if (drawing.overlaps[0].has_value()) {
        *buffer++ = (unsigned char) drawing.overlaps[0]->attachmentType;
        *((float *) buffer) = drawing.overlaps[0]->width;
        buffer += sizeof(float);
        *((unsigned *) buffer) = drawing.overlaps[0]->material().handle();
        buffer += sizeof(unsigned);
    }
    // Right overlap
    if (drawing.overlaps[1].has_value()) {
        *buffer++ = (unsigned char) drawing.overlaps[1]->attachmentType;
        *((float *) buffer) = drawing.overlaps[1]->width;
        buffer += sizeof(float);
        *((unsigned *) buffer) = drawing.overlaps[1]->material().handle();
        buffer += sizeof(unsigned);
    }

    // Top layer material ID
    *((unsigned *) buffer) = drawing.topLayerThicknessHandle;
    buffer += sizeof(unsigned);

    // Bottom layer material ID
    if (drawing.bottomLayerThicknessHandle.has_value()) {
        *((unsigned *) buffer) = drawing.bottomLayerThicknessHandle.value();
        buffer += sizeof(unsigned);
    }

    // Load warnings
    *((unsigned *) buffer) = drawing.loadWarnings;
    buffer += sizeof(unsigned);
}

unsigned DrawingSerialiser::serialisedSize(const Drawing &drawing) {
    unsigned size = 0;
    // Drawing Number
    size += sizeof(unsigned char) + drawing.__drawingNumber.size();
    // Date
    size += sizeof(Date);
    // Width
    size += sizeof(float);
    // Length
    size += sizeof(float);
    // Hyperlink
    size += sizeof(unsigned char) + drawing.__hyperlink.size();
    // Notes
    size += sizeof(unsigned char) + drawing.__notes.size();
    // Machine Template: Machine ID, Quantity on Deck, Position string (<256 chars), Deck ID
    size += sizeof(unsigned) + sizeof(unsigned) + sizeof(unsigned char) + drawing.__machineTemplate.position.size() +
            sizeof(unsigned);
    // Product ID
    size += sizeof(unsigned);
    // Aperture ID
    size += sizeof(unsigned);
    // Tension Type
    size += sizeof(unsigned char);
    // Press Drawing Links
    size += sizeof(unsigned char) +
            std::accumulate(drawing.__pressDrawingHyperlinks.begin(), drawing.__pressDrawingHyperlinks.end(),
                            0, [](unsigned t, const std::string &s) { return sizeof(unsigned char) + s.size() + t; });
    // Bar spacings
    size += sizeof(unsigned char) + drawing.barSpacings.size() * sizeof(float);
    // Bar widths
    size += sizeof(unsigned char) + drawing.barWidths.size() * sizeof(float);
    // Side Irons
    size += 2 * (sizeof(unsigned) + sizeof(unsigned char));
    // A byte for flags: UNUSED, UNUSED, UNUSED, HAS_BOTTOM_LAYER, OL_R, OL_L, SL_R, SL_L
    size += sizeof(unsigned char);
    // Lap: Attachment Type, Width, Material ID
    // Left sidelap
    if (drawing.sidelaps[0].has_value()) {
        size += sizeof(unsigned char) + sizeof(float) + sizeof(unsigned);
    }
    // Right sidelap
    if (drawing.sidelaps[1].has_value()) {
        size += sizeof(unsigned char) + sizeof(float) + sizeof(unsigned);
    }
    // Left overlap
    if (drawing.overlaps[0].has_value()) {
        size += sizeof(unsigned char) + sizeof(float) + sizeof(unsigned);
    }
    // Right overlap
    if (drawing.overlaps[1].has_value()) {
        size += sizeof(unsigned char) + sizeof(float) + sizeof(unsigned);
    }
    // Top layer material ID
    size += sizeof(unsigned);
    // Bottom layer material ID
    if (drawing.bottomLayerThicknessHandle.has_value()) {
        size += sizeof(unsigned);
    }
    // Load Warnings
    size += sizeof(unsigned);

    return size;
}

Drawing &DrawingSerialiser::deserialise(void *data) {
    Drawing *drawing = new Drawing();

    unsigned char *buffer = (unsigned char *) data;

    // Drawing Number
    unsigned char drawingNumberSize = *buffer++;
    drawing->__drawingNumber = std::string((const char *) buffer, drawingNumberSize);
    buffer += drawingNumberSize;

    // Date
    memcpy(&drawing->__date, buffer, sizeof(Date));
    buffer += sizeof(Date);

    // Width
    drawing->__width = *((float *) buffer);
    buffer += sizeof(float);

    // Length
    drawing->__length = *((float *) buffer);
    buffer += sizeof(float);

    // Hyperlink
    unsigned char hyperlinkSize = *buffer++;
    drawing->__hyperlink = std::string((const char *) buffer, hyperlinkSize);
    buffer += hyperlinkSize;

    // Notes
    unsigned char notesSize = *buffer++;
    drawing->__notes = std::string((const char *) buffer, notesSize);
    buffer += notesSize;

    // Machine Template: Machine ID, Quantity on Deck, Position string (<256 chars), Deck ID
    unsigned machineHandle = *((unsigned *) buffer);
    buffer += sizeof(unsigned);
    unsigned quantityOnDeck = *((unsigned *) buffer);
    buffer += sizeof(unsigned);
    unsigned char machinePositionSize = *buffer++;
    std::string position = std::string((const char *) buffer, machinePositionSize);
    buffer += machinePositionSize;
    unsigned deckHandle = *((unsigned *) buffer);
    buffer += sizeof(unsigned);

    drawing->setMachineTemplate(DrawingComponentManager<Machine>::getComponentByHandle(machineHandle), quantityOnDeck, position,
                                DrawingComponentManager<MachineDeck>::getComponentByHandle(deckHandle));

    // Product ID
    drawing->productHandle = *((unsigned *) buffer);
    buffer += sizeof(unsigned);

    // Aperture ID
    drawing->apertureHandle = *((unsigned *) buffer);
    buffer += sizeof(unsigned);

    // Tension Type
    drawing->__tensionType = (Drawing::TensionType) *buffer++;

    // Press Drawing Links
    unsigned char noPressDrawings = *buffer++;
    for (unsigned char pdl = 0; pdl < noPressDrawings; pdl++) {
        unsigned char pdlSize = *buffer++;
        drawing->__pressDrawingHyperlinks.emplace_back(std::string((const char *) buffer, pdlSize));
        buffer += pdlSize;
    }

    // Bar spacings
    unsigned char noBarSpacings = *buffer++;
    for (unsigned char b = 0; b < noBarSpacings; b++) {
        drawing->barSpacings.push_back(*((float *) buffer));
        buffer += sizeof(float);
    }

    // Bar widths
    unsigned char noBarWidths = *buffer++;
    for (unsigned char b = 0; b < noBarWidths; b++) {
        drawing->barWidths.push_back(*((float *) buffer));
        buffer += sizeof(float);
    }

    // Side Irons
    drawing->sideIronHandles[0] = *((unsigned *) buffer);
    buffer += sizeof(unsigned);
    drawing->sideIronsInverted[0] = *buffer++;

    drawing->sideIronHandles[1] = *((unsigned *) buffer);
    buffer += sizeof(unsigned);
    drawing->sideIronsInverted[1] = *buffer++;

    // A byte for flags: UNUSED, UNUSED, UNUSED, HAS_BOTTOM_LAYER, OL_R, OL_L, SL_R, SL_L
    enum Flags {
        SIDELAP_L = 0x01,
        SIDELAP_R = 0x02,
        OVERLAP_L = 0x04,
        OVERLAP_R = 0x08,
        HAS_BOTTOM_LAYER = 0x10
    };

    unsigned char flags = *buffer++;

    // Lap: Attachment Type, Width, Material ID
    // Left sidelap
    if (flags & SIDELAP_L) {
        LapAttachment attachment = (LapAttachment) *buffer++;
        float width = *((float *) buffer);
        buffer += sizeof(float);
        unsigned materialHandle = *((unsigned *) buffer);
        buffer += sizeof(unsigned);

        Drawing::Lap lap(width, attachment, DrawingComponentManager<Material>::getComponentByHandle(materialHandle));
        drawing->sidelaps[0] = lap;
    } else {
        drawing->sidelaps[0] = std::nullopt;
    }
    // Right sidelap
    if (flags & SIDELAP_R) {
        LapAttachment attachment = (LapAttachment) *buffer++;
        float width = *((float *) buffer);
        buffer += sizeof(float);
        unsigned materialHandle = *((unsigned *) buffer);
        buffer += sizeof(unsigned);

        Drawing::Lap lap(width, attachment, DrawingComponentManager<Material>::getComponentByHandle(materialHandle));
        drawing->sidelaps[1] = lap;
    } else {
        drawing->sidelaps[1] = std::nullopt;
    }
    // Left overlap
    if (flags & OVERLAP_L) {
        LapAttachment attachment = (LapAttachment) *buffer++;
        float width = *((float *) buffer);
        buffer += sizeof(float);
        unsigned materialHandle = *((unsigned *) buffer);
        buffer += sizeof(unsigned);

        Drawing::Lap lap(width, attachment, DrawingComponentManager<Material>::getComponentByHandle(materialHandle));
        drawing->overlaps[0] = lap;
    } else {
        drawing->overlaps[0] = std::nullopt;
    }
    // Right overlap
    if (flags & OVERLAP_R) {
        LapAttachment attachment = (LapAttachment) *buffer++;
        float width = *((float *) buffer);
        buffer += sizeof(float);
        unsigned materialHandle = *((unsigned *) buffer);
        buffer += sizeof(unsigned);

        Drawing::Lap lap(width, attachment, DrawingComponentManager<Material>::getComponentByHandle(materialHandle));
        drawing->overlaps[1] = lap;
    } else {
        drawing->overlaps[1] = std::nullopt;
    }

    // Top layer material ID
    drawing->topLayerThicknessHandle = *((unsigned *) buffer);
    buffer += sizeof(unsigned);

    // Bottom layer material ID
    if (flags & HAS_BOTTOM_LAYER) {
        drawing->bottomLayerThicknessHandle = *((unsigned *) buffer);
        buffer += sizeof(unsigned);
    } else {
        drawing->bottomLayerThicknessHandle = std::nullopt;
    }

    // Load Warnings
    drawing->loadWarnings = *((unsigned *) buffer);
    buffer += sizeof(unsigned);

    return *drawing;
}

bool DrawingSummary::hasTwoLayers() const {
    return thicknessHandles[1] != 0;
}

unsigned DrawingSummary::numberOfLaps() const {
    if (__lapSizes[3] != 0) {
        return 4;
    } else if (__lapSizes[2] != 0) {
        return 3;
    } else if (__lapSizes[1] != 0) {
        return 2;
    } else if (__lapSizes[0] != 0) {
        return 1;
    }
    return 0;
}

std::string DrawingSummary::summaryString() const {
    std::stringstream s;

    if (__lapSizes[0] != 0) {
        s << lapSize(0) << "+";
    }
    s << width();
    if (__lapSizes[1] != 0) {
        s << "+" << lapSize(1);
    }
    s << " x ";
    if (__lapSizes[2] != 0) {
        s << lapSize(2) << "+";
    }
    s << length();
    if (__lapSizes[3] != 0) {
        s << "+" << lapSize(3);
    }
    s << " x " << DrawingComponentManager<Material>::getComponentByHandle(thicknessHandles[0]).thickness;
    if (thicknessHandles[1] != 0) {
        s << "+" << DrawingComponentManager<Material>::getComponentByHandle(thicknessHandles[1]).thickness;
    }
    s << " x " << DrawingComponentManager<Aperture>::getComponentByHandle(apertureHandle).apertureName();

    return s.str();
}

float DrawingSummary::width() const {
    return ((float) __width) / 2;
}

float DrawingSummary::length() const {
    return ((float) __length) / 2;
}

void DrawingSummary::setWidth(float width) {
    __width = (unsigned) (width * 2);
}

void DrawingSummary::setLength(float length) {
    __length = (unsigned) (length * 2);
}

float DrawingSummary::lapSize(unsigned index) const {
    if (index < 0 || index > 3) {
        ERROR_RAW("Invalid lap index.", std::cerr)
    }
    return ((float) __lapSizes[index]) / 2;
}

void DrawingSummary::setLapSize(unsigned index, float size) {
    if (index < 0 || index > 3) {
        ERROR_RAW("Invalid lap index.", std::cerr)
    }
    __lapSizes[index] = (unsigned) (size * 2);
}

DrawingSummaryCompressionSchema::DrawingSummaryCompressionSchema(unsigned int maxMatID, float maxWidth, float maxLength,
                                                                 unsigned int maxThicknessHandle, float maxLapSize,
                                                                 unsigned int maxApertureHandle,
                                                                 unsigned char maxDrawingLength) {
    this->matIDSize = MIN_COVERING_BITS(maxMatID);
    this->widthSize = MIN_COVERING_BITS((unsigned) (maxWidth * 2));
    this->lengthSize = MIN_COVERING_BITS((unsigned) (maxLength * 2));
    this->thicknessHandleSize = MIN_COVERING_BITS(maxThicknessHandle);
    this->lapSize = MIN_COVERING_BITS((unsigned) (maxLapSize * 2));
    this->apertureHandleSize = MIN_COVERING_BITS(maxApertureHandle);

    this->maxDrawingLength = maxDrawingLength;

    matIDBytes = MIN_COVERING_BYTES(matIDSize);
    widthBytes = MIN_COVERING_BYTES(widthSize);
    lengthBytes = MIN_COVERING_BYTES(lengthSize);
    thicknessHandleBytes = MIN_COVERING_BYTES(thicknessHandleSize);
    lapBytes = MIN_COVERING_BYTES(lapSize);
    apertureHandleBytes = MIN_COVERING_BYTES(apertureHandleSize);
}

unsigned DrawingSummaryCompressionSchema::compressedSize(const DrawingSummary &summary) const {
    return sizeof(unsigned char) + summary.drawingNumber.size() + MIN_COVERING_BYTES(
            matIDSize + widthSize + lengthSize + apertureHandleSize + thicknessHandleSize + 1 +
            (summary.hasTwoLayers() ? thicknessHandleSize : 0) + 3 + summary.numberOfLaps() * lapSize);
}

void DrawingSummaryCompressionSchema::compressSummary(const DrawingSummary &summary, void *target) const {
    unsigned char *buff = (unsigned char *) target;
    *buff++ = summary.drawingNumber.size();
    memcpy(buff, summary.drawingNumber.c_str(), summary.drawingNumber.size());
    buff += summary.drawingNumber.size();

    unsigned offset = 0;
    writeAtBitOffset((void *) &summary.matID, matIDBytes, buff, offset);
    offset += matIDSize;
    writeAtBitOffset((void *) &summary.__width, widthBytes, buff, offset);
    offset += widthSize;
    writeAtBitOffset((void *) &summary.__length, lengthBytes, buff, offset);
    offset += lengthSize;
    writeAtBitOffset((void *) &summary.apertureHandle, apertureHandleBytes, buff, offset);
    offset += apertureHandleSize;
    writeAtBitOffset((void *) &summary.thicknessHandles[0], thicknessHandleBytes, buff, offset);
    offset += thicknessHandleSize;
    bool hasTwoLayers = summary.hasTwoLayers();
    writeAtBitOffset(&hasTwoLayers, 1, buff, offset);
    offset += 1;
    if (hasTwoLayers) {
        writeAtBitOffset((void *) &summary.thicknessHandles[1], thicknessHandleBytes, buff, offset);
        offset += thicknessHandleSize;
    }
    unsigned noOfLaps = summary.numberOfLaps();
    writeAtBitOffset(&noOfLaps, 1, buff, offset);
    offset += 3;
    for (unsigned i = 0; i < noOfLaps; i++) {
        writeAtBitOffset((void *) &summary.__lapSizes[i], lapBytes, buff, offset);
        offset += lapSize;
    }
}

DrawingSummary DrawingSummaryCompressionSchema::uncompressSummary(void *data, unsigned &size) const {
    DrawingSummary summary{};

    unsigned char *buff = (unsigned char *) data;
    unsigned char drawingNumberSize = *buff++;

    summary.drawingNumber = std::string((const char *) buff, drawingNumberSize);
    buff += drawingNumberSize;

    unsigned offset = 0;
    readFromBitOffset(buff, offset, &summary.matID, matIDSize);
    offset += matIDSize;
    readFromBitOffset(buff, offset, &summary.__width, widthSize);
    offset += widthSize;
    readFromBitOffset(buff, offset, &summary.__length, lengthSize);
    offset += lengthSize;
    readFromBitOffset(buff, offset, &summary.apertureHandle, apertureHandleSize);
    offset += apertureHandleSize;
    readFromBitOffset(buff, offset, &summary.thicknessHandles[0], thicknessHandleSize);
    offset += thicknessHandleSize;
    bool hasTwoLayers;
    readFromBitOffset(buff, offset, &hasTwoLayers, 1);
    offset += 1;
    if (hasTwoLayers) {
        readFromBitOffset(buff, offset, &summary.thicknessHandles[1], thicknessHandleSize);
        offset += thicknessHandleSize;
    }
    unsigned noOfLaps = 0;
    readFromBitOffset(buff, offset, &noOfLaps, 3);
    offset += 3;
    for (unsigned i = 0; i < noOfLaps; i++) {
        readFromBitOffset(buff, offset, &summary.__lapSizes[i], lapSize);
        offset += lapSize;
    }

    size = MIN_COVERING_BYTES(offset) + sizeof(unsigned char) + summary.drawingNumber.size();

    return summary;
}

unsigned DrawingSummaryCompressionSchema::maxCompressedSize() const {
    return sizeof(unsigned char) + maxDrawingLength + MIN_COVERING_BYTES(
            matIDSize + widthSize + lengthSize + thicknessHandleSize * 2 + 4 + lapSize * 4 + apertureHandleSize);
}

#pragma clang diagnostic pop
