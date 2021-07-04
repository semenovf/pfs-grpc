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

option(FORCE_PRELOADED_GRPC "Force process preloaded version of gRPC" OFF)

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
if (NOT FORCE_PRELOADED_GRPC AND EXISTS "${pfs_grpc_SOURCE_DIR}/3rdparty/grpc/CMakeLists.txt")
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

if (ANDROID)
   if (NOT CUSTOM_PROTOC)
      set(pfs_grpc_PROTOC_BIN "${CMAKE_BINARY_DIR}/protoc${CMAKE_EXECUTABLE_SUFFIX}")
    else()
       set(pfs_grpc_PROTOC_BIN "${CUSTOM_PROTOC}")
    endif()

    if (NOT CUSTOM_GRPC_CPP_PLUGIN)
        set(pfs_grpc_CPP_PLUGIN "${CMAKE_BINARY_DIR}/grpc_cpp_plugin${CMAKE_EXECUTABLE_SUFFIX}")
    else()
        set(pfs_grpc_CPP_PLUGIN "${CUSTOM_GRPC_CPP_PLUGIN}")
    endif()

    set(_gRPC_PROTOBUF_PROTOC_EXECUTABLE ${pfs_grpc_PROTOC_BIN})
endif()

if (MSVC)
    # Build with multiple processes
    #add_definitions(/MP) # --eugene--
    # MSVC warning suppressions
    add_definitions(
        /wd4018 # 'expression' : signed/unsigned mismatch
        /wd4800 # 'type' : forcing value to bool 'true' or 'false' (performance warning)
        /wd4146 # unary minus operator applied to unsigned type, result still unsigned
        /wd4334 # 'operator' : result of 32-bit shift implicitly converted to 64 bits (was 64-bit shift intended?)
        /wd4065 # switch statement contains 'default' but no 'case' labels
        /wd4244 # 'conversion' conversion from 'type1' to 'type2', possible loss of data
        /wd4251 # 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
        /wd4267 # 'var' : conversion from 'size_t' to 'type', possible loss of data
        /wd4305 # 'identifier' : truncation from 'type1' to 'type2'
        /wd4307 # 'operator' : integral constant overflow
        /wd4309 # 'conversion' : truncation of constant value
        /wd4355 # 'this' : used in base member initializer list
        /wd4506 # no definition for inline function 'function'
        /wd4996 # The compiler encountered a deprecated declaration.
        /wd5208 # unnamed class used in typedef name cannot declare members other than non-static data members, member enumerations, or member classes
    )
endif(MSVC)

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

# Workaround for ANDROID (boringssl)
if (ANDROID)
    target_compile_options(crypto_base PRIVATE "-Wno-implicit-fallthrough")
    target_compile_options(cipher_extra PRIVATE "-Wno-implicit-fallthrough")
    target_compile_options(bio PRIVATE "-Wno-implicit-fallthrough")
    target_compile_options(asn1 PRIVATE "-Wno-implicit-fallthrough")
    target_compile_options(cast PRIVATE "-Wno-implicit-fallthrough")
    target_compile_options(fipsmodule PRIVATE "-Wno-implicit-fallthrough")
    target_compile_options(blowfish PRIVATE "-Wno-implicit-fallthrough")
    target_compile_options(des_decrepit PRIVATE "-Wno-implicit-fallthrough")
endif()

if (NOT ANDROID)
    if (CMAKE_RUNTIME_OUTPUT_DIRECTORY)
        set(pfs_grpc_PROTOC_BIN "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/protoc${CMAKE_EXECUTABLE_SUFFIX}")
        set(pfs_grpc_CPP_PLUGIN "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/grpc_cpp_plugin${CMAKE_EXECUTABLE_SUFFIX}")
    else()
        set_target_properties(protoc PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
        set_target_properties(grpc_cpp_plugin PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

        set(pfs_grpc_PROTOC_BIN "${CMAKE_BINARY_DIR}/bin/${CMAKE_CFG_INTDIR}/protoc${CMAKE_EXECUTABLE_SUFFIX}")
        set(pfs_grpc_CPP_PLUGIN "${CMAKE_BINARY_DIR}/bin/${CMAKE_CFG_INTDIR}/grpc_cpp_plugin${CMAKE_EXECUTABLE_SUFFIX}")
    endif()
endif()

# OUTPUT VARIABLE: Protobuf compiler
set(pfs_protobuf_PROTOC_BIN ${pfs_grpc_PROTOC_BIN})

message(STATUS "Protobuf compiler      : ${pfs_grpc_PROTOC_BIN}")
message(STATUS "gRPC source dir        : ${pfs_grpc_GRPC_SOURCE_DIR}")
message(STATUS "gRPC C++ plugin        : ${pfs_grpc_CPP_PLUGIN}")
message(STATUS "gRPC codegen script    : ${pfs_grpc_CODEGEN}")
message(STATUS "Protobuf codegen script: ${pfs_protobuf_CODEGEN}")
