//
// Created by matthew on 10/07/2020.
//

#ifndef DATABASE_MANAGER_DRAWINGCOMPONENTS_H
#define DATABASE_MANAGER_DRAWINGCOMPONENTS_H

#include <cstdio>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <set>

#include "RequestType.h"
#include "../networking/NetworkMessage.h"
#include "ComboboxDataSource.h"

#include "../../guard.h"

/// <summary>
/// DrawingComponent
/// Base class for each type of drawing component that is associated with an entire
/// drawing. The idea is that these components can be used on many drawings, and so
/// can be synchronised independently and then referenced through internal handles.
/// </summary>
struct DrawingComponent {
    /// <summary>
    /// Serialises this component into the provided buffer. The default implementation
    /// simply writes the handle into the buffer, as this should be synchronised with the
    /// server's handle for the same object, and as such is the minimum amount of information
    /// needed to transmit this component
    /// </summary>
    /// <param name="buffer">The buffer to serialise into.</param>
    virtual void serialise(void *buffer) const;

    /// <summary>
    /// Deserialises a component handle from the provided buffer. At this level,
    /// it is not known what sort of component is being deserialised, as there is no
    /// static method polymorphism. So, the method just returns the handle and the typing
    /// is left to a higher level's responsibility
    /// </summary>
    /// <param name="buffer">The buffer to deserialise from.</param>
    /// <returns>The handle for the deserialised component.</returns>
    static unsigned deserialise(void *buffer);

    /// <summary>
    /// Getter for the size this component will take in a buffer. This is the size
    /// of an unsigned representing the handle by default
    /// </summary>
    /// <returns>The size this component will occupy in a data buffer.</returns>
    constexpr size_t serialisedSize() const { return sizeof(unsigned); };

    /// <summary>
    /// Virtual method for converting a component into a ComboboxDataElement used for displaying
    /// these components in data sourced comboboxes
    /// </summary>
    /// <param name="mode">An optional mode parameter which can be used by the overriding
    /// methods to particularise the mapping to a data element.</param>
    /// <returns>A ComboboxDataElement object containing the desired information for representing
    /// a component in a combobox.</returns>
    virtual ComboboxDataElement toDataElement(unsigned mode = 0) const = 0;

    /// <summary>
    /// Getter for the internal component ID as stored in the database. Note that this
    /// is not necessarily unique
    /// </summary>
    /// <returns>The component ID associated with this entity in the database.</returns>
    unsigned componentID() const;

    /// <summary>
    /// Getter for the internal component handle used in the application. Note that this
    /// IS necessarily unique and there is no necessary bijection from handles to components
    /// </summary>
    /// <returns>The handle assocaited with this entity in the internal representation.</returns>
    unsigned handle() const;

protected:
    /// <summary>
    /// Internal constructor based on the component ID for this entity. This is hidden
    /// as it should not be possible to create components which do not actually exist
    /// </summary>
    /// <param name="id">The component ID as in the database.</param>
    DrawingComponent(unsigned id);

    // Hidden values for the component ID and handle for this component.
    unsigned __componentID, __handle;
};

template<typename T>
class DrawingComponentManager;

/// <summary>
/// Product
/// Represents a product type that a drawing may be of.
/// </summary>
struct Product : public DrawingComponent {
    // Friend the DrawingComponentManager associated with this type such that it may access the internal
    // values
    friend class DrawingComponentManager<Product>;

public:
    // A product has a product name, e.g. "Rubber Screen Cloth"
    std::string productName;

    /// <summary>
    /// Overridden method to adapt this Product into a ComboboxDataElement
    /// </summary>
    /// <param name="mode">Optional mode parameter to specify which adapter method to use.
    /// For products, this value is unusued.</param>
    /// <returns>A ComboboxDataElement summary for this Product.</returns>
    ComboboxDataElement toDataElement(unsigned mode = 0) const override;

private:
    /// <summary>
    /// Private constructor based on the component ID
    /// </summary>
    /// <param name="id">The component ID for this product as in the database.</param>
    Product(unsigned id);

    /// <summary>
    /// Static deserialiser to convert a raw data buffer into a product element
    /// </summary>
    /// <param name="buffer">The buffer to deserialise from.</param>
    /// <param name="elementSize">A reference to an element size variable which will be set to
    /// the number of bytes used to represent the product in the buffer.</param>
    /// <returns></returns>
    static Product * fromSource(unsigned char** buff);
};

