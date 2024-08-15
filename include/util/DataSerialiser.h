//
// Created by Matthew.Sirman on 14/08/2020.
//

#ifndef DATABASE_MANAGER_DATASERIALISER_H
#define DATABASE_MANAGER_DATASERIALISER_H

/// <summary>
/// UNUSED
/// </summary>
#define BASIC_TYPE_SERIALISER(type, ...)                             \
public:                                                              \
inline void serialise(void *&buffer) {                               \
    DataSerialiser::serialiseToBuffer(buffer, __VA_ARGS__);          \
}                                                                    \
inline unsigned serialisedSize() {                                   \
    unsigned dataByteStreamSize = 0;                                 \
    DataSerialiser::serialisedSize(dataByteStreamSize, __VA_ARGS__); \
    return dataByteStreamSize;                                       \
}                                                                    \
inline static type &deserialise(void *&buffer) {                     \
    return *new type(buffer);                                        \
}                                                                    \
private:                                                             \
inline type(void *&buffer) {                                         \
    DataSerialiser::deserialiseFromBuffer(buffer, __VA_ARGS__);      \
}                                                                    \
public:

typedef unsigned char byte;

/// <summary>
/// UNUSED
/// </summary>
class ISerialisable {
public:
    virtual void serialise(void *&buffer) = 0;

    virtual size_t serialisedSize() = 0;
};


/// <summary>
/// UNUSED
/// </summary>
class DataSerialiser {
public:
    template<typename ...Args>
    static void serialiseToBuffer(void *&buffer, Args &&... args);

    template<typename ...Args>
    static void deserialiseFromBuffer(void *&buffer, Args &...args);

    template<typename T>
    static void serialiseObject(void *&buffer, const T &object, void (T::*func)(void *&) const);

    template<typename ...Args>
    static void serialisedSize(unsigned &size, Args &&... args);

private:
    template<typename T, typename ...Args>
    static void impl_serialiseToBuffer(void *&buffer, T &&t, Args &&...args);

    template<typename T>
    static void impl_serialiseToBuffer(void *&buffer, T &&t);

    template<typename T, typename ...Args>
    static void impl_deserialiseFromBuffer(void *&buffer, T &t, Args &...args);

    template<typename T>
    static void impl_deserialiseFromBuffer(void *&buffer, T &t);

    template<typename T>
    static void impl_writeElement(void *&buffer, T &&t);

    template<typename T>
    static void impl_readElement(void *&buffer, T &t);

    template<typename T, typename ...Args>
    static void impl_serialisedSize(unsigned &size, T &&t, Args &&...args);

    template<typename T>
    static void impl_serialisedSize(unsigned &size, T &&t);
};

template<typename... Args>
void DataSerialiser::serialiseToBuffer(void *&buffer, Args &&... args) {
    impl_serialiseToBuffer(buffer, args...);
}

template<typename... Args>
void DataSerialiser::deserialiseFromBuffer(void *&buffer, Args &... args) {
    impl_deserialiseFromBuffer(buffer, args...);
}

template<typename T>
void DataSerialiser::serialiseObject(void *&buffer, const T &object, void (T::*func)(void *&) const) {
    object.*func(buffer);
}

template<typename... Args>
void DataSerialiser::serialisedSize(unsigned int &size, Args &&... args) {
    size = 0;
    impl_serialisedSize(size, args...);
}

template<typename T, typename... Args>
void DataSerialiser::impl_serialiseToBuffer(void *&buffer, T &&t, Args &&... args) {
    impl_writeElement(buffer, t);
    impl_serialiseToBuffer(buffer, args...);
}

template<typename T>
void DataSerialiser::impl_serialiseToBuffer(void *&buffer, T &&t) {
    impl_writeElement(buffer, t);
}

template<typename T, typename... Args>
void DataSerialiser::impl_deserialiseFromBuffer(void *&buffer, T &t, Args &... args) {
    impl_readElement(buffer, t);
    impl_deserialiseFromBuffer(buffer, args...);
}

template<typename T>
void DataSerialiser::impl_deserialiseFromBuffer(void *&buffer, T &t) {
    impl_readElement(buffer, t);
}

template<typename T>
void DataSerialiser::impl_writeElement(void *&buffer, T &&t) {
    byte *buff = (byte *) buffer;
    *((T *) buff) = t;
    buff += sizeof(T);
    buffer = buff;
}

template<>
inline void DataSerialiser::impl_writeElement(void *&buffer, ISerialisable &&object) {
    object.serialise(buffer);
}

template<typename T>
void DataSerialiser::impl_readElement(void *&buffer, T &t) {
    byte *buff = (byte *) buffer;
    t = *((T *) buff);
    buff += sizeof(T);
    buffer = buff;
}

template<typename T, typename... Args>
void DataSerialiser::impl_serialisedSize(unsigned int &size, T &&t, Args &&... args) {
    size += sizeof(T);
    impl_serialisedSize(size, args...);
}

template<typename T>
void DataSerialiser::impl_serialisedSize(unsigned int &size, T &&t) {
    size += sizeof(T);
}

#endif //DATABASE_MANAGER_DATASERIALISER_H
