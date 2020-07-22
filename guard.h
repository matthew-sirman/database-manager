//
// Created by matthew on 12/05/2020.
//

#ifndef DATABASE_SERVER_GUARD_H
#define DATABASE_SERVER_GUARD_H

#include <iostream>
#include <cstring>

#define __FILENAME__ (std::strrchr(__FILE__, '/') ? std::strrchr(__FILE__, '/') + 1 : __FILE__)

#define ERROR_TO(errorMessage, stream) { \
    stream << "ERROR::" << __FILENAME__ << ":" << __LINE__ <<  ": " << errorMessage << ": " << std::strerror(errno) << std::endl; \
    exit(1); \
}

#define STD_ERROR(errorMessage) ERROR_TO(errorMessage, std::cerr)

#define SAFE_ERROR_TO(errorMessage, stream){ \
    stream << "ERROR::" << __FILENAME__ << ":" << __LINE__ <<  ": " << errorMessage << ": " << std::strerror(errno) << std::endl; \
}

#ifdef _WIN32

#define SOCK_ERROR_TO(errorMessage, stream) { \
    stream << "ERROR::" << __FILENAME__ << ":" << __LINE__ << ": " << errorMessage << ": " << WSAGetLastError() << std::endl; \
    exit(1); \
}

#define SAFE_SOCK_ERROR_TO(errorMessage, stream) { \
    stream << "ERROR::" << __FILENAME__ << ":" << __LINE__ << ": " << errorMessage << ": " << WSAGetLastError() << std::endl; \
}

#else

#define SOCK_ERROR_TO(errorMessage, stream) ERROR_TO(errorMessage, stream)

#define SAFE_SOCK_ERROR_TO(errorMessage, stream) SAFE_ERROR_TO(errorMessage, stream)

#endif

#define ERROR_RAW(errorMessage) { \
    std::cerr << "ERROR::" << __FILENAME__ << ":" << __LINE__ <<  ": " << errorMessage << std::endl; \
    exit(1); \
}

#define ERROR_RAW_SAFE(errorMessage) { \
    std::cerr << "ERROR::" << __FILENAME__ << ":" << __LINE__ <<  ": " << errorMessage << std::endl; \
}

/*#define SQL_ERROR(error) { \
    std::cerr << "ERROR::" << __FILENAME__ << ":" << __LINE__ << ": " << "SQL Error " << error << std::endl; \
    exit(1); \
}

#define SQL_ERROR_SAFE(error) { \
    std::cerr << "ERROR::" << __FILENAME__ << ":" << __LINE__ <<  ": " << "SQL Error " << error << std::endl; \
}*/

#define SQL_ERROR(error) { \
    std::cerr << "ERROR::" << __FILENAME__ << ":" << __LINE__ << ": " << "SQL Error " << error.what() << std::endl; \
    exit(1); \
}

#define SQL_ERROR_SAFE(error) { \
    std::cerr << "ERROR::" << __FILENAME__ << ":" << __LINE__ <<  ": " << "SQL Error " << error.what() << std::endl; \
}

#endif //DATABASE_SERVER_GUARD_H
