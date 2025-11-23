// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <limits>
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

using ::google::protobuf::Arena;
using ::starlark::bigint::number;
using ::starlark::bigint::parse_number;
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
  EXPECT_EQ(-4, starlark_integer(-0x8000000000000000).hash());
  EXPECT_EQ(-4, starlark_integer(std::numeric_limits<int64_t>::min()).hash());
}

TEST(StarlarkInteger, UnaryMinusEdgeCase) {
  google::protobuf::Arena arena;
  auto* result = starlark_integer(std::numeric_limits<int64_t>::min()).unary_minus(arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_EQ("9223372036854775808", result->str());
}

TEST(StarlarkInteger, OrderVsBigInt) {
  EXPECT_THAT(starlark_integer(-2).cmp(starlark_bigint(parse_number("-2", nullptr)), "cmp", nullptr), Eq(0));
  EXPECT_THAT(starlark_integer(-2).cmp(starlark_bigint(parse_number("-1", nullptr)), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_integer(-2).cmp(starlark_bigint(parse_number("0", nullptr)), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_integer(-2).cmp(starlark_bigint(parse_number("1", nullptr)), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_integer(-2).cmp(starlark_bigint(parse_number("2", nullptr)), "cmp", nullptr), Lt(0));

  EXPECT_THAT(starlark_integer(-1).cmp(starlark_bigint(parse_number("-2", nullptr)), "cmp", nullptr), Gt(0));
  EXPECT_THAT(starlark_integer(-1).cmp(starlark_bigint(parse_number("-1", nullptr)), "cmp", nullptr), Eq(0));
  EXPECT_THAT(starlark_integer(-1).cmp(starlark_bigint(parse_number("0", nullptr)), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_integer(-1).cmp(starlark_bigint(parse_number("1", nullptr)), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_integer(-1).cmp(starlark_bigint(parse_number("2", nullptr)), "cmp", nullptr), Lt(0));

  EXPECT_THAT(starlark_integer(0).cmp(starlark_bigint(parse_number("-2", nullptr)), "cmp", nullptr), Gt(0));
  EXPECT_THAT(starlark_integer(0).cmp(starlark_bigint(parse_number("-1", nullptr)), "cmp", nullptr), Gt(0));
  EXPECT_THAT(starlark_integer(0).cmp(starlark_bigint(parse_number("0", nullptr)), "cmp", nullptr), Eq(0));
  EXPECT_THAT(starlark_integer(0).cmp(starlark_bigint(parse_number("1", nullptr)), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_integer(0).cmp(starlark_bigint(parse_number("2", nullptr)), "cmp", nullptr), Lt(0));

  EXPECT_THAT(starlark_integer(1).cmp(starlark_bigint(parse_number("-2", nullptr)), "cmp", nullptr), Gt(0));
  EXPECT_THAT(starlark_integer(1).cmp(starlark_bigint(parse_number("-1", nullptr)), "cmp", nullptr), Gt(0));
  EXPECT_THAT(starlark_integer(1).cmp(starlark_bigint(parse_number("0", nullptr)), "cmp", nullptr), Gt(0));
  EXPECT_THAT(starlark_integer(1).cmp(starlark_bigint(parse_number("1", nullptr)), "cmp", nullptr), Eq(0));
  EXPECT_THAT(starlark_integer(1).cmp(starlark_bigint(parse_number("2", nullptr)), "cmp", nullptr), Lt(0));

  EXPECT_THAT(starlark_integer(2).cmp(starlark_bigint(parse_number("-2", nullptr)), "cmp", nullptr), Gt(0));
  EXPECT_THAT(starlark_integer(2).cmp(starlark_bigint(parse_number("-1", nullptr)), "cmp", nullptr), Gt(0));
  EXPECT_THAT(starlark_integer(2).cmp(starlark_bigint(parse_number("0", nullptr)), "cmp", nullptr), Gt(0));
  EXPECT_THAT(starlark_integer(2).cmp(starlark_bigint(parse_number("1", nullptr)), "cmp", nullptr), Gt(0));
  EXPECT_THAT(starlark_integer(2).cmp(starlark_bigint(parse_number("2", nullptr)), "cmp", nullptr), Eq(0));

  EXPECT_THAT(starlark_integer(1).cmp(starlark_bigint(number::one << 64), "cmp", nullptr), Lt(0));
}

}  // namespace
