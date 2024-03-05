cmake_minimum_required(VERSION 3.20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(cxx-common-opts)

project(pc_gps_tracker)

find_package(SerialPort REQUIRED)
find_package(GTest REQUIRED)

add_subdirectory(src)
