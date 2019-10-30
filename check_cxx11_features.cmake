include(CheckCXXSourceCompiles)

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

message("+++ cxx11_is_trivially_destructible: [${cxx11_is_trivially_destructible}]")

if (NOT cxx11_is_trivially_destructible)
    message(WARNING "--- C++ library does not support `std::is_trivially_destructible`")
    add_definitions("-DPFS_HAVE_NOT_is_trivially_destructible")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include \"${CMAKE_CURRENT_LIST_DIR}/include/pfs/cxx/type_traits.hpp\"")
endif()

################################################################################
# Check std::map::emplace
################################################################################
set(CMAKE_REQUIRED_FLAGS "-std=c++11")
CHECK_CXX_SOURCE_COMPILES(
"#include <utility>
#include <map>
int main ()
{
    std::map<int,int> m;
    m.emplace(std::make_pair(1,2));
    return 0;
}" cxx11_map_emplace)

message("+++ cxx11_map_emplace: [${cxx11_map_emplace}]")

if (NOT cxx11_map_emplace)
    message(WARNING "--- C++ library does not support `std::map::emplace`")
    add_definitions("-DPFS_HAVE_NOT_map_emplace")
endif()

################################################################################
# Check std::unordered_map::emplace
################################################################################
set(CMAKE_REQUIRED_FLAGS "-std=c++11")
CHECK_CXX_SOURCE_COMPILES(
"#include <utility>
#include <unordered_map>
int main ()
{
    std::unordered_map<int,int> m;
    m.emplace(std::make_pair(1,2));
    return 0;
}" cxx11_unordered_map_emplace)

message("+++ cxx11_unordered_map_emplace: [${cxx11_unordered_map_emplace}]")

if (NOT cxx11_unordered_map_emplace)
    message(WARNING "--- C++ library does not support `std::unordered_map::emplace`")
    add_definitions("-DPFS_HAVE_NOT_unordered_map_emplace")
endif()
