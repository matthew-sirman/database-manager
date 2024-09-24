//
// Created by matthew on 06/07/2020.
//

#ifndef DATABASE_MANAGER_DRAWING_H
#define DATABASE_MANAGER_DRAWING_H

#include <cstring>
#include <numeric>
#include <regex>
#include <sstream>

#include "../../packer.h"
#include "drawingComponents.h"
#include "Logger.h"

// Macro to find the minimum number of bytes required to cover a given number of
// bits (e.g. 15 bits -> 2 bytes, 17 bits -> 3 bytes)
#define MIN_COVERING_BYTES(x) (((x) / 8) + ((x) % 8 != 0))
// Macro to find the minimum number of bits required to represent a given value
// (e.g. 513 = 512 + 1 = 2^9 + 2^0 -> 10 bits required)
#define MIN_COVERING_BITS(x) floor(log2(x)) + 1 //(32u - 0)//__builtin_clz(x))

// Typedef a byte type representing a single byte of data
typedef unsigned char byte;

/// <summary>
/// Writes a value at an arbitrary bit offset, rather than a standard byte
/// offset for compression
/// </summary>
/// <param name="value">The value data to write.</param>
/// <param name="valueByteLength">The size of the value data (in bytes).</param>
/// <param name="target">The target buffer to write the value to.</param>
/// <param name="bitOffset">The offset (in bits) to write at.</param>
void writeAtBitOffset(void *value, size_t valueByteLength, void *target,
                      size_t bitOffset);

/// <summary>
/// Reads a value from an arbitrary bit offset, rather than a standard byte
/// offset
/// </summary>
/// <param name="data">The source data stream to read from.</param>
/// <param name="bitOffset">The offset (in bits) to read from.</param>
/// <param name="target">The target buffer to write the read value to.</param>
/// <param name="bitReadSize">The number of bits to read from the
/// buffer.</param>
void readFromBitOffset(void *data, size_t bitOffset, void *target,
                       size_t bitReadSize);

/// <summary>
/// Simple function to turn an arbitrary data type to a string through the use
/// of a stringstream
/// </summary>
/// <typeparam name="T">The type to convert (this will generally be implicitly
/// inferred).</typeparam> <param name="t">The value to convert to a
/// string.</param> <returns>The string representation of the passed in
/// value.</returns>
template <typename T>
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
    /// <summary>
    /// The year of the date.
    /// </summary>
    unsigned short year;
    /// <summary>
    /// The month of the date.
    /// </summary>
    unsigned char month;
    /// <summary>
    /// The day of the date.
    /// </summary>
    unsigned char day;

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
    /// Converts the date to a MySQL date formatted string for use in a database.
    /// Note that the time is always set to 00:00:00.
    /// </summary>
    /// <returns>A MySQL string representation of the date.</returns>
    std::string toMySQLDateString() const;

    /// <summary>
    /// Parses a raw std::time_t object into its year month and day components,
    /// based on the localtime
    /// </summary>
    /// <param name="rawDate">The raw date information to parse.</param>
    /// <returns>A Date object with the components separated.</returns>
    static Date parse(std::time_t rawDate);

    /// <summary>
    /// Static function to get the current date.
    /// </summary>
    /// <returns>A separated Date object containing the year month and day when
    /// the function was called.</returns>
    static Date today();
}
///@cond IGNORE
PACK_END
    ///@endcond

    // Forward declaration of the DrawingSerialiser struct (for friending)
    struct DrawingSerialiser;
// Forward declaration of the DrawingSummary struct
struct DrawingSummary;

template <typename T, DrawingComponentConcept... Ds>
class GroupGraphicsItem;
/// <summary>
/// Drawing
/// The Drawing object contains the entire record data for a particular drawing,
/// as stored in the database. This is the C++ representation of a particular
/// drawing. The drawing can be edited and read from through the provided
/// interface. Through the various provided query objects, drawings can be read
/// from and written to the database.
/// </summary>
struct Drawing {
    // Friend the DrawingSerialiser class so that it can read the private data
    // stored in a Drawing object.
    friend struct DrawingSerialiser;

  public:
    /// <summary>
    /// LoadWarning enum
    /// Represents a flag based warning system for different warnings when loading
    /// the drawing from the database so the receiver can determine if there were
    /// any broken components.
    /// </summary>
    enum LoadWarning {
        /// <summary>
        /// Loading the drawing failed.
        /// </summary>
        LOAD_FAILED = 0x01,
        /// <summary>
        /// The drawing has invalid laps.
        /// </summary>
        INVALID_LAPS_DETECTED = 0x02,
        /// <summary>
        /// The drawing does not have side irons.
        /// </summary>
        MISSING_SIDE_IRONS_DETECTED = 0x04,
        /// <summary>
        /// The drawing does not have a valid material.
        /// </summary>
        MISSING_MATERIAL_DETECTED = 0x08,
        /// <summary>
        /// The drawing does not have a valid aperture.
        /// </summary>
        INVALID_APERTURE_DETECTED = 0x10,
        /// <summary>
        /// the drawing has a corrupted impact pad.
        /// </summary>
        INVALID_IMPACT_PAD_DETECTED = 0x20,
        /// <summary>
        /// The drawing does not have valid backing strips.
        /// </summary>
        INVALID_BACKING_STRIP_DETECTED = 0x40,
        /// <summary>
        /// The drawing has a corrupted centre hole.
        /// </summary>
        INVALID_CENTRE_HOLE_DETECTED = 0x80
    };

    /// <summary>
    /// BuildWarning enum
    /// Represents a flag based warning system for different warnings when
    /// constructing a drawing object which indicate any issues the drawing has.
    /// As long as a drawing has such issues, it will be declined from being added
    /// to the database in order to preserve integrity.
    /// </summary>
    enum BuildWarning {
        /// <summary>
        /// The drawing was successfully built.
        /// </summary>
        SUCCESS = 0x0000,
        /// <summary>
        /// The drawing has an invalid drawing number.
        /// </summary>
        INVALID_DRAWING_NUMBER = 0x0001,
        /// <summary>
        /// The drawing has an invalid product.
        /// </summary>
        INVALID_PRODUCT = 0x0002,
        /// <summary>
        /// The drawing has an invalid width.
        /// </summary>
        INVALID_WIDTH = 0x0004,
        /// <summary>
        /// The drawing has an invalid length.
        /// </summary>
        INVALID_LENGTH = 0x0008,
        /// <summary>
        /// The drawing has an invalid top material.
        /// </summary>
        INVALID_TOP_MATERIAL = 0x0010,
        /// <summary>
        /// The drawing has an invalid bottom material.
        /// </summary>
        INVALID_BOTTOM_MATERIAL = 0x0020,
        /// <summary>
        /// The drawing has an invalid aperture.
        /// </summary>
        INVALID_APERTURE = 0x0040,
        /// <summary>
        /// The drawing has invalid bar spacings.
        /// </summary>
        INVALID_BAR_SPACINGS = 0x0080,
        /// <summary>
        /// The drawing has invalid bar widths.
        /// </summary>
        INVALID_BAR_WIDTHS = 0x0100,
        /// <summary>
        /// The drawing has invalid side irons.
        /// </summary>
        INVALID_SIDE_IRONS = 0x0200,
        /// <summary>
        /// The drawing has an invalid machine.
        /// </summary>
        INVALID_MACHINE = 0x0400,
        /// <summary>
        /// The drawing has an invalid machine position.
        /// </summary>
        INVALID_MACHINE_POSITION = 0x0800,
        /// <summary>
        /// The drawing has an invalid machine deck.
        /// </summary>
        INVALID_MACHINE_DECK = 0x1000,
        /// <summary>
        /// The drawing has an invalid hyperlink.
        /// </summary>
        INVALID_HYPERLINK = 0x2000,
        /// <summary>
        /// The drawing has an invalid side iron rubber cover strap/wear tear liner
        /// </summary>
        INVALID_STRAPS = 0x4000
    };

    /// <summary>
    /// Side enum
    /// Represents whether an entity resides on the "Left" or "Right" side of the
    /// drawing. Note that sometimes "Left" and "Right" actually represent the top
    /// and bottom of a mat.
    /// </summary>
    enum Side {
        /// <summary>
        /// Represents the left or top of the mat, for sidelaps and overlaps
        /// respectively.
        /// </summary>
        LEFT,
        /// <summary>
        /// Represents the right or bottom of the mat, for sidelaps and overlaps
        /// respectively.
        /// </summary>
        RIGHT
    };

    /// <summary>
    /// HookOrientation enum
    /// Represents the orientation of the hook
    /// </summary>
    enum HookOrientation {
        /// <summary>
        /// The hook orientation is not known.
        /// </summary>
        UNKNOWN,
        /// <summary>
        /// The hook is oriented up.
        /// </summary>
        HOOK_UP,
        /// <summary>
        /// The hook is oriented down.
        /// </summary>
        HOOK_DOWN
    };

    /// <summary>
    /// Ending enum
    /// Describes whether the mat is tentioned at the end or perpendictular, or
    /// default when it is clear/not recorded.
    /// </summary>
    enum Ending {
        /// <summary>
        /// This ending is as expected.
        /// </summary>
        DEFAULT,
        /// <summary>
        /// This ending is fixed.
        /// </summary>
        FIXED_END,
        /// <summary>
        /// This ending is tensioned.
        /// </summary>
        TENSION_END
    };

    /// <summary>
    /// MaterialLayer enum
    /// Represents whether an entity represents the top or bottom layer of a
    /// material. Note that if a mat has only a single layer, this is considered
    /// to be the "Top" layer.
    /// </summary>
    enum MaterialLayer {
        /// <summary>
        /// This material is on the top, if there is only one material, this is the
        /// only material.
        /// </summary>
        TOP,
        /// <summary>
        /// This material is on the bottom.
        /// </summary>
        BOTTOM
    };

    /// <summary>
    /// TensionType enum
    /// Represents whether a mat is Side or End tensioned.
    /// </summary>
    enum TensionType {
        /// <summary>
        /// The mat is side tensioned.
        /// </summary>
        SIDE,
        /// <summary>
        /// The mat is end tensioned.
        /// </summary>
        END
    };

    /// <summary>
    /// MachineTemplate
    /// This is a datatype for storing the information about a mat's machine
    /// template. This is stored in a particular table in the database. This C++
    /// object represents all the necessary properties of the machine and is used
    /// internally by the Drawing struct.
    /// </summary>
    struct MachineTemplate {
        // Friend the Drawing struct
        friend struct Drawing;

        /// <summary>
        /// The quantity on deck property of the template
        /// </summary>
        unsigned quantityOnDeck;
        /// <summary>
        /// The position property of the template
        /// </summary>
        std::string position;

