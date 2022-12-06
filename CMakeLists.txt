cmake_minimum_required(VERSION 3.22)

project(sniffer)

find_package(PkgConfig REQUIRED)

include(etc/build_type.cmake)
include(etc/cflags.cmake)
include(etc/clang_format.cmake)
include(etc/tests.cmake)

include_directories("${PROJECT_SOURCE_DIR}/src/util")

add_subdirectory("${PROJECT_SOURCE_DIR}/src/util")
add_subdirectory("${PROJECT_SOURCE_DIR}/src/frontend")

