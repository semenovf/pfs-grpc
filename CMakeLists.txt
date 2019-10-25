cmake_minimum_required (VERSION 3.1)
enable_testing()
include_directories(${CMAKE_CURRENT_LIST_DIR}/include ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
option(pfs_GRPC_BUILD_TESTS "Build tests" OFF)

# Submodules downloaded (allow use of last version of gRPC library and
# it's dependences)
if (EXISTS 3rdparty/grpc/CMakeLists.txt)
    add_subdirectory(3rdparty/grpc)
else()
    add_subdirectory(3rdparty/preloaded/grpc)
endif()

if (pfs_GRPC_BUILD_TESTS)
    add_subdirectory(src/tests)
endif()