struct BackingStrip : public DrawingComponent {
    friend class DrawingComponentManager<BackingStrip>;

public:
    unsigned materialID{};

    std::string backingStripName() const;

    ComboboxDataElement toDataElement(unsigned mode = 0) const override;

private:
    BackingStrip(unsigned id);

    static BackingStrip* fromSource(unsigned char** buff);
};

/// <summary>
/// Aperture
/// Represents an aperture tool
/// </summary>
struct Aperture : public DrawingComponent {
    friend class DrawingComponentManager<Aperture>;

public:
    float width{}, length{};
    unsigned short baseWidth{}, baseLength{};
    unsigned apertureShapeID{};
    unsigned short quantity{};
    bool isNibble;
    unsigned nibbleApertureId;

    std::string apertureName() const;

    ComboboxDataElement toDataElement(unsigned mode = 0) const override;

private:
    Aperture(unsigned id);

    static Aperture * fromSource(unsigned char** buff);
};

struct ApertureShape : public DrawingComponent {
    friend class DrawingComponentManager<ApertureShape>;

public:
    std::string shape;

    ComboboxDataElement toDataElement(unsigned mode = 0) const override;

private:
    ApertureShape(unsigned id);

    static ApertureShape* fromSource(unsigned char** buff);
};

struct Material : public DrawingComponent {
    friend class DrawingComponentManager<Material>;

public:
    std::string materialName;
    unsigned short hardness{}, thickness{};

    // width, length if exists, price, type
    std::vector<std::tuple<float, float, float, MaterialPricingType>> materialPrices;

    std::string material() const;

    ComboboxDataElement toDataElement(unsigned mode = 0) const override;

private:
    Material(unsigned id);

    static Material* fromSource(unsigned char** buff);
};

template<ExtraPriceType T>
struct ExtraPriceTrait {

};

template<>
struct ExtraPriceTrait<ExtraPriceType::SIDE_IRON_NUTS> {
    using numType = unsigned;
};

template<>
struct ExtraPriceTrait<ExtraPriceType::SIDE_IRON_SCREWS> {
    using numType = unsigned;
};

template<>
struct ExtraPriceTrait<ExtraPriceType::TACKYBACK_GLUE> {
    using numType = float;
};

template<>
struct ExtraPriceTrait<ExtraPriceType::LABOUR> {
    using numType = float;
};

template<>
struct ExtraPriceTrait<ExtraPriceType::PRIMER> {
    using numType = float;
};

template<>
struct ExtraPriceTrait<ExtraPriceType::DIVERTOR> {
    using numType = unsigned;
};

template<>
struct ExtraPriceTrait<ExtraPriceType::DEFLECTOR> {
    using numType = unsigned;
};

template<>
struct ExtraPriceTrait<ExtraPriceType::DAM_BAR> {
    using numType = unsigned;
};

struct ExtraPrice : public DrawingComponent {
    friend class DrawingComponentManager<ExtraPrice>;

public:
    ExtraPriceType type;
    float price;
    std::optional<float> squareMetres;
    std::optional<unsigned> amount;

    std::string extraPrice() const;

    ComboboxDataElement toDataElement(unsigned mode = 0) const override;

    template<ExtraPriceType T>
    float getPrice(typename ExtraPriceTrait<T>::numType n);
    //float getPrice(float n);

private:
    ExtraPrice(unsigned id);

    static ExtraPrice* fromSource(unsigned char** buff);
};


struct LabourTime : public DrawingComponent {
    friend class DrawingComponentManager<LabourTime>;

public:
    LabourTimeType type;
    unsigned time;

    std::string labourTime() const;
    ComboboxDataElement toDataElement(unsigned mode = 0) const override;
private:
    LabourTime(unsigned id);

    static LabourTime* fromSource(unsigned char** buff);
};

enum class SideIronType {
    None,
    A,
    B,
    C,
    D,
    E
};

struct SideIron : public DrawingComponent {
    friend class DrawingComponentManager<SideIron>;

public:
    SideIronType type = SideIronType::A;
    unsigned short length{};
    std::string drawingNumber;
    std::string hyperlink;

    std::string sideIronStr() const;

    ComboboxDataElement toDataElement(unsigned mode = 0) const override;

private:
    SideIron(unsigned id);

    static SideIron* fromSource(unsigned char** buff);
};

struct SideIronPrice : DrawingComponent {
    friend class DrawingComponentManager<SideIronPrice>;

    SideIronType type;
    std::vector<std::tuple<unsigned, float, float, unsigned, bool>> prices;

