################################################################################
# Check std::is_trivially_destructible
################################################################################
set(CMAKE_REQUIRED_FLAGS "-std=c++11")
CHECK_CXX_SOURCE_COMPILES(
"#include <type_traits>
#include <cassert>
int main ()
{
    assert(std::is_trivially_destructible<int>::value);
    return 0;
}" cxx11_is_trivially_destructible)

if (NOT cxx11_is_trivially_destructible)
    message(WARNING "W: C++ library does not support `std::is_trivially_destructible`")
    add_definitions("-DPFS_HAVE_NOT_is_trivially_destructible")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include \"${CMAKE_CURRENT_LIST_DIR}/../include/pfs/cxx/type_traits.hpp\"")
    endif()
endif()

