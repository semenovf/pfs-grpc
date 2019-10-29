////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs](https://github.com/semenovf/pfs) library.
//
// Changelog:
//      2019.10.29 Initial version
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <type_traits>

namespace pfs {

//
//                           _________________AstraLinux 1.5 (GCC version 4.7.2)
//                           |
//                           v
//==============================================================================
// std::is_destructible    | + |
// std::integral_constant  | + |
// __and_                  | + |
// __has_trivial_destructor| + |
//==============================================================================

/// is_trivially_destructible
#if defined(__GNUC__)
template <typename T>
struct is_trivially_destructible : public std::__and_<std::is_destructible<T>
        , std::integral_constant<bool, __has_trivial_destructor(T)>>
{};
#endif // __GNUC__

} // pfs

namespace std {

#if PFS_HAVE_NOT_is_trivially_destructible

template <typename T>
using is_trivially_destructible = pfs::is_trivially_destructible;

#endif // HAVE_NOT_is_trivially_destructible
} // std
