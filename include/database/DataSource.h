//
// Created by matthew on 12/07/2020.
//

#ifndef DATABASE_MANAGER_DATASOURCE_H
#define DATABASE_MANAGER_DATASOURCE_H

#include <stdexcept>
#include <functional>

template<typename T, typename IType>
class DataSource;

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

    std::function<T(IType)> __adapter;

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
DataSourceIterator<T, IType>::DataSourceIterator(IType iter, const std::function<T(IType)> &adapter)
        : DataSourceIterator<T, IType>(iter) {
    __adapter = adapter;
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
    return ret;
}

template<typename T, typename IType>
DataSourceIterator<T, IType> &DataSourceIterator<T, IType>::operator++() {
    iter++;
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

    unsigned state() const;

protected:
    IType __begin, __end;

    std::function<T(IType)> __adapter = nullptr;

    unsigned stateID = 0;
};

template<typename T, typename IType>
DataSource<T, IType>::DataSource(IType begin, IType end) {
    this->__begin = __begin;
    this->__end = __end;
}

template<typename T, typename IType>
DataSourceIterator<T, IType> DataSource<T, IType>::begin() const {
    return DataSourceIterator<T, IType>(__begin, __adapter);
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
unsigned DataSource<T, IType>::state() const {
    return stateID;
}

#endif //DATABASE_MANAGER_DATASOURCE_H