        /// <summary>
        /// Default constructor which initialises the two component fields to
        /// specific default values
        /// </summary>
        MachineTemplate();

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
        /// Default destructor
        /// </summary>
        inline ~MachineTemplate() = default;

        /// <summary>
        /// Getter for the machine in the form of a reference variable
        /// </summary>
        /// <returns>The machine corresponding to the stored machine handle
        /// variable.</returns>
        Machine &machine() const;

        /// <summary>
        /// Getter for the machine deck in the form of a reference variable
        /// </summary>
        /// <returns>The machine deck corresponding to the stored machine deck
        /// handle variable.</returns>
        MachineDeck &deck() const;

      private:
        // Private handles for the machine and deck - these are not directly exposed
        unsigned machineHandle, deckHandle;
    };

    /// <summary>
    /// This is a simple structure for storing the information about an overlap or
    /// sidelap on a particular drawing.
    /// On a side tensioned mat, the left and right are considered "sidelaps" and the top and bottom "overlaps",
    /// and on an end tensioned mat, the left are right are considered "overlaps" and the top and bottom "sidelaps".
    /// When refering to top and bottom laps, top is Side::LEFT and bottom is Side::RIGHT.
    /// </summary>
    struct Lap { // Friend the drawing structure
        friend struct Drawing;

        /// <summary>
        /// The width of the lap
        /// </summary>
        float width;

        /// <summary>
        /// Whether the lap is integral to the mat or it is bonded on
        /// </summary>
        LapAttachment attachmentType;

        /// <summary>
        /// Default constructor initialising the properties of the lap
        /// </summary>
        inline Lap() {
            // Default the width to 0
            width = 0;
            // Default the attachment type to be integral
            attachmentType = LapAttachment::INTEGRAL;
            // Default to the material handle of 0 which does not represent an actual
            // material
            materialHandle = 0;
        }

        /// <summary>
        /// Constructor based on the lap properties
        /// </summary>
        /// <param name="width">The width of the lap.</param>
        /// <param name="attachmentType">The attachment type of the lap.</param>
        /// <param name="material">The material the lap is made from.</param>
        inline Lap(float width, LapAttachment attachmentType,
                   const Material &material) {
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
        /// Default destructor
        /// </summary>
        inline ~Lap() = default;

        /// <summary>
        /// Getter for the material
        /// </summary>
        /// <returns>A reference to the material this lap is made from.</returns>
        Material &material() const;

        /// <summary>
        /// Direct setter for the material
        /// </summary>
        /// <param name="handle">The handle of the target material.</param>
        inline void setMaterial(unsigned handle) { materialHandle = handle; }

        /// <summary>
        /// Indirect setter for the material through a material reference
        /// </summary>
        /// <param name="material">The material reference to set the material
        /// to.</param>
        inline void setMaterial(const Material &material) {
            setMaterial(material.handle());
        }

        /// <summary>
        /// Creates a string representation of this lap for data display as a
        /// sidelap
        /// </summary>
        /// <returns>A string contianing the details about the lap.</returns>
        inline std::string strAsSidelap() const {
            std::stringstream ss;
            ss << width << "mm "
               << (attachmentType == LapAttachment::INTEGRAL
                       ? "integral"
                       : material().materialName)
               << " sidelap (" << material().material() << ")";
            return ss.str();
        }

        /// <summary>
        /// Creates a string representation of this lap for data display as an
        /// overlap
        /// </summary>
        /// <returns>A string contianing the details about the lap.</returns>
        inline std::string strAsOverlap() const {
            std::stringstream ss;
            ss << width << "mm "
               << (attachmentType == LapAttachment::INTEGRAL ? "integral" : "bonded")
               << " overlap (" << material().material() << ")";
            return ss.str();
        }

      private:
        // The internal material handle representation
        unsigned materialHandle;
    };

    /// <summary>
    /// Coordinate
    /// Very primitive storage structure for a single floating point precision 2
    /// dimensional coordinate
    /// </summary>
    struct Coordinate {
        /// <summary>
        /// The x position of this coordinate.
        /// </summary>
        float x;
        /// <summary>
        /// The y position of this coordiate.
        /// </summary>
        float y;

        /// <summary>
        /// Equality operator
        /// </summary>
        /// <param name="other">A reference to the coordinate to compare to.</param>
        /// <returns>A boolean stating whether the coordinates are equal or
        /// not.</returns>
        inline bool operator==(const Coordinate &other) {
            // We consider coordinates equal if both their x and y positions are
            // equal.
            return x == other.x && y == other.y;
        }
    };

    /// <summary>
    /// ImpactPad
    /// A data structure containing the information needed to represent an impact
    /// pad component of a drawing.
    /// </summary>
    struct ImpactPad {
        // Friend the drawing structure
        friend struct Drawing;

        /// <summary>
        /// The coordinate position of the top left corner of this impact pad (in
        /// mat coordinates)
        /// </summary>
        Coordinate pos;

        /// <summary>
        /// The width of this impact pad (in mat coordinates).
        /// </summary>
        float width;
        /// <summary>
        /// The length of this impact pad (in mat coordinates).
        /// </summary>
        float length;

        /// <summary>
        /// Default constructor
        /// </summary>
        inline ImpactPad() = default;

        /// <summary>
        /// Copy constructor
        /// </summary>
        /// <param name="pad">The Impact pad to copy.</param>
        inline ImpactPad(const ImpactPad &pad) {
            this->pos.x = pad.pos.x;
            this->pos.y = pad.pos.y;
            this->width = pad.width;
            this->length = pad.length;
            this->materialHandle = pad.materialHandle;
            this->apertureHandle = pad.apertureHandle;
        }

        /// <summary>
        /// Default destructor
        /// </summary>
        inline ~ImpactPad() = default;

        /// <summary>
        /// Equality operator
        /// </summary>
        /// <param name="other">A reference to another impact pad object to compare
        /// to.</param> <returns>Whether or not the impact pads are considered
        /// equal.</returns>
        inline bool operator==(const ImpactPad &other) {
            // Two impact pads are considered equal if and only if their top left
            // corners, width, length, material and aperture are all equal.
            return pos == other.pos && width == other.width &&
                   length == other.length && materialHandle == other.materialHandle &&
                   apertureHandle == other.apertureHandle;
        }

        /// <summary>
        /// Inequality operator
        /// </summary>
        /// <param name="other">A reference to another impact pad object to compare
        /// to.</param> <returns>Whether or not the impact pads are considered
        /// inequal.</returns>
        inline bool operator!=(const ImpactPad &other) {
            // Return the boolean NOT of whether the mats are considered equal
            return !(*this == other);
        }

        /// <summary>
        /// Getter for the material of the impact pad
        /// </summary>
        /// <returns>A material reference for the material present on the impact
        /// pad.</returns>
        virtual Material &material() const;

        /// <summary>
        /// Setter for the material
        /// </summary>
        /// <param name="material">The material reference to set the material
        /// from.</param>
        inline void setMaterial(const Material &material) {
            // Set the material handle to the handle property on the passed in
            // material.
            materialHandle = material.handle();
        }

        /// <summary>
        /// Getter for the aperture punched on the impact pad
        /// </summary>
        /// <returns>An aperture reference to the aperture present on the impact
        /// pad.</returns>
        virtual Aperture &aperture() const;

        /// <summary>
        /// Setter for the aperture
        /// </summary>
        /// <param name="aperture">An aperture reference to set the aperture
        /// from.</param>
        inline void setAperture(const Aperture &aperture) {
            // Set the aperture handle to the handle property on the passed in
            // aperture.
            apertureHandle = aperture.handle();
        }

        /// <summary>
        /// Getter for the serialised size of this impact pad in bytes
        /// when serialised to a data buffer
        /// </summary>
        /// <returns>The number of bytes needed to represent an impact pad for data
        /// transfer.</returns>
        inline unsigned serialisedSize() const {
            // An impact pad is specified by 4 float values for the X, Y, W, H of the
            // rectangle, and two unsigned integer handles representing the material
            // and aperture handles.
            return sizeof(float) * 4 + sizeof(unsigned) * 2;
        }

        /// <summary>
        /// Serialises the impact pad data into (the start of) a target data buffer
        /// </summary>
        /// <param name="target">The data buffer to write to.</param>
        inline void serialise(void *target) const {
            // Cast the target buffer to a byte buffer so we can perform pointer
            // arithmetic
            unsigned char *buff = (unsigned char *)target;

            // Write each value to the buffer in turn, each time incrementing the
            // buffer pointer by the size of the object added
            *((float *)buff) = pos.x;
            buff += sizeof(float);
            *((float *)buff) = pos.y;
            buff += sizeof(float);
            *((float *)buff) = width;
            buff += sizeof(float);
            *((float *)buff) = length;
            buff += sizeof(float);
            *((unsigned *)buff) = materialHandle;
            buff += sizeof(unsigned);
            *((unsigned *)buff) = apertureHandle;
            buff += sizeof(unsigned);
        }

        /// <summary>
        /// Deserialises an impact pad from (the start of) a data buffer
        /// </summary>
        /// <param name="buffer">The data buffer to interpret as an impact
        /// pad.</param> <returns>A newly constructed impact pad object specified by
        /// the data from the buffer.</returns>
        inline static ImpactPad &deserialise(void *buffer) {
            // Cast the source buffer to a byte buffer so we can perform pointer
            // arithmetic
            unsigned char *buff = (unsigned char *)buffer;

            // Construct a new ImpactPad object for returning (hence on the heap)
            ImpactPad *pad = new ImpactPad();

            // Read each value in turn in the same sequence as specified by the
            // serialise function and write to each property in the pad itself.
            pad->pos.x = *((float *)buff);
            buff += sizeof(float);
            pad->pos.y = *((float *)buff);
            buff += sizeof(float);
            pad->width = *((float *)buff);
            buff += sizeof(float);
            pad->length = *((float *)buff);
            buff += sizeof(float);
            pad->materialHandle = *((unsigned *)buff);
            buff += sizeof(unsigned);
            pad->apertureHandle = *((unsigned *)buff);
            buff += sizeof(unsigned);

            // Return the impact pad object we constructed
            return *pad;
        }

      private:
        // The material and aperture handles for this impact pad which are accessed
        // through the interface specified above
        unsigned materialHandle;
        unsigned apertureHandle;
    };

    /// <summary>
    /// DamBar
    /// A data structure contatining the information needed to represent a dam bar
    /// component of a drawing.
    /// </summary>
    struct DamBar {
        // Friend the drawing structure
        friend struct Drawing;

        /// <summary>
        /// The coordiate of the top left corner of this dam bar (in mat
        /// coordinates)
        /// </summary>
        Coordinate pos;

        /// <summary>
        /// The width of this dam bar (in mat coordinates).
        /// </summary>
        float width;
        /// <summary>
        /// The length of this dam bar (in mat coordinates).
        /// </summary>
        float length;

