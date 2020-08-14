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
#include "../util/DataSerialiser.h"
#include "../../packer.h"

// Macro to find the minimum number of bytes required to cover a given number of bits (e.g. 15 bits -> 2 bytes, 17 bits -> 3 bytes)
#define MIN_COVERING_BYTES(x) (((x) / 8) + ((x) % 8 != 0))
// Macro to find the minimum number of bites required to represent a given value (e.g. 513 = 512 + 1 = 2^9 + 2^0 -> 9 bits required)
#define MIN_COVERING_BITS(x) (32u - __builtin_clz(x))

// Typedef a byte type representing a single byte of data
typedef unsigned char byte;

/// <summary>
/// Writes a value at an arbitrary bit offset, rather than a standard byte offset for compression
/// </summary>
/// <param name="value">The value data to write.</param>
/// <param name="valueByteLength">The size of the value data (in bytes).</param>
/// <param name="target">The target buffer to write the value to.</param>
/// <param name="bitOffset">The offset (in bits) to write at.</param>
void writeAtBitOffset(void *value, size_t valueByteLength, void *target, size_t bitOffset);

/// <summary>
/// Reads a value from an arbitrary bit offset, rather than a standard byte offset
/// </summary>
/// <param name="data">The source data stream to read from.</param>
/// <param name="bitOffset">The offset (in bits) to read from.</param>
/// <param name="target">The target buffer to write the read value to.</param>
/// <param name="bitReadSize">The number of bits to read from the buffer.</param>
void readFromBitOffset(void *data, size_t bitOffset, void *target, size_t bitReadSize);

/// <summary>
/// Simple function to turn an arbitrary data type to a string through the use of a stringstream
/// </summary>
/// <typeparam name="T">The type to convert (this will generally be implicitly inferred).</typeparam>
/// <param name="t">The value to convert to a string.</param>
/// <returns>The string representation of the passed in value.</returns>
template<typename T>
std::string to_str(const T &t) {
    // Create a string stream
    std::stringstream ss;
    // Write the value to the stream
    ss << t;
    // Return the string data from the stream
    return ss.str();
}

PACK_START
/// <summary>
/// Date
/// A simple structure to hold a date value, divided into year month and day.
/// Designed to fit in the space of 4 bytes.
/// </summary>
struct Date {
    // The year of the date
    unsigned short year;
    // The month and day of the date
    unsigned char month, day;

    /// <summary>
    /// Default constructor
    /// </summary>
    Date() = default;

    /// <summary>
    /// Constructor for date
    /// </summary>
    /// <param name="year">The year to set the date to.</param>
    /// <param name="month">The month to set the date to.</param>
    /// <param name="day">The day to set the date to.</param>
    Date(unsigned year, unsigned month, unsigned day);

    /// <summary>
    /// Converts the date to a MySQL date formatted string for use in a database. Note that the time is
    /// always set to 00:00:00.
    /// </summary>
    /// <returns>A MySQL string representation of the date.</returns>
    std::string toMySQLDateString() const;

    /// <summary>
    /// Parses a raw std::time_t object into its year month and day components, based on the localtime
    /// </summary>
    /// <param name="rawDate">The raw date information to parse.</param>
    /// <returns>A Date object with the components separated.</returns>
    static Date parse(std::time_t rawDate);

    /// <summary>
    /// Static function to get the current date.
    /// </summary>
    /// <returns>A separated Date object containing the year month and day when the function was called.</returns>
    static Date today();
}
PACK_END

// Forward declaration of the DrawingSerialiser struct (for friending)
struct DrawingSerialiser;
// Forward declaration of the DrawingSummary struct
struct DrawingSummary;

/// <summary>
/// Drawing
/// The Drawing object contains the entire record data for a particular drawing, as stored in the database.
/// This is the C++ representation of a particular drawing. The drawing can be edited and read from through
/// the provided interface. Through the various provided query objects, drawings can be read from and written
/// to the database.
/// </summary>
struct Drawing {
    // Friend the DrawingSerialiser class so that it can read the private data stored in a Drawing object.
    friend struct DrawingSerialiser;
public:
    /// <summary>
    /// LoadWarning enum
    /// Represents a flag based warning system for different warnings when loading the drawing from the database
    /// so the receiver can determine if there were any broken components.
    /// </summary>
    enum LoadWarning {
        LOAD_FAILED = 0x01,
        INVALID_LAPS_DETECTED = 0x02,
        MISSING_SIDE_IRONS_DETECTED = 0x04,
        MISSING_MATERIAL_DETECTED = 0x08,
        INVALID_APERTURE_DETECTED = 0x10,
        INVALID_IMPACT_PAD_DETECTED = 0x20
    };

