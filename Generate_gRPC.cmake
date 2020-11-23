################################################################################
# Copyright (c) 2019 Vladislav Trifochkin
#
# This file is part of [pfs-grpc](https://github.com/semenovf/pfs-grpc) library.
#
################################################################################

include(CMakeParseArguments)

#!
# @function Generate_gRPC(aPROTOBUF_INPUT_DIRECTORY PROTOS aPROTOBUF_PROTOS...)
# @param aPROTOBUF_INPUT_DIRECTORY
#        Directory where proto definition files are located
# @param aPROTOBUF_PROTOS
#        Proto definition filenames
#
function (Generate_gRPC)
    set(boolparm)
    set(singleparm PREFIX DLL_API_MACRO)
    set(multiparm PROTOS)

    cmake_parse_arguments(_arg "${boolparm}" "${singleparm}" "${multiparm}" ${ARGN})

    if (NOT _arg_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Generate_gRPC: Protobuf Input Directory must be specified")
    endif()

    if (NOT _arg_PREFIX)
        set(_arg_PREFIX proto)
    endif()

    set(_pfs_grpc_PROTO_DIRECTORY ${_arg_UNPARSED_ARGUMENTS})
    set(_pfs_grpc_SOURCES_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${_arg_PREFIX}")

    # Clear output variable
    set(pfs_grpc_SOURCES)
    set(pfs_grpc_INCLUDE_DIRS)
    set(pfs_grpc_LIBRARY_DIRS)
    set(pfs_grpc_LIBRARIES)

    foreach (_pfs_grpc_proto ${_arg_PROTOS})
        get_filename_component(_pfs_grpc_basename ${_pfs_grpc_proto} NAME_WE)

        list(APPEND _pfs_grpc_PROTOS "${_pfs_grpc_PROTO_DIRECTORY}/${_pfs_grpc_proto}")

        list(APPEND _pfs_protobuf_OUTPUT "${_pfs_grpc_SOURCES_DIRECTORY}/${_pfs_grpc_basename}.pb.cc")
        list(APPEND _pfs_protobuf_OUTPUT "${_pfs_grpc_SOURCES_DIRECTORY}/${_pfs_grpc_basename}.pb.h")
        list(APPEND _pfs_grpc_OUTPUT "${_pfs_grpc_SOURCES_DIRECTORY}/${_pfs_grpc_basename}.grpc.pb.cc")
        list(APPEND _pfs_grpc_OUTPUT "${_pfs_grpc_SOURCES_DIRECTORY}/${_pfs_grpc_basename}.grpc.pb.h")

        list(APPEND pfs_grpc_SOURCES "${_pfs_grpc_SOURCES_DIRECTORY}/${_pfs_grpc_basename}.pb.cc")
        list(APPEND pfs_grpc_SOURCES "${_pfs_grpc_SOURCES_DIRECTORY}/${_pfs_grpc_basename}.grpc.pb.cc")
    endforeach()

    # OUTPUT VARIABLE: Generated sources
    set(pfs_grpc_SOURCES ${pfs_grpc_SOURCES})

    ################################################################################
    # Create Protobuf output directory
    ################################################################################
    file(MAKE_DIRECTORY ${_pfs_grpc_SOURCES_DIRECTORY})

    ################################################################################
    # Generate gRPC-specific source codes
    ################################################################################
    add_custom_command(COMMAND ${pfs_grpc_PROTOC_BIN}
            --proto_path=\"${_pfs_grpc_PROTO_DIRECTORY}\"
            --grpc_out=\"${_pfs_grpc_SOURCES_DIRECTORY}\"
            --plugin=protoc-gen-grpc=\"${pfs_grpc_CPP_PLUGIN}\"
            ${_arg_PROTOS}
        OUTPUT ${_pfs_grpc_OUTPUT}
        DEPENDS ${pfs_grpc_CPP_PLUGIN} ${_pfs_grpc_PROTOS})

    ################################################################################
    # Generate Protobuf-specific source codes
    ################################################################################
    if(_arg_DLL_API_MACRO)
        set(_pfs_protobuf_CPP_OUT "dllexport_decl=${_arg_DLL_API_MACRO}:${_pfs_grpc_SOURCES_DIRECTORY}")
    else()
        set(_pfs_protobuf_CPP_OUT ${_pfs_grpc_SOURCES_DIRECTORY})
    endif()

    if(ANDROID)
    endif(ANDROID)

    add_custom_command(COMMAND ${pfs_grpc_PROTOC_BIN}
            --proto_path=\"${_pfs_grpc_PROTO_DIRECTORY}\"
            --cpp_out=\"${_pfs_protobuf_CPP_OUT}\"
            ${_arg_PROTOS}
        OUTPUT ${_pfs_protobuf_OUTPUT}
        DEPENDS ${pfs_grpc_PROTOC_BIN} ${_pfs_grpc_PROTOS})

    # OUTPUT VARIABLE: Include directories
    set(pfs_grpc_INCLUDE_DIRS
        ${pfs_grpc_SOURCE_DIR}/include
        ${pfs_grpc_GRPC_SOURCE_DIR}/include
        ${pfs_grpc_GRPC_SOURCE_DIR}/third_party/protobuf/src
        ${_pfs_grpc_SOURCES_DIRECTORY})

    # OUTPUT VARIABLE: Libraries directories
    set(pfs_grpc_LIBRARY_DIRS
        ${CMAKE_BINARY_DIR}/3rdparty/grpc
        ${CMAKE_BINARY_DIR}/3rdparty/grpc/third_party/protobuf
    #     ${CMAKE_BINARY_DIR}/3rdparty/grpc/third_party/cares/cares/lib
    #     ${CMAKE_BINARY_DIR}/3rdparty/grpc/third_party/zlib
    )

    # OUTPUT VARIABLE: Libraries
    # Note: gRPC libraries with dependences:
    #       grpc++_reflection grpc gpr address_sorting cares protobuf z
    set(pfs_grpc_LIBRARIES grpc++)

    set(${_arg_PREFIX}_PROTO_SOURCES ${pfs_grpc_SOURCES} PARENT_SCOPE)
    set(${_arg_PREFIX}_PROTO_INCLUDE_DIRS ${pfs_grpc_INCLUDE_DIRS} PARENT_SCOPE)
    set(${_arg_PREFIX}_PROTO_LIB_DIRS ${pfs_grpc_LIBRARY_DIRS} PARENT_SCOPE)
    set(${_arg_PREFIX}_PROTO_LIBS ${pfs_grpc_LIBRARIES} PARENT_SCOPE)
endfunction(Generate_gRPC)
