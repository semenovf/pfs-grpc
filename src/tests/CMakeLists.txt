project(pfs-io-tests CXX)

find_package(Threads REQUIRED)

# This is the "object library" target: compiles the sources only once
add_library(CATCH_OBJLIB OBJECT catch-main.cpp)

# Optional: shared libraries need PIC
set_property(TARGET CATCH_OBJLIB PROPERTY POSITION_INDEPENDENT_CODE 1)
add_library(catch SHARED $<TARGET_OBJECTS:CATCH_OBJLIB>)

Generate_gRPC(${CMAKE_CURRENT_SOURCE_DIR}/async
    PREFIX proto
    PROTOS async.proto)

add_executable(test-async async/async.cpp ${pfs_grpc_SOURCES})
link_directories(${pfs_grpc_LIBRARY_DIRS})
target_include_directories(test-async PRIVATE ${pfs_grpc_INCLUDE_DIRS})
target_link_libraries(test-async catch ${pfs_grpc_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME async COMMAND test-async)

# add_executable(${PROJECT_NAME} main.cpp ${pfs_grpc_SOURCES})
# link_directories(${pfs_grpc_LIBRARY_DIRS})
# target_include_directories(${PROJECT_NAME} PRIVATE ${pfs_grpc_INCLUDE_DIRS})
# target_link_libraries(${PROJECT_NAME} ${pfs_grpc_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

# list(APPEND MY_TEST_TARGETS async)
# set(async_SOURCES async.cpp)
# set(async_PROTOBUF_SOURCES async.proto)


#
# Process all unit test targets
#
# foreach (MY_TEST_TARGET ${MY_TEST_TARGETS})
#     set(PROTOBUF_INPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${MY_TEST_TARGET}")
#     set(PROTOBUF_PROTOS ${${MY_TEST_TARGET}_PROTOBUF_SOURCES})
#     include(${pfs_grpc_SOURCE_DIR}/gRPC.cmake)
#
#     add_executable(test-${MY_TEST_TARGET} ${MY_TEST_TARGET}/${${MY_TEST_TARGET}_SOURCES} ${pfs_grpc_SOURCES})
#     link_directories(${pfs_GRPC_LIBRARY_DIRS})
#     target_include_directories(test-${MY_TEST_TARGET} PRIVATE ${pfs_grpc_INCLUDE_DIRS})
#     target_link_libraries(test-${MY_TEST_TARGET} catch ${test-${MY_TEST_TARGET}_LIBS} ${pfs_grpc_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
#
#     add_test(NAME ${MY_TEST_TARGET} COMMAND test-${MY_TEST_TARGET})
# endforeach(MY_TEST_TARGET)

include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(test-type_traits type_traits.cpp catch)
target_link_libraries(test-type_traits catch)
add_test(NAME type_traits COMMAND test-type_traits)