        /// <summary>
        /// Default constructor
        /// </summary>
        inline DamBar() = default;

        /// <summary>
        /// Copy constructor.
        /// </summary>
        /// <param name="bar">The bar to copy.</param>
        inline DamBar(const DamBar &bar) {
            this->pos.x = bar.pos.x;
            this->pos.y = bar.pos.y;
            this->width = bar.width;
            this->length = bar.length;
            this->materialID = bar.materialID;
        }

        /// <summary>
        /// Default destructor
        /// </summary>
        inline ~DamBar() = default;

        /// <summary>
        /// Equality operator
        /// </summary>
        /// <param name="other">A reference to another dam bar object to compare
        /// to.</param> <returns>Whether or not the dam bars are considered
        /// equal.</returns>
        inline bool operator==(const DamBar &other) {
            // Two dam bars are considered equal if and only if their top left
            // corners, width, length and thickness are equal.
            return pos == other.pos && width == other.width &&
                   length == other.length && materialID == other.materialID;
        }

        /// <summary>
        /// Inequality operator
        /// </summary>
        /// <param name="other">A reference to another dam bar object to compare
        /// to.</param> <returns>Whether or not the dam bar are considered
        /// inequal.</returns>
        inline bool operator!=(const DamBar &other) {
            // Return the boolean NOT of whether the bars are considered equal
            return !(*this == other);
        }

        /// <summary>
        /// Calculates the serialised size of this dam bar.
        /// </summary>
        /// <returns>The size of this object when serialised.</returns>
        inline unsigned serialisedSize() const {
            // A dam bar is specified by 4 float values for the X, Y, W, H of the
            // rectangle, and the materialID
            return sizeof(float) * 4 + sizeof(unsigned);
        }

        /// <summary>
        /// Serialises the dam bar data into (the start of) a target data buffer
        /// </summary>
        /// <param name="target">The data buffer to write to.</param>
        inline void serialise(void *target) const {
            // Cast the target buffer to a byte buffer so we can perform pointer
            // arithmetic
            unsigned char *buff = (unsigned char *)target;

            // Write each value to the buffer in turn, each time incrementing the
            // buffer pointer by the size of the object added
            *((float *)buff) = pos.x;
            buff += sizeof(float);
            *((float *)buff) = pos.y;
            buff += sizeof(float);
            *((float *)buff) = width;
            buff += sizeof(float);
            *((float *)buff) = length;
            buff += sizeof(float);
            *((unsigned *)buff) = materialID;
            buff += sizeof(unsigned);
        }

        /// <summary>
        /// Deserialises an dam bar from (the start of) a data buffer
        /// </summary>
        /// <param name="buffer">The data buffer to interpret as an dam bar.</param>
        /// <returns>A newly constructed dam bar object specified by the data from
        /// the buffer.</returns>
        inline static DamBar &deserialise(void *buffer) {
            // Cast the source buffer to a byte buffer so we can perform pointer
            // arithmetic
            unsigned char *buff = (unsigned char *)buffer;

            // Construct a new DamBar object for returning (hence on the heap)
            DamBar *bar = new DamBar();

            // Read each value in turn in the same sequence as specified by the
            // serialise function and write to each property in the bar itself.
            bar->pos.x = *((float *)buff);
            buff += sizeof(float);
            bar->pos.y = *((float *)buff);
            buff += sizeof(float);
            bar->width = *((float *)buff);
            buff += sizeof(float);
            bar->length = *((float *)buff);
            buff += sizeof(float);
            bar->materialID = *((float *)buff);
            buff += sizeof(unsigned);

            // Return the dam bar object we constructed
            return *bar;
        }

        /// <summary>
        /// Getter for the material this Dam Bar is made from.
        /// </summary>
        /// <returns></returns>
        Material &material() const;

        /// <summary>
        /// Setter for the material this Dam Bar is made from.
        /// </summary>
        /// <param name="m">The new material for the Dam Bar.</param>
        inline void setMaterial(const Material &m) { materialID = m.componentID(); }

      private:
        unsigned materialID;
    };

    /// <summary>
    /// BlankSpace
    /// A data structure containing the information needed to represent a blank
    /// space component of a drawing.
    /// </summary>
    struct BlankSpace {
        friend struct Drawing;

        /// <summary>
        /// The position on the mat of this blank spaces top left corner.
        /// </summary>
        Coordinate pos;

        /// <summary>
        /// The width of the blank space.
        /// </summary>
        float width;
        /// <summary>
        /// The length of the blank space.
        /// </summary>
        float length;

        /// <summary>
        /// Default constructor
        /// </summary>
        inline BlankSpace() = default;

        /// <summary>
        /// Copy Constuctor
        /// </summary>
        /// <param name="space">The blank space to copy.</param>
        inline BlankSpace(const BlankSpace &space) {
            this->pos.x = space.pos.x;
            this->pos.y = space.pos.y;
            this->width = space.width;
            this->length = space.length;
        }
        /// <summary>
        /// Default destructor
        /// </summary>
        inline ~BlankSpace() = default;

        /// <summary>
        /// Blank Space equality operator.
        /// </summary>
        /// <param name="other">A blank space to compare if this blank space is
        /// equal.</param> <returns>A boolean representing whether or not the blank
        /// spaces are equal.</returns>
        inline bool operator==(const BlankSpace &other) {
            // Two Blank Spaces are considered equal if and only if their top left
            // corners, width, and length are the same
            return pos == other.pos && width == other.width && length == other.length;
        }

        /// <summary>
        /// Blank Space inequality operator
        /// </summary>
        /// <param name="other">A blank space to compare equality against.</param>
        /// <returns>A boolean representing whether or not the blank spaces are
        /// inequal.</returns>
        inline bool operator!=(const BlankSpace &other) {
            // Return the boolean NOT of whether the mats are considered equal
            return !(*this == other);
        }
        /// <summary>
        /// Calculates the serialised size of a blank space
        /// </summary>
        /// <returns>The size of a blank space in memory</returns>
        inline unsigned serialisedSize() const {
            // An BlankSpace is specified by 4 float values for the X, Y, W, H of the
            // rectangle.
            return sizeof(float) * 4;
        }
        /// <summary>
        /// Serialises a blank space object to be transmitted over networks.
        /// </summary>
        /// <param name="target">A buffer to write the serialised object to.</param>
        inline void serialise(void *target) const {
            // Cast the target buffer to a byte buffer so we can perform pointer
            // arithmetic
            unsigned char *buff = (unsigned char *)target;

            // Write each value to the buffer in turn, each time incrementing the
            // buffer pointer by the size of the object added
            *((float *)buff) = pos.x;
            buff += sizeof(float);
            *((float *)buff) = pos.y;
            buff += sizeof(float);
            *((float *)buff) = width;
            buff += sizeof(float);
            *((float *)buff) = length;
            buff += sizeof(float);
        }

        /// <summary>
        /// A static deserialisation function for blank spaces.
        /// </summary>
        /// <param name="buffer">Buffer to read the serialised blank space
        /// off.</param> <returns>A newly constructed blank space object</returns>
        inline static BlankSpace &deserialise(void *buffer) {
            // Cast the source buffer to a byte buffer so we can perform pointer
            // arithmetic
            unsigned char *buff = (unsigned char *)buffer;

            // Construct a new BlankSpace object for returning (hence on the heap)
            BlankSpace *pad = new BlankSpace();

            // Read each value in turn in the same sequence as specified by the
            // serialise function and write to each property in the pad itself.
            pad->pos.x = *((float *)buff);
            buff += sizeof(float);
            pad->pos.y = *((float *)buff);
            buff += sizeof(float);
            pad->width = *((float *)buff);
            buff += sizeof(float);
            pad->length = *((float *)buff);
            buff += sizeof(float);

            // Return the BlankSpace object we constructed
            return *pad;
        }
    };
    /// <summary>
    /// ExtraAperture
    /// Stores all relevant information for drawings that need multiple apertures
    /// </summary>
    struct ExtraAperture {
        friend struct Drawing;

        /// <summary>
        /// The position of the top left corner of the area (in mat coordinates).
        /// </summary>
        Coordinate pos;

        /// <summary>
        /// The width of the area (in mat coordinates).
        /// </summary>
        float width;
        /// <summary>
        /// The length of the area (in mat coordinates).
        /// </summary>
        float length;

        /// <summary>
        /// Default constructor.
        /// </summary>
        inline ExtraAperture() = default;
        /// <summary>
        /// Copy constructor.
        /// </summary>
        /// <param name="aperture">Extra aperture to copy.</param>
        inline ExtraAperture(const ExtraAperture &aperture) {
            this->pos.x = aperture.pos.x;
            this->pos.y = aperture.pos.y;
            this->width = aperture.width;
            this->length = aperture.length;
            this->apertureID = aperture.apertureID;
        }
        /// <summary>
        /// Default destructor
        /// </summary>
        inline ~ExtraAperture() = default;

        /// <summary>
        /// Equality Operator.
        /// </summary>
        /// <param name="other">Extra aperture to be compared against.</param>
        /// <returns>Boolean representing equality of extra apertures.</returns>
        inline bool operator==(const ExtraAperture &other) {
            // Two aperture are considered equal if and only if their top left
            // corners, width, and length and shape are the same
            return pos == other.pos && width == other.width &&
                   length == other.length && apertureID == other.apertureID;
        }

        /// <summary>
        /// Inequality Operator.
        /// </summary>
        /// <param name="other">Extra aperture to be compared against.</param>
        /// <returns>Boolean representing inequality of extra apertures.</returns>
        inline bool operator!=(const ExtraAperture &other) {
            // Return the boolean NOT of whether the mats are considered equal
            return !(*this == other);
        }

        /// <summary>
        /// Calculates the serialised sized of an extra aperture.
        /// </summary>
        /// <returns>Size in memory of this extra aperture.</returns>
        inline unsigned serialisedSize() const {
            // An aperture is specified by 4 float values for the X, Y, W, H of the
            // rectangle and unsigned for shape ID.
            return sizeof(float) * 4 + sizeof(unsigned);
        }

        /// <summary>
        /// Serialises an extra aperture to be transmitted across a network.
        /// </summary>
        /// <param name="target">The buffer to write the serialised object
        /// to.</param>
        inline void serialise(void *target) const {
            // Cast the target buffer to a byte buffer so we can perform pointer
            // arithmetic
            unsigned char *buff = (unsigned char *)target;

            // Write each value to the buffer in turn, each time incrementing the
            // buffer pointer by the size of the object added
            *((float *)buff) = pos.x;
            buff += sizeof(float);
            *((float *)buff) = pos.y;
            buff += sizeof(float);
            *((float *)buff) = width;
            buff += sizeof(float);
            *((float *)buff) = length;
            buff += sizeof(float);
            *((unsigned *)buff) = apertureID;
            buff += sizeof(unsigned);
        }

