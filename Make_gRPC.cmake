################################################################################
# Copyright (c) 2019 Vladislav Trifochkin
#
# This file is part of [pfs-grpc](https://github.com/semenovf/pfs-grpc) library.
#
################################################################################

################################################################################
#
# This file must be included once to build gRPC library with dependences and set
# global variables (see 'OUTPUT VARIABLE' prefixed comments).
# This sctipt choose appropriate version of gRPC: preloaded (with modifications
# to use with GCCC 4.7.2) or loaded as submodule
#
################################################################################

cmake_minimum_required (VERSION 3.5.1) # Minimal version for gRPC

# Workaround for GCC 4.7.2
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
    include(CheckCXXSourceCompiles)
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/cxx_gxx_permissive.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/cxx11_is_trivially_destructible.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/cxx11_map_emplace.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/cxx11_unordered_map_emplace.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/cxx11_gcc_47_compiler_error_1035.cmake)
endif()

# OUTPUT VARIABLE: pfs-grpc source directory
set(pfs_grpc_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}")

# OUTPUT VARIABLE: root pfs-grpc binary output directory
set(pfs_grpc_BINARY_DIR "${CMAKE_BINARY_DIR}/pfs-grpc")

# Submodules downloaded (allow use of last version of gRPC library and
# it's dependences)
if (EXISTS "${pfs_grpc_SOURCE_DIR}/3rdparty/grpc/CMakeLists.txt")
    # OUTPUT VARIABLE: root gRPC source directory
    set(pfs_grpc_GRPC_SOURCE_DIR "${pfs_grpc_SOURCE_DIR}/3rdparty/grpc")

    set(_pfs_grpc_GRPC_BINARY_SUBDIR "pfs-grpc/3rdparty/grpc")
else()
    # OUTPUT VARIABLE: root gRPC source directory
    set(pfs_grpc_GRPC_SOURCE_DIR "${pfs_grpc_SOURCE_DIR}/3rdparty/preloaded/grpc")

    set(_pfs_grpc_GRPC_BINARY_SUBDIR "pfs-grpc/3rdparty/preloaded/grpc")
endif()

if (NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(_pfs_grpc_GRPC_BINARY_DIR "${CMAKE_BINARY_DIR}/${_pfs_grpc_GRPC_BINARY_SUBDIR}")

    # OUTPUT VARIABLE: Protobuf compiler
    set(pfs_grpc_PROTOC_BIN "${_pfs_grpc_GRPC_BINARY_DIR}/third_party/protobuf/protoc")

    # OUTPUT VARIABLE: gRPC C++ plugin path
    set(pfs_grpc_CPP_PLUGIN_PATH "${_pfs_grpc_GRPC_BINARY_DIR}/grpc_cpp_plugin")
else()
    set(_pfs_grpc_GRPC_BINARY_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

    # OUTPUT VARIABLE: Protobuf compiler
    set(pfs_grpc_PROTOC_BIN "${_pfs_grpc_GRPC_BINARY_DIR}/protoc")

    # OUTPUT VARIABLE: gRPC C++ plugin path
    set(pfs_grpc_CPP_PLUGIN_PATH "${_pfs_grpc_GRPC_BINARY_DIR}/grpc_cpp_plugin")
endif()

if (ANDROID)
    if (NOT CUSTOM_PROTOC)
        set(pfs_grpc_PROTOC_BIN "${CMAKE_BINARY_DIR}/protoc")
    else()
        set(pfs_grpc_PROTOC_BIN "${CUSTOM_PROTOC}")
    endif()

    if (NOT CUSTOM_GRPC_CPP_PLUGIN)
        set(pfs_grpc_CPP_PLUGIN_PATH "${CMAKE_BINARY_DIR}/grpc_cpp_plugin")
    else()
        set(pfs_grpc_CPP_PLUGIN_PATH "${CUSTOM_GRPC_CPP_PLUGIN}")
    endif()

    set(_gRPC_PROTOBUF_PROTOC_EXECUTABLE ${pfs_grpc_PROTOC_BIN})
endif(ANDROID)

# OUTPUT VARIABLE: Protobuf compiler
set(pfs_protobuf_PROTOC_BIN ${pfs_grpc_PROTOC_BIN})

# OUTPUT VARIABLE: Path to gRPC codegen cmake file
set(pfs_grpc_CODEGEN "${pfs_grpc_SOURCE_DIR}/Generate_gRPC.cmake")

# OUTPUT VARIABLE: Path to Protobuf codegen cmake file
set(pfs_protobuf_CODEGEN "${pfs_grpc_SOURCE_DIR}/Generate_proto.cmake")

################################################################################
# Protobuf compiler version (informative purpose only)
################################################################################
# execute_process(COMMAND ${pfs_PROTOC_BIN} --version
#     OUTPUT_VARIABLE pfs_PROTOC_VERSION)
# string(STRIP ${pfs_PROTOC_VERSION} pfs_PROTOC_VERSION)
#

message(
"================================================================================
Protobuf compiler      : ${pfs_grpc_PROTOC_BIN}
gRPC source dir        : ${pfs_grpc_GRPC_SOURCE_DIR}
gRPC binary dir        : ${_pfs_grpc_GRPC_BINARY_DIR}
gRPC C++ plugin        : ${pfs_grpc_CPP_PLUGIN_PATH}
gRPC codegen script    : ${pfs_grpc_CODEGEN}
Protobuf codegen script: ${pfs_protobuf_CODEGEN}
================================================================================")

add_subdirectory(${pfs_grpc_GRPC_SOURCE_DIR} ${_pfs_grpc_GRPC_BINARY_SUBDIR})

# Workaround for GCC 4.7.2
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
    target_compile_definitions(gpr PRIVATE "-D__STDC_LIMIT_MACROS")
    target_compile_definitions(grpc PRIVATE "-D__STDC_LIMIT_MACROS")
    target_compile_definitions(grpc_cronet PRIVATE "-D__STDC_LIMIT_MACROS")
    target_compile_definitions(grpc_unsecure PRIVATE "-D__STDC_LIMIT_MACROS")
    target_compile_definitions(grpc++ PRIVATE "-D__STDC_LIMIT_MACROS")
    target_compile_definitions(grpc++_unsecure PRIVATE "-D__STDC_LIMIT_MACROS")
endif()
