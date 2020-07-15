//
// Created by matthew on 10/07/2020.
//

#ifndef DATABASE_MANAGER_DRAWINGCOMPONENTS_H
#define DATABASE_MANAGER_DRAWINGCOMPONENTS_H

#include <cstdio>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "RequestType.h"
#include "../networking/NetworkMessage.h"
#include "ComboboxDataSource.h"

#include "../../guard.h"

struct DrawingComponent {
    virtual void serialise(void *buffer) const;

    static unsigned deserialiseID(void *buffer);

    constexpr size_t serialisedSize() const { return sizeof(unsigned); };

    virtual constexpr ComboboxDataElement toDataElement() const = 0;

    unsigned componentID;

protected:
    DrawingComponent(unsigned id);
};

template<typename T>
class DrawingComponentManager;

struct Product : public DrawingComponent {
    friend class DrawingComponentManager<Product>;

public:
    std::string productName;

    ComboboxDataElement toDataElement() const override;

private:
    Product(unsigned id);

    static Product *fromSource(void *buffer, unsigned &elementSize);
};

struct Aperture : public DrawingComponent {
    friend class DrawingComponentManager<Aperture>;

public:
    unsigned short width{}, length{};
    unsigned short baseWidth{}, baseLength{};
    unsigned apertureShapeID{};
    unsigned short quantity{};

    std::string apertureName() const;

    ComboboxDataElement toDataElement() const override;

private:
    Aperture(unsigned id);

    static Aperture *fromSource(void *buffer, unsigned &elementSize);
};

struct ApertureShape : public DrawingComponent {
    friend class DrawingComponentManager<ApertureShape>;

public:
    std::string shape;

    ComboboxDataElement toDataElement() const override;

private:
    ApertureShape(unsigned id);

    static ApertureShape *fromSource(void *buffer, unsigned &elementSize);
};

struct Material : public DrawingComponent {
    friend class DrawingComponentManager<Material>;

public:
    std::string materialName;
    unsigned short hardness{}, thickness{};

    ComboboxDataElement toDataElement() const override;

private:
    Material(unsigned id);

    static Material *fromSource(void *buffer, unsigned &elementSize);
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

    ComboboxDataElement toDataElement() const override;

private:
    SideIron(unsigned id);

    static SideIron *fromSource(void *buffer, unsigned &elementSize);
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

    ComboboxDataElement toDataElement() const override;

private:
    Machine(unsigned id);

    static Machine *fromSource(void *buffer, unsigned &elementSize);
};

struct MachineDeck : public DrawingComponent {
    friend class DrawingComponentManager<MachineDeck>;

public:
    std::string deck;

    ComboboxDataElement toDataElement() const override;

private:
    MachineDeck(unsigned id);

    static MachineDeck *fromSource(void *buffer, unsigned &elementSize);
};

template<typename T>
class ComboboxComponentDataSource : public ComboboxDataSource {
public:
    ComboboxComponentDataSource();

    void updateSource() override;

private:
    void setAdapter(const std::function<ComboboxDataElement(std::vector<unsigned>::const_iterator)> &adapter) override {}

    std::vector<unsigned> indexSet;
};

template<typename T>
ComboboxComponentDataSource<T>::ComboboxComponentDataSource() {
    ComboboxDataSource::setAdapter([](std::vector<unsigned>::const_iterator iter) {
        return DrawingComponentManager<T>::getComponentByID(*iter).toDataElement();
    });
}

template<typename T>
void ComboboxComponentDataSource<T>::updateSource() {
    indexSet = DrawingComponentManager<T>::dataIndexSet();
    this->__begin = indexSet.begin();
    this->__end = indexSet.end();

    DataSource::updateSource();
}

template<typename T>
class DrawingComponentManager {
public:
    static void sourceComponentTable(void *data, unsigned dataSize);

    static bool dirty();

    static void setDirty();

    static T &getComponentByID(unsigned id);

    static void *rawSourceData();

    static unsigned rawSourceDataSize();

    static std::vector<unsigned> dataIndexSet();

    static void addCallback(const std::function<void()> &callback);

private:
    static std::unordered_map<unsigned, T *> componentLookup;
    static std::vector<unsigned> indexSet;

    static bool sourceDirty;

    static void *sourceData;
    static unsigned sourceDataSize;

    static std::vector<std::function<void()>> updateCallbacks;
};

template<typename T>
std::unordered_map<unsigned, T *> DrawingComponentManager<T>::componentLookup;

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

    componentLookup[0] = new T(0);

    unsigned char *buff = (unsigned char *) data;

    RequestType type = *((RequestType *) buff);
    buff += sizeof(RequestType);

    unsigned elements = *((unsigned *) buff);
    buff += sizeof(unsigned);

    for (unsigned i = 0; i < elements; i++) {
        unsigned elementSize;
        T *element = T::fromSource(buff, elementSize);
        componentLookup[element->componentID] = element;
        indexSet.push_back(element->componentID);

        buff += elementSize;
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
T &DrawingComponentManager<T>::getComponentByID(unsigned int id) {
    if (componentLookup.find(id) == componentLookup.end()) {
        ERROR_RAW("Invalid component lookup ID.")
    }

    return *componentLookup[id];
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
