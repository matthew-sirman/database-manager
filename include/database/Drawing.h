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

PACK_START
struct Date {
    unsigned short year;
    unsigned char month, day;

    Date() = default;

    Date(unsigned year, unsigned month, unsigned day);

    std::string toMySQLDateString() const;

    static Date parse(std::time_t rawDate);

    static Date today();
}
PACK_END

struct DrawingSerialiser;
struct DrawingSummary;

struct Drawing {
    friend struct DrawingSerialiser;
public:
    enum LoadWarning {
        LOAD_FAILED = 0x01,
        INVALID_LAPS_DETECTED = 0x02,
        MISSING_SIDE_IRONS_DETECTED = 0x04,
        MISSING_MATERIAL_DETECTED = 0x08,
        INVALID_APERTURE_DETECTED = 0x10,
        INVALID_IMPACT_PAD_DETECTED = 0x20
    };

    enum BuildWarning {
        SUCCESS = 0x0000,
        INVALID_DRAWING_NUMBER = 0x0001,
        INVALID_PRODUCT = 0x0002,
        INVALID_WIDTH = 0x0004,
        INVALID_LENGTH = 0x0008,
        INVALID_TOP_MATERIAL = 0x0010,
        INVALID_BOTTOM_MATERIAL = 0x0020,
        INVALID_APERTURE = 0x0040,
        INVALID_BAR_SPACINGS = 0x0080,
        INVALID_BAR_WIDTHS = 0x0100,
        INVALID_SIDE_IRONS = 0x0200,
        INVALID_MACHINE = 0x0400,
        INVALID_MACHINE_POSITION = 0x0800,
        INVALID_MACHINE_DECK = 0x1000,
        INVALID_HYPERLINK = 0x2000
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

    struct MachineTemplate {
        friend struct Drawing;

        unsigned quantityOnDeck;
        std::string position;

        inline MachineTemplate() {
            machineHandle = DrawingComponentManager<Machine>::findComponentByID(1).handle();
            quantityOnDeck = 0;
            position = std::string();
            deckHandle = DrawingComponentManager<MachineDeck>::findComponentByID(1).handle();
        }

        inline MachineTemplate(const MachineTemplate &machineTemplate) {
            this->machineHandle = machineTemplate.machineHandle;
            this->quantityOnDeck = machineTemplate.quantityOnDeck;
            this->position = machineTemplate.position;
            this->deckHandle = machineTemplate.deckHandle;
        }

        inline Machine &machine() const {
            return DrawingComponentManager<Machine>::getComponentByHandle(machineHandle);
        }

        inline MachineDeck &deck() const {
            return DrawingComponentManager<MachineDeck>::getComponentByHandle(deckHandle);
        }

    private:
        unsigned machineHandle, deckHandle;
    };

    struct Lap {
        friend struct Drawing;

        float width;
        LapAttachment attachmentType;

        inline Lap() {
            width = 0;
            attachmentType = LapAttachment::INTEGRAL;
            materialHandle = 0;
        }

        inline Lap(float width, LapAttachment attachmentType, const Material &material) {
            this->width = width;
            this->attachmentType = attachmentType;
            setMaterial(material);
        }

        inline Lap(const Lap &lap) {
            this->width = lap.width;
            this->attachmentType = lap.attachmentType;
            setMaterial(lap.materialHandle);
        }

        inline Material &material() const {
            return DrawingComponentManager<Material>::getComponentByHandle(materialHandle);
        }

        inline void setMaterial(unsigned handle) {
            materialHandle = handle;
        }

        inline void setMaterial(const Material &material) {
            setMaterial(material.handle());
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
        unsigned materialHandle;
    };

    struct Coordinate {
        float x, y;

        inline bool operator==(const Coordinate &other) {
            return x == other.x && y == other.y;
        }
    };

    struct ImpactPad {
        friend struct Drawing;

        Coordinate pos;
        float width, length;

        inline bool operator==(const ImpactPad &other) {
            return pos == other.pos && width == other.width && length == other.length &&
                   materialHandle == other.materialHandle && apertureHandle == other.apertureHandle;
        }

