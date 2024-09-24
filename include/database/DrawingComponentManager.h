#ifndef DATABASE_DRAWINGCOMPONENTMANAGER_H
#define DATABASE_DRAWINGCOMPONENTMANAGER_H

#include "ExtraPriceManager.h"
#include "drawingComponents.h"

/// @private
template <typename T>
void DrawingComponentManager<T>::addComponent(T *component) {
  component->__handle = DrawingComponentManager<T>::maximumHandle() + 1;
  DrawingComponentManager<T>::componentLookup.emplace(component->__handle,
                                                      component);
  DrawingComponentManager<T>::handleToIDMap.emplace(component->__handle,
                                                    component->__componentID);
  DrawingComponentManager<T>::indexSet.push_back(component->__handle);
};

/// @private
template <typename T>
void DrawingComponentManager<T>::clear() {
  DrawingComponentManager<T>::componentLookup.clear();
  DrawingComponentManager<T>::handleToIDMap.clear();
  DrawingComponentManager<T>::indexSet.clear();
}

template <typename T>
std::unordered_map<unsigned, T *> DrawingComponentManager<T>::componentLookup;

template <typename T>
std::unordered_map<unsigned, unsigned>
    DrawingComponentManager<T>::handleToIDMap;

template <typename T>
std::vector<unsigned> DrawingComponentManager<T>::indexSet;

template <typename T>
bool DrawingComponentManager<T>::sourceDirty = true;

template <typename T>
void *DrawingComponentManager<T>::sourceData = nullptr;

template <typename T>
unsigned DrawingComponentManager<T>::sourceDataSize = 0;

template <typename T>
std::vector<std::function<void()>> DrawingComponentManager<T>::updateCallbacks;

template <typename T>
void DrawingComponentManager<T>::sourceComponentTable(void *&&data,
                                                      unsigned dataSize) {
  for (std::pair<unsigned, T *> component : componentLookup) {
    delete component.second;
  }
  componentLookup.clear();
  indexSet.clear();
  handleToIDMap.clear();

  componentLookup[0] = new T(0);
  componentLookup[0]->__handle = 0;

  unsigned char *buff = (unsigned char *)data;

  RequestType type = *((RequestType *)buff);
  buff += sizeof(RequestType);

  unsigned elements = *((unsigned *)buff);
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

  sourceData = data;
  sourceDataSize = dataSize;
  sourceDirty = false;

  for (const std::function<void()> &callback : updateCallbacks) {
    callback();
  }
}

template <typename T>
bool DrawingComponentManager<T>::dirty() {
  return sourceDirty;
}

template <typename T>
void DrawingComponentManager<T>::setDirty() {
  sourceDirty = true;
}

template <typename T>
T &DrawingComponentManager<T>::getComponentByHandle(unsigned handle) {
  if (!validComponentHandle(handle)) {
    ERROR_RAW("Invalid component lookup handle.", std::cerr)
  }

  return *componentLookup[handle];
}

// returns the highest current handle
template <typename T>
unsigned DrawingComponentManager<T>::maximumHandle() {
  if (indexSet.empty()) return 0;
  return *std::max_element(indexSet.begin(), indexSet.end());
}

template <typename T>
T &DrawingComponentManager<T>::findComponentByID(unsigned id) {
  for (const std::pair<unsigned, unsigned> handleMap : handleToIDMap) {
    if (handleMap.second == id) {
      return *componentLookup[handleMap.first];
    }
  }
  ERROR_RAW("Component was not found. (" + std::string(typeid(T).name()) +
                ": " + std::to_string(id) + ")",
            std::cerr)
}

template <typename T>
std::vector<T *> DrawingComponentManager<T>::allComponentsByID(unsigned id) {
  std::vector<T *> components;
  for (const std::pair<unsigned, unsigned> handleMap : handleToIDMap) {
    if (handleMap.second == id) {
      components.push_back(componentLookup[handleMap.first]);
    }
  }
  return components;
}

template <typename T>
bool DrawingComponentManager<T>::validComponentID(unsigned int id) {
  for (const std::pair<unsigned, T *> component : componentLookup) {
    if (component.second->componentID() == id) {
      return true;
    }
  }
  return false;
}

