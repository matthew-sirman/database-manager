cmake_minimum_required(VERSION 3.16)
project(database_manager)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS -pthread)

add_subdirectory(${PROJECT_SOURCE_DIR}/../encrypt encrypt-build)

include_directories(${PROJECT_SOURCE_DIR}/../encrypt/include)

add_executable(${PROJECT_NAME} main.cpp src/Server.cpp include/Server.h src/Client.cpp include/Client.h guard.h src/NetworkMessage.cpp include/NetworkMessage.h src/TCPSocket.cpp include/TCPSocket.h)

target_link_libraries(${PROJECT_NAME} encrypt)