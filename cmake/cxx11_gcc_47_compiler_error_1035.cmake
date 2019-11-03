################################################################################
# GCC 4.7.2 compiler issue
#-------------------------------------------------------------------------------
# gcc_internal_error.cpp: In lambda function:
# gcc_internal_error.cpp:24:1: internal compiler error: in get_expr_operands, at tree-ssa-operands.c:1035
# Please submit a full bug report,
# with preprocessed source if appropriate.
# See <file:///usr/share/doc/gcc-4.7/README.Bugs> for instructions.
#-------------------------------------------------------------------------------
# It's due to:
#       * calling a member function from a lambda in a templated class.
#       * calling a templated member function from a lambda.
# Fixed: http://gcc.gnu.org/bugzilla/show_bug.cgi?id=55538
################################################################################
set(CMAKE_REQUIRED_FLAGS "-std=c++11")
CHECK_CXX_SOURCE_COMPILES(
"template <typename T>
void foo (T &) {}

struct Bar
{
    template <typename T>
    int & get ();

    template <typename Func>
    void bar (Func c) { c(); }

    template <typename T>
    void baz (T const &)
    {
        bar([this] () { foo(get<T>()); });
    }

    void boo (int x);
};

void Bar::boo (int x)
{
    baz(x);
}" cxx11_gcc_47_compiler_error_1035)

if (NOT cxx11_gcc_47_compiler_error_1035)
    message(WARNING
        "W: It's seems your compiler has bug (known for gcc version 4.7.2):\n"
        "W: `internal compiler error: in get_expr_operands, at tree-ssa-operands.c:1035`\n"
        "W: It is recommended to upgrade your compiler.")
    add_definitions("-DPFS_GCC_47_COMPILER_ERROR_1035")
endif()
