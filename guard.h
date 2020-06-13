//
// Created by matthew on 12/05/2020.
//

#ifndef DATABASE_SERVER_GUARD_H
#define DATABASE_SERVER_GUARD_H

#include <iostream>
#include <cstring>

#define __FILENAME__ (std::strrchr(__FILE__, '/') ? std::strrchr(__FILE__, '/') + 1 : __FILE__)

#define ERROR(errorMessage) { \
    std::cerr << "ERROR::" << __FILENAME__ << ": " << errorMessage << ": " << std::strerror(errno) << std::endl; \
    exit(1); \
}

#define ERROR_RAW(errorMessage) { \
    std::cerr << "ERROR::" << __FILENAME__ << ": " << errorMessage << std::endl; \
    exit(1); \
}

#endif //DATABASE_SERVER_GUARD_H
