//
// Created by matthew on 12/07/2020.
//

#ifndef DATABASE_MANAGER_DATASOURCE_H
#define DATABASE_MANAGER_DATASOURCE_H

#include <stdexcept>
#include <functional>

template<typename T, typename IType>
class DataSource;

template<typename IType>
class SourceFilter {
public:
    bool filter(IType element) const;

    void setEndpoint(IType endpoint);

    void attachFilterUpdateCallback(const std::function<void()> &callback);

protected:
    std::function<void()> filterUpdateCallback = nullptr;

private:
    virtual bool __filter(IType element) const = 0;

    IType __iterEndpoint;
};

template<typename IType>
inline bool SourceFilter<IType>::filter(IType element) const {
    if (element == __iterEndpoint) {
        return true;
    }

    return __filter(element);
}

template<typename IType>
inline void SourceFilter<IType>::setEndpoint(IType endpoint) {
    __iterEndpoint = endpoint;
}

template<typename IType>
inline void SourceFilter<IType>::attachFilterUpdateCallback(const std::function<void()> &callback) {
    filterUpdateCallback = callback;
}

template<typename T, typename IType>
class DataSourceIterator {
    friend class DataSource<T, IType>;

private:
    class DataHolder {
    public:
        DataHolder(const T value) : __value(value) {}

        T operator*() const { return __value; }

    private:
        const T __value;
    };

public:
    typedef T value_type;
    typedef std::ptrdiff_t difference_type;
    typedef T *pointer;
    typedef T &reference;
    typedef std::input_iterator_tag iterator_category;

    explicit DataSourceIterator(IType iter);

    explicit DataSourceIterator(const DataSourceIterator<T, IType> &it);

    T operator*() const;

    bool operator==(const DataSourceIterator<T, IType> &other) const;

    bool operator!=(const DataSourceIterator<T, IType> &other) const;

    DataHolder operator++(int);

    DataSourceIterator<T, IType> &operator++();

    void setAdapter(const std::function<T(IType)> &adapter);

protected:
    DataSourceIterator(IType iter, const std::function<T(IType)> &adapter);

    DataSourceIterator(IType iter, const std::function<T(IType)> &adapter, const SourceFilter<IType> &filter);

    std::function<T(IType)> __adapter = nullptr;

    const SourceFilter<IType> *__filter = nullptr;

    IType iter;
};

template<typename T, typename IType>
DataSourceIterator<T, IType>::DataSourceIterator(IType iter)
        : iter(iter) {

}

template<typename T, typename IType>
DataSourceIterator<T, IType>::DataSourceIterator(const DataSourceIterator<T, IType> &it) {
    this->iter = it.iter;
    this->__adapter = it.__adapter;
}

template<typename T, typename IType>
inline DataSourceIterator<T, IType>::DataSourceIterator(IType iter, const std::function<T(IType)> &adapter) 
    : DataSourceIterator<T, IType>(iter) {
    __adapter = adapter;
}

template<typename T, typename IType>
DataSourceIterator<T, IType>::DataSourceIterator(IType iter, const std::function<T(IType)> &adapter, const SourceFilter<IType> &filter)
        : DataSourceIterator<T, IType>(iter, adapter) {
    __filter = &filter;
    while (!__filter->filter(this->iter)) {
        this->iter++;
    }
}

template<typename T, typename IType>
T DataSourceIterator<T, IType>::operator*() const {
    if (__adapter == nullptr) {
        throw std::logic_error("Attempted to iterate through data source with no adapter function");
    }
    return __adapter(iter);
}

template<typename T, typename IType>
bool DataSourceIterator<T, IType>::operator==(const DataSourceIterator<T, IType> &other) const {
    return iter == other.iter;
}

template<typename T, typename IType>
bool DataSourceIterator<T, IType>::operator!=(const DataSourceIterator<T, IType> &other) const {
    return iter != other.iter;
}

template<typename T, typename IType>
typename DataSourceIterator<T, IType>::DataHolder DataSourceIterator<T, IType>::operator++(int) {
    DataHolder ret(**this);
    iter++;
    if (__filter) {
        while (!__filter->filter(iter)) {
            iter++;
        }
    }
    return ret;
}

template<typename T, typename IType>
DataSourceIterator<T, IType> &DataSourceIterator<T, IType>::operator++() {
    iter++;
    if (__filter) {
        while (!__filter->filter(iter)) {
            iter++;
        }
    }
    return *this;
}

template<typename T, typename IType>
void DataSourceIterator<T, IType>::setAdapter(const std::function<T(IType)> &adapter) {
    __adapter = adapter;
}

template<typename T, typename IType>
class DataSource {
//    static_assert(std::is_base_of<DataSourceIterator<T, BaseIType>, IterType>::value, "IterType must derive from DataSourceIterator<T, BaseIType>");
public:
    DataSource() = default;

    DataSource(IType begin, IType end);

    DataSourceIterator<T, IType> begin() const;

    DataSourceIterator<T, IType> end() const;

    virtual void updateSource();

    virtual void setAdapter(const std::function<T(IType)> &adapter);

    template<typename Filter>
    Filter *setFilter();

    void removeFilter();

    unsigned state() const;

protected:
    IType __begin, __end;

    std::function<T(IType)> __adapter = nullptr;

    SourceFilter<IType> *__filter = nullptr;

    unsigned stateID = 0;
};

template<typename T, typename IType>
DataSource<T, IType>::DataSource(IType begin, IType end) {
    this->__begin = __begin;
    this->__end = __end;
}

template<typename T, typename IType>
DataSourceIterator<T, IType> DataSource<T, IType>::begin() const {
    if (__filter) {
        __filter->setEndpoint(__end);
        return DataSourceIterator<T, IType>(__begin, __adapter, *__filter);
    } else {
        return DataSourceIterator<T, IType>(__begin, __adapter);
    }
}

template<typename T, typename IType>
DataSourceIterator<T, IType> DataSource<T, IType>::end() const {
    return DataSourceIterator<T, IType>(__end);
}

template<typename T, typename IType>
void DataSource<T, IType>::updateSource() {
    stateID++;
}

template<typename T, typename IType>
void DataSource<T, IType>::setAdapter(const std::function<T(IType)> &adapter) {
    __adapter = adapter;
}

template<typename T, typename IType>
template<typename Filter>
Filter *DataSource<T, IType>::setFilter() {
    static_assert(std::is_base_of<SourceFilter<IType>, Filter>::value, "Filter must derive from SourceFilter<IType>");
    __filter = new Filter();
    __filter->attachFilterUpdateCallback([this]() { updateSource(); });
    updateSource();
    return (Filter *)__filter;
}

template<typename T, typename IType>
inline void DataSource<T, IType>::removeFilter() {
    __filter = nullptr;
    updateSource();
}

template<typename T, typename IType>
unsigned DataSource<T, IType>::state() const {
    return stateID;
}

#endif //DATABASE_MANAGER_DATASOURCE_H