template <typename T>
bool DrawingComponentManager<T>::validComponentHandle(unsigned int id) {
  return componentLookup.find(id) != componentLookup.end();
}

template <typename T>
void *DrawingComponentManager<T>::rawSourceData() {
  return sourceData;
}

template <typename T>
unsigned DrawingComponentManager<T>::rawSourceDataSize() {
  return sourceDataSize;
}

template <typename T>
std::vector<unsigned> DrawingComponentManager<T>::dataIndexSet() {
  return indexSet;
}

template <typename T>
void DrawingComponentManager<T>::addCallback(
    const std::function<void()> &callback) {
  updateCallbacks.push_back(callback);
}


/// @cond
template<>
struct ExtraPriceTrait<ExtraPriceType::SIDE_IRON_NUTS> {
  /// <summary>
  /// Sets side iron nuts to be measured using an unsigned integer, for
  /// quantity.
  /// </summary>
  using numType = unsigned;
  inline static std::optional<float> calc(ExtraPrice* p, unsigned n) {
      return n * (p->price / p->amount.value());
    return std::nullopt;
  };
};

/// <summary>
/// Sets side iron screws to be measured using a unsigned.
/// </summary>
template<>
struct ExtraPriceTrait<ExtraPriceType::SIDE_IRON_SCREWS> {
  /// <summary>
  /// Sets side iron screws to be measured using an unsigned integer, for
  /// quantity.
  /// </summary>
  using numType = unsigned;
  inline static std::optional<float> calc(ExtraPrice* p, unsigned n) {
      if (p != nullptr && p->amount.has_value())
      return n * (p->price / p->amount.value());
    return std::nullopt;
  }
};

/// <summary>
/// Sets tackyback glue to be measured using a float.
/// </summary>
template<>
struct ExtraPriceTrait<ExtraPriceType::TACKYBACK_GLUE> {
  /// <summary>
  /// Sets tackyback glue to be measured using a float, for area coverage.
  /// </summary>
  using numType = float;
  inline static std::optional<float> calc(ExtraPrice* p, float n) {
      if (p != nullptr && p->squareMetres.has_value())
        return n * (p->price / p->squareMetres.value());
    return std::nullopt;
  }
};

/// <summary>
/// Sets labour to be measured using a float.
/// </summary>
template<>
struct ExtraPriceTrait<ExtraPriceType::LABOUR> {
  /// <summary>
  /// Sets labour to be measured using a float, for fractional hours.
  /// </summary>
  using numType = float;
  inline static std::optional<float> calc(ExtraPrice* p, float n) {
      if (p != nullptr && p->squareMetres.has_value())
      return n * (p->price / 60);
    return std::nullopt;
  }
};

/// <summary>
/// Sets primer to be measured using a float.
/// </summary>
template<>
struct ExtraPriceTrait<ExtraPriceType::PRIMER> {
  /// <summary>
  /// Sets primer to be measured using a float, for area coverage.
  /// </summary>
  using numType = float;
  inline static std::optional<float> calc(ExtraPrice* p, float n) {
      if (p != nullptr && p->squareMetres.has_value())
      return n * (p->price / p->squareMetres.value());
    return std::nullopt;
  }
};

template<>
struct ExtraPriceTrait<ExtraPriceType::SHOT_BLASTING> {
  using numType = unsigned;
  inline static std::optional<float> calc(ExtraPrice* p, unsigned n) {
      if (p != nullptr && p->amount.has_value())
      return n * (p->price / p->amount.value());
    return std::nullopt;
  }
};
/// @endcond
template <ExtraPriceType T>
void ExtraPriceManager<T>::setExtraPrice(ExtraPrice *price) {
  ExtraPriceManager<T>::extraPrice = price;
}

template <ExtraPriceType T>
ExtraPrice *ExtraPriceManager<T>::getExtraPrice() {
  return ExtraPriceManager<T>::extraPrice;
}

template <ExtraPriceType T>
std::optional<float> ExtraPriceManager<T>::getPrice(typename ExtraPriceTrait<T>::numType n) {
  return ExtraPriceTrait<T>::calc(ExtraPriceManager<T>::getExtraPrice(), n);
}
#endif