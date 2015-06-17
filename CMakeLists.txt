cmake_minimum_required(VERSION 2.6)
project(c3do)

add_definitions(-std=c99)
add_definitions(-Wall)
add_definitions(-Wextra)

add_subdirectory(lib/Nano-BMP)
include_directories(lib/Nano-BMP/include)
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
add_subdirectory(src)