        /// <summary>
        /// Deserialises an extra aperture recieved from the network.
        /// </summary>
        /// <param name="buffer">The buffer to read the serialised extra aperture
        /// from.</param> <returns>An extra aperture object from the
        /// buffer.</returns>
        inline static ExtraAperture &deserialise(void *buffer) {
            // Cast the source buffer to a byte buffer so we can perform pointer
            // arithmetic
            unsigned char *buff = (unsigned char *)buffer;

            // Construct a new aperture object for returning (hence on the heap)
            ExtraAperture *aperture = new ExtraAperture();

            // Read each value in turn in the same sequence as specified by the
            // serialise function and write to each property in the aperture itself.
            aperture->pos.x = *((float *)buff);
            buff += sizeof(float);
            aperture->pos.y = *((float *)buff);
            buff += sizeof(float);
            aperture->width = *((float *)buff);
            buff += sizeof(float);
            aperture->length = *((float *)buff);
            buff += sizeof(float);
            aperture->apertureID = *((unsigned *)buff);
            buff += sizeof(unsigned);

            // Return the aperture object we constructed
            return *aperture;
        }

        /// <summary>
        /// Gets a const reference of the current aperture used by this centre hole.
        /// </summary>
        /// <returns>THe aperture used by this centre hole.</returns>
        const Aperture &aperture() const;

        /// <summary>
        /// Setter for the aperture.
        /// </summary>
        /// <param name="ap">The new aperture.</param>
        inline void setAperture(const Aperture &ap) {
            apertureID = ap.componentID();
        }

      private:
        unsigned apertureID;
    };

    /// <summary>
    /// CentreHole
    /// A data structure containing the information needed to represent a single
    /// centre hole on a drawing.
    /// </summary>
    struct CentreHole {
        // Friend the drawing structure
        friend struct Drawing;
        friend class GroupGraphicsItem<CentreHole, Aperture>;

        /// <summary>
        /// Shape
        /// Internal Shape grouping structure for the shape of a centre hole
        /// </summary>
        /*struct Shape {
        // The width and length of the shape which is cut out of the mat
        float width, length;
        // Whether or not the shape is cut from a rounded tool
        bool rounded;

        /// <summary>
        /// Equality operator
        /// </summary>
        /// <param name="other">A reference to another shape object to compare
    to.</param>
        /// <returns>Whether or not the two shapes are considered to be
    equal.</returns> inline bool operator==(const Shape &other) {
            // We consider two shapes to be equal if and only if they have the
    same width, lenght and roundedness. return width == other.width && length ==
    other.length && rounded == other.rounded;
        }

        /// <summary>
        /// Inequality operator
        /// </summary>
        /// <param name="other">A reference to another shape object to compare
    to.</param>
        /// <returns>Whether or not the two shapes are considered to be
    inequal.</returns> inline bool operator!=(const Shape &other) {
            // Return the boolean NOT of whether the two shapes are considered
    equal. return !(*this == other);
        }
    } centreHoleShape;*/ // The shame of this particular centre hole

        // The position of the centre of the where the centre hole is punched

        /// <summary>
        /// The position of the centre of the centre hole.
        /// </summary>
        Coordinate pos;

        /// <summary>
        /// Default constructor
        /// </summary>
        inline CentreHole() = default;

        /// <summary>
        /// Copy constructor.
        /// </summary>
        /// <param name="hole">Centre Hole to copy.</param>
        inline CentreHole(const CentreHole &hole) {
            this->pos.x = hole.pos.x;
            this->pos.y = hole.pos.y;
            this->apertureID = hole.apertureID;
        }

        /// <summary>
        /// Default destructor
        /// </summary>
        inline ~CentreHole() = default;

        /// <summary>
        /// Equality operator
        /// </summary>
        /// <param name="other">A reference to another centre hole object to compare
        /// to.</param> <returns>Whether or not the two centre holes are considered
        /// to be equal.</returns>
        inline bool operator==(const CentreHole &other) {
            // We consider two centre holes to be equal if and only if their shapes
            // are equal and their positions are equal.
            return apertureID == other.apertureID && pos == other.pos;
        }

        /// <summary>
        /// Inequality operator
        /// </summary>
        /// <param name="other">A reference to another centre hole object to compare
        /// to.</param> <returns>Whether or not the two center holes are considered
        /// to be inequal.</returns>
        inline bool operator!=(const CentreHole &other) {
            // Return the boolean NOT of whether the centre holes are considered equal
            return !(*this == other);
        }

        /// <summary>
        /// Getter for the serialised size of this centre hole when written to a
        /// data buffer
        /// </summary>
        /// <returns>The number of bytes this object will occupy in a data
        /// buffer.</returns>
        inline unsigned serialisedSize() const {
            // A centre hole is defined by its X, Y, W, H and roundedness, and so is
            // fully specified by 4 floats and a single boolean.
            return sizeof(float) * 4 + sizeof(bool);
        }

        /// <summary>
        /// Serialises the centre hole into (the start of) a target data buffer
        /// </summary>
        /// <param name="target">The data buffer to write to.</param>
        inline void serialise(void *target) const {
            // Cast the
            unsigned char *buff = (unsigned char *)target;

            *((float *)buff) = pos.x;
            buff += sizeof(float);
            *((float *)buff) = pos.y;
            buff += sizeof(float);
            *((unsigned *)buff) = apertureID;
            buff += sizeof(unsigned);
        }

        /// <summary>
        /// Deserialises the centre hole from (the start of) a data buffer.
        /// </summary>
        /// <param name="buffer">The data buffer to read from.</param>
        /// <returns>The newly constructed Centre Hole.</returns>
        inline static CentreHole &deserialise(void *buffer) {
            unsigned char *buff = (unsigned char *)buffer;

            CentreHole *hole = new CentreHole();

            hole->pos.x = *((float *)buff);
            buff += sizeof(float);
            hole->pos.y = *((float *)buff);
            buff += sizeof(float);
            hole->apertureID = *((unsigned *)buff);
            buff += sizeof(unsigned);

            return *hole;
        }

        /// <summary>
        /// Gets a reference to the aperture this centre hole is punched by.
        /// </summary>
        /// <returns>The aperture.</returns>
        Aperture &aperture() const;

        /// <summary>
        /// Sets the aperture to the provided aperture.
        /// </summary>
        /// <param name="ap">The new aperture for the centre hole.</param>
        inline void setAperture(const Aperture &ap) {
            apertureID = ap.componentID();
        }

      private:
        unsigned apertureID;
    };

    /// <summary>
    /// Deflector
    /// A data structure representing the information required to represent a
    /// single deflector on a mat
    /// </summary>
    struct Deflector {
        // Friend the drawing structure
        friend struct Drawing;
        friend class GroupGraphicsItem<Deflector, Material>;

        /// <summary>
        /// The position of the centre of the deflector.
        /// </summary>
        Coordinate pos;

        /// <summary>
        /// The side length of the square of material used as the deflector
        /// </summary>
        float size = 45;

        /// <summary>
        /// Default constructor
        /// </summary>
        inline Deflector() = default;

        /// <summary>
        /// Copy constructor
        /// </summary>
        /// <param name="defl">The deflector to copy.</param>
        inline Deflector(const Deflector &defl) {
            this->pos.x = defl.pos.x;
            this->pos.y = defl.pos.y;
            this->size = defl.size;
            this->materialHandle = defl.materialHandle;
        }

        /// <summary>
        /// Default destructor
        /// </summary>
        inline ~Deflector() = default;

        /// <summary>
        /// Equality operator
        /// </summary>
        /// <param name="other">A reference to another deflector object to compare
        /// to.</param> <returns>Whether or not the two deflectors are considered to
        /// be equal.</returns>
        inline bool operator==(const Deflector &other) {
            // We consider two deflectors to be equal if they have the same centre,
            // size and material.
            return pos == other.pos && size == other.size &&
                   materialHandle == other.materialHandle;
        }

        /// <summary>
        /// Inequality operator
        /// </summary>
        /// <param name="other">A reference to another deflector object to compare
        /// to.</param> <returns>Whether or not the two deflectors are considered to
        /// be inequal.</returns>
        inline bool operator!=(const Deflector &other) {
            // Return the boolean NOT of whether the two defelctors are equal.
            return !(*this == other);
        }

        /// <summary>
        /// Getter for the material
        /// </summary>
        /// <returns>A reference to the material pointed to by the material
        /// handle.</returns>
        Material &material() const;

        /// <summary>
        /// Setter for the material
        /// </summary>
        /// <param name="material">The material to set this deflector's material
        /// to.</param>
        void setMaterial(const Material &material) {
            materialHandle = material.handle();
        }
        /// <summary>
        /// Calculates the serialised sized of this object
        /// </summary>
        /// <returns>The serialised size</returns>
        inline unsigned serialisedSize() const {
            return sizeof(float) * 4 + sizeof(unsigned);
        }

        /// <summary>
        /// Serialises the deflector to be sent across the network.
        /// </summary>
        /// <param name="target">Buffer to write the object to.</param>
        inline void serialise(void *target) const {
            unsigned char *buff = (unsigned char *)target;

            *((float *)buff) = pos.x;
            buff += sizeof(float);
            *((float *)buff) = pos.y;
            buff += sizeof(float);
            *((float *)buff) = size;
            buff += sizeof(float);
            *((unsigned *)buff) = materialHandle;
            buff += sizeof(unsigned);
        }

        /// <summary>
        /// Deserialises a deflector.
        /// </summary>
        /// <param name="buffer">Buffer to read the serialised object from.</param>
        /// <returns>Newly created Deflector object</returns>
        inline static Deflector &deserialise(void *buffer) {
            unsigned char *buff = (unsigned char *)buffer;

            Deflector *deflector = new Deflector();

            deflector->pos.x = *((float *)buff);
            buff += sizeof(float);
            deflector->pos.y = *((float *)buff);
            buff += sizeof(float);
            deflector->size = *((float *)buff);
            buff += sizeof(float);
            deflector->materialHandle = *((unsigned *)buff);
            buff += sizeof(unsigned);

            return *deflector;
        }

      private:
        unsigned materialHandle;
    };

    /// <summary>
    /// Divertor
    /// A data structure representing the information required to represent a
    /// single divertor on the mat.
    /// </summary>
    struct Divertor {
        // Friend the drawing structure
        friend struct Drawing;
        friend class GroupGraphicsItem<Divertor, Material>;

        /// <summary>
        /// The side of the mat this divertor is on
        /// </summary>
        Side side;
        /// <summary>
        /// The vertical position defined as the distance from the top of the mat to
        /// the centre of line where the divertor touches the side of the mat.
        /// </summary>
        float verticalPosition;
        /// <summary>
        /// The width of the divertor measured as the perpendicular distance between
        /// the two diagonal parallel lines
        /// </summary>
        float width = 25;
        /// <summary>
        /// The length of the divertor measured as the length of each diagonal line
        /// </summary>
        float length = 150;