    /// <summary>
    /// BuildWarning enum
    /// Represents a flag based warning system for different warnings when constructing a drawing object
    /// which indicate any issues the drawing has. As long as a drawing has such issues, it will be declined
    /// from being added to the database in order to preserve integrity.
    /// </summary>
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

    /// <summary>
    /// Side enum
    /// Represents whether an entity resides on the "Left" or "Right" side of the drawing. Note that sometimes
    /// "Left" and "Right" actually represent the top and bottom of a mat.
    /// </summary>
    enum Side {
        LEFT,
        RIGHT
    };

    /// <summary>
    /// MaterialLayer enum
    /// Represents whether an entity represents the top or bottom layer of a material. Note that if a mat has
    /// only a single layer, this is considered to be the "Top" layer.
    /// </summary>
    enum MaterialLayer {
        TOP,
        BOTTOM
    };

    /// <summary>
    /// TensionType enum
    /// Represents whether a mat is Side or End tensioned.
    /// </summary>
    enum TensionType {
        SIDE,
        END
    };

    /// <summary>
    /// MachineTemplate
    /// This is a datatype for storing the information about a mat's machine template. This is stored
    /// in a particular table in the database. This C++ object represents all the necessary properties
    /// of the machine and is used internally by the Drawing struct.
    /// </summary>
    struct MachineTemplate {
        // Friend the Drawing struct
        friend struct Drawing;

        // The quantity on deck property of the template
        unsigned quantityOnDeck;
        // The position property of the template
        std::string position;
        
        /// <summary>
        /// Default constructor which initialises the two component fields to specific default values
        /// </summary>
        inline MachineTemplate() {
            // Set the machine handle to the handle of the default machine
            machineHandle = DrawingComponentManager<Machine>::findComponentByID(1).handle();
            // Default the quantity on deck to 0
            quantityOnDeck = 0;
            // Default the position string to an empty string
            position = std::string();
            // Set the machine deck handle to the handle of the default deck
            deckHandle = DrawingComponentManager<MachineDeck>::findComponentByID(1).handle();
        }

        /// <summary>
        /// Copy constructor for machine template
        /// </summary>
        /// <param name="machineTemplate">The machine template to copy from</param>
        inline MachineTemplate(const MachineTemplate &machineTemplate) {
            // Copy each of the properties from the source to this object
            this->machineHandle = machineTemplate.machineHandle;
            this->quantityOnDeck = machineTemplate.quantityOnDeck;
            this->position = machineTemplate.position;
            this->deckHandle = machineTemplate.deckHandle;
        }

        /// <summary>
        /// Getter for the machine in the form of a reference variable
        /// </summary>
        /// <returns>The machine corresponding to the stored machine handle variable.</returns>
        inline Machine &machine() const {
            // Gets the machine from the drawing component manager
            return DrawingComponentManager<Machine>::getComponentByHandle(machineHandle);
        }

        /// <summary>
        /// Getter for the machine deck in the form of a reference variable
        /// </summary>
        /// <returns>The machine deck corresponding to the stored machine deck handle variable.</returns>
        inline MachineDeck &deck() const {
            // Gets the machine deck from the drawing component manager
            return DrawingComponentManager<MachineDeck>::getComponentByHandle(deckHandle);
        }

    private:
        // Private handles for the machine and deck - these are not directly exposed
        unsigned machineHandle, deckHandle;
    };

    /// <summary>
    /// Lap
    /// This is a simple structure for storing the information about an overlap or sidelap on a particular drawing.
    /// </summary>
    struct Lap {
        // Friend the drawing structure
        friend struct Drawing;

        // The width of the lap
        float width;
        // Whether the lap is integral to the mat or it is bonded on
        LapAttachment attachmentType;

        /// <summary>
        /// Default constructor initialising the properties of the lap
        /// </summary>
        inline Lap() {
            // Default the width to 0
            width = 0;
            // Default the attachment type to be integral
            attachmentType = LapAttachment::INTEGRAL;
            // Default to the material handle of 0 which does not represent an actual material
            materialHandle = 0;
        }

        /// <summary>
        /// Constructor based on the lap properties
        /// </summary>
        /// <param name="width">The width of the lap.</param>
        /// <param name="attachmentType">The attachment type of the lap.</param>
        /// <param name="material">The material the lap is made from.</param>
        inline Lap(float width, LapAttachment attachmentType, const Material &material) {
            // Set the width and attachment type directly
            this->width = width;
            this->attachmentType = attachmentType;
            // Set the material based on the setter method
            setMaterial(material);
        }

        /// <summary>
        /// Copy constructor
        /// </summary>
        /// <param name="lap">The source lap to copy from</param>
        inline Lap(const Lap &lap) {
            // Set the width and attachment type directly from the lap source
            this->width = lap.width;
            this->attachmentType = lap.attachmentType;
            // Set the material from the lap source's material handle
            setMaterial(lap.materialHandle);
        }

