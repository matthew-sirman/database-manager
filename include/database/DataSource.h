//
// Created by matthew on 12/07/2020.
//

#ifndef DATABASE_MANAGER_DATASOURCE_H
#define DATABASE_MANAGER_DATASOURCE_H

#include <stdexcept>
#include <functional>

// Forward declaration of DataSource template class
template<typename T, typename IType>
class DataSource;

/// <summary>
/// SourceFilter
/// Base type for an installable filter object on a DataSource. The filter should be extended to
/// suit a particular data source. This will filter a data source based on an arbitrary set of 
/// criteria.
/// </summary>
/// <typeparam name="IType"></typeparam>
template<typename IType>
class SourceFilter {
public:
    /// <summary>
    /// Interface filter function. This will return a boolean pertaining to whether or not the element
    /// should be left in the result or not. A true value indicates that the element is a member of the filtered
    /// set, and a false indicates that it is not.
    /// </summary>
    /// <param name="element">The iterator to check for presence in the filtered set.</param>
    /// <returns>Whether or not the element pointed to by this iterator is in the filtered set.</returns>
    bool filter(IType element) const;

    /// <summary>
    /// Setter for the iterator endpoint. This must be known so that the filtering can determine when
    /// the data source iterator ends. Otherwise, it may attempt to filter beyond the end of the set,
    /// resulting in an error.
    /// </summary>
    /// <param name="endpoint">The iterator of the endpoint of the source</param>
    void setEndpoint(IType endpoint);

    /// <summary>
    /// Adds a callback function invoked whenever the filter is updated. This is not always necessary,
    /// though useful for dynamic filters.
    /// </summary>
    /// <param name="callback"></param>
    void attachFilterUpdateCallback(const std::function<void()> &callback);

protected:
    /// <summary>
    /// The update callback function to be invoked if the filter's criteria change.
    /// </summary>
    std::function<void()> filterUpdateCallback = nullptr;

private:
    /// <summary>
    /// Internal filter function. This should be implemented in any specific filter type. This function
    /// actually determines whether or not an element should be included in the filtered set; the interface 
    /// filter function is just a wrapped.
    /// </summary>
    /// <param name="element">The iterator to check for presence in the filtered set.</param>
    /// <returns>Whether or not the element pointed to by this iterator is in the filtered set.</returns>
    virtual bool __filter(IType element) const = 0;

    // The endpoint of the data source to avoid exceeding the end when filtering.
    IType __iterEndpoint;
};

// Filter function implementation
template<typename IType>
inline bool SourceFilter<IType>::filter(IType element) const {
    // First we check if the current element is the endpoint. If it is, we simply
    // return true (else the iterator will attempt to keep finding the next available
    // element)
    if (element == __iterEndpoint) {
        return true;
    }

    // If the element is a proper element in the set, we return the actual filter function of 
    // the element.
    return __filter(element);
}

// Set endpoint implementation
template<typename IType>
inline void SourceFilter<IType>::setEndpoint(IType endpoint) {
    // Sets the internal variable to the one passed in
    __iterEndpoint = endpoint;
}

// Attach filter update callback implementation
template<typename IType>
inline void SourceFilter<IType>::attachFilterUpdateCallback(const std::function<void()> &callback) {
    // Sets the callback function to the one passed in
    filterUpdateCallback = callback;
}

/// <summary>
/// DataSourceIterator
/// The iterator object for the datasource itself. Acts as a wrapper for an iterator of type
/// IType and acts on a set of type T. The iterator has the ability to be filtered and adapted.
/// </summary>
/// <typeparam name="T">The type of elements in the set. Each iterator should have an adapter function to map from
/// IType to T.</typeparam>
/// <typeparam name="IType">The base iterator type to wrap.</typeparam>
template<typename T, typename IType>
class DataSourceIterator {
    // Friend class of the corrent DataSource type with matching template parameters
    friend class DataSource<T, IType>;

private:
    /// <summary>
    /// Internal class representing the actual value of a particular data element.
    /// </summary>
    class DataHolder {
    public:
        DataHolder(const T value) : __value(value) {}

        T operator*() const { return __value; }

