cmake_minimum_required(VERSION 3.15)
project(data_service)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ../proto/data.proto)

add_executable(data_service DataService.cpp Cache.cpp Database.cpp ${PROTO_SRCS})

target_include_directories(data_service PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(data_service PRIVATE protobuf::libprotobuf sqlite3 zmq)