        /// <summary>
        /// Default constructor
        /// </summary>
        inline Divertor() = default;

        /// <summary>
        /// Copy constructor
        /// </summary>
        /// <param name="divertor">The Divertor to copy.</param>
        inline Divertor(const Divertor &divertor) {
            this->side = divertor.side;
            this->verticalPosition = divertor.verticalPosition;
            this->width = divertor.width;
            this->length = divertor.length;
            this->materialHandle = divertor.materialHandle;
        }

        /// <summary>
        /// Default destructor
        /// </summary>
        inline ~Divertor() = default;

        /// <summary>
        /// Equality operator
        /// </summary>
        /// <param name="other">A reference to another divertor to compare
        /// to.</param> <returns>Whether or not the two divertors are considered to
        /// be equal.</returns>
        inline bool operator==(const Divertor &other) {
            // We consider two divertors equal if they are on the same side of the mat
            // and have the same vertical position, width, length and material.
            return side == other.side && verticalPosition == other.verticalPosition &&
                   width == other.width && length == other.length &&
                   materialHandle == other.materialHandle;
        }

        /// <summary>
        /// Inequality operator
        /// </summary>
        /// <param name="other">A reference to another divertor to compare
        /// to.</param> <returns>Whether or not the two divertors are considered to
        /// be inequal.</returns>
        inline bool operator!=(const Divertor &other) {
            // Return the boolean NOT of whether they are considered to be equal
            return !(*this == other);
        }

        /// <summary>
        /// Getter for the material
        /// </summary>
        /// <returns>A reference to the material pointed to by the material
        /// handle.</returns>
        Material &material() const;

        /// <summary>
        /// Setter for the material
        /// </summary>
        /// <param name="material">The material to set this divertor to.</param>
        void setMaterial(const Material &material) {
            materialHandle = material.handle();
        }

        /// <summary>
        /// Calculates the serialised size of this object.
        /// </summary>
        /// <returns>The size of the serialised object.</returns>
        inline unsigned serialisedSize() const {
            return sizeof(unsigned char) + sizeof(float) * 3 + sizeof(unsigned);
        }

        /// <summary>
        /// Serialises a divertor object onto a buffer.
        /// </summary>
        /// <param name="target">Buffer to seralise divertor object onto.</param>
        inline void serialise(void *target) const {
            unsigned char *buff = (unsigned char *)target;

            *buff++ = (unsigned char)side;
            *((float *)buff) = verticalPosition;
            buff += sizeof(float);
            *((float *)buff) = width;
            buff += sizeof(float);
            *((float *)buff) = length;
            buff += sizeof(float);
            *((unsigned *)buff) = materialHandle;
            buff += sizeof(unsigned);
        }

        /// <summary>
        /// Deserialises a divertor object off a buffer.
        /// </summary>
        /// <param name="buffer">Buffer to read divertor object from.</param>
        /// <returns>A newly created divertor object.</returns>
        inline static Divertor &deserialise(void *buffer) {
            unsigned char *buff = (unsigned char *)buffer;

            Divertor *divertor = new Divertor();

            divertor->side = (Side)(*buff++);
            divertor->verticalPosition = *((float *)buff);
            buff += sizeof(float);
            divertor->width = *((float *)buff);
            buff += sizeof(float);
            divertor->length = *((float *)buff);
            buff += sizeof(float);
            divertor->materialHandle = *((unsigned *)buff);
            buff += sizeof(unsigned);

            return *divertor;
        }

      private:
        // Handle for the material this divertor is made from
        unsigned materialHandle;
    };

    /// <summary>
    /// Default constructor for a Drawing object
    /// </summary>
    Drawing();

    /// <summary>
    /// Copy constructor for a Drawing object
    /// </summary>
    /// <param name="drawing">The Drawing to copy the data from.</param>
    explicit Drawing(const Drawing &drawing);

    /// <summary>
    /// Destructor for a Drawing object
    /// </summary>
    ~Drawing();

    /// <summary>
    /// Sets each field in this drawing to its default value.
    /// </summary>
    void setAsDefault();

    /// <summary>
    /// Getter for the drawing number
    /// </summary>
    /// <returns>The drawing number for this drawing as a string.</returns>
    std::string drawingNumber() const;

    /// <summary>
    /// Setter for the drawing number
    /// </summary>
    /// <param name="newDrawingNumber">The string of the drawing number to set
    /// this drawing to.</param>
    void setDrawingNumber(const std::string &newDrawingNumber);

    /// <summary>
    /// Getter for the date
    /// </summary>
    /// <returns>The date this drawing was created.</returns>
    Date date() const;

    /// <summary>
    /// Setter for the date
    /// </summary>
    /// <param name="newDate">Date to set this drawing to.</param>
    void setDate(Date newDate);

    /// <summary>
    /// Getter for the width
    /// </summary>
    /// <returns>The width of the drawing in millimetres.</returns>
    virtual float width() const;

    /// <summary>
    /// Setter for the width
    /// </summary>
    /// <param name="newWidth">The width value to set this drawing to in
    /// millimetres.</param>
    void setWidth(float newWidth);

    /// <summary>
    /// Getter for the length
    /// </summary>
    /// <returns>The length of this drawing in millimetres.</returns>
    virtual float length() const;

    /// <summary>
    /// Setter for the length
    /// </summary>
    /// <param name="newLength">The length value to set this drawing to in
    /// millimetres.</param>
    void setLength(float newLength);

    /// <summary>
    /// Getter for the drawing PDF hyperlink
    /// </summary>
    /// <returns>A file path the hyperlink for the drawing PDF.</returns>
    std::filesystem::path hyperlink() const;

    /// <summary>
    /// Setter for the drawing PDF hyperlink
    /// </summary>
    /// <param name="newHyperlink">The hyperlink path to set this drawing
    /// to.</param>
    void setHyperlink(const std::filesystem::path &newHyperlink);

    /// <summary>
    /// Getter for the free-text drawing notes field
    /// </summary>
    /// <returns>A string of the notes associated with this drawing.</returns>
    std::string notes() const;

    /// <summary>
    /// Setter for the free-text drawing notes field
    /// </summary>
    /// <param name="newNotes">The string to set this drawing's notes to.</param>
    void setNotes(const std::string &newNotes);

    /// <summary>
    /// Getter for the machine template
    /// </summary>
    /// <returns>A copy of the machine template details assocaited with this
    /// drawing.</returns>
    MachineTemplate machineTemplate() const;

    /// <summary>
    /// Component-wise setter for the machine template of this drawing
    /// </summary>
    /// <param name="machine">The machine to update the template to use.</param>
    /// <param name="quantityOnDeck">The quantity on deck to update this template
    /// to have.</param> <param name="position">The position string to update this
    /// template to have.</param> <param name="deck">The deck to update this
    /// template to use.</param>
    void setMachineTemplate(const Machine &machine, unsigned quantityOnDeck,
                            const std::string &position, const MachineDeck &deck);

    /// <summary>
    /// Individual setter for the machine
    /// </summary>
    /// <param name="machine">The machine to update the template to use.</param>
    void setMachine(const Machine &machine);

    /// <summary>
    /// Individual setter for the quantity on deck
    /// </summary>
    /// <param name="quantityOnDeck">The quantity on deck to update this template
    /// to have.</param>
    void setQuantityOnDeck(unsigned quantityOnDeck);

    /// <summary>
    /// Individual setter for the position
    /// </summary>
    /// <param name="position">The position string to update this template to
    /// have.</param>
    void setMachinePosition(const std::string &position);

    /// <summary>
    /// Individual setter for the machine deck
    /// </summary>
    /// <param name="deck">The deck to update this template to use.</param>
    void setMachineDeck(const MachineDeck &deck);

    /// <summary>
    /// Getter for the product
    /// </summary>
    /// <returns>The Product object associated with this drawing.</returns>
    virtual Product product() const;

    /// <summary>
    /// Setter for the product
    /// </summary>
    /// <param name="prod">The product to set this drawing to.</param>
    void setProduct(const Product &prod);

    /// <summary>
    /// Getter for the aperture
    /// </summary>
    /// <returns>The Aperture object associated with this drawing.</returns>
    virtual Aperture &aperture() const;

    /// <summary>
    /// Setter for the aperture
    /// </summary>
    /// <param name="ap">The Aperture to set this drawing to.</param>
    void setAperture(const Aperture &ap);

    /// <summary>
    /// Getter for backing strip optional.
    /// </summary>
    /// <returns>Backing strip if exists.</returns>
    virtual std::optional<BackingStrip> backingStrip() const;

    /// <summary>
    /// Setter for backing strip.
    /// </summary>
    /// <param name="strip">Backing strip to attach to drawing.</param>
    void setBackingStrip(const BackingStrip &strip);

    /// <summary>
    /// Removes the backing strip
    /// </summary>
    void removeBackingStrip();

    /// <summary>
    /// Checks existence of backing strips.
    /// </summary>
    /// <returns>Boolean representing existence of backing strips.</returns>
    virtual bool hasBackingStrips() const;

    /// <summary>
    /// Getter for the tension type
    /// </summary>
    /// <returns>The TensionType associated with this drawing.</returns>
    virtual TensionType tensionType() const;

    /// <summary>
    /// Setter for the tension type
    /// </summary>
    /// <param name="newTensionType">The TensionType to set this drawing
    /// to.</param>
    void setTensionType(TensionType newTensionType);

    /// <summary>
    /// Getter for the rebated property
    /// </summary>
    /// <returns>Whether the drawing is rebated or not.</returns>
    virtual bool rebated() const;

    /// <summary>
    /// Setter for the rebated property
    /// </summary>
    /// <param name="isRebated">Value to set the rebated property to.</param>
    void setRebated(bool isRebated);

    /// <summary>
    /// Gets the material from the given material layer.
    /// </summary>
    /// <param name="layer">The layer to get the material from.</param>
    /// <returns>The material at the given layer.</returns>
    virtual std::optional<Material> material(MaterialLayer layer) const;

    /// <summary>
    /// Setter for the material
    /// </summary>
    /// <param name="layer">The layer this material is used on. Either TOP or
    /// BOTTOM.</param> <param name="mat">The material to set the specified layer
    /// to use.</param>
    void setMaterial(MaterialLayer layer, const Material &mat);

    /// <summary>
    /// Removes the bottom layer of the mat, such that the mat now only has a
    /// single layer.
    /// </summary>
    void removeBottomLayer();

    /// <summary>
    /// Getter for the number of bars on the mat
    /// </summary>
    /// <returns>The number of support bars used to support this mat.</returns>
    virtual unsigned numberOfBars() const;