    std::string sideIronPriceStr() const;

    ComboboxDataElement toDataElement(unsigned mode = 0) const override;

private:
    SideIronPrice(unsigned id);

    static SideIronPrice* fromSource(unsigned char** buff);
};

enum class LapSetting {
    HAS_NONE,
    HAS_ONE,
    HAS_BOTH
};

enum class LapAttachment {
    INTEGRAL,
    BONDED
};

struct Machine : public DrawingComponent {
    friend class DrawingComponentManager<Machine>;

public:
    std::string manufacturer, model;

    std::string machineName() const;

    ComboboxDataElement toDataElement(unsigned mode = 0) const override;

private:
    Machine(unsigned id);

    static Machine* fromSource(unsigned char** buff);
};

struct MachineDeck : public DrawingComponent {
    friend class DrawingComponentManager<MachineDeck>;

public:
    std::string deck;

    ComboboxDataElement toDataElement(unsigned mode = 0) const override;

private:
    MachineDeck(unsigned id);

    static MachineDeck* fromSource(unsigned char** buff);
};

template<typename T>
class ComboboxComponentDataSource : public ComboboxDataSource {
public:
    ComboboxComponentDataSource();

    void updateSource() override;

    void sort(const std::function<bool(const T &, const T &)> &comparator);

    void makeDistinct();

    void setMode(unsigned mode);

    bool empty() const;
private:
    void setAdapter(const std::function<ComboboxDataElement(std::vector<unsigned>::const_iterator)> &adapter) override {}

    std::vector<unsigned> handleSet;

    unsigned elementMode = 0;
};

template<typename T>
ComboboxComponentDataSource<T>::ComboboxComponentDataSource() {
    ComboboxDataSource::setAdapter([](std::vector<unsigned>::const_iterator iter) {
        return DrawingComponentManager<T>::getComponentByHandle(*iter).toDataElement();
    });
}

template<typename T>
void ComboboxComponentDataSource<T>::updateSource() {
    handleSet = DrawingComponentManager<T>::dataIndexSet();
    this->__begin = handleSet.begin();
    this->__end = handleSet.end();

    DataSource::updateSource();
}

template<typename T>
void ComboboxComponentDataSource<T>::sort(const std::function<bool(const T &, const T &)> &comparator) {
    std::sort(handleSet.begin(), handleSet.end(), [comparator](unsigned a, unsigned b) {
        return comparator(DrawingComponentManager<T>::getComponentByHandle(a),
                          DrawingComponentManager<T>::getComponentByHandle(b));
    });
}

template<typename T>
void ComboboxComponentDataSource<T>::makeDistinct() {
    std::vector<unsigned>::const_iterator end = std::unique(handleSet.begin(), handleSet.end(), [this](unsigned a, unsigned b) {
        return DrawingComponentManager<T>::getComponentByHandle(a).toDataElement(elementMode).text ==
            DrawingComponentManager<T>::getComponentByHandle(b).toDataElement(elementMode).text;
    });
    handleSet.erase(end, handleSet.end());
    this->__end = handleSet.end();
}

template<typename T>
void ComboboxComponentDataSource<T>::setMode(unsigned mode) {
    ComboboxDataSource::setAdapter([mode](std::vector<unsigned>::const_iterator iter) {
        return DrawingComponentManager<T>::getComponentByHandle(*iter).toDataElement(mode);
    });
    elementMode = mode;
}

template<typename T>
bool ComboboxComponentDataSource<T>::empty() const {
    return handleSet.empty();
}
template<typename T>
class DrawingComponentManager {
    static_assert(std::is_base_of<DrawingComponent, T>::value, "DrawingComponentManager can only be used with types deriving from DrawingComponent.");
public:
    static void sourceComponentTable(void *data, unsigned dataSize);

    static bool dirty();

    static void setDirty();

    static void refreshTable();

    static T &getComponentByHandle(unsigned handle);

    static unsigned maximumHandle();

    static T& findComponentByID(unsigned id);

    static std::vector<T *> allComponentsByID(unsigned id);

    static bool validComponentID(unsigned id);

    static bool validComponentHandle(unsigned handle);

    static void *rawSourceData();

    static unsigned rawSourceDataSize();

    static std::vector<unsigned> dataIndexSet();

    static void addCallback(const std::function<void()> &callback);

private:
    static std::unordered_map<unsigned, T *> componentLookup;
    static std::unordered_map<unsigned, unsigned> handleToIDMap;
    static std::vector<unsigned> indexSet;