        inline bool operator!=(const ImpactPad &other) {
            return !(*this == other);
        }

        inline Material &material() const {
            return DrawingComponentManager<Material>::getComponentByHandle(materialHandle);
        }

        inline void setMaterial(const Material &material) {
            materialHandle = material.handle();
        }

        inline Aperture &aperture() const {
            return DrawingComponentManager<Aperture>::getComponentByHandle(apertureHandle);
        }

        inline void setAperture(const Aperture &aperture) {
            apertureHandle = aperture.handle();
        }

        inline unsigned serialisedSize() const {
            return sizeof(float) * 4 + sizeof(unsigned) * 2;
        }

        inline void serialise(void *target) const {
            unsigned char *buff = (unsigned char *) target;

            *((float *) buff) = pos.x;
            buff += sizeof(float);
            *((float *) buff) = pos.y;
            buff += sizeof(float);
            *((float *) buff) = width;
            buff += sizeof(float);
            *((float *) buff) = length;
            buff += sizeof(float);
            *((unsigned *) buff) = materialHandle;
            buff += sizeof(unsigned);
            *((unsigned *) buff) = apertureHandle;
            buff += sizeof(unsigned);
        }

        inline static ImpactPad &deserialise(void *buffer) {
            unsigned char *buff = (unsigned char *) buffer;

            ImpactPad *pad = new ImpactPad();

            pad->pos.x = *((float *) buff);
            buff += sizeof(float);
            pad->pos.y = *((float *) buff);
            buff += sizeof(float);
            pad->width = *((float *) buff);
            buff += sizeof(float);
            pad->length = *((float *) buff);
            buff += sizeof(float);
            pad->materialHandle = *((unsigned *) buff);
            buff += sizeof(unsigned);
            pad->apertureHandle = *((unsigned *) buff);
            buff += sizeof(unsigned);

            return *pad;
        }

    private:
        unsigned materialHandle;
        unsigned apertureHandle;
    };

    struct CentreHole {
        friend struct Drawing;

        struct Shape {
            float width, length;
            bool rounded;

            inline bool operator==(const Shape &other) {
                return width == other.width && length == other.length && rounded == other.rounded;
            }

            inline bool operator!=(const Shape &other) {
                return !(*this == other);
            }
        } centreHoleShape;

        Coordinate pos;

        inline bool operator==(const CentreHole &other) {
            return centreHoleShape == other.centreHoleShape && pos == other.pos;
        }

        inline bool operator!=(const CentreHole &other) {
            return !(*this == other);
        }

        inline unsigned serialisedSize() const {
            return sizeof(float) * 4 + sizeof(bool);
        }

        inline void serialise(void *target) const {
            unsigned char *buff = (unsigned char *) target;

            *((float *) buff) = pos.x;
            buff += sizeof(float);
            *((float *) buff) = pos.y;
            buff += sizeof(float);
            *((float *) buff) = centreHoleShape.width;
            buff += sizeof(float);
            *((float *) buff) = centreHoleShape.length;
            buff += sizeof(float);
            *buff++ = centreHoleShape.rounded;
        }

        inline static CentreHole &deserialise(void *buffer) {
            unsigned char *buff = (unsigned char *) buffer;

            CentreHole *hole = new CentreHole();

            hole->pos.x = *((float *) buff);
            buff += sizeof(float);
            hole->pos.y = *((float *) buff);
            buff += sizeof(float);
            hole->centreHoleShape.width = *((float *) buff);
            buff += sizeof(float);
            hole->centreHoleShape.length = *((float *) buff);
            buff += sizeof(float);
            hole->centreHoleShape.rounded = *buff++;

            return *hole;
        }
    };

    struct Deflector {
        friend struct Drawing;

        Coordinate pos;
        float size;

        inline bool operator==(const Deflector &other) {
            return pos == other.pos && size == other.size && materialHandle == other.materialHandle;
        }

        inline bool operator!=(const Deflector &other) {
            return !(*this == other);
        }

