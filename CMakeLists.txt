cmake_minimum_required (VERSION 3.1)
project(pfs-grpc)

set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
option(pfs_grpc_BUILD_TESTS "Build tests" ON)

# Submodules downloaded (allow use of last version of gRPC library and
# it's dependences)
if (EXISTS "${PROJECT_SOURCE_DIR}/3rdparty/grpc/CMakeLists.txt")
    set(pfs_grpc_GRPC_SOURCE_ROOT_DIR "${PROJECT_SOURCE_DIR}/3rdparty/grpc")
    set(pfs_grpc_GRPC_BINARY_ROOT_DIR "${PROJECT_BINARY_DIR}/3rdparty/grpc")
else()
    set(pfs_grpc_GRPC_SOURCE_ROOT_DIR "${PROJECT_SOURCE_DIR}/3rdparty/preloaded/grpc")
    set(pfs_grpc_GRPC_BINARY_ROOT_DIR "${PROJECT_BINARY_DIR}/3rdparty/preloaded/grpc")
endif()

set(pfs_grpc_PROTOC_BIN "${pfs_grpc_GRPC_BINARY_ROOT_DIR}/third_party/protobuf/protoc")
set(pfs_grpc_GRPC_CPP_PLUGIN_PATH "${pfs_grpc_GRPC_BINARY_ROOT_DIR}/grpc_cpp_plugin")

################################################################################
# Protobuf compiler version (informative purpose only)
################################################################################
# execute_process(COMMAND ${pfs_PROTOC_BIN} --version
#     OUTPUT_VARIABLE pfs_PROTOC_VERSION)
# string(STRIP ${pfs_PROTOC_VERSION} pfs_PROTOC_VERSION)
#

set(pfs_grpc_INCLUDE_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${pfs_grpc_GRPC_SOURCE_ROOT_DIR}/include
    ${pfs_grpc_GRPC_SOURCE_ROOT_DIR}/third_party/protobuf/src)

set(pfs_grpc_LIBRARY_DIRS
#     ${CMAKE_BINARY_DIR}/3rdparty/grpc
#     ${CMAKE_BINARY_DIR}/3rdparty/grpc/third_party/cares/cares/lib
#     ${CMAKE_BINARY_DIR}/3rdparty/grpc/third_party/protobuf
#     ${CMAKE_BINARY_DIR}/3rdparty/grpc/third_party/zlib
)

# Note: gRPC libraries with dependences:
#       grpc++_reflection grpc gpr address_sorting cares protobuf z
set(pfs_grpc_LIBRARIES grpc++)

message("
================================================================================
gRPC source dir  : ${pfs_grpc_GRPC_SOURCE_ROOT_DIR}
Protobuf compiler: ${pfs_grpc_PROTOC_BIN}
gRPC C++ plugin  : ${pfs_grpc_GRPC_CPP_PLUGIN_PATH}
================================================================================")

add_subdirectory(${pfs_grpc_GRPC_SOURCE_ROOT_DIR})

if (pfs_grpc_BUILD_TESTS)
    enable_testing()
    add_subdirectory(src/tests)
endif()