    private:
        const T __value;
    };

public:
    /// <summary>
    /// Standard typedefs to make a valid C++ iterator object
    /// </summary>
    typedef T value_type;
    /// <summary>
    /// Standard typedefs to make a valid C++ iterator object
    /// </summary>
    typedef std::ptrdiff_t difference_type;
    /// <summary>
    /// Standard typedefs to make a valid C++ iterator object
    /// </summary>
    typedef T *pointer;
    /// <summary>
    /// Standard typedefs to make a valid C++ iterator object
    /// </summary>
    typedef T &reference;
    /// <summary>
    /// Standard typedefs to make a valid C++ iterator object
    /// </summary>
    typedef std::input_iterator_tag iterator_category;

    /// <summary>
    /// Constructor for DataSourceIterator based on the corresponding base iterator
    /// </summary>
    /// <param name="iter">The base iterator for this wrapper object.</param>
    explicit DataSourceIterator(IType iter);

    /// <summary>
    /// Copy constructor
    /// </summary>
    /// <param name="it"></param>
    explicit DataSourceIterator(const DataSourceIterator<T, IType> &it);

    /// <summary>
    /// Dereference operator
    /// </summary>
    /// <returns>The adapted object of type T corresponding to this iterator.</returns>
    T operator*() const;

    /// <summary>
    /// Equality operator
    /// </summary>
    /// <param name="other">Comparison iterator object</param>
    /// <returns>Whether the two objects' underlying iterators are equal.</returns>
    bool operator==(const DataSourceIterator<T, IType> &other) const;

    /// <summary>
    /// Inequality operator
    /// </summary>
    /// <param name="other">Comparison iterator object</param>
    /// <returns>Whether the two objects' underlying iterators are inequal.</returns>
    bool operator!=(const DataSourceIterator<T, IType> &other) const;

    /// <summary>
    /// Post increment operator
    /// </summary>
    /// <returns>The value of the iterator before incrementing.</returns>
    DataHolder operator++(int);

    /// <summary>
    /// Pre increment operator
    /// </summary>
    /// <returns>The value of the iterator after incrementing.</returns>
    DataSourceIterator<T, IType> &operator++();

    /// <summary>
    /// Setter for the adapter function for this iterator.
    /// </summary>
    /// <param name="adapter">The adapter function to use to convert from IType to T.</param>
    void setAdapter(const std::function<T(IType)> &adapter);

protected:
    /// <summary>
    /// Protected constructor
    /// </summary>
    /// <param name="iter">The underlying iterator to use.</param>
    /// <param name="adapter">The adapter function to use.</param>
    DataSourceIterator(IType iter, const std::function<T(IType)> &adapter);

    /// <summary>
    /// Protected constructor
    /// </summary>
    /// <param name="iter">The underlying iterator to use.</param>
    /// <param name="adapter">The adapter function to use.</param>
    /// <param name="filter">The filter object to use.</param>
    DataSourceIterator(IType iter, const std::function<T(IType)> &adapter, const SourceFilter<IType> &filter);

    /// <summary>
    /// The adapter function to use. This must be set to use the iterator. If it is left as nullptr,
    /// an error will occur.
    /// </summary>
    std::function<T(IType)> __adapter = nullptr;

    /// <summary>
    /// The filter object to use. This is not essential. If left (or set) as nullptr, no filter will be applied.
    /// </summary>
    const SourceFilter<IType> *__filter = nullptr;

    /// <summary>
    /// The current underlying iterator object.
    /// </summary>
    IType iter;
};

// Implementation of constructor
template<typename T, typename IType>
DataSourceIterator<T, IType>::DataSourceIterator(IType iter)
        : iter(iter) {

}

// Implementation of constructor
template<typename T, typename IType>
DataSourceIterator<T, IType>::DataSourceIterator(const DataSourceIterator<T, IType> &it) {
    // Sets the properties of this element based on the properties of the element to copy.
    this->iter = it.iter;
    this->__adapter = it.__adapter;
    this->__filter = it.__filter;
}

// Implementation of constructor
template<typename T, typename IType>
inline DataSourceIterator<T, IType>::DataSourceIterator(IType iter, const std::function<T(IType)> &adapter) 
    : DataSourceIterator<T, IType>(iter) {
    // Sets the adapter
    __adapter = adapter;
}

// Implementation of constructor
template<typename T, typename IType>
DataSourceIterator<T, IType>::DataSourceIterator(IType iter, const std::function<T(IType)> &adapter, const SourceFilter<IType> &filter)
        : DataSourceIterator<T, IType>(iter, adapter) {
    // Sets the filter
    __filter = &filter;
    // Filters through the set to the first valid point
    if (__filter) {
        while (!__filter->filter(this->iter)) {
            this->iter++;
        }
    }
}

