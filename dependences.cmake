cmake_minimum_required (VERSION 3.5.1) # Minimal version for gRPC

set(pfs_grpc_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(pfs_grpc_BINARY_DIR "${CMAKE_BINARY_DIR}/pfs-grpc")

# Submodules downloaded (allow use of last version of gRPC library and
# it's dependences)
if (EXISTS "${pfs_grpc_SOURCE_DIR}/3rdparty/grpc/CMakeLists.txt")
    set(pfs_grpc_GRPC_SOURCE_DIR "${pfs_grpc_SOURCE_DIR}/3rdparty/grpc")
    set(pfs_grpc_GRPC_BINARY_SUBDIR "pfs-grpc/3rdparty/grpc")
else()
    set(pfs_grpc_GRPC_SOURCE_DIR "${pfs_grpc_SOURCE_DIR}/3rdparty/preloaded/grpc")
    set(pfs_grpc_GRPC_BINARY_SUBDIR "pfs-grpc/3rdparty/preloaded/grpc")
endif()

if (NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(pfs_grpc_GRPC_BINARY_DIR "${CMAKE_BINARY_DIR}/${pfs_grpc_GRPC_BINARY_SUBDIR}")
    set(pfs_grpc_PROTOC_BIN "${pfs_grpc_GRPC_BINARY_DIR}/third_party/protobuf/protoc")
    set(pfs_grpc_GRPC_CPP_PLUGIN_PATH "${pfs_grpc_GRPC_BINARY_DIR}/grpc_cpp_plugin")
else()
    set(pfs_grpc_GRPC_BINARY_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(pfs_grpc_PROTOC_BIN "${pfs_grpc_GRPC_BINARY_DIR}/protoc")
    set(pfs_grpc_GRPC_CPP_PLUGIN_PATH "${pfs_grpc_GRPC_BINARY_DIR}/grpc_cpp_plugin")
endif()

set(pfs_grpc_GRPC_CMAKE "${pfs_grpc_SOURCE_DIR}/gRPC.cmake")

################################################################################
# Protobuf compiler version (informative purpose only)
################################################################################
# execute_process(COMMAND ${pfs_PROTOC_BIN} --version
#     OUTPUT_VARIABLE pfs_PROTOC_VERSION)
# string(STRIP ${pfs_PROTOC_VERSION} pfs_PROTOC_VERSION)
#

set(pfs_grpc_INCLUDE_DIRS
    ${pfs_grpc_SOURCE_DIR}/include
    ${pfs_grpc_GRPC_SOURCE_DIR}/include
    ${pfs_grpc_GRPC_SOURCE_DIR}/third_party/protobuf/src)

set(pfs_grpc_LIBRARY_DIRS
#     ${CMAKE_BINARY_DIR}/3rdparty/grpc
#     ${CMAKE_BINARY_DIR}/3rdparty/grpc/third_party/cares/cares/lib
#     ${CMAKE_BINARY_DIR}/3rdparty/grpc/third_party/protobuf
#     ${CMAKE_BINARY_DIR}/3rdparty/grpc/third_party/zlib
)

# Note: gRPC libraries with dependences:
#       grpc++_reflection grpc gpr address_sorting cares protobuf z
set(pfs_grpc_LIBRARIES grpc++)

message(
"================================================================================
Protobuf compiler: ${pfs_grpc_PROTOC_BIN}
gRPC source dir  : ${pfs_grpc_GRPC_SOURCE_DIR}
gRPC binary dir  : ${pfs_grpc_GRPC_BINARY_DIR}
gRPC C++ plugin  : ${pfs_grpc_GRPC_CPP_PLUGIN_PATH}
gRPC.cmake path  : ${pfs_grpc_GRPC_CMAKE}
================================================================================")

add_subdirectory(${pfs_grpc_GRPC_SOURCE_DIR} ${pfs_grpc_GRPC_BINARY_SUBDIR})
