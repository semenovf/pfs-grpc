project(pfs-io-tests CXX)

# This is the "object library" target: compiles the sources only once
add_library(CATCH_OBJLIB OBJECT catch-main.cpp)

# Optional: shared libraries need PIC
set_property(TARGET CATCH_OBJLIB PROPERTY POSITION_INDEPENDENT_CODE 1)
add_library(catch SHARED $<TARGET_OBJECTS:CATCH_OBJLIB>)

################################################################################
# Unit test targets
################################################################################
list(APPEND MY_TEST_TARGETS async)
set(async_SOURCES async.cpp)
set(async_PROTOBUF_SOURCES async.proto)

find_package(Threads REQUIRED)

#
# Process all unit test targets
#
foreach (MY_TEST_TARGET ${MY_TEST_TARGETS})
    set(PROTOBUF_INPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${MY_TEST_TARGET}")
    set(PROTOBUF_SOURCES ${${MY_TEST_TARGET}_PROTOBUF_SOURCES})
    include(${pfs_grpc_SOURCE_DIR}/gRPC.cmake)

    add_executable(test-${MY_TEST_TARGET} ${MY_TEST_TARGET}/${${MY_TEST_TARGET}_SOURCES} ${pfs_grpc_SOURCES})
    link_directories(${pfs_GRPC_LIBRARY_DIRS})
    target_include_directories(test-${MY_TEST_TARGET} PRIVATE ${pfs_grpc_INCLUDE_DIRS})
    target_link_libraries(test-${MY_TEST_TARGET} catch ${test-${MY_TEST_TARGET}_LIBS} ${pfs_grpc_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

    add_test(NAME ${MY_TEST_TARGET} COMMAND test-${MY_TEST_TARGET})
endforeach(MY_TEST_TARGET)