    /// <summary>
    /// Setter for the bar spacings and bar widths
    /// </summary>
    /// <param name="spacings">The set of bar spacings for this mat, namely the
    /// distance between each bar centre line.</param> <param name="widths">The
    /// width of each bar itself on the mat.</param>
    void setBars(const std::vector<float> &spacings,
                 const std::vector<float> &widths);

    /// <summary>
    /// Getter for an indexed bar spacing
    /// </summary>
    /// <param name="index">The index of the bar spacing to return.</param>
    /// <returns>The spacing at the specified index.</returns>
    virtual float barSpacing(unsigned index) const;

    /// <summary>
    /// Getter for an indexed bar width
    /// </summary>
    /// <param name="index">The index of the bar to retrieve. Note this function
    /// includes the left and right margins, and so the left most true "bar" is
    /// indexed as 0.</param> <returns></returns>
    virtual float barWidth(unsigned index) const;

    /// <summary>
    /// Getter for the left margin, the space between the left of the mat and the
    /// first punch area (not including overlap).
    /// </summary>
    /// <returns>The width of the left margin.</returns>
    virtual float leftMargin() const;

    /// <summary>
    /// Getter for the right margin, the space between the last punch area and the
    /// right of the mat (not including overlap).
    /// </summary>
    /// <returns>The width of the right margin.</returns>
    virtual float rightMargin() const;

    /// <summary>
    /// Getter for the vector of all bar spacings, does not include margins.
    /// </summary>
    /// <returns>A vector of a copy of each bar spacing in this drawing.</returns>
    virtual std::vector<float> allBarSpacings() const;

    /// <summary>
    /// Getter for the vector of all bar widths. Note: this returns the left and
    /// right margins as well
    /// </summary>
    /// <returns>A vector of a copy of each bar width (and margin) in this
    /// drawing.</returns>
    virtual const std::vector<float> &allBarWidths() const;

    /// <summary>
    /// Getter for each side iron
    /// </summary>
    /// <param name="side">The side to retrieve the side iron for.</param>
    /// <returns>The side iron associated with the specified side.</returns>
    virtual SideIron sideIron(Side side) const;

    /// <summary>
    /// Getter for whether each side iron is inverted or not.
    /// </summary>
    /// <param name="side">The side to retrieve the property from.</param>
    /// <returns>Whether the specified side iron is inverted.</returns>
    virtual bool sideIronInverted(Side side) const;

    /// <summary>
    /// Getter for whether each side iron is cut down or not.
    /// </summary>
    /// <param name="side">The side to retrieve the property from.</param>
    /// <returns>Whether the specified side iron is cut down.</returns>
    virtual bool sideIronCutDown(Side side) const;

    /// <summary>
    /// Returns a bool whether or not the either ends have a value.
    /// </summary>
    /// <returns>True if either of the sides have an ending, false
    /// otherwise.</returns>
    virtual bool sideIronFixedEnd() const;

    /// <summary>
    /// Getter for a specific ending on the given side.
    /// </summary>
    /// <param name="side">The side to get the ending of.</param>
    /// <returns>The requested ending, given it exists.</returns>
    virtual std::optional<Ending> sideIronFixedEnd(Side side) const;

    /// <summary>
    /// Getter for the feed end.
    /// </summary>
    /// <returns>The side that is the feed end.</returns>
    virtual std::optional<Side> sideIronFeedEnd() const;

    /// <summary>
    /// Getter for the hook orientation on the given side
    /// </summary>
    /// <param name="side"></param>
    /// <returns></returns>
    virtual std::optional<HookOrientation> sideIronHookOrientation(
        Side side) const;

    /// <summary>
    /// Setter for a side iron
    /// </summary>
    /// <param name="side">The side to specify the side iron for.</param>
    /// <param name="sideIron">The side iron to set the specified side to
    /// use.</param>
    void setSideIron(Side side, const SideIron &sideIron);

    /// <summary>
    /// Setter for the whether the side iron is inverted
    /// </summary>
    /// <param name="side">The side to are specifying for.</param>
    /// <param name="inverted">The value to set the invertedness of the specified
    /// side to.</param>
    void setSideIronInverted(Side side, bool inverted);

    /// <summary>
    /// Setter for whether the side iron is cut down
    /// </summary>
    /// <param name="side">The side to are specifying for.</param>
    /// <param name="cutDown">The value to set the specified side to.</param>
    void setSideIronCutDown(Side side, bool cutDown);

    /// <summary>
    /// Sets the type of endings of the side iron on a given side.
    /// </summary>
    /// <param name="side">Side to change ending of.</param>
    /// <param name="ending">New ending.</param>
    void setSideIronEnding(Side side, Ending ending);

    /// <summary>
    /// Sets the given side to be the feed side.
    /// </summary>
    /// <param name="side">The side to set as feed.</param>
    void setSideIronFeed(Side side);

    /// <summary>
    /// Sets the hook orientation for a given side.
    /// </summary>
    /// <param name="side">Side to change hook orientation.</param>
    /// <param name="orientation">The new hook orientation.</param>
    void setSideIronHookOrientation(Side side, HookOrientation orientation);

    /// <summary>
    /// Checks if the drawing has straps.
    /// </summary>
    /// <returns>True if the drawing has straps, false otherwise.</returns>
    bool hasStraps() const;

    /// <summary>
    /// Checks if a specific side has a strap.
    /// </summary>
    /// <param name="side">The side to check for straps.</param>
    /// <returns>True if the specified side has a strap, false otherwise.</returns>
    bool hasStrap(Side side) const;

    /// <summary>
    /// Counts the amount of straps the drawing has.
    /// </summary>
    /// <returns>The amount of straps the drawing has.</returns>
    unsigned char strapsCount() const;

    /// <summary>
    /// Gets a reference to a strap on a given side. Will throw an error if a there is no strap on the provided side.
    /// </summary>
    /// <param name="side">The side to get the strap from.</param>
    /// <returns>The strap on the provided side.</returns>
    Strap &sideIronStrap(Side side) const;

    /// <summary>
    /// Sets the side iron on the given side to have the given strap.
    /// </summary>
    /// <param name="side">The side of the iron.</param>
    /// <param name="strap">The strap to add to the iron.</param>
    void setSideIronStrap(Side side, const Strap &strap);

    /// <summary>
    /// Removes the strap from the side iron on the given side.
    /// </summary>
    /// <param name="side">The side to remove the strap from.</param>
    void removeSideIronStrap(Side side);

    /// <summary>
    /// Remove a side iron ending from the given side.
    /// </summary>
    /// <param name="side">Side to remove ending from.</param>
    void removeSideIronEnding(Side side);

    /// <summary>
    /// remove's the side iron feed option from a given side.
    /// </summary>
    /// <param name="side">Side to remove the feed flag from.</param>
    void removeSideIronFeed(Side side);

    /// <summary>
    /// Resets the side iron hook orientation from a given side.
    /// </summary>
    /// <param name="side">The side to remove the hook orientation from.</param>
    void removeSideIronHookOrientation(Side side);

    /// <summary>
    /// Remover for a side iron
    /// </summary>
    /// <param name="side">The side to remove the side iron from.</param>
    void removeSideIron(Side side);

    /// <summary>
    /// Getter for a sidelap
    /// </summary>
    /// <param name="side">The side to retrieve the sidelap for.</param>
    /// <returns>The sidelap associated with the given side.</returns>
    virtual std::optional<Lap> sidelap(Side side) const;

    /// <summary>
    /// Setter for a sidelap
    /// </summary>
    /// <param name="side">The side to specify the sidelap for.</param>
    /// <param name="lap">The sidelap value to set the given side to.</param>
    void setSidelap(Side side, const Lap &lap);

    /// <summary>
    /// Remover for a sidelap
    /// </summary>
    /// <param name="side">The side to remove the sidelap from.</param>
    void removeSidelap(Side side);

    /// <summary>
    /// Getter for an overlap
    /// </summary>
    /// <param name="side">The side to retrieve the overlap for.</param>
    /// <returns>The overlap associated with the given side.</returns>
    virtual std::optional<Lap> overlap(Side side) const;

    /// <summary>
    /// Setter for an overlap
    /// </summary>
    /// <param name="side">The side to specify the overlap for.</param>
    /// <param name="lap">The overlap value to set the given side to.</param>
    void setOverlap(Side side, const Lap &lap);

    /// <summary>
    /// Remover for an overlap
    /// </summary>
    /// <param name="side">The side to remove the overlap from.</param>
    void removeOverlap(Side side);

    /// <summary>
    /// Getter for the press punch program PDF hyperlinks
    /// </summary>
    /// <returns>A vector of each file path for the additional press drawing
    /// hyperlinks.</returns>
    std::vector<std::filesystem::path> pressDrawingHyperlinks() const;

    /// <summary>
    /// Setter for the press punch program PDF hyperlinks
    /// </summary>
    /// <param name="hyperlinks">A vector of each file path for the additional
    /// press drawing hyperlinks to set.</param>
    void setPressDrawingHyperlinks(
        const std::vector<std::filesystem::path> &hyperlinks);

    /// <summary>
    /// Helper function to return if the drawing has any sidelaps
    /// </summary>
    /// <returns>Whether or not the drawing has sidelaps.</returns>
    virtual bool hasSidelaps() const;

    /// <summary>
    /// Helper function to return if the drawing has any overlaps
    /// </summary>
    /// <returns>Whether or not the drawing has overlaps.</returns>
    virtual bool hasOverlaps() const;

    /// <summary>
    /// Adds an impact pad to the drawing
    /// </summary>
    /// <param name="impactPad">The impact pad to add to the drawing.</param>
    void addImpactPad(const ImpactPad &impactPad);

    /// <summary>
    /// Getter for the impact pads
    /// </summary>
    /// <returns>A vector of impact pads on this drawing.</returns>
    virtual std::vector<ImpactPad> impactPads() const;

    /// <summary>
    /// Getter for an individual impact pad
    /// </summary>
    /// <param name="index">The index to retrieve the impact pad from</param>
    /// <returns>A reference to the impact pad at the given index.</returns>
    virtual ImpactPad &impactPad(unsigned index);

    /// <summary>
    /// Getter for an individual const impact pad.
    /// </summary>
    /// <param name="index">index of impact pad.</param>
    /// <returns>Impact pad at index.</returns>
    virtual const ImpactPad &safeImpactPad(unsigned index) const;

    /// <summary>
    /// Remover for an impact pad from the drawing
    /// </summary>
    /// <param name="pad">The impact pad to remove.</param>
    void removeImpactPad(const ImpactPad &pad);

    /// <summary>
    /// Getter for the number of impact pads
    /// </summary>
    /// <returns>The size of the impact pads vector.</returns>
    virtual unsigned numberOfImpactPads() const;

