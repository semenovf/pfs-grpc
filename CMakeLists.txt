cmake_minimum_required (VERSION 3.1)
enable_testing()
include_directories(${CMAKE_CURRENT_LIST_DIR}/include ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

add_subdirectory(3rdparty/grpc)
add_subdirectory(src/tests)
