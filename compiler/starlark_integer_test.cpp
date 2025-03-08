// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <limits>
#include <string>

#include "compiler/starlark_function.hpp"
#include "compiler/starlark_bigint.hpp"
#include "compiler/starlark_bool.hpp"
#include "compiler/starlark_bytes.hpp"
#include "compiler/starlark_dictionary.hpp"
#include "compiler/starlark_float.hpp"
#include "compiler/starlark_integer.hpp"
#include "compiler/starlark_list.hpp"
#include "compiler/starlark_none.hpp"
#include "compiler/starlark_range.hpp"
#include "compiler/starlark_set.hpp"
#include "compiler/starlark_string.hpp"
#include "compiler/starlark_struct.hpp"
#include "compiler/starlark_tuple.hpp"

using starlark::compiler::starlark_bigint;
using starlark::compiler::starlark_bool;
using starlark::compiler::starlark_built_in_function;
using starlark::compiler::starlark_bytes;
using starlark::compiler::starlark_dictionary;
using starlark::compiler::starlark_float;
using starlark::compiler::starlark_function;
using starlark::compiler::starlark_integer;
using starlark::compiler::starlark_list;
using starlark::compiler::starlark_none;
using starlark::compiler::starlark_range;
using starlark::compiler::starlark_set;
using starlark::compiler::starlark_string;
using starlark::compiler::starlark_struct;
using starlark::compiler::starlark_tuple;

namespace {

TEST(StarlarkInteger, Type) {
  EXPECT_EQ("int", starlark_integer(1).type());
}

TEST(StarlarkInteger, Str) {
  EXPECT_EQ("1234", starlark_integer(1234).str());
  EXPECT_EQ("-1234", starlark_integer(-1234).str());
}


TEST(StarlarkInteger, Truthy) {
  EXPECT_FALSE(starlark_integer(0).truthy());
  EXPECT_TRUE(starlark_integer(1).truthy());
  EXPECT_TRUE(starlark_integer(-1).truthy());
}

TEST(StarlarkInteger, Equals) {
  EXPECT_TRUE(starlark_integer(-1).equals(starlark_integer(-1)));
  EXPECT_TRUE(starlark_integer(-1).equals(starlark_bigint(-1)));
  EXPECT_TRUE(starlark_integer(-1).equals(starlark_float(-1)));

  EXPECT_TRUE(starlark_integer(0).equals(starlark_integer(0)));
  EXPECT_TRUE(starlark_integer(0).equals(starlark_bigint(0)));
  EXPECT_TRUE(starlark_integer(0).equals(starlark_float(0)));

  EXPECT_TRUE(starlark_integer(1).equals(starlark_integer(1)));
  EXPECT_TRUE(starlark_integer(1).equals(starlark_bigint(1)));
  EXPECT_TRUE(starlark_integer(1).equals(starlark_float(1)));

  EXPECT_FALSE(starlark_integer(1).equals(starlark_integer(0)));
  EXPECT_FALSE(starlark_integer(1).equals(starlark_bigint(0)));
  EXPECT_FALSE(starlark_integer(1).equals(starlark_float(0)));

  EXPECT_FALSE(starlark_integer(1).equals(starlark_integer(-1)));
  EXPECT_FALSE(starlark_integer(1).equals(starlark_bigint(-1)));
  EXPECT_FALSE(starlark_integer(1).equals(starlark_float(-1)));

  starlark::bigint::number min_int64 = starlark::bigint::number::parse_hex("-8000000000000000");
  EXPECT_TRUE(starlark_integer(std::numeric_limits<int64_t>::min()).equals(starlark_bigint(min_int64)));
  EXPECT_TRUE(starlark_integer(std::numeric_limits<int64_t>::min() + 1).equals(starlark_bigint(min_int64 + starlark::bigint::number::one)));

  EXPECT_FALSE(starlark_integer(0).equals(starlark_string("")));
}

TEST(StarlarkInteger, Hash) {
  EXPECT_EQ(0, starlark_integer(0).hash());
  EXPECT_EQ(1, starlark_integer(1).hash());
  EXPECT_EQ(2, starlark_integer(2).hash());
  EXPECT_EQ(0x1ffffffffffffffe, starlark_integer(0x1ffffffffffffffe).hash());
  EXPECT_EQ(0, starlark_integer(0x1fffffffffffffff).hash());
  EXPECT_EQ(0x1000000000000000, starlark_integer(0x2fffffffffffffff).hash());
  EXPECT_EQ(1, starlark_integer(0x3fffffffffffffff).hash());
  EXPECT_EQ(0x1000000000000001, starlark_integer(0x4fffffffffffffff).hash());
  EXPECT_EQ(2, starlark_integer(0x5fffffffffffffff).hash());
  EXPECT_EQ(0x1000000000000002, starlark_integer(0x6fffffffffffffff).hash());
  EXPECT_EQ(3, starlark_integer(0x7fffffffffffffff).hash());
  EXPECT_EQ(-2, starlark_integer(-1).hash());
  EXPECT_EQ(-2, starlark_integer(-2).hash());
  EXPECT_EQ(-0x1ffffffffffffffe, starlark_integer(-0x1ffffffffffffffe).hash());
  EXPECT_EQ(0, starlark_integer(-0x1fffffffffffffff).hash());
  EXPECT_EQ(-0x1000000000000000, starlark_integer(-0x2fffffffffffffff).hash());
  EXPECT_EQ(-2, starlark_integer(-0x3fffffffffffffff).hash());
  EXPECT_EQ(-0x1000000000000001, starlark_integer(-0x4fffffffffffffff).hash());
  EXPECT_EQ(-2, starlark_integer(-0x5fffffffffffffff).hash());
  EXPECT_EQ(-0x1000000000000002, starlark_integer(-0x6fffffffffffffff).hash());
  EXPECT_EQ(-3, starlark_integer(-0x7fffffffffffffff).hash());
}

}  // namespace