    static bool sourceDirty;

    static void *sourceData;
    static unsigned sourceDataSize;

    static std::vector<std::function<void()>> updateCallbacks;
};

template<typename T>
std::unordered_map<unsigned, T *> DrawingComponentManager<T>::componentLookup;

template<typename T>
std::unordered_map<unsigned, unsigned> DrawingComponentManager<T>::handleToIDMap;

template<typename T>
std::vector<unsigned> DrawingComponentManager<T>::indexSet;

template<typename T>
bool DrawingComponentManager<T>::sourceDirty = true;

template<typename T>
void *DrawingComponentManager<T>::sourceData = nullptr;

template<typename T>
unsigned DrawingComponentManager<T>::sourceDataSize = 0;

template<typename T>
std::vector<std::function<void()>> DrawingComponentManager<T>::updateCallbacks;

template<typename T>
void DrawingComponentManager<T>::sourceComponentTable(void *data, unsigned dataSize) {
    for (std::pair<unsigned, T *> component : componentLookup) {
        delete component.second;
    }
    componentLookup.clear();
    indexSet.clear();
    handleToIDMap.clear();

    componentLookup[0] = new T(0);
    componentLookup[0]->__handle = 0;

    unsigned char *buff = (unsigned char *) data;

    RequestType type = *((RequestType *) buff);
    buff += sizeof(RequestType);


    unsigned elements = *((unsigned *) buff);
    buff += sizeof(unsigned);

    for (unsigned i = 0; i < elements; i++) {
        unsigned handle = *((unsigned *)buff);
        buff += sizeof(unsigned);

        T *element = T::fromSource(&buff);
        element->__handle = handle;
        componentLookup[handle] = element;
        handleToIDMap[handle] = element->__componentID;
        indexSet.push_back(handle);
    }

    free(sourceData);
    sourceData = data;
    sourceDataSize = dataSize;
    sourceDirty = false;

    for (const std::function<void()> &callback : updateCallbacks) {
        callback();
    }
}

template<typename T>
bool DrawingComponentManager<T>::dirty() {
    return sourceDirty;
}

template<typename T>
void DrawingComponentManager<T>::setDirty() {
    sourceDirty = true;
}

template<typename T>
void DrawingComponentManager<T>::refreshTable() {

}

template<typename T>
T &DrawingComponentManager<T>::getComponentByHandle(unsigned handle) {
    if (!validComponentHandle(handle)) {
        ERROR_RAW("Invalid component lookup handle.", std::cerr)
    }

    return *componentLookup[handle];
}

template<typename T>
unsigned DrawingComponentManager<T>::maximumHandle() {
    return *std::max_element(indexSet.begin(), indexSet.end());
}

template<typename T>
T &DrawingComponentManager<T>::findComponentByID(unsigned id) {
    for (const std::pair<unsigned, unsigned> handleMap : handleToIDMap) {
        if (handleMap.second == id) {
            return *componentLookup[handleMap.first];
        }
    }
        ERROR_RAW("Component was not found. (" + std::to_string(id) + ")", std::cerr)
}

template<typename T>
std::vector<T *> DrawingComponentManager<T>::allComponentsByID(unsigned id) {
    std::vector<T *> components;
    for (const std::pair<unsigned, unsigned> handleMap : handleToIDMap) {
        if (handleMap.second == id) {
            components.push_back(componentLookup[handleMap.first]);
        }
    }
    return components;
}

template<typename T>
bool DrawingComponentManager<T>::validComponentID(unsigned int id) {
    for (const std::pair<unsigned, T *> component : componentLookup) {
        if (component.second->componentID() == id) {
            return true;
        }
    }
    return false;
}

template<typename T>
bool DrawingComponentManager<T>::validComponentHandle(unsigned int id) {
    return componentLookup.find(id) != componentLookup.end();
}

template<typename T>
void *DrawingComponentManager<T>::rawSourceData() {
    return sourceData;
}

template<typename T>
unsigned DrawingComponentManager<T>::rawSourceDataSize() {
    return sourceDataSize;
}

template<typename T>
std::vector<unsigned> DrawingComponentManager<T>::dataIndexSet() {
    return indexSet;
}

template<typename T>
void DrawingComponentManager<T>::addCallback(const std::function<void()> &callback) {
    updateCallbacks.push_back(callback);
}


#endif //DATABASE_MANAGER_DRAWINGCOMPONENTS_H
