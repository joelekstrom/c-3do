cmake_minimum_required(VERSION 2.8)
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
project(c3do)

if (${CMAKE_C_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wextra")
endif()

include_directories(include lib/Nano-BMP/include)
add_subdirectory(lib/Nano-BMP)
add_subdirectory(src)

file(COPY ${CMAKE_SOURCE_DIR}/model DESTINATION ${CMAKE_BINARY_DIR})