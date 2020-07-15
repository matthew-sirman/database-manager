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

bool DrawingSummary::hasTwoLayers() const {
    return thicknessIDs[1] != 0;
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
    s << " x " << DrawingComponentManager<Material>::getComponentByID(thicknessIDs[0]).thickness;
    if (thicknessIDs[1] != 0) {
        s << "+" << DrawingComponentManager<Material>::getComponentByID(thicknessIDs[1]).thickness;
    }
    s << " x " << DrawingComponentManager<Aperture>::getComponentByID(thicknessIDs[0]).apertureName();

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
        ERROR_RAW("Invalid lap index.")
    }
    return ((float) __lapSizes[index]) / 2;
}

void DrawingSummary::setLapSize(unsigned index, float size) {
    if (index < 0 || index > 3) {
        ERROR_RAW("Invalid lap index.")
    }
    __lapSizes[index] = (unsigned) (size * 2);
}

DrawingSummaryCompressionSchema::DrawingSummaryCompressionSchema(unsigned int maxMatID, float maxWidth, float maxLength,
                                                                 unsigned int maxThicknessID, float maxLapSize,
                                                                 unsigned int maxApertureID,
                                                                 unsigned char maxDrawingLength) {
    this->matIDSize = MIN_COVERING_BITS(maxMatID);
    this->widthSize = MIN_COVERING_BITS((unsigned) (maxWidth * 2));
    this->lengthSize = MIN_COVERING_BITS((unsigned) (maxLength * 2));
    this->thicknessIDSize = MIN_COVERING_BITS(maxThicknessID);
    this->lapSize = MIN_COVERING_BITS((unsigned) (maxLapSize * 2));
    this->apertureIDSize = MIN_COVERING_BITS(maxApertureID);

    this->maxDrawingLength = maxDrawingLength;

    matIDBytes = MIN_COVERING_BYTES(matIDSize);
    widthBytes = MIN_COVERING_BYTES(widthSize);
    lengthBytes = MIN_COVERING_BYTES(lengthSize);
    thicknessIDBytes = MIN_COVERING_BYTES(thicknessIDSize);
    lapBytes = MIN_COVERING_BYTES(lapSize);
    apertureIDBytes = MIN_COVERING_BYTES(apertureIDSize);
}

unsigned DrawingSummaryCompressionSchema::compressedSize(const DrawingSummary &summary) const {
    return sizeof(unsigned char) + summary.drawingNumber.size() + MIN_COVERING_BYTES(
            matIDSize + widthSize + lengthSize + apertureIDSize + thicknessIDSize + 1 +
            (summary.hasTwoLayers() ? thicknessIDSize : 0) + 3 + summary.numberOfLaps() * lapSize);
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
    writeAtBitOffset((void *) &summary.apertureID, apertureIDBytes, buff, offset);
    offset += apertureIDSize;
    writeAtBitOffset((void *) &summary.thicknessIDs[0], thicknessIDBytes, buff, offset);
    offset += thicknessIDSize;
    bool hasTwoLayers = summary.hasTwoLayers();
    writeAtBitOffset(&hasTwoLayers, 1, buff, offset);
    offset += 1;
    if (hasTwoLayers) {
        writeAtBitOffset((void *) &summary.thicknessIDs[1], thicknessIDBytes, buff, offset);
        offset += thicknessIDSize;
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
    readFromBitOffset(buff, offset, &summary.apertureID, apertureIDSize);
    offset += apertureIDSize;
    readFromBitOffset(buff, offset, &summary.thicknessIDs[0], thicknessIDSize);
    offset += thicknessIDSize;
    bool hasTwoLayers;
    readFromBitOffset(buff, offset, &hasTwoLayers, 1);
    offset += 1;
    if (hasTwoLayers) {
        readFromBitOffset(buff, offset, &summary.thicknessIDs[1], thicknessIDSize);
        offset += thicknessIDSize;
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
            matIDSize + widthSize + lengthSize + thicknessIDSize * 2 + 4 + lapSize * 4 + apertureIDSize);
}

#pragma clang diagnostic pop