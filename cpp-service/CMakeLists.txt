cmake_minimum_required(VERSION 3.16)
project(cpp_service LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Cross-compilation settings for ARM64
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)



find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ../proto/data.proto)

add_library(sensor_service SHARED src/service.cpp ${PROTO_SRCS})
target_include_directories(sensor_service PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(sensor_service PRIVATE protobuf::libprotobuf sqlite3)

add_executable(data_service DataService.cpp Cache.cpp Database.cpp ${PROTO_SRCS})
target_include_directories(data_service PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(data_service PRIVATE protobuf::libprotobuf sqlite3 zmq)