    /// <summary>
    /// Adds a new dam bar to the drawing.
    /// </summary>
    /// <param name="bar">The dam bar too add.</param>
    void addDamBar(const DamBar &bar);

    /// <summary>
    /// Returns all dam bars attached to this drawing.
    /// </summary>
    /// <returns>The dam bars.</returns>
    virtual std::vector<DamBar> damBars() const;

    /// <summary>
    /// Getter for a specific dam bar by index in the dam bars vector.
    /// </summary>
    /// <param name="index">The index of the dam bar.</param>
    /// <returns>The dam bar at the given index.</returns>
    virtual DamBar &damBar(unsigned index);

    /// <summary>
    /// Removes a given dam bar.
    /// </summary>
    /// <param name="bar">The dam bar to be removed.</param>
    void removeDamBar(const DamBar &bar);

    /// <summary>
    /// Returns the number of dam bars currently attached to the drawing.
    /// </summary>
    /// <returns>The number of dam bars.</returns>
    virtual unsigned numberOfDamBars() const;

    /// <summary>
    /// Adds a new blank space to the drawing.
    /// </summary>
    /// <param name="blankSpace">The blank space to add.</param>
    void addBlankSpace(const BlankSpace &blankSpace);

    /// <summary>
    /// Returns a vector of all blank spaces attached to the drawing.
    /// </summary>
    /// <returns>Vector of blank spaces.</returns>
    virtual std::vector<BlankSpace> blankSpaces() const;

    /// <summary>
    /// Getter for a specific blank space by its index in the vector of blank
    /// spaces.
    /// </summary>
    /// <param name="index">The index of the blank space.</param>
    /// <returns>The blank space.</returns>
    virtual BlankSpace &blankSpace(unsigned index);

    /// <summary>
    /// Removes a blank space from the drawing that is equal to the given blank
    /// space.
    /// </summary>
    /// <param name="space">The given blank space to delete a equal blank
    /// space.</param>
    void removeBlankSpace(const BlankSpace &space);

    /// <summary>
    /// The number of blank spaces attached to the drawing.
    /// </summary>
    /// <returns></returns>
    virtual unsigned numberOfBlankSpaces() const;

    /// <summary>
    /// Adds an extra aperture area to the drawing
    /// </summary>
    /// <param name="extraAperture">The extra aperture to be added to the
    /// drawing.</param>
    void addExtraAperture(const ExtraAperture &extraAperture);

    /// <summary>
    /// Getter for all the extra apertures.
    /// </summary>
    /// <returns>Vector of all extra apertures.</returns>
    virtual std::vector<ExtraAperture> extraApertures() const;

    /// <summary>
    /// Gets an extra aperture via its index in the vector.
    /// </summary>
    /// <param name="index">The index of the extra aperture.</param>
    /// <returns>The extra aperture at the index.</returns>
    virtual ExtraAperture &extraAperture(unsigned index);

    /// <summary>
    /// Removes an extra aperture that is equal to the given extra aperture.
    /// </summary>
    /// <param name="aperture">Extra aperture to delete equal extra
    /// apertures.</param>
    void removeExtraAperture(const ExtraAperture &aperture);

    /// <summary>
    /// Getter for the amount of extra apertures attached to the drawing.
    /// </summary>
    /// <returns></returns>
    virtual unsigned numberOfExtraApertures() const;

    /// <summary>
    /// Adds a centre hole to the drawing
    /// </summary>
    /// <param name="centreHole">The centre hole to add to the drawing.</param>
    void addCentreHole(const CentreHole &centreHole);

    /// <summary>
    /// Const reference getter for the centre holes.
    /// </summary>
    /// <returns>A const ref to the vector of centre holes on this
    /// drawing.</returns>
    virtual const std::vector<CentreHole> &centreHoles() const;

    /// <summary>
    /// Reference getter for the centre holes.
    /// </summary>
    /// <returns>A reference to the vector of centre holes on this
    /// drawing.</returns>
    std::vector<CentreHole> &centreHoles();

    /// <summary>
    /// Getter for an individual centre hole
    /// </summary>
    /// <param name="index">The index to retrieve the centre hole from</param>
    /// <returns>A reference to the centre hole at the given index.</returns>
    virtual CentreHole &centreHole(unsigned index);

    /// <summary>
    /// Remover for a centre hole from the drawing
    /// </summary>
    /// <param name="hole">The centre hole to remove.</param>
    void removeCentreHole(const CentreHole &hole);

    /// <summary>
    /// Getter for the number of centre holes
    /// </summary>
    /// <returns>The size of the center holes vector.</returns>
    virtual unsigned numberOfCentreHoles() const;

    /// <summary>
    /// Adds a deflector to the drawing
    /// </summary>
    /// <param name="deflector">The deflector to add to the drawing.</param>
    void addDeflector(const Deflector &deflector);

    /// <summary>
    /// Const ref getter for the deflectors
    /// </summary>
    /// <returns>A const reference to the vector of deflectors on this
    /// drawing.</returns>
    virtual const std::vector<Deflector> &deflectors() const;

    /// <summary>
    /// Ref getter for the deflectors
    /// </summary>
    /// <returns>A reference to the vector of deflectors on this
    /// drawing.</returns>
    std::vector<Deflector> &deflectors();

    /// <summary>
    /// Getter for an individual deflector
    /// </summary>
    /// <param name="index">The index to retrieve the deflector from</param>
    /// <returns>A reference to the deflector at the given index.</returns>
    virtual Deflector &deflector(unsigned index);

    /// <summary>
    /// Remover for a deflector from the drawing
    /// </summary>
    /// <param name="deflector">The deflector to remove.</param>
    void removeDeflector(const Deflector &deflector);

    /// <summary>
    /// Getter for the number of deflectors
    /// </summary>
    /// <returns>The size of the deflectors vector.</returns>
    virtual unsigned numberOfDeflectors() const;

    /// <summary>
    /// Adds a divertor to the drawing
    /// </summary>
    /// <param name="divertor">The divertor to add to the drawing.</param>
    void addDivertor(const Divertor &divertor);

    /// <summary>
    /// Const ref getter for the divertors
    /// </summary>
    /// <returns>A const reference to the vector of divertors on this
    /// drawing.</returns>
    virtual std::vector<Divertor> &divertors();

    /// <summary>
    /// Ref getter for the divertors
    /// </summary>
    /// <returns>A reference to the vector of divertors on this drawing.</returns>
    const std::vector<Divertor> &divertors() const;

    /// <summary>
    /// Getter for an individual divertor
    /// </summary>
    /// <param name="index">The index to retrieve the divertor from</param>
    /// <returns>A reference to the divertor at the given index.</returns>
    virtual Divertor &divertor(unsigned index);

    /// <summary>
    /// Remover for a divertor from the drawing
    /// </summary>
    /// <param name="divertor">The divertor to remove.</param>
    void removeDivertor(const Divertor &divertor);

    /// <summary>
    /// Getter for the number of divertors
    /// </summary>
    /// <returns>The size of the divertors vector.</returns>
    virtual unsigned numberOfDivertors() const;

    /// <summary>
    /// Method to check whether a drawing is valid
    /// </summary>
    /// <param name="exclusions">A set of flags specifying if any checks should be
    /// ignored.</param> <returns>The first encountered build warning when
    /// checking the drawing for errors.</returns>
    BuildWarning checkDrawingValidity(unsigned exclusions = 0) const;

    /// <summary>
    /// Getter for a specific load warning
    /// </summary>
    /// <param name="warning">The warning to check for.</param>
    /// <returns>Whether the drawing has the given load warning set or
    /// not.</returns>
    bool loadWarning(LoadWarning warning) const;

    /// <summary>
    /// Setter for a specific load warning
    /// </summary>
    /// <param name="warning">The load warning to set on the drawing.</param>
    void setLoadWarning(LoadWarning warning);

    /// <summary>
    /// Method to add an update callback. The set of update callbacks will be
    /// called each time a change is made to the drawing.
    /// </summary>
    /// <param name="callback">The callback function to invoke whenever an update
    /// is made.</param>
    void addUpdateCallback(const std::function<void()> &callback);

  private:
    // Helper function to call each update callback
    void invokeUpdateCallbacks() const;

    // The regular expression pattern for checking a drawing number is valid.
    // The expression checks that either a drawing is a valid automatic drawing
    // number, namely one or two letters followed by exactly two numbers followed
    // optionally by a revision letter. or a manual drawing number, namely the
    // letter M followed by 3 or more numbers followed optionally by a revision
    // letter.
    static constexpr char drawingNumberRegexPattern[] =
        "^([a-zA-Z]{1,2}[0-9]{2}[a-zA-Z]?|M[0-9]{3,}[a-zA-Z]?)$";
    // The regular expression pattern for the machine position. The expression
    // checks that either the string is empty, or the word "all" or a single
    // number or a pair of numbers separated by a dash (e.g. 1-5)
    static constexpr char positionRegexPattern[] =
        "(^$)|(^[0-9]+([-][0-9]+)?$)|(^[Aa][Ll]{2}$)";

    // The following fields are all parts of the drawing specified in the
    // database. Each field is private and accessed through the public interfacing
    // methods.
    std::string __drawingNumber;
    Date __date;
    float __width, __length;
    SideIronType __sideIronType = SideIronType::None;
    std::filesystem::path __hyperlink;
    std::string __notes;
    MachineTemplate __machineTemplate;

    unsigned productHandle;
    unsigned apertureHandle;
    std::optional<unsigned> backingStripHandle;

    TensionType __tensionType;

    bool __rebated;

    std::vector<std::filesystem::path> __pressDrawingHyperlinks;

    std::vector<float> barSpacings;
    std::vector<float> barWidths;

  public:
    /// <summary>
    /// The handles for both the side irons.
    /// </summary>
    unsigned sideIronHandles[2];

  private:
    bool sideIronsInverted[2];
    bool sideIronsCutDown[2];
    std::optional<Side> feedEnd;
    std::optional<Ending> ending[2];
    std::optional<HookOrientation> hookOrientation[2];

    std::optional<Lap> sidelaps[2], overlaps[2];

    unsigned topLayerThicknessHandle;
    std::optional<unsigned> bottomLayerThicknessHandle;
    std::optional<unsigned> strapHandle[2];

    std::vector<ImpactPad> __impactPads;
    std::vector<BlankSpace> __blankSpaces;
    std::vector<ExtraAperture> __extraApertures;
    std::vector<DamBar> __damBars;
    std::vector<CentreHole> __centreHoles;
    std::vector<Deflector> __deflectors;
    std::vector<Divertor> __divertors;

    // A flag based value containing any load warnings found. This is incorporated
    // in the class so it is serialised when a drawing is sent. This allows the
    // receiver to check for any exceptions when loading a drawing.
    unsigned loadWarnings = 0;