// Implementation of dereference operator
template<typename T, typename IType>
T DataSourceIterator<T, IType>::operator*() const {
    // Checks if the adapter is null
    if (__adapter == nullptr) {
        // If the adapter is null, throw a logic error.
        throw std::logic_error("Attempted to iterate through data source with no adapter function");
    }
    // Return the adapted value
    return __adapter(iter);
}

// Implementation of equality operator
template<typename T, typename IType>
bool DataSourceIterator<T, IType>::operator==(const DataSourceIterator<T, IType> &other) const {
    // Returns whether the underlying operators are equal
    return iter == other.iter;
}

// Implementation of inequality operator
template<typename T, typename IType>
bool DataSourceIterator<T, IType>::operator!=(const DataSourceIterator<T, IType> &other) const {
    // Returns whether the underlying operators are inequal
    return iter != other.iter;
}

// Implementation of pre increment operator
template<typename T, typename IType>
typename DataSourceIterator<T, IType>::DataHolder DataSourceIterator<T, IType>::operator++(int) {
    // Gets the adapted data object for this iterator and stores it for return
    DataHolder ret(**this);
    // Increments the iterator once
    iter++;
    // If there is a filter
    if (__filter) {
        // Increment the filter until the element is valid
        while (!__filter->filter(iter)) {
            iter++;
        }
    }
    // Return the cached pre increment value
    return ret;
}

// Implementation of post increment operator
template<typename T, typename IType>
DataSourceIterator<T, IType> &DataSourceIterator<T, IType>::operator++() {
    // Increments the iterator once
    iter++;
    // If there is a filter
    if (__filter) {
        // Increment the filter until the element is valid
        while (!__filter->filter(iter)) {
            iter++;
        }
    }
    // Return the new dereferenced value
    return *this;
}

// Implementation of the set adapter function
template<typename T, typename IType>
void DataSourceIterator<T, IType>::setAdapter(const std::function<T(IType)> &adapter) {
    // Sets the internal adapter to the one passed in
    __adapter = adapter;
}

/// <summary>
/// DataSource
/// Represents an iterable data source of arbitrary type. This iterable is based on an adapter model
/// from an underlying iterator type.
/// </summary>
/// <typeparam name="T">The type of the data source set.</typeparam>
/// <typeparam name="IType">The type of the underlying iterator.</typeparam>
template<typename T, typename IType>
class DataSource {
public:
    /// <summary>
    /// Default constructor
    /// </summary>
    DataSource() = default;

    /// <summary>
    /// Constructor based on the start and endpoints for the source
    /// </summary>
    /// <param name="begin">The underlying iterator point to start from.</param>
    /// <param name="end">The underlying iterator point to end at.</param>
    DataSource(IType begin, IType end);

    /// <summary>
    /// Destructor for source
    /// </summary>
    ~DataSource();

    /// <summary>
    /// Getter for the begin point. This is used in C++ range based for loops.
    /// </summary>
    /// <returns>The iterator for the start of the set.</returns>
    DataSourceIterator<T, IType> begin() const;

    /// <summary>
    /// Getter for the end point. This is used in C++ range based for loops.
    /// </summary>
    /// <returns>The iterator for the end of the set.</returns>
    DataSourceIterator<T, IType> end() const;

    /// <summary>
    /// Updates the data source. Can be overridden to provide custom responses.
    /// </summary>
    virtual void updateSource();

    /// <summary>
    /// Sets the adapter for the data source
    /// </summary>
    /// <param name="adapter">The adapter function to use for the mapping the underlying iterators to the
    /// desired type.</param>
    virtual void setAdapter(const std::function<T(IType)> &adapter);

    /// <summary>
    /// Setter for the filter. Based on a template type for an arbitrary filter.
    /// </summary>
    /// <typeparam name="Filter">The type of filter to install on the source. Must inherit from SourceFilter of type IType.</typeparam>
    /// <returns>A newly constructed filter object of type Filter which has been installed on this data source.</returns>
    template<typename Filter>
    Filter *setFilter();

    /// <summary>
    /// Remover for the filter. After calling this, the set will have no filter applied and return to default.
    /// </summary>
    void removeFilter();

