// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <limits>
#include <string>
#include <vector>

#include "runtime/starlark_bigint.hpp"
#include "runtime/starlark_bool.hpp"
#include "runtime/starlark_bytes.hpp"
#include "runtime/starlark_dictionary.hpp"
#include "runtime/starlark_float.hpp"
#include "runtime/starlark_function.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_list.hpp"
#include "runtime/starlark_none.hpp"
#include "runtime/starlark_range.hpp"
#include "runtime/starlark_set.hpp"
#include "runtime/starlark_string.hpp"
#include "runtime/starlark_struct.hpp"
#include "runtime/starlark_testing.hpp"
#include "runtime/starlark_tuple.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;
using ::starlark::bigint::parse_number;
using ::starlark::runtime::from_int64;
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
using ::starlark::runtime::starlark_obj;
using ::starlark::runtime::starlark_range;
using ::starlark::runtime::starlark_set;
using ::starlark::runtime::starlark_string;
using ::starlark::runtime::starlark_struct;
using ::starlark::runtime::starlark_tuple;
using ::starlark::testing::error_handler;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::IsEmpty;
using ::testing::Lt;
using ::testing::Not;
using ::testing::SizeIs;

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
  Arena arena;
  error_handler error_callback;

  auto* result = starlark_integer(std::numeric_limits<int64_t>::min()).unary_minus(arena, error_callback);
  ASSERT_NE(result, nullptr);
  EXPECT_EQ("9223372036854775808", result->str());
}

TEST(StarlarkInteger, OrderVsBigInt) {
  error_handler error_callback;

  EXPECT_THAT(starlark_integer(-2).cmp(starlark_bigint(parse_number("-2", nullptr)), "cmp", error_callback), Eq(0));
  EXPECT_THAT(starlark_integer(-2).cmp(starlark_bigint(parse_number("-1", nullptr)), "cmp", error_callback), Lt(0));
  EXPECT_THAT(starlark_integer(-2).cmp(starlark_bigint(parse_number("0", nullptr)), "cmp", error_callback), Lt(0));
  EXPECT_THAT(starlark_integer(-2).cmp(starlark_bigint(parse_number("1", nullptr)), "cmp", error_callback), Lt(0));
  EXPECT_THAT(starlark_integer(-2).cmp(starlark_bigint(parse_number("2", nullptr)), "cmp", error_callback), Lt(0));

  EXPECT_THAT(starlark_integer(-1).cmp(starlark_bigint(parse_number("-2", nullptr)), "cmp", error_callback), Gt(0));
  EXPECT_THAT(starlark_integer(-1).cmp(starlark_bigint(parse_number("-1", nullptr)), "cmp", error_callback), Eq(0));
  EXPECT_THAT(starlark_integer(-1).cmp(starlark_bigint(parse_number("0", nullptr)), "cmp", error_callback), Lt(0));
  EXPECT_THAT(starlark_integer(-1).cmp(starlark_bigint(parse_number("1", nullptr)), "cmp", error_callback), Lt(0));
  EXPECT_THAT(starlark_integer(-1).cmp(starlark_bigint(parse_number("2", nullptr)), "cmp", error_callback), Lt(0));

  EXPECT_THAT(starlark_integer(0).cmp(starlark_bigint(parse_number("-2", nullptr)), "cmp", error_callback), Gt(0));
  EXPECT_THAT(starlark_integer(0).cmp(starlark_bigint(parse_number("-1", nullptr)), "cmp", error_callback), Gt(0));
  EXPECT_THAT(starlark_integer(0).cmp(starlark_bigint(parse_number("0", nullptr)), "cmp", error_callback), Eq(0));
  EXPECT_THAT(starlark_integer(0).cmp(starlark_bigint(parse_number("1", nullptr)), "cmp", error_callback), Lt(0));
  EXPECT_THAT(starlark_integer(0).cmp(starlark_bigint(parse_number("2", nullptr)), "cmp", error_callback), Lt(0));

  EXPECT_THAT(starlark_integer(1).cmp(starlark_bigint(parse_number("-2", nullptr)), "cmp", error_callback), Gt(0));
  EXPECT_THAT(starlark_integer(1).cmp(starlark_bigint(parse_number("-1", nullptr)), "cmp", error_callback), Gt(0));
  EXPECT_THAT(starlark_integer(1).cmp(starlark_bigint(parse_number("0", nullptr)), "cmp", error_callback), Gt(0));
  EXPECT_THAT(starlark_integer(1).cmp(starlark_bigint(parse_number("1", nullptr)), "cmp", error_callback), Eq(0));
  EXPECT_THAT(starlark_integer(1).cmp(starlark_bigint(parse_number("2", nullptr)), "cmp", error_callback), Lt(0));

  EXPECT_THAT(starlark_integer(2).cmp(starlark_bigint(parse_number("-2", nullptr)), "cmp", error_callback), Gt(0));
  EXPECT_THAT(starlark_integer(2).cmp(starlark_bigint(parse_number("-1", nullptr)), "cmp", error_callback), Gt(0));
  EXPECT_THAT(starlark_integer(2).cmp(starlark_bigint(parse_number("0", nullptr)), "cmp", error_callback), Gt(0));
  EXPECT_THAT(starlark_integer(2).cmp(starlark_bigint(parse_number("1", nullptr)), "cmp", error_callback), Gt(0));
  EXPECT_THAT(starlark_integer(2).cmp(starlark_bigint(parse_number("2", nullptr)), "cmp", error_callback), Eq(0));

  EXPECT_THAT(starlark_integer(1).cmp(starlark_bigint(number::one << 64), "cmp", error_callback), Lt(0));
}

TEST(StarlarkInteger, OrderVsBool) {
  error_handler error_callback;
  starlark_bool obj_true(true);

  EXPECT_THAT(starlark_integer(1).cmp(obj_true, "<", error_callback), Eq(0));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: '<' not supported between instances of 'int' and 'bool'");
}

TEST(StarlarkInteger, ShiftZero) {
  Arena arena;
  error_handler error_callback;

  auto* result = starlark_integer(0).binary_lshift(starlark_integer(1l << 62), arena, error_callback);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(0).equals(*result));
  result = starlark_integer(0).binary_rshift(starlark_integer(1l << 62), arena, error_callback);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(0).equals(*result));
}

TEST(StarlarkInteger, LShift) {
  Arena arena;
  error_handler error_callback;

  auto* result = starlark_integer(1).binary_lshift(starlark_integer(3), arena, error_callback);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(1 << 3).equals(*result));

  result = starlark_integer(-11).binary_lshift(starlark_integer(10), arena, error_callback);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(-11264).equals(*result));

  result = starlark_integer(1).binary_lshift(starlark_integer(100), arena, error_callback);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_bigint(number::one << 100).equals(*result));

  result = starlark_integer(1).binary_lshift(starlark_integer(1 << 28), arena, error_callback);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_bigint(number::one << (1 << 28)).equals(*result));

  result = starlark_integer(1).binary_lshift(starlark_bigint(62), arena, error_callback);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(1L << 62).equals(*result));

  result = starlark_integer(1).binary_lshift(starlark_bigint(63), arena, error_callback);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_bigint(number::one << 63).equals(*result));
}

TEST(StarlarkInteger, RShift) {
  Arena arena;
  error_handler error_callback;

  auto* result = starlark_integer(100).binary_rshift(starlark_integer(3), arena, error_callback);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(100 >> 3).equals(*result));

  result = starlark_integer(-11264).binary_rshift(starlark_integer(10), arena, error_callback);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(-11).equals(*result));

  result = starlark_integer(1).binary_rshift(starlark_integer(64), arena, error_callback);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(0).equals(*result));

  result = starlark_integer(-2).binary_rshift(starlark_integer(64), arena, error_callback);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(-1).equals(*result));

  result = starlark_integer(20).binary_rshift(starlark_bigint(64), arena, error_callback);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(0).equals(*result));

  result = starlark_integer(20).binary_rshift(starlark_bigint(number::one << 64), arena, error_callback);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(0).equals(*result));

  result = starlark_integer(20).binary_rshift(starlark_bigint(number::one), arena, error_callback);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(10).equals(*result));
}

