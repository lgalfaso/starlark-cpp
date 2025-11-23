// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <string>

#include "runtime/starlark_function.hpp"
#include "runtime/starlark_bigint.hpp"
#include "runtime/starlark_bool.hpp"
#include "runtime/starlark_bytes.hpp"
#include "runtime/starlark_dictionary.hpp"
#include "runtime/starlark_float.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_list.hpp"
#include "runtime/starlark_none.hpp"
#include "runtime/starlark_range.hpp"
#include "runtime/starlark_set.hpp"
#include "runtime/starlark_string.hpp"
#include "runtime/starlark_struct.hpp"
#include "runtime/starlark_tuple.hpp"

using ::starlark::runtime::starlark_bigint;
using ::starlark::runtime::starlark_bool;
using ::starlark::runtime::starlark_built_in_function;
using ::starlark::runtime::starlark_bytes;
using ::starlark::runtime::starlark_dictionary;
using ::starlark::runtime::starlark_float;
using ::starlark::runtime::starlark_function;
using ::starlark::runtime::starlark_integer;
using ::starlark::runtime::starlark_list;
using ::starlark::runtime::starlark_none;
using ::starlark::runtime::starlark_range;
using ::starlark::runtime::starlark_set;
using ::starlark::runtime::starlark_string;
using ::starlark::runtime::starlark_struct;
using ::starlark::runtime::starlark_tuple;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::Lt;