    /// <summary>
    /// Getter for the current source state. This can be used to determine whether a user of the source is up to date 
    /// with the current data source. The state value is incremented each time an update to the source is made, so
    /// a user is able to determine whether they are accessing the correct version, or whether the source has since 
    /// changed.
    /// </summary>
    /// <returns>The current state of the data source.</returns>
    unsigned state() const;

protected:
    /// <summary>
    /// Internal holder for the beginning of the underlying iterator source.
    /// </summary>
    IType __begin;
    /// <summary>
    /// Internal holder for the end of the underlying iterator source.
    /// </summary>
    IType __end;

    /// <summary>
    /// The adapter function to use to map from the iterator type to the data source set type
    /// </summary>
    std::function<T(IType)> __adapter = nullptr;

    /// <summary>
    /// The filter object installed on this data source. Defaults to nullptr indicating no filter.
    /// </summary>
    SourceFilter<IType> *__filter = nullptr;


    /// <summary>
    /// The current state ID of the data source. This should be incremented whenever a change is made, 
    /// so any users of the source can tell if they are using the correct version.
    /// </summary>
    unsigned stateID = 0;
};

// Implementation of constructor
template<typename T, typename IType>
DataSource<T, IType>::DataSource(IType begin, IType end) {
    // Sets the start and end points of the iterator
    this->__begin = __begin;
    this->__end = __end;
}

// Implementation of destructor
template<typename T, typename IType>
DataSource<T, IType>::~DataSource() {
    delete __filter;
}

// Implementation of begin function
template<typename T, typename IType>
DataSourceIterator<T, IType> DataSource<T, IType>::begin() const {
    // If there is a filter
    if (__filter) {
        // Update the fitler such that it knows the correct endpoint
        __filter->setEndpoint(__end);
        // Return a data source iterator for the start with the appropriate adapter and filter.
        return DataSourceIterator<T, IType>(__begin, __adapter, *__filter);
    } else {
        // Return a data source iterator for the start with the appropriate adapter, but no filter.
        return DataSourceIterator<T, IType>(__begin, __adapter);
    }
}

// Implementation of end function
template<typename T, typename IType>
DataSourceIterator<T, IType> DataSource<T, IType>::end() const {
    // Returns a data source iterator containing just the end value. The end doesn't
    // actually point to a true element, and as such doesn't need to know about the adapter
    // or filter.
    return DataSourceIterator<T, IType>(__end);
}

// Implementation of the udpate source function
template<typename T, typename IType>
void DataSource<T, IType>::updateSource() {
    // By default, this does nothing but update the current source ID. Can be overridden for
    // added functionality, though any implementations should call the base implementation
    stateID++;
}

// Implementation of the set adapter function
template<typename T, typename IType>
void DataSource<T, IType>::setAdapter(const std::function<T(IType)> &adapter) {
    // Sets the internal adapter to the one passed in
    __adapter = adapter;
}

// Implementation for the set filter function
template<typename T, typename IType>
template<typename Filter>
Filter *DataSource<T, IType>::setFilter() {
    // First we assert that this is a valid filter for the data source type. This can be done statically as the data source system
    // is based on template types.
    static_assert(std::is_base_of<SourceFilter<IType>, Filter>::value, "Filter must derive from SourceFilter<IType>");
    // Delete the filter in case there was already one installed
    delete __filter;
    // We then construct the filter object
    __filter = new Filter();
    // We then attach an update callback which simply updates the source
    __filter->attachFilterUpdateCallback([this]() { updateSource(); });
    // We then manually assert that the source has updated so the filter can apply
    updateSource();
    // Finally we return the filter object cast to the derived type (which we know it is) so the user has
    // access to its pointer.
    return (Filter *)__filter;
}

// Implementation of remove filter function
template<typename T, typename IType>
inline void DataSource<T, IType>::removeFilter() {
    // If the filter exists, delete it
    if (__filter) {
        delete __filter;
    }
    // Set the filter to nullptr so we know it has been removed
    __filter = nullptr;
    // Update the source to indicate that the filter is now gone
    updateSource();
}

// Implementation of state function
template<typename T, typename IType>
unsigned DataSource<T, IType>::state() const {
    // Returns the current state ID.
    return stateID;
}

#endif //DATABASE_MANAGER_DATASOURCE_H
