//
// Created by matthew on 06/07/2020.
//

#ifndef DATABASE_MANAGER_DRAWING_H
#define DATABASE_MANAGER_DRAWING_H

#include <sstream>
#include <cstring>

#include "drawingComponents.h"

#define MIN_COVERING_BYTES(x) (((x) / 8) + ((x) % 8 != 0))
#define MIN_COVERING_BITS(x) (32u - __builtin_clz(x))

typedef unsigned char byte;

void writeAtBitOffset(void *value, size_t valueByteLength, void *target, size_t bitOffset);

void readFromBitOffset(void *data, size_t bitOffset, void *target, size_t bitReadSize);

struct Drawing {

};

struct DrawingSummaryCompressionSchema;

struct DrawingSummary {
    friend class DrawingSummaryCompressionSchema;

    unsigned matID;
    unsigned thicknessIDs[2];
    unsigned apertureID;
    std::string drawingNumber;

    bool hasTwoLayers() const;

    unsigned numberOfLaps() const;

    std::string summaryString() const;

    float width() const;

    float length() const;

    void setWidth(float width);

    void setLength(float length);

    float lapSize(unsigned index) const;

    void setLapSize(unsigned index, float size);

private:
    unsigned __width, __length;

    unsigned __lapSizes[4];
};

struct DrawingSummaryCompressionSchema {
    DrawingSummaryCompressionSchema(unsigned maxMatID, float maxWidth, float maxLength, unsigned maxThicknessID,
            float maxLapSize, unsigned maxApertureID, unsigned char maxDrawingLength);

    unsigned compressedSize(const DrawingSummary& summary) const;

    void compressSummary(const DrawingSummary &summary, void *target) const;

    DrawingSummary uncompressSummary(void *data, unsigned &size) const;

    unsigned maxCompressedSize() const;

private:
    unsigned char matIDSize;
    unsigned char widthSize;
    unsigned char lengthSize;
    unsigned char thicknessIDSize;
    unsigned char lapSize;
    unsigned char apertureIDSize;
    unsigned char maxDrawingLength;

    unsigned char matIDBytes;
    unsigned char widthBytes;
    unsigned char lengthBytes;
    unsigned char thicknessIDBytes;
    unsigned char lapBytes;
    unsigned char apertureIDBytes;
} __attribute__((packed));

#endif //DATABASE_MANAGER_DRAWING_H
