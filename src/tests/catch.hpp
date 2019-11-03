#if __cplusplus < 201103L \
        || (defined __GNUC__  && ( __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 8)))
#    include "catch1.hpp"
#else
#    include "catch2.hpp"
#endif
