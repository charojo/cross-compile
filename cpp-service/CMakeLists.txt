cmake_minimum_required(VERSION 3.16)
project(cpp_service LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ZMQ REQUIRED libzmq)

add_library(sensor_service SHARED src/service.cpp proto.cpp)
target_include_directories(sensor_service PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(sensor_service PRIVATE sqlite3)

add_executable(data_service DataService.cpp Cache.cpp Database.cpp)
target_include_directories(data_service PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(data_service PRIVATE ${ZMQ_INCLUDE_DIRS})
target_link_libraries(data_service PRIVATE sqlite3 ${ZMQ_LIBRARIES})
