//
// Created by matthew on 06/07/2020.
//

#ifndef DATABASE_MANAGER_DRAWING_H
#define DATABASE_MANAGER_DRAWING_H

#include <sstream>
#include <cstring>
#include <numeric>
#include <regex>

#include "drawingComponents.h"
#include "../../packer.h"

#define MIN_COVERING_BYTES(x) (((x) / 8) + ((x) % 8 != 0))
#define MIN_COVERING_BITS(x) (32u - __builtin_clz(x))

typedef unsigned char byte;

void writeAtBitOffset(void *value, size_t valueByteLength, void *target, size_t bitOffset);

void readFromBitOffset(void *data, size_t bitOffset, void *target, size_t bitReadSize);

template<typename T>
std::string to_str(const T &t) {
    std::stringstream ss;
    ss << t;
    return ss.str();
}

PACK(struct Date {
    unsigned short year;
    unsigned char month, day;

    std::string toMySQLDateString() const;

    static Date parse(const std::string &dateString);
});

struct DrawingSerialiser;
struct DrawingSummary;

struct Drawing {
    friend struct DrawingSerialiser;
public:
    enum LoadWarning {
        LOAD_FAILED = 0x01,
        INVALID_LAPS_DETECTED = 0x02,
        MISSING_SIDE_IRONS_DETECTED = 0x04,
        MISSING_MATERIAL_DETECTED = 0x08
    };

    enum BuildWarning {
        SUCCESS,
        INVALID_DRAWING_NUMBER,
        INVALID_PRODUCT,
        INVALID_WIDTH,
        INVALID_LENGTH,
        INVALID_TOP_MATERIAL,
        INVALID_BOTTOM_MATERIAL,
        INVALID_APERTURE,
        INVALID_BAR_SPACINGS,
        INVALID_BAR_WIDTHS,
        INVALID_SIDE_IRONS,
        INVALID_MACHINE,
        INVALID_MACHINE_POSITION,
        INVALID_MACHINE_DECK,
        INVALID_HYPERLINK
    };

    struct MachineTemplate {
        friend struct Drawing;

        unsigned quantityOnDeck;
        std::string position;

        inline MachineTemplate() = default;

        inline MachineTemplate(const MachineTemplate &machineTemplate) {
            this->machineID = machineTemplate.machineID;
            this->quantityOnDeck = machineTemplate.quantityOnDeck;
            this->position = machineTemplate.position;
            this->deckID = machineTemplate.deckID;
        }

        inline Machine &machine() const {
            return DrawingComponentManager<Machine>::getComponentByID(machineID);
        }

        inline MachineDeck &deck() const {
            return DrawingComponentManager<MachineDeck>::getComponentByID(deckID);
        }

    private:
        unsigned machineID, deckID;
    };

    struct Lap {
        friend struct Drawing;

        float width;
        LapAttachment attachmentType;

        inline Lap() {
            width = 0;
            attachmentType = LapAttachment::INTEGRAL;
            materialID = 0;
        }

        inline Lap(float width, LapAttachment attachmentType, const Material &material) {
            this->width = width;
            this->attachmentType = attachmentType;
            setMaterial(material);
        }

        inline Lap(const Lap &lap) {
            this->width = lap.width;
            this->attachmentType = lap.attachmentType;
            setMaterial(lap.materialID);
        }

        inline Material &material() const {
            return DrawingComponentManager<Material>::getComponentByID(materialID);
        }

        inline void setMaterial(unsigned id) {
            materialID = id;
        }

        inline void setMaterial(const Material &material) {
            setMaterial(material.componentID);
        }

        inline std::string strAsSidelap() const {
            std::stringstream ss;
            ss << width << "mm " << (attachmentType == LapAttachment::INTEGRAL ? "integral" : "bonded") << " sidelap (" << material().material() << ")";
            return ss.str();
        }

        inline std::string strAsOverlap() const {
            std::stringstream ss;
            ss << width << "mm " << (attachmentType == LapAttachment::INTEGRAL ? "integral" : "bonded") << " overlap (" << material().material() << ")";
            return ss.str();
        }

    private:
        unsigned materialID;
    };

    enum Side {
        LEFT,
        RIGHT
    };

    enum MaterialLayer {
        TOP,
        BOTTOM
    };

    enum TensionType {
        SIDE,
        END
    };

    Drawing();

