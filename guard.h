//
// Created by matthew on 12/05/2020.
//

#ifndef DATABASE_SERVER_GUARD_H
#define DATABASE_SERVER_GUARD_H

#include <iostream>
#include <cstring>

#define __FILENAME__ (std::strrchr(__FILE__, '/') ? std::strrchr(__FILE__, '/') + 1 : __FILE__)

//#define ERROR(errorMessage) { \
//    std::cerr << "ERROR::" << __FILENAME__ << ": " << errorMessage << ": " << std::strerror(errno) << std::endl; \
//    exit(1); \
//}

#define ERROR_TO(errorMessage, stream) { \
    stream << "ERROR::" << __FILENAME__ << ": " << errorMessage << ": " << std::strerror(errno) << std::endl; \
    exit(1); \
}

#define STD_ERROR(errorMessage) ERROR_TO(errorMessage, std::cerr)

#define SAFE_ERROR_TO(errorMessage, stream){ \
    stream << "ERROR::" << __FILENAME__ << ": " << errorMessage << ": " << std::strerror(errno) << std::endl; \
}

#define ERROR_RAW(errorMessage) { \
    std::cerr << "ERROR::" << __FILENAME__ << ": " << errorMessage << std::endl; \
    exit(1); \
}

#define SQL_ERROR(error) { \
    std::cerr << "SQL Error " << error.getErrorCode() << " (" << error.getSQLState() << "): " << error.what() << std::endl; \
    exit(1); \
}

#define SQL_ERROR_SAFE(error) { \
    std::cerr << "SQL Error " << error.getErrorCode() << " (" << error.getSQLState() << "): " << error.what() << std::endl; \
}

#endif //DATABASE_SERVER_GUARD_H