        inline Material &material() const {
            return DrawingComponentManager<Material>::getComponentByHandle(materialHandle);
        }

        void setMaterial(const Material &material) {
            materialHandle = material.handle();
        }

        inline unsigned serialisedSize() const {
            return sizeof(float) * 4 + sizeof(unsigned);
        }

        inline void serialise(void *target) const {
            unsigned char *buff = (unsigned char *) target;

            *((float *) buff) = pos.x;
            buff += sizeof(float);
            *((float *) buff) = pos.y;
            buff += sizeof(float);
            *((float *) buff) = size;
            buff += sizeof(float);
            *((unsigned *) buff) = materialHandle;
            buff += sizeof(unsigned);
        }

        inline static Deflector &deserialise(void *buffer) {
            unsigned char *buff = (unsigned char *) buffer;

            Deflector *deflector = new Deflector();

            deflector->pos.x = *((float *) buff);
            buff += sizeof(float);
            deflector->pos.y = *((float *) buff);
            buff += sizeof(float);
            deflector->size = *((float *) buff);
            buff += sizeof(float);
            deflector->materialHandle = *((unsigned *) buff);
            buff += sizeof(unsigned);

            return *deflector;
        }

    private:
        unsigned materialHandle;
    };

    struct Divertor {
        friend struct Drawing;

        Side side;
        float verticalPosition;
        float width, length;

        inline bool operator==(const Divertor &other) {
            return side == other.side && verticalPosition == other.verticalPosition && width == other.width &&
                   length == other.length && materialHandle == other.materialHandle;
        }

        inline bool operator!=(const Divertor &other) {
            return !(*this == other);
        }

        inline Material &material() const {
            return DrawingComponentManager<Material>::getComponentByHandle(materialHandle);
        }

        void setMaterial(const Material &material) {
            materialHandle = material.handle();
        }

        inline unsigned serialisedSize() const {
            return sizeof(unsigned char) + sizeof(float) * 3 + sizeof(unsigned);
        }

        inline void serialise(void *target) const {
            unsigned char *buff = (unsigned char *) target;

            *buff++ = (unsigned char) side;
            *((float *) buff) = verticalPosition;
            buff += sizeof(float);
            *((float *) buff) = width;
            buff += sizeof(float);
            *((float *) buff) = length;
            buff += sizeof(float);
            *((unsigned *) buff) = materialHandle;
            buff += sizeof(unsigned);
        }

        inline static Divertor &deserialise(void *buffer) {
            unsigned char *buff = (unsigned char *) buffer;

            Divertor *divertor = new Divertor();

            divertor->side = (Side) (*buff++);
            divertor->verticalPosition = *((float *) buff);
            buff += sizeof(float);
            divertor->width = *((float *) buff);
            buff += sizeof(float);
            divertor->length = *((float *) buff);
            buff += sizeof(float);
            divertor->materialHandle = *((unsigned *) buff);
            buff += sizeof(unsigned);

            return *divertor;
        }

    private:
        unsigned materialHandle;
    };

    Drawing();

    explicit Drawing(const Drawing &drawing);

    void setAsDefault();

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

    TensionType tensionType() const;

    void setTensionType(TensionType newTensionType);

    bool rebated() const;

    void setRebated(bool isRebated);

    bool hasBackingStrips() const;

    void setHasBackingStrips(bool backingStrips);

    std::optional<Material> material(MaterialLayer layer) const;

    void setMaterial(MaterialLayer layer, const Material &mat);

    void removeBottomLayer();

    unsigned numberOfBars() const;

    void setBars(const std::vector<float>& spacings, const std::vector<float>& widths);

    float barSpacing(unsigned index) const;

    float barWidth(unsigned index) const;

    float leftBar() const;

    float rightBar() const;

    std::vector<float> allBarSpacings() const;

    std::vector<float> allBarWidths() const;

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

    void addImpactPad(const ImpactPad &impactPad);

    std::vector<ImpactPad> impactPads() const;

    ImpactPad &impactPad(unsigned index);

    void removeImpactPad(const ImpactPad &pad);

