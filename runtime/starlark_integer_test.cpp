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
using ::starlark::runtime::error_fn;
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
using ::testing::SizeIs;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::Lt;
using ::testing::Not;

namespace {

struct error_handler : public error_fn {
  void add_error(std::string_view error_msg) override {
    messages.push_back(std::string(error_msg));
  }

  std::vector<std::string> messages;
};

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

TEST(StarlarkInteger, ShiftZero) {
  google::protobuf::Arena arena;
  auto* result = starlark_integer(0).binary_lshift(starlark_integer(1l << 62), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(0).equals(*result));
  result = starlark_integer(0).binary_rshift(starlark_integer(1l << 62), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(0).equals(*result));
}

TEST(StarlarkInteger, LShift) {
  google::protobuf::Arena arena;
  auto* result = starlark_integer(1).binary_lshift(starlark_integer(3), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(1 << 3).equals(*result));

  result = starlark_integer(-11).binary_lshift(starlark_integer(10), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(-11264).equals(*result));

  result = starlark_integer(1).binary_lshift(starlark_integer(100), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_bigint(number::one << 100).equals(*result));

  result = starlark_integer(1).binary_lshift(starlark_integer(1 << 28), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_bigint(number::one << (1 << 28)).equals(*result));

  result = starlark_integer(1).binary_lshift(starlark_bigint(62), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(1L << 62).equals(*result));

  result = starlark_integer(1).binary_lshift(starlark_bigint(63), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_bigint(number::one << 63).equals(*result));
}

TEST(StarlarkInteger, RShift) {
  google::protobuf::Arena arena;
  auto* result = starlark_integer(100).binary_rshift(starlark_integer(3), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(100 >> 3).equals(*result));

  result = starlark_integer(-11264).binary_rshift(starlark_integer(10), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(-11).equals(*result));

  result = starlark_integer(1).binary_rshift(starlark_integer(64), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(0).equals(*result));

  result = starlark_integer(-2).binary_rshift(starlark_integer(64), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(-1).equals(*result));

  result = starlark_integer(20).binary_rshift(starlark_bigint(64), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(0).equals(*result));

  result = starlark_integer(20).binary_rshift(starlark_bigint(number::one << 64), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(0).equals(*result));

  result = starlark_integer(20).binary_rshift(starlark_bigint(number::one), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(10).equals(*result));
}

TEST(StarlarkInteger, ShiftInvalidInput) {
  google::protobuf::Arena arena;
  error_handler error_callback;

  auto* result = starlark_integer(100).binary_rshift(starlark_float(3.0), arena, &error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for >>: 'int' and 'float'");
  error_callback.messages.clear();

  result = starlark_integer(100).binary_lshift(starlark_float(3.0), arena, &error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for <<: 'int' and 'float'");
  error_callback.messages.clear();

  result = starlark_integer(100).binary_lshift(starlark_integer(-1), arena, &error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: negative shift count");
  error_callback.messages.clear();

  result = starlark_integer(100).binary_rshift(starlark_integer(-1), arena, &error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: negative shift count");
  error_callback.messages.clear();

  result = starlark_integer(100).binary_lshift(starlark_bigint(number::minus_one), arena, &error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: negative shift count");
  error_callback.messages.clear();

  result = starlark_integer(100).binary_rshift(starlark_bigint(number::minus_one), arena, &error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: negative shift count");
  error_callback.messages.clear();

  result = starlark_integer(1).binary_lshift(starlark_integer(1 << 29), arena, &error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: too many digits in integer");
  error_callback.messages.clear();

  result = starlark_integer(1).binary_lshift(starlark_integer(0x7fff'ffff'ffff'ffffL), arena, &error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: too many digits in integer");
  error_callback.messages.clear();

  result = starlark_integer(100).binary_lshift(starlark_bigint(number::one << 100), arena, &error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: too many digits in integer");
  error_callback.messages.clear();

  result = starlark_integer(100).binary_lshift(starlark_bigint(number::one << 63), arena, &error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: too many digits in integer");
  error_callback.messages.clear();

  result = starlark_integer(100).binary_lshift(starlark_bigint(number::one << 62), arena, &error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: too many digits in integer");
  error_callback.messages.clear();
}

}  // namespace