TEST(StarlarkInteger, ShiftInvalidInput) {
  Arena arena;
  error_handler error_callback;

  auto* result = starlark_integer(100).binary_rshift(starlark_float(3.0), arena, error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for >>: 'int' and 'float'");
  error_callback.messages.clear();

  result = starlark_integer(100).binary_lshift(starlark_float(3.0), arena, error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for <<: 'int' and 'float'");
  error_callback.messages.clear();

  result = starlark_integer(100).binary_lshift(starlark_integer(-1), arena, error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: negative shift count");
  error_callback.messages.clear();

  result = starlark_integer(100).binary_rshift(starlark_integer(-1), arena, error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: negative shift count");
  error_callback.messages.clear();

  result = starlark_integer(100).binary_lshift(starlark_bigint(number::minus_one), arena, error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: negative shift count");
  error_callback.messages.clear();

  result = starlark_integer(100).binary_rshift(starlark_bigint(number::minus_one), arena, error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: negative shift count");
  error_callback.messages.clear();

  result = starlark_integer(1).binary_lshift(starlark_integer(1 << 29), arena, error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: too many digits in integer");
  error_callback.messages.clear();

  result = starlark_integer(1).binary_lshift(starlark_integer(0x7fff'ffff'ffff'ffffL), arena, error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: too many digits in integer");
  error_callback.messages.clear();

  result = starlark_integer(100).binary_lshift(starlark_bigint(number::one << 100), arena, error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: too many digits in integer");
  error_callback.messages.clear();

  result = starlark_integer(100).binary_lshift(starlark_bigint(number::one << 63), arena, error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: too many digits in integer");
  error_callback.messages.clear();

  result = starlark_integer(100).binary_lshift(starlark_bigint(number::one << 62), arena, error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: too many digits in integer");
  error_callback.messages.clear();
}

TEST(StarlarkInteger, Membership) {
  starlark_integer zero(0);
  error_handler error_callback;

  EXPECT_FALSE(zero.binary_in(zero, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: argument of type 'int' is not a container or iterable");
}

TEST(StarlarkInteger, Unpack) {
  starlark_integer zero(0);
  error_handler error_callback;
  std::vector<starlark_obj*> consumer;

  zero.unpack(0, consumer, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: cannot unpack non-iterable int object");
}

TEST(StarlarkInteger, BinaryAnd) {
  std::vector<int64_t> values = {
    std::numeric_limits<int64_t>::min(),
    std::numeric_limits<int64_t>::min() + 1,
    std::numeric_limits<int64_t>::min() + 2,
    -2, -1, 0, 1, 1,
    std::numeric_limits<int64_t>::max() - 2,
    std::numeric_limits<int64_t>::max() - 1,
    std::numeric_limits<int64_t>::max(),
  };
  Arena arena;
  error_handler error_callback;

  for (const auto a : values) {
    for (const auto b : values) {
      auto* r = starlark_integer(a).binary_and(starlark_integer(b), arena, error_callback);
      ASSERT_NE(r, nullptr);
      EXPECT_TRUE(r->equals(starlark_integer(a & b)));
      r = starlark_integer(a).binary_and(starlark_bigint(from_int64(b)), arena, error_callback);
      ASSERT_NE(r, nullptr);
      EXPECT_TRUE(r->equals(starlark_integer(a & b)));
    }
  }
}

TEST(StarlarkInteger, BinaryAndError) {
  starlark_integer zero(0);
  starlark_float float_zero(0);
  Arena arena;
  error_handler error_callback;

  zero.binary_and(float_zero, arena, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for &: 'int' and 'float'");
}

TEST(StarlarkInteger, BinaryOr) {
  std::vector<int64_t> values = {
    std::numeric_limits<int64_t>::min(),
    std::numeric_limits<int64_t>::min() + 1,
    std::numeric_limits<int64_t>::min() + 2,
    -2, -1, 0, 1, 1,
    std::numeric_limits<int64_t>::max() - 2,
    std::numeric_limits<int64_t>::max() - 1,
    std::numeric_limits<int64_t>::max(),
  };
  Arena arena;
  error_handler error_callback;

  for (const auto a : values) {
    for (const auto b : values) {
      auto* r = starlark_integer(a).binary_pipe(starlark_integer(b), arena, error_callback);
      ASSERT_NE(r, nullptr);
      EXPECT_TRUE(r->equals(starlark_integer(a | b)));
      r = starlark_integer(a).binary_pipe(starlark_bigint(from_int64(b)), arena, error_callback);
      ASSERT_NE(r, nullptr);
      EXPECT_TRUE(r->equals(starlark_integer(a | b)));
    }
  }
}

TEST(StarlarkInteger, BinaryOrError) {
  starlark_integer zero(0);
  starlark_float float_zero(0);
  Arena arena;
  error_handler error_callback;

  zero.binary_pipe(float_zero, arena, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for |: 'int' and 'float'");
}

TEST(StarlarkInteger, BinaryXor) {
  std::vector<int64_t> values = {
    std::numeric_limits<int64_t>::min(),
    std::numeric_limits<int64_t>::min() + 1,
    std::numeric_limits<int64_t>::min() + 2,
    -2, -1, 0, 1, 1,
    std::numeric_limits<int64_t>::max() - 2,
    std::numeric_limits<int64_t>::max() - 1,
    std::numeric_limits<int64_t>::max(),
  };
  Arena arena;
  error_handler error_callback;

  for (const auto a : values) {
    for (const auto b : values) {
      auto* r = starlark_integer(a).binary_hat(starlark_integer(b), arena, error_callback);
      ASSERT_NE(r, nullptr);
      EXPECT_TRUE(r->equals(starlark_integer(a ^ b)));
      r = starlark_integer(a).binary_hat(starlark_bigint(from_int64(b)), arena, error_callback);
      ASSERT_NE(r, nullptr);
      EXPECT_TRUE(r->equals(starlark_integer(a ^ b)));
    }
  }
}

TEST(StarlarkInteger, BinaryXorError) {
  starlark_integer zero(0);
  starlark_float float_zero(0);
  Arena arena;
  error_handler error_callback;

  zero.binary_hat(float_zero, arena, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for ^: 'int' and 'float'");
}

TEST(StarlarkInteger, BinaryPlus) {
  starlark_float f1(1.0);
  starlark_integer i1(3);
  starlark_integer i2(16);
  starlark_bigint b1(number::one << 2);
  Arena arena;
  error_handler error_callback;

  auto* result1 = i2.binary_plus(b1, arena, error_callback);
  auto* result2 = i2.binary_plus(i1, arena, error_callback);
  auto* result3 = i2.binary_plus(f1, arena, error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("20", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("19", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("17.0", result3->str());
}

TEST(StarlarkInteger, BinaryPlusIntegerOverflowDetection) {
  auto test = [](int64_t n, int64_t m, std::string_view r1) {
    starlark_integer nn(n);
    starlark_integer mm(m);
    Arena arena;
    error_handler error_callback;

    auto* result = nn.binary_plus(mm, arena, error_callback);

    EXPECT_THAT(error_callback.messages, SizeIs(0));
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(r1, result->str()) << "N: " << n << ", M: " << m;
  };

  /*
  ```python
  a = [-1<<63, (-1<<63) + 1, (-1<<63) + 2, -2, -1, 0, 1, 2, (1<<63) - 3, (1<<63) - 2, (1<<63) - 1]
  for n in a:
      for m in a:
          print('  test({}, {}, "{}");'.format(str(n) + "L" if n != a[0] else "std::numeric_limits<std::int64_t>::min()", str(m) + "L" if m != a[0] else "std::numeric_limits<std::int64_t>::min()", n + m))
  ```
  */
  test(std::numeric_limits<std::int64_t>::min(), std::numeric_limits<std::int64_t>::min(), "-18446744073709551616");
  test(std::numeric_limits<std::int64_t>::min(), -9223372036854775807L, "-18446744073709551615");
  test(std::numeric_limits<std::int64_t>::min(), -9223372036854775806L, "-18446744073709551614");
  test(std::numeric_limits<std::int64_t>::min(), -2L, "-9223372036854775810");
  test(std::numeric_limits<std::int64_t>::min(), -1L, "-9223372036854775809");
  test(std::numeric_limits<std::int64_t>::min(), 0L, "-9223372036854775808");
  test(std::numeric_limits<std::int64_t>::min(), 1L, "-9223372036854775807");
  test(std::numeric_limits<std::int64_t>::min(), 2L, "-9223372036854775806");
  test(std::numeric_limits<std::int64_t>::min(), 9223372036854775805L, "-3");
  test(std::numeric_limits<std::int64_t>::min(), 9223372036854775806L, "-2");
  test(std::numeric_limits<std::int64_t>::min(), 9223372036854775807L, "-1");
  test(-9223372036854775807L, std::numeric_limits<std::int64_t>::min(), "-18446744073709551615");
  test(-9223372036854775807L, -9223372036854775807L, "-18446744073709551614");
  test(-9223372036854775807L, -9223372036854775806L, "-18446744073709551613");
  test(-9223372036854775807L, -2L, "-9223372036854775809");
  test(-9223372036854775807L, -1L, "-9223372036854775808");
  test(-9223372036854775807L, 0L, "-9223372036854775807");
  test(-9223372036854775807L, 1L, "-9223372036854775806");
  test(-9223372036854775807L, 2L, "-9223372036854775805");
  test(-9223372036854775807L, 9223372036854775805L, "-2");
  test(-9223372036854775807L, 9223372036854775806L, "-1");
  test(-9223372036854775807L, 9223372036854775807L, "0");
  test(-9223372036854775806L, std::numeric_limits<std::int64_t>::min(), "-18446744073709551614");
  test(-9223372036854775806L, -9223372036854775807L, "-18446744073709551613");
  test(-9223372036854775806L, -9223372036854775806L, "-18446744073709551612");
  test(-9223372036854775806L, -2L, "-9223372036854775808");
  test(-9223372036854775806L, -1L, "-9223372036854775807");
  test(-9223372036854775806L, 0L, "-9223372036854775806");
  test(-9223372036854775806L, 1L, "-9223372036854775805");
  test(-9223372036854775806L, 2L, "-9223372036854775804");
  test(-9223372036854775806L, 9223372036854775805L, "-1");
  test(-9223372036854775806L, 9223372036854775806L, "0");
  test(-9223372036854775806L, 9223372036854775807L, "1");
  test(-2L, std::numeric_limits<std::int64_t>::min(), "-9223372036854775810");
  test(-2L, -9223372036854775807L, "-9223372036854775809");
  test(-2L, -9223372036854775806L, "-9223372036854775808");
  test(-2L, -2L, "-4");
  test(-2L, -1L, "-3");
  test(-2L, 0L, "-2");
  test(-2L, 1L, "-1");
  test(-2L, 2L, "0");
  test(-2L, 9223372036854775805L, "9223372036854775803");
  test(-2L, 9223372036854775806L, "9223372036854775804");
  test(-2L, 9223372036854775807L, "9223372036854775805");
  test(-1L, std::numeric_limits<std::int64_t>::min(), "-9223372036854775809");
  test(-1L, -9223372036854775807L, "-9223372036854775808");
  test(-1L, -9223372036854775806L, "-9223372036854775807");
  test(-1L, -2L, "-3");
  test(-1L, -1L, "-2");
  test(-1L, 0L, "-1");
  test(-1L, 1L, "0");
  test(-1L, 2L, "1");
  test(-1L, 9223372036854775805L, "9223372036854775804");
  test(-1L, 9223372036854775806L, "9223372036854775805");
  test(-1L, 9223372036854775807L, "9223372036854775806");
  test(0L, std::numeric_limits<std::int64_t>::min(), "-9223372036854775808");
  test(0L, -9223372036854775807L, "-9223372036854775807");
  test(0L, -9223372036854775806L, "-9223372036854775806");
  test(0L, -2L, "-2");
  test(0L, -1L, "-1");
  test(0L, 0L, "0");
  test(0L, 1L, "1");
  test(0L, 2L, "2");
  test(0L, 9223372036854775805L, "9223372036854775805");
  test(0L, 9223372036854775806L, "9223372036854775806");
  test(0L, 9223372036854775807L, "9223372036854775807");
  test(1L, std::numeric_limits<std::int64_t>::min(), "-9223372036854775807");
  test(1L, -9223372036854775807L, "-9223372036854775806");
  test(1L, -9223372036854775806L, "-9223372036854775805");
  test(1L, -2L, "-1");
  test(1L, -1L, "0");
  test(1L, 0L, "1");
  test(1L, 1L, "2");
  test(1L, 2L, "3");
  test(1L, 9223372036854775805L, "9223372036854775806");
  test(1L, 9223372036854775806L, "9223372036854775807");
  test(1L, 9223372036854775807L, "9223372036854775808");
  test(2L, std::numeric_limits<std::int64_t>::min(), "-9223372036854775806");
  test(2L, -9223372036854775807L, "-9223372036854775805");
  test(2L, -9223372036854775806L, "-9223372036854775804");
  test(2L, -2L, "0");
  test(2L, -1L, "1");
  test(2L, 0L, "2");
  test(2L, 1L, "3");
  test(2L, 2L, "4");
  test(2L, 9223372036854775805L, "9223372036854775807");
  test(2L, 9223372036854775806L, "9223372036854775808");
  test(2L, 9223372036854775807L, "9223372036854775809");
  test(9223372036854775805L, std::numeric_limits<std::int64_t>::min(), "-3");
  test(9223372036854775805L, -9223372036854775807L, "-2");
  test(9223372036854775805L, -9223372036854775806L, "-1");
  test(9223372036854775805L, -2L, "9223372036854775803");
  test(9223372036854775805L, -1L, "9223372036854775804");
  test(9223372036854775805L, 0L, "9223372036854775805");
  test(9223372036854775805L, 1L, "9223372036854775806");
  test(9223372036854775805L, 2L, "9223372036854775807");
  test(9223372036854775805L, 9223372036854775805L, "18446744073709551610");
  test(9223372036854775805L, 9223372036854775806L, "18446744073709551611");
  test(9223372036854775805L, 9223372036854775807L, "18446744073709551612");
  test(9223372036854775806L, std::numeric_limits<std::int64_t>::min(), "-2");
  test(9223372036854775806L, -9223372036854775807L, "-1");
  test(9223372036854775806L, -9223372036854775806L, "0");
  test(9223372036854775806L, -2L, "9223372036854775804");
  test(9223372036854775806L, -1L, "9223372036854775805");
  test(9223372036854775806L, 0L, "9223372036854775806");
  test(9223372036854775806L, 1L, "9223372036854775807");
  test(9223372036854775806L, 2L, "9223372036854775808");
  test(9223372036854775806L, 9223372036854775805L, "18446744073709551611");
  test(9223372036854775806L, 9223372036854775806L, "18446744073709551612");
  test(9223372036854775806L, 9223372036854775807L, "18446744073709551613");
  test(9223372036854775807L, std::numeric_limits<std::int64_t>::min(), "-1");
  test(9223372036854775807L, -9223372036854775807L, "0");
  test(9223372036854775807L, -9223372036854775806L, "1");
  test(9223372036854775807L, -2L, "9223372036854775805");
  test(9223372036854775807L, -1L, "9223372036854775806");
  test(9223372036854775807L, 0L, "9223372036854775807");
  test(9223372036854775807L, 1L, "9223372036854775808");
  test(9223372036854775807L, 2L, "9223372036854775809");
  test(9223372036854775807L, 9223372036854775805L, "18446744073709551612");
  test(9223372036854775807L, 9223372036854775806L, "18446744073709551613");
  test(9223372036854775807L, 9223372036854775807L, "18446744073709551614");
}

TEST(StarlarkInteger, BinaryPlusError) {
  starlark_integer zero(0);
  starlark_bool true_obj(true);
  Arena arena;
  error_handler error_callback;

  zero.binary_plus(true_obj, arena, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for +: 'int' and 'bool'");
}

TEST(StarlarkInteger, BinaryMinus) {
  starlark_float f1(1.0);
  starlark_integer i1(3);
  starlark_integer i2(16);
  starlark_bigint b1(number::one << 2);
  Arena arena;
  error_handler error_callback;

  auto* result1 = i2.binary_minus(b1, arena, error_callback);
  auto* result2 = i2.binary_minus(i1, arena, error_callback);
  auto* result3 = i2.binary_minus(f1, arena, error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("12", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("13", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("15.0", result3->str());
}

TEST(StarlarkInteger, BinaryMinusIntegerOverflowDetection) {
  auto test = [](int64_t n, int64_t m, std::string_view r1) {
    starlark_integer nn(n);
    starlark_integer mm(m);
    Arena arena;
    error_handler error_callback;

    auto* result = nn.binary_minus(mm, arena, error_callback);

    EXPECT_THAT(error_callback.messages, SizeIs(0));
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(r1, result->str()) << "N: " << n << ", M: " << m;
  };

  test(std::numeric_limits<std::int64_t>::min(), std::numeric_limits<std::int64_t>::min(), "0");
  test(std::numeric_limits<std::int64_t>::min(), -9223372036854775807L, "-1");
  test(std::numeric_limits<std::int64_t>::min(), -9223372036854775806L, "-2");
  test(std::numeric_limits<std::int64_t>::min(), -2L, "-9223372036854775806");
  test(std::numeric_limits<std::int64_t>::min(), -1L, "-9223372036854775807");
  test(std::numeric_limits<std::int64_t>::min(), 0L, "-9223372036854775808");
  test(std::numeric_limits<std::int64_t>::min(), 1L, "-9223372036854775809");
  test(std::numeric_limits<std::int64_t>::min(), 2L, "-9223372036854775810");
  test(std::numeric_limits<std::int64_t>::min(), 9223372036854775805L, "-18446744073709551613");
  test(std::numeric_limits<std::int64_t>::min(), 9223372036854775806L, "-18446744073709551614");
  test(std::numeric_limits<std::int64_t>::min(), 9223372036854775807L, "-18446744073709551615");
  test(-9223372036854775807L, std::numeric_limits<std::int64_t>::min(), "1");
  test(-9223372036854775807L, -9223372036854775807L, "0");
  test(-9223372036854775807L, -9223372036854775806L, "-1");
  test(-9223372036854775807L, -2L, "-9223372036854775805");
  test(-9223372036854775807L, -1L, "-9223372036854775806");
  test(-9223372036854775807L, 0L, "-9223372036854775807");
  test(-9223372036854775807L, 1L, "-9223372036854775808");
  test(-9223372036854775807L, 2L, "-9223372036854775809");
  test(-9223372036854775807L, 9223372036854775805L, "-18446744073709551612");
  test(-9223372036854775807L, 9223372036854775806L, "-18446744073709551613");
  test(-9223372036854775807L, 9223372036854775807L, "-18446744073709551614");
  test(-9223372036854775806L, std::numeric_limits<std::int64_t>::min(), "2");
  test(-9223372036854775806L, -9223372036854775807L, "1");
  test(-9223372036854775806L, -9223372036854775806L, "0");
  test(-9223372036854775806L, -2L, "-9223372036854775804");
  test(-9223372036854775806L, -1L, "-9223372036854775805");
  test(-9223372036854775806L, 0L, "-9223372036854775806");
  test(-9223372036854775806L, 1L, "-9223372036854775807");
  test(-9223372036854775806L, 2L, "-9223372036854775808");
  test(-9223372036854775806L, 9223372036854775805L, "-18446744073709551611");
  test(-9223372036854775806L, 9223372036854775806L, "-18446744073709551612");
  test(-9223372036854775806L, 9223372036854775807L, "-18446744073709551613");
  test(-2L, std::numeric_limits<std::int64_t>::min(), "9223372036854775806");
  test(-2L, -9223372036854775807L, "9223372036854775805");
  test(-2L, -9223372036854775806L, "9223372036854775804");
  test(-2L, -2L, "0");
  test(-2L, -1L, "-1");
  test(-2L, 0L, "-2");
  test(-2L, 1L, "-3");
  test(-2L, 2L, "-4");
  test(-2L, 9223372036854775805L, "-9223372036854775807");
  test(-2L, 9223372036854775806L, "-9223372036854775808");
  test(-2L, 9223372036854775807L, "-9223372036854775809");
  test(-1L, std::numeric_limits<std::int64_t>::min(), "9223372036854775807");
  test(-1L, -9223372036854775807L, "9223372036854775806");
  test(-1L, -9223372036854775806L, "9223372036854775805");
  test(-1L, -2L, "1");
  test(-1L, -1L, "0");
  test(-1L, 0L, "-1");
  test(-1L, 1L, "-2");
  test(-1L, 2L, "-3");
  test(-1L, 9223372036854775805L, "-9223372036854775806");
  test(-1L, 9223372036854775806L, "-9223372036854775807");
  test(-1L, 9223372036854775807L, "-9223372036854775808");
  test(0L, std::numeric_limits<std::int64_t>::min(), "9223372036854775808");
  test(0L, -9223372036854775807L, "9223372036854775807");
  test(0L, -9223372036854775806L, "9223372036854775806");
  test(0L, -2L, "2");
  test(0L, -1L, "1");
  test(0L, 0L, "0");
  test(0L, 1L, "-1");
  test(0L, 2L, "-2");
  test(0L, 9223372036854775805L, "-9223372036854775805");
  test(0L, 9223372036854775806L, "-9223372036854775806");
  test(0L, 9223372036854775807L, "-9223372036854775807");
  test(1L, std::numeric_limits<std::int64_t>::min(), "9223372036854775809");
  test(1L, -9223372036854775807L, "9223372036854775808");
  test(1L, -9223372036854775806L, "9223372036854775807");
  test(1L, -2L, "3");
  test(1L, -1L, "2");
  test(1L, 0L, "1");
  test(1L, 1L, "0");
  test(1L, 2L, "-1");
  test(1L, 9223372036854775805L, "-9223372036854775804");
  test(1L, 9223372036854775806L, "-9223372036854775805");
  test(1L, 9223372036854775807L, "-9223372036854775806");
  test(2L, std::numeric_limits<std::int64_t>::min(), "9223372036854775810");
  test(2L, -9223372036854775807L, "9223372036854775809");
  test(2L, -9223372036854775806L, "9223372036854775808");
  test(2L, -2L, "4");
  test(2L, -1L, "3");
  test(2L, 0L, "2");
  test(2L, 1L, "1");
  test(2L, 2L, "0");
  test(2L, 9223372036854775805L, "-9223372036854775803");
  test(2L, 9223372036854775806L, "-9223372036854775804");
  test(2L, 9223372036854775807L, "-9223372036854775805");
  test(9223372036854775805L, std::numeric_limits<std::int64_t>::min(), "18446744073709551613");
  test(9223372036854775805L, -9223372036854775807L, "18446744073709551612");
  test(9223372036854775805L, -9223372036854775806L, "18446744073709551611");
  test(9223372036854775805L, -2L, "9223372036854775807");
  test(9223372036854775805L, -1L, "9223372036854775806");
  test(9223372036854775805L, 0L, "9223372036854775805");
  test(9223372036854775805L, 1L, "9223372036854775804");
  test(9223372036854775805L, 2L, "9223372036854775803");
  test(9223372036854775805L, 9223372036854775805L, "0");
  test(9223372036854775805L, 9223372036854775806L, "-1");
  test(9223372036854775805L, 9223372036854775807L, "-2");
  test(9223372036854775806L, std::numeric_limits<std::int64_t>::min(), "18446744073709551614");
  test(9223372036854775806L, -9223372036854775807L, "18446744073709551613");
  test(9223372036854775806L, -9223372036854775806L, "18446744073709551612");
  test(9223372036854775806L, -2L, "9223372036854775808");
  test(9223372036854775806L, -1L, "9223372036854775807");
  test(9223372036854775806L, 0L, "9223372036854775806");
  test(9223372036854775806L, 1L, "9223372036854775805");
  test(9223372036854775806L, 2L, "9223372036854775804");
  test(9223372036854775806L, 9223372036854775805L, "1");
  test(9223372036854775806L, 9223372036854775806L, "0");
  test(9223372036854775806L, 9223372036854775807L, "-1");
  test(9223372036854775807L, std::numeric_limits<std::int64_t>::min(), "18446744073709551615");
  test(9223372036854775807L, -9223372036854775807L, "18446744073709551614");
  test(9223372036854775807L, -9223372036854775806L, "18446744073709551613");
  test(9223372036854775807L, -2L, "9223372036854775809");
  test(9223372036854775807L, -1L, "9223372036854775808");
  test(9223372036854775807L, 0L, "9223372036854775807");
  test(9223372036854775807L, 1L, "9223372036854775806");
  test(9223372036854775807L, 2L, "9223372036854775805");
  test(9223372036854775807L, 9223372036854775805L, "2");
  test(9223372036854775807L, 9223372036854775806L, "1");
  test(9223372036854775807L, 9223372036854775807L, "0");
}

TEST(StarlarkInteger, BinaryMinusError) {
  starlark_integer zero(0);
  starlark_bool true_obj(true);
  Arena arena;
  error_handler error_callback;

  zero.binary_minus(true_obj, arena, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for -: 'int' and 'bool'");
}

TEST(StarlarkInteger, BinaryStar) {
  starlark_float f1(1.0);
  starlark_integer i1(3);
  starlark_integer i2(16);
  starlark_bigint b1(number::one << 2);
  Arena arena;
  error_handler error_callback;

  auto* result1 = i2.binary_star(b1, arena, error_callback);
  auto* result2 = i2.binary_star(i1, arena, error_callback);
  auto* result3 = i2.binary_star(f1, arena, error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("64", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("48", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("16.0", result3->str());
}

TEST(StarlarkInteger, BinaryStarError) {
  starlark_integer zero(0);
  starlark_bool true_obj(true);
  Arena arena;
  error_handler error_callback;

  zero.binary_star(true_obj, arena, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for *: 'int' and 'bool'");
}

TEST(StarlarkInteger, BinarySlash) {
  starlark_float f1(1.0);
  starlark_integer i1(3);
  starlark_integer i2(16);
  starlark_bigint b1(number::one << 2);
  Arena arena;
  error_handler error_callback;

  auto* result1 = i2.binary_slash(b1, arena, error_callback);
  auto* result2 = i2.binary_slash(i1, arena, error_callback);
  auto* result3 = i2.binary_slash(f1, arena, error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("4.0", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("5.333333333333333", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("16.0", result3->str());
}

TEST(StarlarkInteger, BinarySlashError) {
  starlark_integer zero(0);
  starlark_bool true_obj(true);
  Arena arena;
  error_handler error_callback;

  zero.binary_slash(true_obj, arena, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for /: 'int' and 'bool'");
}

TEST(StarlarkInteger, BinarySlashOverflowiDenominatorError) {
  starlark_bigint big(number::one << 1200);
  starlark_integer small(1);
  Arena arena;
  error_handler error_callback;

  small.binary_slash(big, arena, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to float");
}

TEST(StarlarkInteger, BinarySlashZeroFloatError) {
  starlark_float f0(0.0);
  starlark_integer small(1);
  Arena arena;
  error_handler error_callback;

  small.binary_slash(f0, arena, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkInteger, BinarySlashZeroIntError) {
  starlark_integer i0(0);
  starlark_integer small(1);
  Arena arena;
  error_handler error_callback;

  small.binary_slash(i0, arena, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkInteger, BinarySlashZeroBigintError) {
  starlark_bigint b0(number::zero);
  starlark_integer small(1);
  Arena arena;
  error_handler error_callback;

  small.binary_slash(b0, arena, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkInteger, BinarySlashSlash) {
  auto test = [](int64_t n, int64_t d, std::string_view r1, std::string_view r2) {
    starlark_integer num(n);
    starlark_bigint denb(d);
    starlark_integer deni(d);
    starlark_float denf(d);
    Arena arena;
    error_handler error_callback;

    auto* resultb = num.binary_slash_slash(denb, arena, error_callback);
    auto* resulti = num.binary_slash_slash(deni, arena, error_callback);
    auto* resultf = num.binary_slash_slash(denf, arena, error_callback);

    EXPECT_THAT(error_callback.messages, SizeIs(0));
    EXPECT_NE(resultb, nullptr);
    EXPECT_NE(resulti, nullptr);
    EXPECT_NE(resultf, nullptr);
    EXPECT_EQ(r1, resultb->str()) << "Num: " << n << ", den: " << d;
    EXPECT_EQ(r1, resulti->str()) << "Num: " << n << ", den: " << d;
    EXPECT_EQ(r2, resultf->str()) << "Num: " << n << ", den: " << d;
  };

  /*
  ```python
  for a in range(-10, 11):
      for b in range(-10, 11):
          if b!=0:
              print('  test({}, {}, "{}", "{}");'.format(a, b, (a // b), (a // float(b))))
  ```
  */
  test(-10, -10, "1", "1.0");
  test(-10, -9, "1", "1.0");
  test(-10, -8, "1", "1.0");
  test(-10, -7, "1", "1.0");
  test(-10, -6, "1", "1.0");
  test(-10, -5, "2", "2.0");
  test(-10, -4, "2", "2.0");
  test(-10, -3, "3", "3.0");
  test(-10, -2, "5", "5.0");
  test(-10, -1, "10", "10.0");
  test(-10, 1, "-10", "-10.0");
  test(-10, 2, "-5", "-5.0");
  test(-10, 3, "-4", "-4.0");
  test(-10, 4, "-3", "-3.0");
  test(-10, 5, "-2", "-2.0");
  test(-10, 6, "-2", "-2.0");
  test(-10, 7, "-2", "-2.0");
  test(-10, 8, "-2", "-2.0");
  test(-10, 9, "-2", "-2.0");
  test(-10, 10, "-1", "-1.0");
  test(-9, -10, "0", "0.0");
  test(-9, -9, "1", "1.0");
  test(-9, -8, "1", "1.0");
  test(-9, -7, "1", "1.0");
  test(-9, -6, "1", "1.0");
  test(-9, -5, "1", "1.0");
  test(-9, -4, "2", "2.0");
  test(-9, -3, "3", "3.0");
  test(-9, -2, "4", "4.0");
  test(-9, -1, "9", "9.0");
  test(-9, 1, "-9", "-9.0");
  test(-9, 2, "-5", "-5.0");
  test(-9, 3, "-3", "-3.0");
  test(-9, 4, "-3", "-3.0");
  test(-9, 5, "-2", "-2.0");
  test(-9, 6, "-2", "-2.0");
  test(-9, 7, "-2", "-2.0");
  test(-9, 8, "-2", "-2.0");
  test(-9, 9, "-1", "-1.0");
  test(-9, 10, "-1", "-1.0");
  test(-8, -10, "0", "0.0");
  test(-8, -9, "0", "0.0");
  test(-8, -8, "1", "1.0");
  test(-8, -7, "1", "1.0");
  test(-8, -6, "1", "1.0");
  test(-8, -5, "1", "1.0");
  test(-8, -4, "2", "2.0");
  test(-8, -3, "2", "2.0");
  test(-8, -2, "4", "4.0");
  test(-8, -1, "8", "8.0");
  test(-8, 1, "-8", "-8.0");
  test(-8, 2, "-4", "-4.0");
  test(-8, 3, "-3", "-3.0");
  test(-8, 4, "-2", "-2.0");
  test(-8, 5, "-2", "-2.0");
  test(-8, 6, "-2", "-2.0");
  test(-8, 7, "-2", "-2.0");
  test(-8, 8, "-1", "-1.0");
  test(-8, 9, "-1", "-1.0");
  test(-8, 10, "-1", "-1.0");
  test(-7, -10, "0", "0.0");
  test(-7, -9, "0", "0.0");
  test(-7, -8, "0", "0.0");
  test(-7, -7, "1", "1.0");
  test(-7, -6, "1", "1.0");
  test(-7, -5, "1", "1.0");
  test(-7, -4, "1", "1.0");
  test(-7, -3, "2", "2.0");
  test(-7, -2, "3", "3.0");
  test(-7, -1, "7", "7.0");
  test(-7, 1, "-7", "-7.0");
  test(-7, 2, "-4", "-4.0");
  test(-7, 3, "-3", "-3.0");
  test(-7, 4, "-2", "-2.0");
  test(-7, 5, "-2", "-2.0");
  test(-7, 6, "-2", "-2.0");
  test(-7, 7, "-1", "-1.0");
  test(-7, 8, "-1", "-1.0");
  test(-7, 9, "-1", "-1.0");
  test(-7, 10, "-1", "-1.0");
  test(-6, -10, "0", "0.0");
  test(-6, -9, "0", "0.0");
  test(-6, -8, "0", "0.0");
  test(-6, -7, "0", "0.0");
  test(-6, -6, "1", "1.0");
  test(-6, -5, "1", "1.0");
  test(-6, -4, "1", "1.0");
  test(-6, -3, "2", "2.0");
  test(-6, -2, "3", "3.0");
  test(-6, -1, "6", "6.0");
  test(-6, 1, "-6", "-6.0");
  test(-6, 2, "-3", "-3.0");
  test(-6, 3, "-2", "-2.0");
  test(-6, 4, "-2", "-2.0");
  test(-6, 5, "-2", "-2.0");
  test(-6, 6, "-1", "-1.0");
  test(-6, 7, "-1", "-1.0");
  test(-6, 8, "-1", "-1.0");
  test(-6, 9, "-1", "-1.0");
  test(-6, 10, "-1", "-1.0");
  test(-5, -10, "0", "0.0");
  test(-5, -9, "0", "0.0");
  test(-5, -8, "0", "0.0");
  test(-5, -7, "0", "0.0");
  test(-5, -6, "0", "0.0");
  test(-5, -5, "1", "1.0");
  test(-5, -4, "1", "1.0");
  test(-5, -3, "1", "1.0");
  test(-5, -2, "2", "2.0");
  test(-5, -1, "5", "5.0");
  test(-5, 1, "-5", "-5.0");
  test(-5, 2, "-3", "-3.0");
  test(-5, 3, "-2", "-2.0");
  test(-5, 4, "-2", "-2.0");
  test(-5, 5, "-1", "-1.0");
  test(-5, 6, "-1", "-1.0");
  test(-5, 7, "-1", "-1.0");
  test(-5, 8, "-1", "-1.0");
  test(-5, 9, "-1", "-1.0");
  test(-5, 10, "-1", "-1.0");
  test(-4, -10, "0", "0.0");
  test(-4, -9, "0", "0.0");
  test(-4, -8, "0", "0.0");
  test(-4, -7, "0", "0.0");
  test(-4, -6, "0", "0.0");
  test(-4, -5, "0", "0.0");
  test(-4, -4, "1", "1.0");
  test(-4, -3, "1", "1.0");
  test(-4, -2, "2", "2.0");
  test(-4, -1, "4", "4.0");
  test(-4, 1, "-4", "-4.0");
  test(-4, 2, "-2", "-2.0");
  test(-4, 3, "-2", "-2.0");
  test(-4, 4, "-1", "-1.0");
  test(-4, 5, "-1", "-1.0");
  test(-4, 6, "-1", "-1.0");
  test(-4, 7, "-1", "-1.0");
  test(-4, 8, "-1", "-1.0");
  test(-4, 9, "-1", "-1.0");
  test(-4, 10, "-1", "-1.0");
  test(-3, -10, "0", "0.0");
  test(-3, -9, "0", "0.0");
  test(-3, -8, "0", "0.0");
  test(-3, -7, "0", "0.0");
  test(-3, -6, "0", "0.0");
  test(-3, -5, "0", "0.0");
  test(-3, -4, "0", "0.0");
  test(-3, -3, "1", "1.0");
  test(-3, -2, "1", "1.0");
  test(-3, -1, "3", "3.0");
  test(-3, 1, "-3", "-3.0");
  test(-3, 2, "-2", "-2.0");
  test(-3, 3, "-1", "-1.0");
  test(-3, 4, "-1", "-1.0");
  test(-3, 5, "-1", "-1.0");
  test(-3, 6, "-1", "-1.0");
  test(-3, 7, "-1", "-1.0");
  test(-3, 8, "-1", "-1.0");
  test(-3, 9, "-1", "-1.0");
  test(-3, 10, "-1", "-1.0");
  test(-2, -10, "0", "0.0");
  test(-2, -9, "0", "0.0");
  test(-2, -8, "0", "0.0");
  test(-2, -7, "0", "0.0");
  test(-2, -6, "0", "0.0");
  test(-2, -5, "0", "0.0");
  test(-2, -4, "0", "0.0");
  test(-2, -3, "0", "0.0");
  test(-2, -2, "1", "1.0");
  test(-2, -1, "2", "2.0");
  test(-2, 1, "-2", "-2.0");
  test(-2, 2, "-1", "-1.0");
  test(-2, 3, "-1", "-1.0");
  test(-2, 4, "-1", "-1.0");
  test(-2, 5, "-1", "-1.0");
  test(-2, 6, "-1", "-1.0");
  test(-2, 7, "-1", "-1.0");
  test(-2, 8, "-1", "-1.0");
  test(-2, 9, "-1", "-1.0");
  test(-2, 10, "-1", "-1.0");
  test(-1, -10, "0", "0.0");
  test(-1, -9, "0", "0.0");
  test(-1, -8, "0", "0.0");
  test(-1, -7, "0", "0.0");
  test(-1, -6, "0", "0.0");
  test(-1, -5, "0", "0.0");
  test(-1, -4, "0", "0.0");
  test(-1, -3, "0", "0.0");
  test(-1, -2, "0", "0.0");
  test(-1, -1, "1", "1.0");
  test(-1, 1, "-1", "-1.0");
  test(-1, 2, "-1", "-1.0");
  test(-1, 3, "-1", "-1.0");
  test(-1, 4, "-1", "-1.0");
  test(-1, 5, "-1", "-1.0");
  test(-1, 6, "-1", "-1.0");
  test(-1, 7, "-1", "-1.0");
  test(-1, 8, "-1", "-1.0");
  test(-1, 9, "-1", "-1.0");
  test(-1, 10, "-1", "-1.0");
  test(0, -10, "0", "-0.0");
  test(0, -9, "0", "-0.0");
  test(0, -8, "0", "-0.0");
  test(0, -7, "0", "-0.0");
  test(0, -6, "0", "-0.0");
  test(0, -5, "0", "-0.0");
  test(0, -4, "0", "-0.0");
  test(0, -3, "0", "-0.0");
  test(0, -2, "0", "-0.0");
  test(0, -1, "0", "-0.0");
  test(0, 1, "0", "0.0");
  test(0, 2, "0", "0.0");
  test(0, 3, "0", "0.0");
  test(0, 4, "0", "0.0");
  test(0, 5, "0", "0.0");
  test(0, 6, "0", "0.0");
  test(0, 7, "0", "0.0");
  test(0, 8, "0", "0.0");
  test(0, 9, "0", "0.0");
  test(0, 10, "0", "0.0");
  test(1, -10, "-1", "-1.0");
  test(1, -9, "-1", "-1.0");
  test(1, -8, "-1", "-1.0");
  test(1, -7, "-1", "-1.0");
  test(1, -6, "-1", "-1.0");
  test(1, -5, "-1", "-1.0");
  test(1, -4, "-1", "-1.0");
  test(1, -3, "-1", "-1.0");
  test(1, -2, "-1", "-1.0");
  test(1, -1, "-1", "-1.0");
  test(1, 1, "1", "1.0");
  test(1, 2, "0", "0.0");
  test(1, 3, "0", "0.0");
  test(1, 4, "0", "0.0");
  test(1, 5, "0", "0.0");
  test(1, 6, "0", "0.0");
  test(1, 7, "0", "0.0");
  test(1, 8, "0", "0.0");
  test(1, 9, "0", "0.0");
  test(1, 10, "0", "0.0");
  test(2, -10, "-1", "-1.0");
  test(2, -9, "-1", "-1.0");
  test(2, -8, "-1", "-1.0");
  test(2, -7, "-1", "-1.0");
  test(2, -6, "-1", "-1.0");
  test(2, -5, "-1", "-1.0");
  test(2, -4, "-1", "-1.0");
  test(2, -3, "-1", "-1.0");
  test(2, -2, "-1", "-1.0");
  test(2, -1, "-2", "-2.0");
  test(2, 1, "2", "2.0");
  test(2, 2, "1", "1.0");
  test(2, 3, "0", "0.0");
  test(2, 4, "0", "0.0");
  test(2, 5, "0", "0.0");
  test(2, 6, "0", "0.0");
  test(2, 7, "0", "0.0");
  test(2, 8, "0", "0.0");
  test(2, 9, "0", "0.0");
  test(2, 10, "0", "0.0");
  test(3, -10, "-1", "-1.0");
  test(3, -9, "-1", "-1.0");
  test(3, -8, "-1", "-1.0");
  test(3, -7, "-1", "-1.0");
  test(3, -6, "-1", "-1.0");
  test(3, -5, "-1", "-1.0");
  test(3, -4, "-1", "-1.0");
  test(3, -3, "-1", "-1.0");
  test(3, -2, "-2", "-2.0");
  test(3, -1, "-3", "-3.0");
  test(3, 1, "3", "3.0");
  test(3, 2, "1", "1.0");
  test(3, 3, "1", "1.0");
  test(3, 4, "0", "0.0");
  test(3, 5, "0", "0.0");
  test(3, 6, "0", "0.0");
  test(3, 7, "0", "0.0");
  test(3, 8, "0", "0.0");
  test(3, 9, "0", "0.0");
  test(3, 10, "0", "0.0");
  test(4, -10, "-1", "-1.0");
  test(4, -9, "-1", "-1.0");
  test(4, -8, "-1", "-1.0");
  test(4, -7, "-1", "-1.0");
  test(4, -6, "-1", "-1.0");
  test(4, -5, "-1", "-1.0");
  test(4, -4, "-1", "-1.0");
  test(4, -3, "-2", "-2.0");
  test(4, -2, "-2", "-2.0");
  test(4, -1, "-4", "-4.0");
  test(4, 1, "4", "4.0");
  test(4, 2, "2", "2.0");
  test(4, 3, "1", "1.0");
  test(4, 4, "1", "1.0");
  test(4, 5, "0", "0.0");
  test(4, 6, "0", "0.0");
  test(4, 7, "0", "0.0");
  test(4, 8, "0", "0.0");
  test(4, 9, "0", "0.0");
  test(4, 10, "0", "0.0");
  test(5, -10, "-1", "-1.0");
  test(5, -9, "-1", "-1.0");
  test(5, -8, "-1", "-1.0");
  test(5, -7, "-1", "-1.0");
  test(5, -6, "-1", "-1.0");
  test(5, -5, "-1", "-1.0");
  test(5, -4, "-2", "-2.0");
  test(5, -3, "-2", "-2.0");
  test(5, -2, "-3", "-3.0");
  test(5, -1, "-5", "-5.0");
  test(5, 1, "5", "5.0");
  test(5, 2, "2", "2.0");
  test(5, 3, "1", "1.0");
  test(5, 4, "1", "1.0");
  test(5, 5, "1", "1.0");
  test(5, 6, "0", "0.0");
  test(5, 7, "0", "0.0");
  test(5, 8, "0", "0.0");
  test(5, 9, "0", "0.0");
  test(5, 10, "0", "0.0");
  test(6, -10, "-1", "-1.0");
  test(6, -9, "-1", "-1.0");
  test(6, -8, "-1", "-1.0");
  test(6, -7, "-1", "-1.0");
  test(6, -6, "-1", "-1.0");
  test(6, -5, "-2", "-2.0");
  test(6, -4, "-2", "-2.0");
  test(6, -3, "-2", "-2.0");
  test(6, -2, "-3", "-3.0");
  test(6, -1, "-6", "-6.0");
  test(6, 1, "6", "6.0");
  test(6, 2, "3", "3.0");
  test(6, 3, "2", "2.0");
  test(6, 4, "1", "1.0");
  test(6, 5, "1", "1.0");
  test(6, 6, "1", "1.0");
  test(6, 7, "0", "0.0");
  test(6, 8, "0", "0.0");
  test(6, 9, "0", "0.0");
  test(6, 10, "0", "0.0");
  test(7, -10, "-1", "-1.0");
  test(7, -9, "-1", "-1.0");
  test(7, -8, "-1", "-1.0");
  test(7, -7, "-1", "-1.0");
  test(7, -6, "-2", "-2.0");
  test(7, -5, "-2", "-2.0");
  test(7, -4, "-2", "-2.0");
  test(7, -3, "-3", "-3.0");
  test(7, -2, "-4", "-4.0");
  test(7, -1, "-7", "-7.0");
  test(7, 1, "7", "7.0");
  test(7, 2, "3", "3.0");
  test(7, 3, "2", "2.0");
  test(7, 4, "1", "1.0");
  test(7, 5, "1", "1.0");
  test(7, 6, "1", "1.0");
  test(7, 7, "1", "1.0");
  test(7, 8, "0", "0.0");
  test(7, 9, "0", "0.0");
  test(7, 10, "0", "0.0");
  test(8, -10, "-1", "-1.0");
  test(8, -9, "-1", "-1.0");
  test(8, -8, "-1", "-1.0");
  test(8, -7, "-2", "-2.0");
  test(8, -6, "-2", "-2.0");
  test(8, -5, "-2", "-2.0");
  test(8, -4, "-2", "-2.0");
  test(8, -3, "-3", "-3.0");
  test(8, -2, "-4", "-4.0");
  test(8, -1, "-8", "-8.0");
  test(8, 1, "8", "8.0");
  test(8, 2, "4", "4.0");
  test(8, 3, "2", "2.0");
  test(8, 4, "2", "2.0");
  test(8, 5, "1", "1.0");
  test(8, 6, "1", "1.0");
  test(8, 7, "1", "1.0");
  test(8, 8, "1", "1.0");
  test(8, 9, "0", "0.0");
  test(8, 10, "0", "0.0");
  test(9, -10, "-1", "-1.0");
  test(9, -9, "-1", "-1.0");
  test(9, -8, "-2", "-2.0");
  test(9, -7, "-2", "-2.0");
  test(9, -6, "-2", "-2.0");
  test(9, -5, "-2", "-2.0");
  test(9, -4, "-3", "-3.0");
  test(9, -3, "-3", "-3.0");
  test(9, -2, "-5", "-5.0");
  test(9, -1, "-9", "-9.0");
  test(9, 1, "9", "9.0");
  test(9, 2, "4", "4.0");
  test(9, 3, "3", "3.0");
  test(9, 4, "2", "2.0");
  test(9, 5, "1", "1.0");
  test(9, 6, "1", "1.0");
  test(9, 7, "1", "1.0");
  test(9, 8, "1", "1.0");
  test(9, 9, "1", "1.0");
  test(9, 10, "0", "0.0");
  test(10, -10, "-1", "-1.0");
  test(10, -9, "-2", "-2.0");
  test(10, -8, "-2", "-2.0");
  test(10, -7, "-2", "-2.0");
  test(10, -6, "-2", "-2.0");
  test(10, -5, "-2", "-2.0");
  test(10, -4, "-3", "-3.0");
  test(10, -3, "-4", "-4.0");
  test(10, -2, "-5", "-5.0");
  test(10, -1, "-10", "-10.0");
  test(10, 1, "10", "10.0");
  test(10, 2, "5", "5.0");
  test(10, 3, "3", "3.0");
  test(10, 4, "2", "2.0");
  test(10, 5, "2", "2.0");
  test(10, 6, "1", "1.0");
  test(10, 7, "1", "1.0");
  test(10, 8, "1", "1.0");
  test(10, 9, "1", "1.0");
  test(10, 10, "1", "1.0");
}

TEST(StarlarkInteger, BinarySlashSlashError) {
  starlark_integer zero(0);
  starlark_bool true_obj(true);
  Arena arena;
  error_handler error_callback;

  zero.binary_slash_slash(true_obj, arena, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for /: 'int' and 'bool'");
}

TEST(StarlarkInteger, BinarySlashSlashZeroFloatError) {
  starlark_float f0(0.0);
  starlark_integer small(1);
  Arena arena;
  error_handler error_callback;

  small.binary_slash_slash(f0, arena, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkInteger, BinarySlashSlashZeroIntError) {
  starlark_integer i0(0);
  starlark_integer small(1);
  Arena arena;
  error_handler error_callback;

  small.binary_slash_slash(i0, arena, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkInteger, BinarySlashSlashZeroBigintError) {
  starlark_bigint b0(number::zero);
  starlark_integer small(1);
  Arena arena;
  error_handler error_callback;

  small.binary_slash_slash(b0, arena, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkInteger, BinarySlashSlashOverflow) {
  starlark_integer big(std::numeric_limits<int64_t>::min());
  starlark_integer small(-1);
  Arena arena;
  error_handler error_callback;

  auto* result = big.binary_slash_slash(small, arena, error_callback);
  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ("9223372036854775808", result->str());
}

TEST(StarlarkInteger, BinaryPercent) {
  auto test = [](int64_t n, int64_t d, std::string_view r1, std::string_view r2) {
    starlark_integer num(n);
    starlark_bigint denb(d);
    starlark_integer deni(d);
    starlark_float denf(d);
    Arena arena;
    error_handler error_callback;

    auto* resultb = num.binary_percent(denb, arena, error_callback);
    auto* resulti = num.binary_percent(deni, arena, error_callback);
    auto* resultf = num.binary_percent(denf, arena, error_callback);

    EXPECT_THAT(error_callback.messages, SizeIs(0));
    EXPECT_NE(resultb, nullptr);
    EXPECT_NE(resulti, nullptr);
    EXPECT_NE(resultf, nullptr);
    EXPECT_EQ(r1, resultb->str()) << "Num: " << n << ", den: " << d;
    EXPECT_EQ(r1, resulti->str()) << "Num: " << n << ", den: " << d;
    EXPECT_EQ(r2, resultf->str()) << "Num: " << n << ", den: " << d;
  };

  /*
  ```python
  for a in range(-10, 11):
      for b in range(-10, 11):
          if b!=0:
              print('  test({}, {}, "{}", "{}");'.format(a, b, (a % b), (a % float(b))))
  ```
  */
  test(-10, -10, "0", "-0.0");
  test(-10, -9, "-1", "-1.0");
  test(-10, -8, "-2", "-2.0");
  test(-10, -7, "-3", "-3.0");
  test(-10, -6, "-4", "-4.0");
  test(-10, -5, "0", "-0.0");
  test(-10, -4, "-2", "-2.0");
  test(-10, -3, "-1", "-1.0");
  test(-10, -2, "0", "-0.0");
  test(-10, -1, "0", "-0.0");
  test(-10, 1, "0", "0.0");
  test(-10, 2, "0", "0.0");
  test(-10, 3, "2", "2.0");
  test(-10, 4, "2", "2.0");
  test(-10, 5, "0", "0.0");
  test(-10, 6, "2", "2.0");
  test(-10, 7, "4", "4.0");
  test(-10, 8, "6", "6.0");
  test(-10, 9, "8", "8.0");
  test(-10, 10, "0", "0.0");
  test(-9, -10, "-9", "-9.0");
  test(-9, -9, "0", "-0.0");
  test(-9, -8, "-1", "-1.0");
  test(-9, -7, "-2", "-2.0");
  test(-9, -6, "-3", "-3.0");
  test(-9, -5, "-4", "-4.0");
  test(-9, -4, "-1", "-1.0");
  test(-9, -3, "0", "-0.0");
  test(-9, -2, "-1", "-1.0");
  test(-9, -1, "0", "-0.0");
  test(-9, 1, "0", "0.0");
  test(-9, 2, "1", "1.0");
  test(-9, 3, "0", "0.0");
  test(-9, 4, "3", "3.0");
  test(-9, 5, "1", "1.0");
  test(-9, 6, "3", "3.0");
  test(-9, 7, "5", "5.0");
  test(-9, 8, "7", "7.0");
  test(-9, 9, "0", "0.0");
  test(-9, 10, "1", "1.0");
  test(-8, -10, "-8", "-8.0");
  test(-8, -9, "-8", "-8.0");
  test(-8, -8, "0", "-0.0");
  test(-8, -7, "-1", "-1.0");
  test(-8, -6, "-2", "-2.0");
  test(-8, -5, "-3", "-3.0");
  test(-8, -4, "0", "-0.0");
  test(-8, -3, "-2", "-2.0");
  test(-8, -2, "0", "-0.0");
  test(-8, -1, "0", "-0.0");
  test(-8, 1, "0", "0.0");
  test(-8, 2, "0", "0.0");
  test(-8, 3, "1", "1.0");
  test(-8, 4, "0", "0.0");
  test(-8, 5, "2", "2.0");
  test(-8, 6, "4", "4.0");
  test(-8, 7, "6", "6.0");
  test(-8, 8, "0", "0.0");
  test(-8, 9, "1", "1.0");
  test(-8, 10, "2", "2.0");
  test(-7, -10, "-7", "-7.0");
  test(-7, -9, "-7", "-7.0");
  test(-7, -8, "-7", "-7.0");
  test(-7, -7, "0", "-0.0");
  test(-7, -6, "-1", "-1.0");
  test(-7, -5, "-2", "-2.0");
  test(-7, -4, "-3", "-3.0");
  test(-7, -3, "-1", "-1.0");
  test(-7, -2, "-1", "-1.0");
  test(-7, -1, "0", "-0.0");
  test(-7, 1, "0", "0.0");
  test(-7, 2, "1", "1.0");
  test(-7, 3, "2", "2.0");
  test(-7, 4, "1", "1.0");
  test(-7, 5, "3", "3.0");
  test(-7, 6, "5", "5.0");
  test(-7, 7, "0", "0.0");
  test(-7, 8, "1", "1.0");
  test(-7, 9, "2", "2.0");
  test(-7, 10, "3", "3.0");
  test(-6, -10, "-6", "-6.0");
  test(-6, -9, "-6", "-6.0");
  test(-6, -8, "-6", "-6.0");
  test(-6, -7, "-6", "-6.0");
  test(-6, -6, "0", "-0.0");
  test(-6, -5, "-1", "-1.0");
  test(-6, -4, "-2", "-2.0");
  test(-6, -3, "0", "-0.0");
  test(-6, -2, "0", "-0.0");
  test(-6, -1, "0", "-0.0");
  test(-6, 1, "0", "0.0");
  test(-6, 2, "0", "0.0");
  test(-6, 3, "0", "0.0");
  test(-6, 4, "2", "2.0");
  test(-6, 5, "4", "4.0");
  test(-6, 6, "0", "0.0");
  test(-6, 7, "1", "1.0");
  test(-6, 8, "2", "2.0");
  test(-6, 9, "3", "3.0");
  test(-6, 10, "4", "4.0");
  test(-5, -10, "-5", "-5.0");
  test(-5, -9, "-5", "-5.0");
  test(-5, -8, "-5", "-5.0");
  test(-5, -7, "-5", "-5.0");
  test(-5, -6, "-5", "-5.0");
  test(-5, -5, "0", "-0.0");
  test(-5, -4, "-1", "-1.0");
  test(-5, -3, "-2", "-2.0");
  test(-5, -2, "-1", "-1.0");
  test(-5, -1, "0", "-0.0");
  test(-5, 1, "0", "0.0");
  test(-5, 2, "1", "1.0");
  test(-5, 3, "1", "1.0");
  test(-5, 4, "3", "3.0");
  test(-5, 5, "0", "0.0");
  test(-5, 6, "1", "1.0");
  test(-5, 7, "2", "2.0");
  test(-5, 8, "3", "3.0");
  test(-5, 9, "4", "4.0");
  test(-5, 10, "5", "5.0");
  test(-4, -10, "-4", "-4.0");
  test(-4, -9, "-4", "-4.0");
  test(-4, -8, "-4", "-4.0");
  test(-4, -7, "-4", "-4.0");
  test(-4, -6, "-4", "-4.0");
  test(-4, -5, "-4", "-4.0");
  test(-4, -4, "0", "-0.0");
  test(-4, -3, "-1", "-1.0");
  test(-4, -2, "0", "-0.0");
  test(-4, -1, "0", "-0.0");
  test(-4, 1, "0", "0.0");
  test(-4, 2, "0", "0.0");
  test(-4, 3, "2", "2.0");
  test(-4, 4, "0", "0.0");
  test(-4, 5, "1", "1.0");
  test(-4, 6, "2", "2.0");
  test(-4, 7, "3", "3.0");
  test(-4, 8, "4", "4.0");
  test(-4, 9, "5", "5.0");
  test(-4, 10, "6", "6.0");
  test(-3, -10, "-3", "-3.0");
  test(-3, -9, "-3", "-3.0");
  test(-3, -8, "-3", "-3.0");
  test(-3, -7, "-3", "-3.0");
  test(-3, -6, "-3", "-3.0");
  test(-3, -5, "-3", "-3.0");
  test(-3, -4, "-3", "-3.0");
  test(-3, -3, "0", "-0.0");
  test(-3, -2, "-1", "-1.0");
  test(-3, -1, "0", "-0.0");
  test(-3, 1, "0", "0.0");
  test(-3, 2, "1", "1.0");
  test(-3, 3, "0", "0.0");
  test(-3, 4, "1", "1.0");
  test(-3, 5, "2", "2.0");
  test(-3, 6, "3", "3.0");
  test(-3, 7, "4", "4.0");
  test(-3, 8, "5", "5.0");
  test(-3, 9, "6", "6.0");
  test(-3, 10, "7", "7.0");
  test(-2, -10, "-2", "-2.0");
  test(-2, -9, "-2", "-2.0");
  test(-2, -8, "-2", "-2.0");
  test(-2, -7, "-2", "-2.0");
  test(-2, -6, "-2", "-2.0");
  test(-2, -5, "-2", "-2.0");
  test(-2, -4, "-2", "-2.0");
  test(-2, -3, "-2", "-2.0");
  test(-2, -2, "0", "-0.0");
  test(-2, -1, "0", "-0.0");
  test(-2, 1, "0", "0.0");
  test(-2, 2, "0", "0.0");
  test(-2, 3, "1", "1.0");
  test(-2, 4, "2", "2.0");
  test(-2, 5, "3", "3.0");
  test(-2, 6, "4", "4.0");
  test(-2, 7, "5", "5.0");
  test(-2, 8, "6", "6.0");
  test(-2, 9, "7", "7.0");
  test(-2, 10, "8", "8.0");
  test(-1, -10, "-1", "-1.0");
  test(-1, -9, "-1", "-1.0");
  test(-1, -8, "-1", "-1.0");
  test(-1, -7, "-1", "-1.0");
  test(-1, -6, "-1", "-1.0");
  test(-1, -5, "-1", "-1.0");
  test(-1, -4, "-1", "-1.0");
  test(-1, -3, "-1", "-1.0");
  test(-1, -2, "-1", "-1.0");
  test(-1, -1, "0", "-0.0");
  test(-1, 1, "0", "0.0");
  test(-1, 2, "1", "1.0");
  test(-1, 3, "2", "2.0");
  test(-1, 4, "3", "3.0");
  test(-1, 5, "4", "4.0");
  test(-1, 6, "5", "5.0");
  test(-1, 7, "6", "6.0");
  test(-1, 8, "7", "7.0");
  test(-1, 9, "8", "8.0");
  test(-1, 10, "9", "9.0");
  test(0, -10, "0", "-0.0");
  test(0, -9, "0", "-0.0");
  test(0, -8, "0", "-0.0");
  test(0, -7, "0", "-0.0");
  test(0, -6, "0", "-0.0");
  test(0, -5, "0", "-0.0");
  test(0, -4, "0", "-0.0");
  test(0, -3, "0", "-0.0");
  test(0, -2, "0", "-0.0");
  test(0, -1, "0", "-0.0");
  test(0, 1, "0", "0.0");
  test(0, 2, "0", "0.0");
  test(0, 3, "0", "0.0");
  test(0, 4, "0", "0.0");
  test(0, 5, "0", "0.0");
  test(0, 6, "0", "0.0");
  test(0, 7, "0", "0.0");
  test(0, 8, "0", "0.0");
  test(0, 9, "0", "0.0");
  test(0, 10, "0", "0.0");
  test(1, -10, "-9", "-9.0");
  test(1, -9, "-8", "-8.0");
  test(1, -8, "-7", "-7.0");
  test(1, -7, "-6", "-6.0");
  test(1, -6, "-5", "-5.0");
  test(1, -5, "-4", "-4.0");
  test(1, -4, "-3", "-3.0");
  test(1, -3, "-2", "-2.0");
  test(1, -2, "-1", "-1.0");
  test(1, -1, "0", "-0.0");
  test(1, 1, "0", "0.0");
  test(1, 2, "1", "1.0");
  test(1, 3, "1", "1.0");
  test(1, 4, "1", "1.0");
  test(1, 5, "1", "1.0");
  test(1, 6, "1", "1.0");
  test(1, 7, "1", "1.0");
  test(1, 8, "1", "1.0");
  test(1, 9, "1", "1.0");
  test(1, 10, "1", "1.0");
  test(2, -10, "-8", "-8.0");
  test(2, -9, "-7", "-7.0");
  test(2, -8, "-6", "-6.0");
  test(2, -7, "-5", "-5.0");
  test(2, -6, "-4", "-4.0");
  test(2, -5, "-3", "-3.0");
  test(2, -4, "-2", "-2.0");
  test(2, -3, "-1", "-1.0");
  test(2, -2, "0", "-0.0");
  test(2, -1, "0", "-0.0");
  test(2, 1, "0", "0.0");
  test(2, 2, "0", "0.0");
  test(2, 3, "2", "2.0");
  test(2, 4, "2", "2.0");
  test(2, 5, "2", "2.0");
  test(2, 6, "2", "2.0");
  test(2, 7, "2", "2.0");
  test(2, 8, "2", "2.0");
  test(2, 9, "2", "2.0");
  test(2, 10, "2", "2.0");
  test(3, -10, "-7", "-7.0");
  test(3, -9, "-6", "-6.0");
  test(3, -8, "-5", "-5.0");
  test(3, -7, "-4", "-4.0");
  test(3, -6, "-3", "-3.0");
  test(3, -5, "-2", "-2.0");
  test(3, -4, "-1", "-1.0");
  test(3, -3, "0", "-0.0");
  test(3, -2, "-1", "-1.0");
  test(3, -1, "0", "-0.0");
  test(3, 1, "0", "0.0");
  test(3, 2, "1", "1.0");
  test(3, 3, "0", "0.0");
  test(3, 4, "3", "3.0");
  test(3, 5, "3", "3.0");
  test(3, 6, "3", "3.0");
  test(3, 7, "3", "3.0");
  test(3, 8, "3", "3.0");
  test(3, 9, "3", "3.0");
  test(3, 10, "3", "3.0");
  test(4, -10, "-6", "-6.0");
  test(4, -9, "-5", "-5.0");
  test(4, -8, "-4", "-4.0");
  test(4, -7, "-3", "-3.0");
  test(4, -6, "-2", "-2.0");
  test(4, -5, "-1", "-1.0");
  test(4, -4, "0", "-0.0");
  test(4, -3, "-2", "-2.0");
  test(4, -2, "0", "-0.0");
  test(4, -1, "0", "-0.0");
  test(4, 1, "0", "0.0");
  test(4, 2, "0", "0.0");
  test(4, 3, "1", "1.0");
  test(4, 4, "0", "0.0");
  test(4, 5, "4", "4.0");
  test(4, 6, "4", "4.0");
  test(4, 7, "4", "4.0");
  test(4, 8, "4", "4.0");
  test(4, 9, "4", "4.0");
  test(4, 10, "4", "4.0");
  test(5, -10, "-5", "-5.0");
  test(5, -9, "-4", "-4.0");
  test(5, -8, "-3", "-3.0");
  test(5, -7, "-2", "-2.0");
  test(5, -6, "-1", "-1.0");
  test(5, -5, "0", "-0.0");
  test(5, -4, "-3", "-3.0");
  test(5, -3, "-1", "-1.0");
  test(5, -2, "-1", "-1.0");
  test(5, -1, "0", "-0.0");
  test(5, 1, "0", "0.0");
  test(5, 2, "1", "1.0");
  test(5, 3, "2", "2.0");
  test(5, 4, "1", "1.0");
  test(5, 5, "0", "0.0");
  test(5, 6, "5", "5.0");
  test(5, 7, "5", "5.0");
  test(5, 8, "5", "5.0");
  test(5, 9, "5", "5.0");
  test(5, 10, "5", "5.0");
  test(6, -10, "-4", "-4.0");
  test(6, -9, "-3", "-3.0");
  test(6, -8, "-2", "-2.0");
  test(6, -7, "-1", "-1.0");
  test(6, -6, "0", "-0.0");
  test(6, -5, "-4", "-4.0");
  test(6, -4, "-2", "-2.0");
  test(6, -3, "0", "-0.0");
  test(6, -2, "0", "-0.0");
  test(6, -1, "0", "-0.0");
  test(6, 1, "0", "0.0");
  test(6, 2, "0", "0.0");
  test(6, 3, "0", "0.0");
  test(6, 4, "2", "2.0");
  test(6, 5, "1", "1.0");
  test(6, 6, "0", "0.0");
  test(6, 7, "6", "6.0");
  test(6, 8, "6", "6.0");
  test(6, 9, "6", "6.0");
  test(6, 10, "6", "6.0");
  test(7, -10, "-3", "-3.0");
  test(7, -9, "-2", "-2.0");
  test(7, -8, "-1", "-1.0");
  test(7, -7, "0", "-0.0");
  test(7, -6, "-5", "-5.0");
  test(7, -5, "-3", "-3.0");
  test(7, -4, "-1", "-1.0");
  test(7, -3, "-2", "-2.0");
  test(7, -2, "-1", "-1.0");
  test(7, -1, "0", "-0.0");
  test(7, 1, "0", "0.0");
  test(7, 2, "1", "1.0");
  test(7, 3, "1", "1.0");
  test(7, 4, "3", "3.0");
  test(7, 5, "2", "2.0");
  test(7, 6, "1", "1.0");
  test(7, 7, "0", "0.0");
  test(7, 8, "7", "7.0");
  test(7, 9, "7", "7.0");
  test(7, 10, "7", "7.0");
  test(8, -10, "-2", "-2.0");
  test(8, -9, "-1", "-1.0");
  test(8, -8, "0", "-0.0");
  test(8, -7, "-6", "-6.0");
  test(8, -6, "-4", "-4.0");
  test(8, -5, "-2", "-2.0");
  test(8, -4, "0", "-0.0");
  test(8, -3, "-1", "-1.0");
  test(8, -2, "0", "-0.0");
  test(8, -1, "0", "-0.0");
  test(8, 1, "0", "0.0");
  test(8, 2, "0", "0.0");
  test(8, 3, "2", "2.0");
  test(8, 4, "0", "0.0");
  test(8, 5, "3", "3.0");
  test(8, 6, "2", "2.0");
  test(8, 7, "1", "1.0");
  test(8, 8, "0", "0.0");
  test(8, 9, "8", "8.0");
  test(8, 10, "8", "8.0");
  test(9, -10, "-1", "-1.0");
  test(9, -9, "0", "-0.0");
  test(9, -8, "-7", "-7.0");
  test(9, -7, "-5", "-5.0");
  test(9, -6, "-3", "-3.0");
  test(9, -5, "-1", "-1.0");
  test(9, -4, "-3", "-3.0");
  test(9, -3, "0", "-0.0");
  test(9, -2, "-1", "-1.0");
  test(9, -1, "0", "-0.0");
  test(9, 1, "0", "0.0");
  test(9, 2, "1", "1.0");
  test(9, 3, "0", "0.0");
  test(9, 4, "1", "1.0");
  test(9, 5, "4", "4.0");
  test(9, 6, "3", "3.0");
  test(9, 7, "2", "2.0");
  test(9, 8, "1", "1.0");
  test(9, 9, "0", "0.0");
  test(9, 10, "9", "9.0");
  test(10, -10, "0", "-0.0");
  test(10, -9, "-8", "-8.0");
  test(10, -8, "-6", "-6.0");
  test(10, -7, "-4", "-4.0");
  test(10, -6, "-2", "-2.0");
  test(10, -5, "0", "-0.0");
  test(10, -4, "-2", "-2.0");
  test(10, -3, "-2", "-2.0");
  test(10, -2, "0", "-0.0");
  test(10, -1, "0", "-0.0");
  test(10, 1, "0", "0.0");
  test(10, 2, "0", "0.0");
  test(10, 3, "1", "1.0");
  test(10, 4, "2", "2.0");
  test(10, 5, "0", "0.0");
  test(10, 6, "4", "4.0");
  test(10, 7, "3", "3.0");
  test(10, 8, "2", "2.0");
  test(10, 9, "1", "1.0");
  test(10, 10, "0", "0.0");
}

TEST(StarlarkInteger, BinaryPercentError) {
  starlark_integer zero(0);
  starlark_bool true_obj(true);
  Arena arena;
  error_handler error_callback;

  zero.binary_percent(true_obj, arena, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for /: 'int' and 'bool'");
}

TEST(StarlarkInteger, BinaryPercentZeroFloatError) {
  starlark_float f0(0.0);
  starlark_integer small(1);
  Arena arena;
  error_handler error_callback;

  small.binary_percent(f0, arena, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkInteger, BinaryPercentZeroIntError) {
  starlark_integer i0(0);
  starlark_integer small(1);
  Arena arena;
  error_handler error_callback;

  small.binary_percent(i0, arena, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkInteger, BinaryPercentZeroBigintError) {
  starlark_bigint b0(number::zero);
  starlark_integer small(1);
  Arena arena;
  error_handler error_callback;

  small.binary_percent(b0, arena, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkInteger, BinaryPercentOverflow) {
  starlark_integer big(std::numeric_limits<int64_t>::min());
  starlark_integer small(-1);
  Arena arena;
  error_handler error_callback;

  auto* result = big.binary_percent(small, arena, error_callback);
  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ("0", result->str());
}

TEST(StarlarkInteger, Len) {
  error_handler error_callback;

  EXPECT_THAT(starlark_integer(0).len(error_callback), Lt(0));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: object of type 'int' has no len()");
}

TEST(StarlarkInteger, GetIterator) {
  Arena arena;
  error_handler error_callback;

  EXPECT_EQ(nullptr, starlark_integer(0).get_iterator(arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'int' object is not iterable");
}

}  // namespace