        /// <summary>
        /// Getter for the material
        /// </summary>
        /// <returns>A reference to the material this lap is made from.</returns>
        inline Material &material() const {
            return DrawingComponentManager<Material>::getComponentByHandle(materialHandle);
        }

        /// <summary>
        /// Direct setter for the material
        /// </summary>
        /// <param name="handle">The handle of the target material.</param>
        inline void setMaterial(unsigned handle) {
            materialHandle = handle;
        }

        /// <summary>
        /// Indirect setter for the material through a material reference
        /// </summary>
        /// <param name="material">The material reference to set the material to.</param>
        inline void setMaterial(const Material &material) {
            setMaterial(material.handle());
        }

        /// <summary>
        /// Creates a string representation of this lap for data display as a sidelap
        /// </summary>
        /// <returns>A string contianing the details about the lap.</returns>
        inline std::string strAsSidelap() const {
            std::stringstream ss;
            ss << width << "mm " << (attachmentType == LapAttachment::INTEGRAL ? "integral" : "bonded") << " sidelap (" << material().material() << ")";
            return ss.str();
        }

        /// <summary>
        /// Creates a string representation of this lap for data display as an overlap
        /// </summary>
        /// <returns>A string contianing the details about the lap.</returns>
        inline std::string strAsOverlap() const {
            std::stringstream ss;
            ss << width << "mm " << (attachmentType == LapAttachment::INTEGRAL ? "integral" : "bonded") << " overlap (" << material().material() << ")";
            return ss.str();
        }

    private:
        // The internal material handle representation
        unsigned materialHandle;
    };

    /// <summary>
    /// Coordinate
    /// Very primitive storage structure for a single floating point precision 2 dimensional coordinate
    /// </summary>
    struct Coordinate {
        // The x and y positions of this coordinate
        float x, y;

        /// <summary>
        /// Equality operator
        /// </summary>
        /// <param name="other">A reference to the coordinate to compare to.</param>
        /// <returns>A boolean stating whether the coordinates are equal or not.</returns>
        inline bool operator==(const Coordinate &other) {
            // We consider coordinates equal if both their x and y positions are equal.
            return x == other.x && y == other.y;
        }
    };

    /// <summary>
    /// ImpactPad
    /// A data structure containing the information needed to represent an impact pad component of a drawing.
    /// </summary>
    struct ImpactPad {
        // Friend the drawing structure
        friend struct Drawing;

        // The coordinate position of the top left corner of this impact pad (in mat coordinates)
        Coordinate pos;
        // The width and length of this impact pad (in mat coordinates)
        float width, length;

        /// <summary>
        /// Equality operator
        /// </summary>
        /// <param name="other">A reference to another impact pad object to compare to.</param>
        /// <returns>Whether or not the impact pads are considered equal.</returns>
        inline bool operator==(const ImpactPad &other) {
            // Two impact pads are considered equal if and only if their top left corners, width, length, material and
            // aperture are all equal.
            return pos == other.pos && width == other.width && length == other.length &&
                   materialHandle == other.materialHandle && apertureHandle == other.apertureHandle;
        }

        /// <summary>
        /// Inequality operator
        /// </summary>
        /// <param name="other">A reference to another impact pad object to compare to.</param>
        /// <returns>Whether or not the impact pads are considered inequal.</returns>
        inline bool operator!=(const ImpactPad &other) {
            // Return the boolean NOT of whether the mats are considered equal
            return !(*this == other);
        }

        /// <summary>
        /// Getter for the material of the impact pad
        /// </summary>
        /// <returns>A material reference for the material present on the impact pad.</returns>
        inline Material &material() const {
            // Get the material from the drawing component manager and return it directly
            return DrawingComponentManager<Material>::getComponentByHandle(materialHandle);
        }

        /// <summary>
        /// Setter for the material
        /// </summary>
        /// <param name="material">The material reference to set the material from.</param>
        inline void setMaterial(const Material &material) {
            // Set the material handle to the handle property on the passed in material.
            materialHandle = material.handle();
        }

        /// <summary>
        /// Getter for the aperture punched on the impact pad
        /// </summary>
        /// <returns>An aperture reference to the aperture present on the impact pad.</returns>
        inline Aperture &aperture() const {
            // Get the material handle from the drawing component manager and return it directly
            return DrawingComponentManager<Aperture>::getComponentByHandle(apertureHandle);
        }

        /// <summary>
        /// Setter for the aperture
        /// </summary>
        /// <param name="aperture">An aperture reference to set the aperture from.</param>
        inline void setAperture(const Aperture &aperture) {
            // Set the aperture handle to the handle property on the passed in aperture.
            apertureHandle = aperture.handle();
        }

