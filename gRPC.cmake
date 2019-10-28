################################################################################
# Input variables
#-------------------------------------------------------------------------------
# PROTOBUF_INPUT_DIRECTORY
#       Directory where proto definition files are located
# PROTOBUF_SOURCES
#       Proto definition filenames
#===============================================================================
# Output variables
#-------------------------------------------------------------------------------
# pfs_grpc_INCLUDE_DIRS
#       include directories for gRPC-specific libraries
# pfs_grpc_LIBRARY_DIRS
#       Link directories for gRPC-specific libraries
# pfs_grpc_LIBRARIES
#       gRPC-specific libraries to be linked
# pfs_grpc_SOURCES
#       Sources generated by Protobuf
################################################################################

set(pfs_grpc_PROTOBUF_INPUT_DIRECTORY ${PROTOBUF_INPUT_DIRECTORY})
set(pfs_grpc_PROTOBUF_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/proto")

foreach (_pfs_grpc_proto_src ${PROTOBUF_SOURCES})
    get_filename_component(_pfs_grpc_proto_basename ${_pfs_grpc_proto_src} NAME_WE)

    list(APPEND pfs_grpc_PROTOBUF_SOURCES "${PROTOBUF_INPUT_DIRECTORY}/${_pfs_grpc_proto_src}")
    list(APPEND pfs_grpc_PROTOBUF_OUTPUT "${pfs_grpc_PROTOBUF_OUTPUT_DIRECTORY}/${_pfs_grpc_proto_basename}.pb.cc")
    list(APPEND pfs_grpc_PROTOBUF_OUTPUT "${pfs_grpc_PROTOBUF_OUTPUT_DIRECTORY}/${_pfs_grpc_proto_basename}.pb.h")
    list(APPEND pfs_grpc_GRPC_PROTOBUF_OUTPUT "${pfs_grpc_PROTOBUF_OUTPUT_DIRECTORY}/${_pfs_grpc_proto_basename}.grpc.pb.cc")
    list(APPEND pfs_grpc_GRPC_PROTOBUF_OUTPUT "${pfs_grpc_PROTOBUF_OUTPUT_DIRECTORY}/${_pfs_grpc_proto_basename}.grpc.pb.h")

    # Output variables here
    list(APPEND pfs_grpc_SOURCES "${pfs_grpc_PROTOBUF_OUTPUT_DIRECTORY}/${_pfs_grpc_proto_basename}.pb.cc")
    list(APPEND pfs_grpc_SOURCES "${pfs_grpc_PROTOBUF_OUTPUT_DIRECTORY}/${_pfs_grpc_proto_basename}.grpc.pb.cc")
endforeach()

################################################################################
# Generate gRPC-specific source codes
################################################################################
add_custom_command(COMMAND ${pfs_grpc_PROTOC_BIN}
        --proto_path=\"${pfs_grpc_PROTOBUF_INPUT_DIRECTORY}\"
        --grpc_out=\"${pfs_grpc_PROTOBUF_OUTPUT_DIRECTORY}\"
        --plugin=protoc-gen-grpc=\"${pfs_grpc_GRPC_CPP_PLUGIN_PATH}\"
        ${pfs_grpc_PROTOBUF_SOURCES}
    OUTPUT ${pfs_grpc_GRPC_PROTOBUF_OUTPUT}
    DEPENDS ${pfs_grpc_PROTOBUF_SOURCES})

################################################################################
# Generate Protobuf-specific source codes
################################################################################
add_custom_command(COMMAND ${pfs_grpc_PROTOC_BIN}
        --proto_path=\"${pfs_grpc_PROTOBUF_INPUT_DIRECTORY}\"
        --cpp_out=\"${pfs_grpc_PROTOBUF_OUTPUT_DIRECTORY}\"
        ${pfs_grpc_PROTOBUF_SOURCES}
    OUTPUT ${pfs_grpc_PROTOBUF_OUTPUT}
    DEPENDS ${pfs_grpc_PROTOBUF_SOURCES})

list(APPEND pfs_grpc_INCLUDE_DIRS ${pfs_grpc_PROTOBUF_OUTPUT_DIRECTORY})