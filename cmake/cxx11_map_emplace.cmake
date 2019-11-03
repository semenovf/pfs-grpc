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

if (NOT cxx11_map_emplace)
    message(WARNING "W: C++ library does not support `std::map::emplace`")
    add_definitions("-DPFS_HAVE_NOT_map_emplace")
endif()

