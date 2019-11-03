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

if (NOT cxx11_unordered_map_emplace)
    message(WARNING "W: C++ library does not support `std::unordered_map::emplace`")
    add_definitions("-DPFS_HAVE_NOT_unordered_map_emplace")
endif()

