################################################################################
# Check std::is_trivially_destructible
#
# Workaround of g++ error:
# ‘<::’ cannot begin a template-argument list [-fpermissive]
# note: ‘<:’ is an alternate spelling for ‘[’. Insert whitespace between ‘<’and ‘::’
# (if you use ‘-fpermissive’ G++ will accept your code)
################################################################################
CHECK_CXX_SOURCE_COMPILES(
"
namespace ns { typedef int type; }

template <typename I>
struct X { I n; };

int main ()
{
    X<::ns::type> x;
    x.n = 0;
    return 0;
}" cxx_gxx_permissive)

if (NOT cxx_gxx_permissive)
    message(WARNING "W: Template-argument cannot begin with '<::' is not default compiler behaviour")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
    endif()
endif()
