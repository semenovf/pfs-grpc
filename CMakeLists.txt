cmake_minimum_required (VERSION 3.5.1)
project(pfs-grpc)

set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
option(pfs_grpc_BUILD_TESTS "Build tests" ON)

include(Make_gRPC.cmake)

if (pfs_grpc_BUILD_TESTS)
    enable_testing()
    add_subdirectory(src/tests)
endif()