        /// <summary>
        /// Getter for the serialised size of this impact pad in bytes 
        /// when serialised to a data buffer
        /// </summary>
        /// <returns>The number of bytes needed to represent an impact pad for data transfer.</returns>
        inline unsigned serialisedSize() const {
            // An impact pad is specified by 4 float values for the X, Y, W, H of the rectangle, and 
            // two unsigned integer handles representing the material and aperture handles.
            return sizeof(float) * 4 + sizeof(unsigned) * 2;
        }

        /// <summary>
        /// Serialises the impact pad data into (the start of) a target data buffer
        /// </summary>
        /// <param name="target">The data buffer to write to.</param>
        inline void serialise(void *target) const {
            // Cast the target buffer to a byte buffer so we can perform pointer arithmetic
            unsigned char *buff = (unsigned char *) target;

            // Write each value to the buffer in turn, each time incrementing the buffer pointer
            // by the size of the object added
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

        /// <summary>
        /// Deserialises an impact pad from (the start of) a data buffer
        /// </summary>
        /// <param name="buffer">The data buffer to interpret as an impact pad.</param>
        /// <returns>A newly constructed impact pad object specified by the data from the buffer.</returns>
        inline static ImpactPad &deserialise(void *buffer) {
            // Cast the source buffer to a byte buffer so we can perform pointer arithmetic
            unsigned char *buff = (unsigned char *) buffer;

            // Construct a new ImpactPad object for returning (hence on the heap)
            ImpactPad *pad = new ImpactPad();

            // Read each value in turn in the same sequence as specified by the serialise function
            // and write to each property in the pad itself.
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

            // Return the impact pad object we constructed
            return *pad;
        }

    private:
        // The material and aperture handles for this impact pad which are accessed through the 
        // interface specified above
        unsigned materialHandle;
        unsigned apertureHandle;
    };

    /// <summary>
    /// CentreHole
    /// A data structure containing the information needed to represent a single centre hole on a drawing.
    /// </summary>
    struct CentreHole {
        // Friend the drawing structure
        friend struct Drawing;

        /// <summary>
        /// Shape
        /// Internal Shape grouping structure for the shape of a centre hole
        /// </summary>
        struct Shape {
            // The width and length of the shape which is cut out of the mat
            float width, length;
            // Whether or not the shape is cut from a rounded tool
            bool rounded;

            /// <summary>
            /// Equality operator
            /// </summary>
            /// <param name="other">A reference to another shape object to compare to.</param>
            /// <returns>Whether or not the two shapes are considered to be equal.</returns>
            inline bool operator==(const Shape &other) {
                // We consider two shapes to be equal if and only if they have the same width, lenght and roundedness.
                return width == other.width && length == other.length && rounded == other.rounded;
            }

            /// <summary>
            /// Inequality operator
            /// </summary>
            /// <param name="other">A reference to another shape object to compare to.</param>
            /// <returns>Whether or not the two shapes are considered to be inequal.</returns>
            inline bool operator!=(const Shape &other) {
                // Return the boolean NOT of whether the two shapes are considered equal.
                return !(*this == other);
            }
        } centreHoleShape; // The shame of this particular centre hole

        // The position of the centre of the where the centre hole is punched
        Coordinate pos;

        /// <summary>
        /// Equality operator
        /// </summary>
        /// <param name="other">A reference to another centre hole object to compare to.</param>
        /// <returns>Whether or not the two centre holes are considered to be equal.</returns>
        inline bool operator==(const CentreHole &other) {
            // We consider two centre holes to be equal if and only if their shapes are equal and their positions are
            // equal.
            return centreHoleShape == other.centreHoleShape && pos == other.pos;
        }

        /// <summary>
        /// Inequality operator
        /// </summary>
        /// <param name="other">A reference to another centre hole object to compare to.</param>
        /// <returns>Whether or not the two center holes are considered to be inequal.</returns>
        inline bool operator!=(const CentreHole &other) {
            // Return the boolean NOT of whether the centre holes are considered equal
            return !(*this == other);
        }

        /// <summary>
        /// Getter for the serialised size of this centre hole when written to a data
        /// buffer
        /// </summary>
        /// <returns>The number of bytes this object will occupy in a data buffer.</returns>
        inline unsigned serialisedSize() const {
            // A centre hole is defined by its X, Y, W, H and roundedness, and so is fully specified by 4 floats
            // and a single boolean.
            return sizeof(float) * 4 + sizeof(bool);
        }

        /// <summary>
        /// Serialises the centre hole into (the start of) a target data buffer
        /// </summary>
        /// <param name="target">The data buffer to write to.</param>
        inline void serialise(void *target) const {
            // Cast the 
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

        /// <summary>
        /// Deserialises the centre hole from (the start of) a data buffer
        /// </summary>
        /// <param name="target">The data buffer to read from.</param>
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
        // Friend the drawing structure
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
        // Friend the drawing structure
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