    // A set of update callbacks. Each time a change is made, each callback is
    // invoked
    std::vector<std::function<void()>> updateCallbacks;
};

/// <summary>
/// DrawingSerialiser
/// Assistant class for serialising drawings to data streams without the
/// serialisation interface directly in the Drawing class.
/// </summary>
struct DrawingSerialiser {
    /// <summary>
    /// Serialises the given drawing to the given data buffer
    /// </summary>
    /// <param name="drawing">The drawing to write out to the buffer.</param>
    /// <param name="target">The byte buffer to write the drawing to.</param>
    static void serialise(const Drawing &drawing, void *target);

    /// <summary>
    /// Getter for the size this drawing will occupy in a buffer
    /// </summary>
    /// <param name="drawing">The drawing to calculate the serialised size
    /// for.</param> <returns></returns>
    static unsigned serialisedSize(const Drawing &drawing);

    /// <summary>
    /// Deserialises a drawing from the given data stream
    /// </summary>
    /// <param name="data">The data stream to deserialise the drawing
    /// from.</param> <returns>A newly constructed drawing object created from the
    /// buffer.</returns>
    static Drawing &deserialise(void *data);
};

struct DrawingSummaryCompressionSchema;

/// <summary>
/// DrawingSummary
/// Summary data representing an incomplete set of information about a drawing.
/// The idea is to have a small amount of information summarising the drawing
/// transferred in a search query in order to minimise the network transfer
/// required. The user can then choose to open a drawing in full, which is
/// essentially requesting the remaining information from the server.
/// </summary>
struct DrawingSummary {
    // Friend the compression schema class so it can access private properties
    friend struct DrawingSummaryCompressionSchema;

    /// <summary>
    /// The mat ID of the drawing as stored in the database
    /// </summary>
    unsigned matID;
    /// <summary>
    /// The array of one or two thickness handles for the bottom and top layers
    /// </summary>
    unsigned thicknessHandles[2];
    /// <summary>
    /// The handle for the aperture this drawing uses
    /// </summary>
    unsigned apertureHandle;
    /// <summary>
    /// The drawing number of this drawing
    /// </summary>
    std::string drawingNumber;

    /// <summary>
    /// Helper function to return whether or not the drawing has two materials
    /// or just one
    /// </summary>
    /// <returns></returns>
    bool hasTwoLayers() const;

    /// <summary>
    /// Helper function to return the number of laps the mat has
    /// </summary>
    /// <returns></returns>
    unsigned numberOfLaps() const;

    /// <summary>
    /// Getter for a summarising string for this drawing
    /// </summary>
    /// <returns>A string summary for the basic properties of this
    /// drawing.</returns>
    std::string summaryString() const;

    /// <summary>
    /// Getter for the width of this drawing. This is used because the internal
    /// representation stores the width as an unsigned.
    /// </summary>
    /// <returns>The corrected width for this drawing.</returns>
    float width() const;

    /// <summary>
    /// Getter for the length of this drawing. This is used because the internal
    /// representation stores the length as an unsigned.
    /// </summary>
    /// <returns>The corrected length for this drawing.</returns>
    float length() const;

    /// <summary>
    /// Setter for the width of this drawing. This is used because the internal
    /// representation stores the width as an unsigned.
    /// </summary>
    /// <param name="width">The true width for this drawing.</param>
    void setWidth(float width);

    /// <summary>
    /// Setter for the length of this drawing. This is used because the internal
    /// representation stores the length as an unsigned.
    /// </summary>
    /// <param name="length">The true length for this drawing.</param>
    void setLength(float length);

    /// <summary>
    /// Getter for the indexed lap size. This is used because the internal
    /// representation stores the lap sizes as unsigned values
    /// </summary>
    /// <param name="index">The index of the lap.</param>
    /// <returns>The corrected lap size.</returns>
    float lapSize(unsigned index) const;

    /// <summary>
    /// Setter for the indexed lap size. This is used because the internal
    /// representation stores the lap sizes as unsigned values
    /// </summary>
    /// <param name="index">The index of the lap.</param>
    /// <param name="size">The true lap size.</param>
    void setLapSize(unsigned index, float size);

    /// <summary>
    /// Getter for the bar spacings vector. This is used because the internal
    /// representation stores the bar spacings as unsigned values
    /// </summary>
    /// <returns>The corrected bar spacings.</returns>
    std::vector<float> barSpacings() const;

    /// <summary>
    /// Method to add another bar spacing to the set of bar spacings. This is used
    /// because the internal representation stores the bar spacings as unsigned
    /// values
    /// </summary>
    /// <param name="spacing">The true spacing value to add.</param>
    void addSpacing(float spacing);

    /// <summary>
    /// Clears the bar spacing set to an empty vector
    /// </summary>
    void clearSpacings();

    /// <summary>
    /// Helper function to return the number of bar spacings stored
    /// </summary>
    /// <returns>The number of bar spacings for this drawing.</returns>
    unsigned barSpacingCount() const;

    /// <summary>
    /// Adds a new extra aperture to the summary by passing its handle.
    /// </summary>
    /// <param name="apertureHandle">The handle of the extra aperture to
    /// add.</param>
    void addExtraAperture(unsigned apertureHandle);

    /// <summary>
    /// Gets a vector of all extra apertures in this summary.
    /// </summary>
    /// <returns>A vector of all the extra apertures' handles.</returns>
    std::vector<unsigned> extraApertures() const;

    /// <summary>
    /// Removes all extra apertures from this summary.
    /// </summary>
    void clearExtraApertures();

    /// <summary>
    /// Gets the amount of extra apertures in this summary.
    /// </summary>
    /// <returns>The amount of extra apertures.</returns>
    unsigned extraApertureCount() const;

  private:
    // The internal representation of theses fields all store TWICE the true
    // value. This allows the "true" values to be either integers or integers plus
    // a half (e.g. 1200.5). Unsigned values are essentially used as "fixed" point
    // numbers, instead of "floating" point numbers. This allows the compression
    // to function correctly on these values, as the method does not work for true
    // floating point number compression (due to normalisation etc)
    unsigned __width, __length;
    unsigned __lapSizes[4];
    std::vector<unsigned> __barSpacings;
    std::vector<unsigned> __extraApertures;
};

PACK_START
/// <summary>
/// DrawingSummaryCompressionSchema
/// Represents compression metadata used to compress and uncompress a drawing or
/// set of drawings based on the maximum values it would need to transmit to
/// represent any given field. For example, the mat_id field will be represented
/// by the minimum number of bits required to represent the largest mat_id in
/// the database. Any more significant bits are guaranteed to be 0s, and so if
/// we essentially communicate that all bits past a point of significance are
/// 0s, we can omit those places in the compressed data.
/// </summary>
struct DrawingSummaryCompressionSchema {
    /// <summary>
    /// Constructor specifying the schema
    /// </summary>
    /// <param name="maxMatID">The largest mat_id from the database.</param>
    /// <param name="maxWidth">The largest drawing width from the
    /// database.</param> <param name="maxLength">The largest drawing length from
    /// the database.</param> <param name="maxThicknessHandle">The largest
    /// thickness handle from the internal representaion of the thickness
    /// components.</param> <param name="maxLapSize">The largest overlap or
    /// sidelap width from the database.</param> <param
    /// name="maxApertureHandle">The largest aperture handle from the internal
    /// representaion of the aperture components.</param> <param
    /// name="maxBarSpacingCount">The most bar spacings on any drawing.</param>
    /// <param name="maxBarSpacing">The largest bar spacing on any drawing from
    /// the database.</param> <param name="maxDrawingLength">The length of the
    /// longest drawing number in the databae.</param> <param
    /// name="maxExtraApertureCount">The highest amount of extra apertures any mat
    /// has.</param>
    DrawingSummaryCompressionSchema(unsigned maxMatID, float maxWidth,
                                    float maxLength, unsigned maxThicknessHandle,
                                    float maxLapSize, unsigned maxApertureHandle,
                                    unsigned char maxBarSpacingCount,
                                    float maxBarSpacing,
                                    unsigned char maxDrawingLength,
                                    unsigned char maxExtraApertureCount);

    /// <summary>
    /// Calculates the compressed size a given summary will occupy (in bits) when
    /// compressed
    /// </summary>
    /// <param name="summary">The summary to calculate the compressed size
    /// for.</param> <returns>The compressed size (in bits) of the target
    /// summary.</returns>
    unsigned compressedSize(const DrawingSummary &summary) const;

    /// <summary>
    /// Compresses a given summary and writes the compressed data to the target
    /// buffer
    /// </summary>
    /// <param name="summary">The summary to compress.</param>
    /// <param name="target">The target buffer to write the compressed data
    /// to.</param>
    void compressSummary(const DrawingSummary &summary, void *target) const;

    /// <summary>
    /// Uncompresses a given summary from the data buffer and increments the size
    /// reference by the amount of bytes the summary occupied
    /// </summary>
    /// <param name="data">The data buffer to decompress the summary from.</param>
    /// <param name="size">A reference to the size variable to write the number of
    /// bytes read.</param> <returns>A decompressed DrawingSummary object
    /// reconstructed from the data buffer.</returns>
    DrawingSummary uncompressSummary(void *data, unsigned &size) const;

    /// <summary>
    /// Calculates the maximum possible size a compressed any summary could
    /// occupy, based on the compression schema metadata.
    /// </summary>
    /// <returns>The maximum compressed size.</returns>
    unsigned maxCompressedSize() const;

  private:
    // These values represent the number of bits required to
    // cover each of the compressed fields
    unsigned char matIDSize;
    unsigned char widthSize;
    unsigned char lengthSize;
    unsigned char thicknessHandleSize;
    unsigned char lapSize;
    unsigned char apertureHandleSize;
    unsigned char maxDrawingLength;
    unsigned char barSpacingCountSize;
    unsigned char barSpacingSize;
    unsigned char extraApertureCountSize;

    // The maximum number of bar spacings from any drawing
    unsigned char maxBarSpacingCount;
    unsigned char maxExtraApertureCount;

    // These values represent the minimum number of covering bytes for each
    // of the above fields in bits. For example, 15 bits can be covered in 2 bytes
    // but 17 bits needs at least 3 bytes. These values are simply stored to avoid
    // recalculating them each time a drawing is compressed.
    unsigned char matIDBytes;
    unsigned char widthBytes;
    unsigned char lengthBytes;
    unsigned char thicknessHandleBytes;
    unsigned char lapBytes;
    unsigned char apertureHandleBytes;
    unsigned char barSpacingCountBytes;
    unsigned char barSpacingBytes;
    unsigned char extraApertureCountBytes;
}
///@cond IGNORE
PACK_END
///@endcond

#endif // DATABASE_MANAGER_DRAWING_H
