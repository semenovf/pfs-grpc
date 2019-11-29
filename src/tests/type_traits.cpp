////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-grpc](https://github.com/semenovf/pfs-grpc) library.
//
// Changelog:
//      2019.10.29 Initial version
////////////////////////////////////////////////////////////////////////////////
#include "pfs/cxx/type_traits.hpp"
#include "catch.hpp"

struct Foo { ~Foo() {} };
struct Bar { ~Bar() = default; };

#if PFS_HAVE_NOT_is_trivially_destructible

TEST_CASE("is_trivially_destructible") {
    CHECK(pfs::is_trivially_destructible<int>::value);
    CHECK(pfs::is_trivially_destructible<Bar>::value);
    CHECK(pfs::is_trivially_destructible<Foo &>::value);
    CHECK(pfs::is_trivially_destructible<Foo *>::value);
    CHECK_FALSE(pfs::is_trivially_destructible<Foo>::value);
    CHECK_FALSE(pfs::is_trivially_destructible<std::string>::value);
}

#endif