namespace {

TEST(StarlarkString, Type) {
  EXPECT_EQ("string", starlark_string("").type());
}

TEST(StarlarkString, Str) {
  EXPECT_EQ("abcdef", starlark_string("abcdef").str());
  EXPECT_EQ("fedcba", starlark_string("fedcba").str());
}

TEST(StarlarkString, Repr) {
  // TODO(lmirelmann): Would be nice to have a test that checks the encoding of all characters.
  EXPECT_EQ("'abcdef'", starlark_string("abcdef").repr());
  EXPECT_EQ("\"'\"", starlark_string("'").repr());
  EXPECT_EQ("'\\'\"'", starlark_string("'\"").repr());
  EXPECT_EQ("'\\t\\r\\n'", starlark_string("\t\r\n").repr());
  EXPECT_EQ("'\\x01\\x02\\x7f'", starlark_string("\001\002\177").repr());
  EXPECT_EQ("'\\x90'", starlark_string("\302\220").repr());
  EXPECT_EQ("'\xC3\xA0'", starlark_string("\303\240").repr());
  EXPECT_EQ("'\xC8\xB4'", starlark_string("\310\264").repr());
  EXPECT_EQ("'\\u0378'", starlark_string("\315\270").repr());
  EXPECT_EQ("'\\ud800'", starlark_string("\355\240\200").repr());
  EXPECT_EQ("'\\U000101c7'", starlark_string("\360\220\207\207").repr());
}

TEST(StarlarkString, Truthy) {
  EXPECT_FALSE(starlark_string("").truthy());
  EXPECT_TRUE(starlark_string("a").truthy());
}

TEST(StarlarkString, Equals) {
  EXPECT_TRUE(starlark_string("").equals(starlark_string("")));
  EXPECT_TRUE(starlark_string("a").equals(starlark_string("a")));
  EXPECT_FALSE(starlark_string("").equals(starlark_string("a")));
  // This is the NFKC decomposition.
  EXPECT_FALSE(starlark_string("\u03C9\u0301").equals(starlark_string("\u03CE")));

  EXPECT_FALSE(starlark_string("").equals(starlark_bigint(0)));
  EXPECT_FALSE(starlark_string("").equals(starlark_bool(false)));
  EXPECT_FALSE(starlark_string("").equals(starlark_bytes("")));
  EXPECT_FALSE(starlark_string("").equals(starlark_dictionary()));
  EXPECT_FALSE(starlark_string("").equals(starlark_float(0.0)));
  EXPECT_FALSE(starlark_string("").equals(starlark_function()));
  EXPECT_FALSE(starlark_string("").equals(starlark_integer(0)));
  EXPECT_FALSE(starlark_string("").equals(starlark_list()));
  EXPECT_FALSE(starlark_string("").equals(starlark_none()));
  EXPECT_FALSE(starlark_string("").equals(starlark_range()));
  EXPECT_FALSE(starlark_string("").equals(starlark_set()));
  EXPECT_FALSE(starlark_string("").equals(starlark_struct()));
  EXPECT_FALSE(starlark_string("").equals(starlark_tuple()));

  EXPECT_FALSE(starlark_string("0").equals(starlark_integer(0)));
}

TEST(StarlarkString, Hash) {
  EXPECT_EQ(0, starlark_string("").hash());
  EXPECT_EQ(0x5915dff8ab3f6382, starlark_string(std::string("\000", 1)).hash());
  EXPECT_EQ(0x47c474f5b32240c8, starlark_string("a").hash());
  EXPECT_EQ(0x69d6efa619d3857d, starlark_string("ab").hash());
  EXPECT_EQ(0x1f64c85d9a3c5aea, starlark_string("abc").hash());
  EXPECT_EQ(-0x0b5b097a9625e685, starlark_string("abcd").hash());
  EXPECT_EQ(0x168632fd4df59211, starlark_string("abcde").hash());
  EXPECT_EQ(-0x1316734e9ae39601, starlark_string("abcdef").hash());
  EXPECT_EQ(-0x046f814a39c03c2e, starlark_string("abcdefg").hash());
  EXPECT_EQ(-0x7fd30d59d72d058a, starlark_string("abcdefgh").hash());
  EXPECT_EQ(-0x6d11b9acae93c056, starlark_string("abcdefghi").hash());
  EXPECT_EQ(-0x045193c9a34bf07a, starlark_string("abcdefghij").hash());
  EXPECT_EQ(-0x493556008d0e92cb, starlark_string("abcdefghijk").hash());
  EXPECT_EQ(0x76ef23c6b008cd7b, starlark_string("abcdefghijkl").hash());
  EXPECT_EQ(-0x14a7e09195326e3d, starlark_string("abcdefghijklm").hash());
  EXPECT_EQ(0x033b8d82044dd206, starlark_string("abcdefghijklmn").hash());
  EXPECT_EQ(0x4b7d9ede3c051bec, starlark_string("abcdefghijklmno").hash());
  EXPECT_EQ(0x6ee54dcefca3d689, starlark_string("abcdefghijklmnop").hash());
  EXPECT_EQ(-0x5ac20331846b0484, starlark_string("abcdefghijklmnopq").hash());
  EXPECT_EQ(0x7b458279e2ffd51b, starlark_string("abcdefghijklmnopqr").hash());
  EXPECT_EQ(0x6d831f8a8e6a7c80, starlark_string("abcdefghijklmnopqrs").hash());
  EXPECT_EQ(-0x5221ec02394a14d7, starlark_string("abcdefghijklmnopqrst").hash());
  EXPECT_EQ(-0x4b38298791b754ea, starlark_string("abcdefghijklmnopqrstu").hash());
  EXPECT_EQ(0x40991f144590c002, starlark_string("abcdefghijklmnopqrstuv").hash());
  EXPECT_EQ(0x4db8fbf39f1ed05a, starlark_string("abcdefghijklmnopqrstuvw").hash());
  EXPECT_EQ(-0x251b735fced053c2, starlark_string("abcdefghijklmnopqrstuvwx").hash());
  EXPECT_EQ(-0xc0739218d5461ec, starlark_string("abcdefghijklmnopqrstuvwxy").hash());
  EXPECT_EQ(0x22b04466815fd83b, starlark_string("abcdefghijklmnopqrstuvwxyz").hash());
  EXPECT_EQ(-0x3cab5efd2f0468f4, starlark_string("abcdefghijklmnopqrstuvwxyz0").hash());
  EXPECT_EQ(-0x3b94d26b8d85c88a, starlark_string("abcdefghijklmnopqrstuvwxyz01").hash());
  EXPECT_EQ(0x4ab8bfbc363cd434, starlark_string("abcdefghijklmnopqrstuvwxyz012").hash());
  EXPECT_EQ(-0x557e62313da24871, starlark_string("abcdefghijklmnopqrstuvwxyz0123").hash());
  EXPECT_EQ(-0x4fcc6fbe77b87973, starlark_string("abcdefghijklmnopqrstuvwxyz01234").hash());
  EXPECT_EQ(-0x151775372a5579d8, starlark_string("abcdefghijklmnopqrstuvwxyz012345").hash());
  EXPECT_EQ(0x6ffd51cc0fe5d3e, starlark_string("abcdefghijklmnopqrstuvwxyz0123456").hash());
  EXPECT_EQ(-0xe990dfb175a2aea, starlark_string("abcdefghijklmnopqrstuvwxyz01234567").hash());
  EXPECT_EQ(0x3f2df2d04784600b, starlark_string("abcdefghijklmnopqrstuvwxyz012345678").hash());
  EXPECT_EQ(-0x62f803d77a091faa, starlark_string("abcdefghijklmnopqrstuvwxyz0123456789").hash());
  EXPECT_EQ(-0x19fb4f2fd515d61, starlark_string("abcdefghijklmnopqrstuvwxyz0123456789@").hash());
  EXPECT_EQ(-0x77d493a040656198, starlark_string("abcdefghijklmnopqrstuvwxyz0123456789@!").hash());
}

TEST(StarlarkString, Order) {
  EXPECT_THAT(starlark_string("").cmp(starlark_string(""), "cmp", nullptr), Eq(0));
  EXPECT_THAT(starlark_string("").cmp(starlark_string("a"), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_string("a").cmp(starlark_string("a"), "cmp", nullptr), Eq(0));
  EXPECT_THAT(starlark_string("a").cmp(starlark_string(""), "cmp", nullptr), Gt(0));
  EXPECT_THAT(starlark_string("a").cmp(starlark_string("b"), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_string("b").cmp(starlark_string("a"), "cmp", nullptr), Gt(0));
}

}  // namespace
