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

# OUTPUT VARIABLE: Path to gRPC codegen cmake file
set(pfs_grpc_CODEGEN "${pfs_grpc_SOURCE_DIR}/Generate_gRPC.cmake")

# OUTPUT VARIABLE: Path to Protobuf codegen cmake file
set(pfs_protobuf_CODEGEN "${pfs_grpc_SOURCE_DIR}/Generate_proto.cmake")

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

set_target_properties(protoc PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set_target_properties(grpc_cpp_plugin PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(pfs_grpc_PROTOC_BIN "${CMAKE_BINARY_DIR}/bin/${CMAKE_CFG_INTDIR}/protoc${CMAKE_EXECUTABLE_SUFFIX}")
set(pfs_grpc_CPP_PLUGIN_PATH "${CMAKE_BINARY_DIR}/bin/${CMAKE_CFG_INTDIR}/grpc_cpp_plugin${CMAKE_EXECUTABLE_SUFFIX}")
set(_pfs_grpc_GRPC_BINARY_DIR "${CMAKE_BINARY_DIR}/bin/${CMAKE_CFG_INTDIR}")

#
# Set paths for executables: `protoc` and `grpc_cpp_plugin`
#
#message(FATAL_ERROR "protoc=${_gRPC_PROTOBUF_PROTOC_EXECUTABLE}")
#set(ASDF $<TARGET_FILE:protoc>)
#message(FATAL_ERROR "protoc=[${ASDF}]")

#if (NOT ANDROID)
#	if (DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
#		set(_pfs_grpc_GRPC_BINARY_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
#	else()
#		set(_pfs_grpc_GRPC_BINARY_DIR "${CMAKE_BINARY_DIR}/${_pfs_grpc_GRPC_BINARY_SUBDIR}")
#	endif()
#
#	list(APPEND _pfs_grpc_PROTOC_BIN_COMPONENTS ${_pfs_grpc_GRPC_BINARY_DIR})
#	list(APPEND _pfs_grpc_CPP_PLUGIN_PATH_COMPONENTS ${_pfs_grpc_GRPC_BINARY_DIR})
#
#	if (MSVC)
#		list(APPEND _pfs_grpc_PROTOC_BIN_COMPONENTS "third_party/protobuf/${CMAKE_BUILD_TYPE}")
#		list(APPEND _pfs_grpc_CPP_PLUGIN_PATH_COMPONENTS ${CMAKE_BUILD_TYPE})
#	else()
#		list(APPEND _pfs_grpc_PROTOC_BIN_COMPONENTS "third_party/protobuf")
#	endif()
#
#	list(APPEND _pfs_grpc_PROTOC_BIN_COMPONENTS "protoc${CMAKE_EXECUTABLE_SUFFIX}")
#	list(APPEND _pfs_grpc_CPP_PLUGIN_PATH_COMPONENTS "grpc_cpp_plugin${CMAKE_EXECUTABLE_SUFFIX}")
#
#	list(JOIN _pfs_grpc_PROTOC_BIN_COMPONENTS "/" pfs_grpc_PROTOC_BIN)
#	list(JOIN _pfs_grpc_CPP_PLUGIN_PATH_COMPONENTS "/" pfs_grpc_CPP_PLUGIN_PATH)
if (ANDROID)
   if (NOT CUSTOM_PROTOC)
      set(pfs_grpc_PROTOC_BIN "${CMAKE_BINARY_DIR}/protoc${CMAKE_EXECUTABLE_SUFFIX}")
    else()
       set(pfs_grpc_PROTOC_BIN "${CUSTOM_PROTOC}")
    endif()

    if (NOT CUSTOM_GRPC_CPP_PLUGIN)
        set(pfs_grpc_CPP_PLUGIN_PATH "${CMAKE_BINARY_DIR}/grpc_cpp_plugin${CMAKE_EXECUTABLE_SUFFIX}")
    else()
        set(pfs_grpc_CPP_PLUGIN_PATH "${CUSTOM_GRPC_CPP_PLUGIN}")
    endif()

    set(_gRPC_PROTOBUF_PROTOC_EXECUTABLE ${pfs_grpc_PROTOC_BIN})
endif()

# OUTPUT VARIABLE: Protobuf compiler
set(pfs_protobuf_PROTOC_BIN ${pfs_grpc_PROTOC_BIN})

message(
"================================================================================
Protobuf compiler      : ${pfs_grpc_PROTOC_BIN}
gRPC source dir        : ${pfs_grpc_GRPC_SOURCE_DIR}
gRPC binary dir        : ${_pfs_grpc_GRPC_BINARY_DIR}
gRPC C++ plugin        : ${pfs_grpc_CPP_PLUGIN_PATH}
gRPC codegen script    : ${pfs_grpc_CODEGEN}
Protobuf codegen script: ${pfs_protobuf_CODEGEN}
================================================================================")
