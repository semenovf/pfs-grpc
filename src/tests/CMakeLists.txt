project(pfs-io-tests CXX)

# This is the "object library" target: compiles the sources only once
add_library(CATCH_OBJLIB OBJECT catch-main.cpp)
# Optional: shared libraries need PIC
set_property(TARGET CATCH_OBJLIB PROPERTY POSITION_INDEPENDENT_CODE 1)

################################################################################
# Unit test targets
################################################################################
list(APPEND MY_TEST_TARGETS async)

add_library(catch SHARED $<TARGET_OBJECTS:CATCH_OBJLIB>)

find_package(Threads REQUIRED)

#
# Process all unit test targets
#
foreach (MY_TEST_TARGET ${MY_TEST_TARGETS})
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/bunch/${MY_TEST_TARGET}.cpp")
        add_executable(test-${MY_TEST_TARGET} "${CMAKE_CURRENT_SOURCE_DIR}/bunch/${MY_TEST_TARGET}.cpp")
    elseif (test-${MY_TEST_TARGET}_SOURCES)
        add_executable(test-${MY_TEST_TARGET} ${test-${MY_TEST_TARGET}_SOURCES})
    else()
        add_executable(test-${MY_TEST_TARGET} ${MY_TEST_TARGET}/test.cpp)
    endif()

    if(test-${MY_TEST_TARGET}_LIBS)
        target_link_libraries(test-${MY_TEST_TARGET} catch ${test-${MY_TEST_TARGET}_LIBS} ${CMAKE_THREAD_LIBS_INIT})
    else()
        target_link_libraries(test-${MY_TEST_TARGET} catch ${CMAKE_THREAD_LIBS_INIT})
    endif()

    add_test(NAME ${MY_TEST_TARGET} COMMAND test-${MY_TEST_TARGET})
endforeach(MY_TEST_TARGET)