    explicit Drawing(const Drawing &drawing);

    std::string drawingNumber() const;

    void setDrawingNumber(const std::string &newDrawingNumber);

    Date date() const;

    void setDate(Date newDate);

    float width() const;

    void setWidth(float newWidth);

    float length() const;

    void setLength(float newLength);

    std::string hyperlink() const;

    void setHyperlink(const std::string &newHyperlink);

    std::string notes() const;

    void setNotes(const std::string &newNotes);

    MachineTemplate machineTemplate() const;

    void setMachineTemplate(const Machine &machine, unsigned quantityOnDeck, const std::string &position, const MachineDeck &deck);

    void setMachine(const Machine &machine);

    void setQuantityOnDeck(unsigned quantityOnDeck);

    void setMachinePosition(const std::string &position);

    void setMachineDeck(const MachineDeck &deck);

    Product product() const;

    void setProduct(const Product &prod);

    Aperture aperture() const;

    void setAperture(const Aperture &ap);

    ApertureDirection apertureDirection() const;

    void setApertureDirection(ApertureDirection direction);

    TensionType tensionType() const;

    void setTensionType(TensionType newTensionType);

    std::optional<Material> material(MaterialLayer layer) const;

    void setMaterial(MaterialLayer layer, const Material &mat);

    void removeBottomLayer();

    unsigned numberOfBars() const;

    void setBars(const std::vector<float>& spacings, const std::vector<float>& widths);

    float barSpacing(unsigned index) const;

    float barWidth(unsigned index) const;

    float leftBar() const;

    float rightBar() const;

    SideIron sideIron(Side side) const;

    bool sideIronInverted(Side side) const;

    void setSideIron(Side side, const SideIron &sideIron);

    void setSideIronInverted(Side side, bool inverted);

    void removeSideIron(Side side);

    std::optional<Lap> sidelap(Side side) const;

    void setSidelap(Side side, const Lap& lap);

    void removeSidelap(Side side);

    std::optional<Lap> overlap(Side side) const;

    void setOverlap(Side side, const Lap& lap);

    void removeOverlap(Side side);

    std::vector<std::string> pressDrawingHyperlinks() const;

    void setPressDrawingHyperlinks(const std::vector<std::string> &hyperlinks);

    bool hasSidelaps() const;

    bool hasOverlaps() const;

    BuildWarning checkDrawingValidity() const;

    bool loadWarning(LoadWarning warning) const;

    void setLoadWarning(LoadWarning warning);

    void addUpdateCallback(const std::function<void()> &callback);

private:
    void invokeUpdateCallbacks() const;

    static constexpr char drawingNumberRegexPattern[] = "^([a-zA-Z]{1,2}[0-9]{2}[a-zA-Z]?|M[0-9]{3}[a-zA-Z]?)$";
    static constexpr char positionRegexPattern[] = "(^$)|(^[0-9]+([-][0-9]+)?$)|(^[Aa][Ll]{2}$)";

    std::string __drawingNumber;
    Date __date;
    float __width, __length;
    std::string __hyperlink;
    std::string __notes;
    MachineTemplate __machineTemplate;

    unsigned productID;
    unsigned apertureID;
    ApertureDirection __apertureDirection;

    TensionType __tensionType;

    std::vector<std::string> __pressDrawingHyperlinks;

    std::vector<float> barSpacings;
    std::vector<float> barWidths;

    unsigned sideIronIDs[2];
    bool sideIronsInverted[2];

    std::optional<Lap> sidelaps[2], overlaps[2];

    unsigned topLayerThicknessID;
    std::optional<unsigned> bottomLayerThicknessID;

    unsigned loadWarnings = 0;

    std::vector<std::function<void()>> updateCallbacks;
};

struct DrawingSerialiser {
    static void serialise(const Drawing &drawing, void *target);

    static unsigned serialisedSize(const Drawing &drawing);

    static Drawing &deserialise(void *data);
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

PACK(struct DrawingSummaryCompressionSchema {
    DrawingSummaryCompressionSchema(unsigned maxMatID, float maxWidth, float maxLength, unsigned maxThicknessID,
        float maxLapSize, unsigned maxApertureID, unsigned char maxDrawingLength);

    unsigned compressedSize(const DrawingSummary &summary) const;

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
});

#endif //DATABASE_MANAGER_DRAWING_H
