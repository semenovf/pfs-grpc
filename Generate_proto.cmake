################################################################################
# Copyright (c) 2019 Vladislav Trifochkin
#
# This file is part of [pfs-grpc](https://github.com/semenovf/pfs-grpc) library.
#
################################################################################

#!
# @function Generate_proto(aPROTOBUF_INPUT_DIRECTORY PROTOS aPROTOBUF_PROTOS...)
# @param aPROTOBUF_INPUT_DIRECTORY
#        Directory where proto definition files are located
# @param aPROTOBUF_PROTOS
#        Proto definition filenames
#
function(Generate_proto)
    set(boolparm)
    set(singleparm PREFIX DLL_API)
    set(multiparm PROTOS)

    cmake_parse_arguments(_arg "${boolparm}" "${singleparm}" "${multiparm}" ${ARGN})

    if (NOT _arg_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Generate_gRPC: Protobuf Input Directory must be specified")
    endif()

    if (NOT _arg_PREFIX)
        set(_arg_PREFIX proto)
    endif()

    set(_pfs_protobuf_PROTO_DIRECTORY ${_arg_UNPARSED_ARGUMENTS})
    set(_pfs_protobuf_SOURCES_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${_arg_PREFIX}")

    # Clear output variable
    set(pfs_protobuf_SOURCES)
    set(pfs_protobuf_INCLUDE_DIRS)
    set(pfs_protobuf_LIBRARY_DIRS)
    set(pfs_protobuf_LIBRARIES)

    foreach (_pfs_protobuf_proto ${_arg_PROTOS})
        get_filename_component(_pfs_protobuf_basename ${_pfs_protobuf_proto} NAME_WE)

        list(APPEND _pfs_protobuf_PROTOS "${_pfs_protobuf_PROTO_DIRECTORY}/${_pfs_protobuf_proto}")
        list(APPEND _pfs_protobuf_OUTPUT "${_pfs_protobuf_SOURCES_DIRECTORY}/${_pfs_protobuf_basename}.pb.cc")
        list(APPEND _pfs_protobuf_OUTPUT "${_pfs_protobuf_SOURCES_DIRECTORY}/${_pfs_protobuf_basename}.pb.h")

        list(APPEND pfs_protobuf_SOURCES "${_pfs_protobuf_SOURCES_DIRECTORY}/${_pfs_protobuf_basename}.pb.cc")
    endforeach()

    # OUTPUT VARIABLE: Generated sources
    set(pfs_protobuf_SOURCES ${pfs_protobuf_SOURCES} PARENT_SCOPE)

    ################################################################################
    # Create Protobuf output directory
    ################################################################################
    file(MAKE_DIRECTORY ${_pfs_protobuf_SOURCES_DIRECTORY})

    ################################################################################
    # Generate Protobuf-specific source codes
    ################################################################################
    if (_arg_DLL_API)
        set(_pfs_protobuf_CPP_OUT "dllexport_decl=${_arg_DLL_API}:${_pfs_protobuf_SOURCES_DIRECTORY}")
    else(_arg_DLL_API)
        set(_pfs_protobuf_CPP_OUT ${_pfs_protobuf_SOURCES_DIRECTORY})
    endif(_arg_DLL_API)

    add_custom_command(COMMAND ${pfs_protobuf_PROTOC_BIN}
            --proto_path=\"${_pfs_protobuf_PROTO_DIRECTORY}\"
            --cpp_out=\"${_pfs_protobuf_CPP_OUT}\"
            ${_arg_PROTOS}
        OUTPUT ${_pfs_protobuf_OUTPUT}
        DEPENDS ${pfs_protobuf_PROTOC_BIN} ${_pfs_protobuf_PROTOS})

    # OUTPUT VARIABLE: Include directories
    set(pfs_protobuf_INCLUDE_DIRS
        ${pfs_grpc_GRPC_SOURCE_DIR}/third_party/protobuf/src
        ${_pfs_protobuf_SOURCES_DIRECTORY}
        PARENT_SCOPE)

    # OUTPUT VARIABLE: Libraries directories
    set(pfs_protobuf_LIBRARY_DIRS
        ${CMAKE_BINARY_DIR}/3rdparty/grpc/third_party/protobuf
        PARENT_SCOPE)

    set(pfs_protobuf_LIBRARIES libprotobuf PARENT_SCOPE)
endfunction(Generate_proto)