    unsigned numberOfImpactPads() const;

    void addCentreHole(const CentreHole &centreHole);

    std::vector<CentreHole> centreHoles() const;

    CentreHole &centreHole(unsigned index);

    void removeCentreHole(const CentreHole &hole);

    unsigned numberOfCentreHoles() const;

    void addDeflector(const Deflector &deflector);

    std::vector<Deflector> deflectors() const;

    Deflector &deflector(unsigned index);

    void removeDeflector(const Deflector &deflector);

    unsigned numberOfDeflectors() const;

    void addDivertor(const Divertor &divertor);

    std::vector<Divertor> divertors() const;

    Divertor &divertor(unsigned index);

    void removeDivertor(const Divertor &divertor);

    unsigned numberOfDivertors() const;

    BuildWarning checkDrawingValidity(unsigned exclusions = 0) const;

    bool loadWarning(LoadWarning warning) const;

    void setLoadWarning(LoadWarning warning);

    void addUpdateCallback(const std::function<void()> &callback);

private:
    void invokeUpdateCallbacks() const;

    static constexpr char drawingNumberRegexPattern[] = "^([a-zA-Z]{1,2}[0-9]{2}[a-zA-Z]?|M[0-9]{3,}[a-zA-Z]?)$";
    static constexpr char positionRegexPattern[] = "(^$)|(^[0-9]+([-][0-9]+)?$)|(^[Aa][Ll]{2}$)";

    std::string __drawingNumber;
    Date __date;
    float __width, __length;
    std::string __hyperlink;
    std::string __notes;
    MachineTemplate __machineTemplate;

    unsigned productHandle;
    unsigned apertureHandle;

    TensionType __tensionType;

    bool __rebated;
    bool __hasBackingStrips;

    std::vector<std::string> __pressDrawingHyperlinks;

    std::vector<float> barSpacings;
    std::vector<float> barWidths;

    unsigned sideIronHandles[2];
    bool sideIronsInverted[2];

    std::optional<Lap> sidelaps[2], overlaps[2];

    unsigned topLayerThicknessHandle;
    std::optional<unsigned> bottomLayerThicknessHandle;

    std::vector<ImpactPad> __impactPads;
    std::vector<CentreHole> __centreHoles;
    std::vector<Deflector> __deflectors;
    std::vector<Divertor> __divertors;

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
    unsigned thicknessHandles[2];
    unsigned apertureHandle;
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

    std::vector<float> barSpacings() const;

    void addSpacing(float spacing);

    void clearSpacings();

    unsigned barSpacingCount() const;

private:
    unsigned __width, __length;
    unsigned __lapSizes[4];
    std::vector<unsigned> __barSpacings;
};

PACK_START
struct DrawingSummaryCompressionSchema {
    DrawingSummaryCompressionSchema(unsigned maxMatID, float maxWidth, float maxLength, unsigned maxThicknessHandle,
        float maxLapSize, unsigned maxApertureHandle, unsigned char maxBarSpacingCount, float maxBarSpacing, unsigned char maxDrawingLength);

    unsigned compressedSize(const DrawingSummary &summary) const;

    void compressSummary(const DrawingSummary &summary, void *target) const;

    DrawingSummary uncompressSummary(void *data, unsigned &size) const;

    unsigned maxCompressedSize() const;

private:
    unsigned char matIDSize;
    unsigned char widthSize;
    unsigned char lengthSize;
    unsigned char thicknessHandleSize;
    unsigned char lapSize;
    unsigned char apertureHandleSize;
    unsigned char maxDrawingLength;
    unsigned char barSpacingCountSize;
    unsigned char barSpacingSize;

    unsigned char maxBarSpacingCount;

    unsigned char matIDBytes;
    unsigned char widthBytes;
    unsigned char lengthBytes;
    unsigned char thicknessHandleBytes;
    unsigned char lapBytes;
    unsigned char apertureHandleBytes;
    unsigned char barSpacingCountBytes;
    unsigned char barSpacingBytes;
}
PACK_END

#endif //DATABASE_MANAGER_DRAWING_H
