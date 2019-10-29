include(CheckCXXSourceCompiles)

# String of compile command line flags
set(CMAKE_REQUIRED_FLAGS "-std=c++11")
CHECK_CXX_SOURCE_COMPILES(
"#include <type_traits>
#include <cassert>
int main ()
{
    assert(std::is_trivially_destructible<int>::value);
    return 0;
}" cxx_is_trivially_destructible)

message("+++ cxx_is_trivially_destructible: [${cxx_is_trivially_destructible}]")

if (${cxx_is_trivially_destructible})
    message("+++ C++ library supports `std::is_trivially_destructible`")
else()
    message("--- C++ library does not support `std::is_trivially_destructible`")
endif()
