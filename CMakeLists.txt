cmake_minimum_required(VERSION 2.8)
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
project(c3do)

add_definitions(-std=c99)
add_definitions(-Wall)
add_definitions(-Wextra)

include_directories(include lib/Nano-BMP/include)
add_subdirectory(lib/Nano-BMP)
add_subdirectory(src)

file(COPY ${CMAKE_SOURCE_DIR}/model DESTINATION ${CMAKE_BINARY_DIR})