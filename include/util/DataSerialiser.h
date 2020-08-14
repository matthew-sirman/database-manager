//
// Created by Matthew.Sirman on 14/08/2020.
//

#ifndef DATABASE_MANAGER_DATASERIALISER_H
#define DATABASE_MANAGER_DATASERIALISER_H

typedef unsigned char byte;

class DataSerialiser {
public:
    template<typename ...Args>
    static void serialiseToBuffer(void *&buffer, const Args &&... args);

    template<typename T>
    static void serialiseObject(void *&buffer, const T &object, void (T::*func)(void *&) const);

    template<typename ...Args>
    static void serialisedSize(unsigned &size, const Args &&... args);

private:
    template<typename T, typename ...Args>
    static void impl_serialiseToBuffer(void *&buffer, const T &&t, const Args &&...args);

    template<typename T>
    static void impl_serialiseToBuffer(void *&buffer, const T &&t);

    template<typename T>
    static void impl_writeElement(void *&buffer, const T &&t);

    template<typename T, typename ...Args>
    static void impl_serialisedSize(unsigned &size, const T &&t, const Args &&...args);

    template<typename T>
    static void impl_serialisedSize(unsigned &size, const T &&t);
};

template<typename... Args>
void DataSerialiser::serialiseToBuffer(void *&buffer, const Args &&... args) {
    impl_serialiseToBuffer(buffer, args...);
}

template<typename T>
void DataSerialiser::serialiseObject(void *&buffer, const T &object, void (T::*func)(void *&) const) {
    object.*func(buffer);
}

template<typename... Args>
void DataSerialiser::serialisedSize(unsigned int &size, const Args &&... args) {
    size = 0;
    impl_serialisedSize(size, args...);
}

template<typename T, typename... Args>
void DataSerialiser::impl_serialiseToBuffer(void *&buffer, const T &&t, const Args &&... args) {
    impl_writeElement(buffer, t);
    impl_serialiseToBuffer(buffer, args...);
}

template<typename T>
void DataSerialiser::impl_serialiseToBuffer(void *&buffer, const T &&t) {
    impl_writeElement(buffer, t);
}

template<typename T>
void DataSerialiser::impl_writeElement(void *&buffer, const T &&t) {
    byte *buff = (byte *) buffer;
    *((T *) buff) = t;
    buff += sizeof(T);
    buffer = buff;
}

template<typename T, typename... Args>
void DataSerialiser::impl_serialisedSize(unsigned int &size, const T &&t, const Args &&... args) {
    size += sizeof(T);
    impl_serialisedSize(size, args...);
}

template<typename T>
void DataSerialiser::impl_serialisedSize(unsigned int &size, const T &&t) {
    size += sizeof(T);
}

#endif //DATABASE_MANAGER_DATASERIALISER_H
