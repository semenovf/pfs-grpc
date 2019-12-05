project(pfs-io-tests CXX)

find_package(Threads REQUIRED)

#set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# This is the "object library" target: compiles the sources only once
add_library(CATCH_OBJLIB OBJECT catch-main.cpp)

# Optional: shared libraries need PIC
set_property(TARGET CATCH_OBJLIB PROPERTY POSITION_INDEPENDENT_CODE 1)
add_library(catch SHARED $<TARGET_OBJECTS:CATCH_OBJLIB>)
# Exports all symbols from catch library
if (MSVC)
    set_target_properties(catch PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif(MSVC)

include(${pfs_grpc_CODEGEN})

Generate_gRPC(${CMAKE_CURRENT_SOURCE_DIR}/async
    PREFIX proto
    PROTOS async.proto)
add_custom_target(async_DEPENDS_TARGET DEPENDS ${pfs_grpc_SOURCES})
add_executable(test-async async/async.cpp ${pfs_grpc_SOURCES})
add_dependencies(test-async async_DEPENDS_TARGET)
link_directories(${pfs_grpc_LIBRARY_DIRS})
target_include_directories(test-async PRIVATE ${pfs_grpc_INCLUDE_DIRS})
target_link_libraries(test-async catch ${pfs_grpc_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME async COMMAND test-async)

Generate_gRPC(${CMAKE_CURRENT_SOURCE_DIR}/connection_recovery
    PREFIX proto
    PROTOS connection_recovery.proto)
add_custom_target(connection_recovery_DEPENDS_TARGET DEPENDS ${pfs_grpc_SOURCES})
add_executable(test-connection_recovery connection_recovery/connection_recovery.cpp ${pfs_grpc_SOURCES})
add_dependencies(test-connection_recovery connection_recovery_DEPENDS_TARGET)
link_directories(${pfs_grpc_LIBRARY_DIRS})
target_include_directories(test-connection_recovery PRIVATE ${pfs_grpc_INCLUDE_DIRS})
target_link_libraries(test-connection_recovery ${pfs_grpc_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME connection_recovery COMMAND test-connection_recovery)

include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(test-type_traits type_traits.cpp catch)
target_link_libraries(test-type_traits catch)
add_test(NAME type_traits COMMAND test-type_traits)
