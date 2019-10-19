cmake_minimum_required (VERSION 3.1)
enable_testing()
include_directories(${CMAKE_CURRENT_LIST_DIR}/include ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

add_subdirectory(src/tests)
