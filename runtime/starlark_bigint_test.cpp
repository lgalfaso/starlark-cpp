// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <limits>
#include <string>
#include <vector>

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
using ::starlark::runtime::error_fn;
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
using ::starlark::runtime::starlark_range;
using ::starlark::runtime::starlark_set;
using ::starlark::runtime::starlark_string;
using ::starlark::runtime::starlark_struct;
using ::starlark::runtime::starlark_tuple;
using ::testing::IsEmpty;
using ::testing::SizeIs;

namespace {

struct error_handler : public error_fn {
  void add_error(std::string_view error_msg) override {
    messages.push_back(std::string(error_msg));
  }

  std::vector<std::string> messages;
};

TEST(StarlarkBigInt, Type) {
  EXPECT_EQ("int", starlark_bigint(1).type());
}

TEST(StarlarkBigInt, Str) {
  EXPECT_EQ("-9223372036854775808", starlark_bigint(std::numeric_limits<int64_t>::min()).str());
  EXPECT_EQ("-9223372036854775807", starlark_bigint(std::numeric_limits<int64_t>::min() + 1).str());
  EXPECT_EQ("-4321", starlark_bigint(-4321).str());
  EXPECT_EQ("0", starlark_bigint(0).str());
  EXPECT_EQ("0", starlark_bigint(starlark::bigint::number::zero).str());
  EXPECT_EQ("4321", starlark_bigint(4321).str());
  EXPECT_EQ("9223372036854775806", starlark_bigint(std::numeric_limits<int64_t>::max() - 1).str());
  EXPECT_EQ("9223372036854775807", starlark_bigint(std::numeric_limits<int64_t>::max()).str());
}

TEST(StarlarkBigInt, Truthy) {
  EXPECT_FALSE(starlark_bigint(0).truthy());
  EXPECT_TRUE(starlark_bigint(1).truthy());
  EXPECT_TRUE(starlark_bigint(-1).truthy());
}

TEST(StarlarkBigInt, Equals) {
  EXPECT_TRUE(starlark_bigint(-1).equals(starlark_integer(-1)));
  EXPECT_TRUE(starlark_bigint(-1).equals(starlark_bigint(-1)));
  EXPECT_TRUE(starlark_bigint(-1).equals(starlark_float(-1)));

  EXPECT_TRUE(starlark_bigint(0).equals(starlark_integer(0)));
  EXPECT_TRUE(starlark_bigint(0).equals(starlark_bigint(0)));
  EXPECT_TRUE(starlark_bigint(0).equals(starlark_float(0)));

  EXPECT_TRUE(starlark_bigint(1).equals(starlark_integer(1)));
  EXPECT_TRUE(starlark_bigint(1).equals(starlark_bigint(1)));
  EXPECT_TRUE(starlark_bigint(1).equals(starlark_float(1)));

  EXPECT_FALSE(starlark_bigint(1).equals(starlark_integer(0)));
  EXPECT_FALSE(starlark_bigint(1).equals(starlark_bigint(0)));
  EXPECT_FALSE(starlark_bigint(1).equals(starlark_float(0)));

  EXPECT_FALSE(starlark_bigint(1).equals(starlark_integer(-1)));
  EXPECT_FALSE(starlark_bigint(1).equals(starlark_bigint(-1)));
  EXPECT_FALSE(starlark_bigint(1).equals(starlark_float(-1)));

  EXPECT_FALSE(starlark_bigint(1).equals(starlark_float(1.1)));
  EXPECT_FALSE(starlark_bigint(0).equals(starlark_float(NAN)));
  EXPECT_FALSE(starlark_bigint(0).equals(starlark_float(std::numeric_limits<double>::infinity())));
  EXPECT_FALSE(starlark_bigint(0).equals(starlark_float(-std::numeric_limits<double>::infinity())));
  EXPECT_FALSE(starlark_bigint(1).equals(starlark_float(16)));
  EXPECT_FALSE(starlark_bigint((starlark::bigint::number::one << 64) + starlark::bigint::number::one).equals(starlark_integer(1)));
}

TEST(StarlarkBigint, Hash) {
  EXPECT_EQ(0, starlark_bigint(0).hash());
  EXPECT_EQ(1, starlark_bigint(1).hash());
  EXPECT_EQ(2, starlark_bigint(2).hash());
  EXPECT_EQ(0x1ffffffffffffffe, starlark_bigint(0x1ffffffffffffffe).hash());
  EXPECT_EQ(0, starlark_bigint(0x1fffffffffffffff).hash());
  EXPECT_EQ(0x1000000000000000, starlark_bigint(0x2fffffffffffffff).hash());
  EXPECT_EQ(1, starlark_bigint(0x3fffffffffffffff).hash());
  EXPECT_EQ(0x1000000000000001, starlark_bigint(0x4fffffffffffffff).hash());
  EXPECT_EQ(2, starlark_bigint(0x5fffffffffffffff).hash());
  EXPECT_EQ(0x1000000000000002, starlark_bigint(0x6fffffffffffffff).hash());
  EXPECT_EQ(-2, starlark_bigint(-1).hash());
  EXPECT_EQ(-2, starlark_bigint(-2).hash());
  EXPECT_EQ(3, starlark_bigint(0x7fffffffffffffff).hash());
  EXPECT_EQ(0x8ec055467e5d2f0, starlark_bigint(starlark::bigint::number::parse_hex("372878134297382479432178392575395348243795483974539854732983475489237589437843728974327985437895798134591087473415034758305861048365874361502763")).hash());
}

TEST(StarlarkBigint, ShiftZero) {
  Arena arena;
  auto* result = starlark_bigint(0).binary_lshift(starlark_integer(1l << 62), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(0).equals(*result));
  result = starlark_bigint(0).binary_rshift(starlark_integer(1l << 62), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(0).equals(*result));
}

TEST(StarlarkBigint, LShift) {
  Arena arena;
  auto* result = starlark_bigint(1).binary_lshift(starlark_integer(3), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_bigint(1 << 3).equals(*result));

  result = starlark_bigint(from_int64(-11)).binary_lshift(starlark_integer(10), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_bigint(from_int64(-11264)).equals(*result));

  result = starlark_bigint(1).binary_lshift(starlark_integer(100), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_bigint(number::one << 100).equals(*result));

  result = starlark_bigint(1).binary_lshift(starlark_integer(1 << 28), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_bigint(number::one << (1 << 28)).equals(*result));

  result = starlark_bigint(1).binary_lshift(starlark_bigint(62), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(1L << 62).equals(*result));

  result = starlark_bigint(1).binary_lshift(starlark_bigint(63), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_bigint(number::one << 63).equals(*result));
}

TEST(StarlarkBigint, RShift) {
  Arena arena;
  auto* result = starlark_bigint(100).binary_rshift(starlark_integer(3), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(100 >> 3).equals(*result));

  result = starlark_bigint(from_int64(-11264)).binary_rshift(starlark_integer(10), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(-11).equals(*result));

  result = starlark_bigint(1).binary_rshift(starlark_integer(64), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(0).equals(*result));

  result = starlark_bigint(from_int64(-2)).binary_rshift(starlark_integer(64), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(-1).equals(*result));

  result = starlark_bigint(20).binary_rshift(starlark_bigint(64), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(0).equals(*result));

  result = starlark_bigint(20).binary_rshift(starlark_bigint(number::one << 64), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(0).equals(*result));

  result = starlark_bigint(20).binary_rshift(starlark_bigint(number::one), arena, nullptr);
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(starlark_integer(10).equals(*result));
}

TEST(StarlarkBigint, ShiftInvalidInput) {
  Arena arena;
  error_handler error_callback;

  auto* result = starlark_bigint(100).binary_rshift(starlark_float(3.0), arena, &error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for >>: 'int' and 'float'");
  error_callback.messages.clear();

  result = starlark_bigint(100).binary_lshift(starlark_float(3.0), arena, &error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for <<: 'int' and 'float'");
  error_callback.messages.clear();

  result = starlark_bigint(100).binary_lshift(starlark_integer(-1), arena, &error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: negative shift count");
  error_callback.messages.clear();

  result = starlark_bigint(100).binary_rshift(starlark_integer(-1), arena, &error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: negative shift count");
  error_callback.messages.clear();

  result = starlark_bigint(100).binary_lshift(starlark_bigint(number::minus_one), arena, &error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: negative shift count");
  error_callback.messages.clear();

  result = starlark_bigint(100).binary_rshift(starlark_bigint(number::minus_one), arena, &error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: negative shift count");
  error_callback.messages.clear();

  result = starlark_bigint(1).binary_lshift(starlark_integer(1 << 29), arena, &error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: too many digits in integer");
  error_callback.messages.clear();

  result = starlark_bigint(1).binary_lshift(starlark_integer(0x7fff'ffff'ffff'ffffL), arena, &error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: too many digits in integer");
  error_callback.messages.clear();

  result = starlark_bigint(100).binary_lshift(starlark_bigint(number::one << 100), arena, &error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: too many digits in integer");
  error_callback.messages.clear();

  result = starlark_bigint(100).binary_lshift(starlark_bigint(number::one << 63), arena, &error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: too many digits in integer");
  error_callback.messages.clear();

  result = starlark_bigint(100).binary_lshift(starlark_bigint(number::one << 62), arena, &error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: too many digits in integer");
  error_callback.messages.clear();
}

TEST(StarlarkBigint, BinaryAnd) {
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
  for (const auto a : values) {
    for (const auto b : values) {
      auto* r = starlark_bigint(from_int64(a)).binary_and(starlark_integer(b), arena, nullptr);
      ASSERT_NE(r, nullptr);
      EXPECT_TRUE(r->equals(starlark_integer(a & b)));
      r = starlark_bigint(from_int64(a)).binary_and(starlark_bigint(from_int64(b)), arena, nullptr);
      ASSERT_NE(r, nullptr);
      EXPECT_TRUE(r->equals(starlark_integer(a & b)));
    }
  }
}

TEST(StarlarkBigint, BinaryAndError) {
  starlark_bigint zero(0);
  starlark_float float_zero(0);
  Arena arena;
  error_handler error_callback;

  zero.binary_and(float_zero, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for &: 'int' and 'float'");
}

TEST(StarlarkBigint, BinaryOr) {
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
  for (const auto a : values) {
    for (const auto b : values) {
      auto* r = starlark_bigint(from_int64(a)).binary_pipe(starlark_integer(b), arena, nullptr);
      ASSERT_NE(r, nullptr);
      EXPECT_TRUE(r->equals(starlark_integer(a | b)));
      r = starlark_bigint(from_int64(a)).binary_pipe(starlark_bigint(from_int64(b)), arena, nullptr);
      ASSERT_NE(r, nullptr);
      EXPECT_TRUE(r->equals(starlark_integer(a | b)));
    }
  }
}

TEST(StarlarkBigint, BinaryOrError) {
  starlark_bigint zero(0);
  starlark_float float_zero(0);
  Arena arena;
  error_handler error_callback;

  zero.binary_pipe(float_zero, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for |: 'int' and 'float'");
}

TEST(StarlarkBigint, BinaryXor) {
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
  for (const auto a : values) {
    for (const auto b : values) {
      auto* r = starlark_bigint(from_int64(a)).binary_hat(starlark_integer(b), arena, nullptr);
      ASSERT_NE(r, nullptr);
      EXPECT_TRUE(r->equals(starlark_integer(a ^ b)));
      r = starlark_bigint(from_int64(a)).binary_hat(starlark_bigint(from_int64(b)), arena, nullptr);
      ASSERT_NE(r, nullptr);
      EXPECT_TRUE(r->equals(starlark_integer(a ^ b)));
    }
  }
}

TEST(StarlarkBigint, BinaryXorError) {
  starlark_bigint zero(0);
  starlark_float float_zero(0);
  Arena arena;
  error_handler error_callback;

  zero.binary_hat(float_zero, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for ^: 'int' and 'float'");
}

TEST(StarlarkBigint, BinaryPlus) {
  starlark_float f1(1.0);
  starlark_integer i1(3);
  starlark_bigint b1(number::one << 2);
  starlark_bigint b2(number::one << 4);
  Arena arena;
  error_handler error_callback;

  auto* result1 = b1.binary_plus(b2, arena, &error_callback);
  auto* result2 = b1.binary_plus(i1, arena, &error_callback);
  auto* result3 = b1.binary_plus(f1, arena, &error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("20", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("7", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("5.0", result3->str());
}

TEST(StarlarkBigint, BinaryPlusError) {
  starlark_bigint zero(0);
  starlark_bool true_obj(true);
  Arena arena;
  error_handler error_callback;

  zero.binary_plus(true_obj, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for +: 'int' and 'bool'");
}

TEST(StarlarkBigint, BinaryPlusOverflowError) {
  starlark_bigint big(number::one << 1200);
  starlark_float f1(1.0);
  Arena arena;
  error_handler error_callback;

  big.binary_plus(f1, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to float");
}

TEST(StarlarkBigint, BinaryMinus) {
  starlark_float f1(2.0);
  starlark_integer i1(4);
  starlark_bigint b1(number::one << 3);
  starlark_bigint b2(number::one << 5);
  Arena arena;
  error_handler error_callback;

  auto* result1 = b1.binary_minus(b2, arena, &error_callback);
  auto* result2 = b1.binary_minus(i1, arena, &error_callback);
  auto* result3 = b1.binary_minus(f1, arena, &error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("-24", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("4", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("6.0", result3->str());
}

TEST(StarlarkBigint, BinaryMinusError) {
  starlark_bigint zero(0);
  starlark_bool true_obj(true);
  Arena arena;
  error_handler error_callback;

  zero.binary_minus(true_obj, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for -: 'int' and 'bool'");
}

TEST(StarlarkBigint, BinaryMinusOverflowError) {
  starlark_bigint big(number::one << 1200);
  starlark_float f1(1.0);
  Arena arena;
  error_handler error_callback;

  big.binary_minus(f1, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to float");
}

TEST(StarlarkBigint, BinaryStar) {
  starlark_float f1(2.0);
  starlark_integer i1(4);
  starlark_bigint b1(number::one << 3);
  starlark_bigint b2(number::one << 5);
  Arena arena;
  error_handler error_callback;

  auto* result1 = b1.binary_star(b2, arena, &error_callback);
  auto* result2 = b1.binary_star(i1, arena, &error_callback);
  auto* result3 = b1.binary_star(f1, arena, &error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("256", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("32", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("16.0", result3->str());
}

TEST(StarlarkBigint, BinaryStarError) {
  starlark_bigint zero(0);
  starlark_bool true_obj(true);
  Arena arena;
  error_handler error_callback;

  zero.binary_star(true_obj, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for *: 'int' and 'bool'");
}

TEST(StarlarkBigint, BinaryStarOverflowError) {
  starlark_bigint big(number::one << 1200);
  starlark_float f1(1.0);
  Arena arena;
  error_handler error_callback;

  big.binary_star(f1, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to float");
}

TEST(StarlarkBigint, BinarySlash) {
  starlark_float f1(2.0);
  starlark_integer i1(4);
  starlark_bigint b1(number::one << 3);
  starlark_bigint b2(number::one << 5);
  Arena arena;
  error_handler error_callback;

  auto* result1 = b1.binary_slash(b2, arena, &error_callback);
  auto* result2 = b1.binary_slash(i1, arena, &error_callback);
  auto* result3 = b1.binary_slash(f1, arena, &error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("0.25", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("2.0", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("4.0", result3->str());
}

TEST(StarlarkBigint, BinarySlashError) {
  starlark_bigint zero(0);
  starlark_bool true_obj(true);
  Arena arena;
  error_handler error_callback;

  zero.binary_slash(true_obj, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for /: 'int' and 'bool'");
}

TEST(StarlarkBigint, BinarySlashOverflowError) {
  starlark_bigint big(number::one << 1200);
  starlark_float f1(1.0);
  Arena arena;
  error_handler error_callback;

  big.binary_slash(f1, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to float");
}

TEST(StarlarkBigint, BinarySlashOverflowDenominatorError) {
  starlark_bigint big(number::one << 1200);
  starlark_bigint small(number::one);
  Arena arena;
  error_handler error_callback;

  small.binary_slash(big, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to float");
}

TEST(StarlarkBigint, BinarySlashZeroFloatError) {
  starlark_float f0(0.0);
  starlark_bigint small(number::one);
  Arena arena;
  error_handler error_callback;

  small.binary_slash(f0, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkBigint, BinarySlashZeroIntError) {
  starlark_integer i0(0);
  starlark_bigint small(number::one);
  Arena arena;
  error_handler error_callback;

  small.binary_slash(i0, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkBigint, BinarySlashZeroBigintError) {
  starlark_bigint b0(number::zero);
  starlark_bigint small(number::one);
  Arena arena;
  error_handler error_callback;

  small.binary_slash(b0, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkBigint, BinarySlashSlash) {
  auto test = [](int64_t n, int64_t d, std::string_view r1, std::string_view r2) {
    starlark_bigint num(n);
    starlark_bigint denb(d);
    starlark_integer deni(d);
    starlark_float denf(d);
    Arena arena;
    error_handler error_callback;

    auto* resultb = num.binary_slash_slash(denb, arena, &error_callback);
    auto* resulti = num.binary_slash_slash(deni, arena, &error_callback);
    auto* resultf = num.binary_slash_slash(denf, arena, &error_callback);

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

TEST(StarlarkBigint, BinarySlashSlashError) {
  starlark_bigint zero(0);
  starlark_bool true_obj(true);
  Arena arena;
  error_handler error_callback;

  zero.binary_slash_slash(true_obj, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for //: 'int' and 'bool'");
}

TEST(StarlarkBigint, BinarySlashSlashOverflowError) {
  starlark_bigint big(number::one << 1200);
  starlark_float f1(1.0);
  Arena arena;
  error_handler error_callback;

  big.binary_slash_slash(f1, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to float");
}

TEST(StarlarkBigint, BinarySlashSlashOverflowiDenominatorError) {
  starlark_bigint big(number::one << 1200);
  starlark_bigint small(number::one);
  Arena arena;
  error_handler error_callback;

  small.binary_slash_slash(big, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkBigint, BinarySlashSlashZeroFloatError) {
  starlark_float f0(0.0);
  starlark_bigint small(number::one);
  Arena arena;
  error_handler error_callback;

  small.binary_slash_slash(f0, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkBigint, BinarySlashSlashZeroIntError) {
  starlark_integer i0(0);
  starlark_bigint small(number::one);
  Arena arena;
  error_handler error_callback;

  small.binary_slash_slash(i0, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkBigint, BinarySlashSlashZeroBigintError) {
  starlark_bigint b0(number::zero);
  starlark_bigint small(number::one);
  Arena arena;
  error_handler error_callback;

  small.binary_slash_slash(b0, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkBigint, BinaryPercent) {
  auto test = [](int64_t n, int64_t d, std::string_view r1, std::string_view r2) {
    starlark_bigint num(n);
    starlark_bigint denb(d);
    starlark_integer deni(d);
    starlark_float denf(d);
    Arena arena;
    error_handler error_callback;

    auto* resultb = num.binary_percent(denb, arena, &error_callback);
    auto* resulti = num.binary_percent(deni, arena, &error_callback);
    auto* resultf = num.binary_percent(denf, arena, &error_callback);

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

TEST(StarlarkBigint, BinaryPercentError) {
  starlark_bigint zero(0);
  starlark_bool true_obj(true);
  Arena arena;
  error_handler error_callback;

  zero.binary_percent(true_obj, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for %: 'int' and 'bool'");
}

TEST(StarlarkBigint, BinaryPercentOverflowError) {
  starlark_bigint big(number::one << 1200);
  starlark_float f1(1.0);
  Arena arena;
  error_handler error_callback;

  big.binary_percent(f1, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to float");
}

TEST(StarlarkBigint, BinaryPercentOverflowiDenominatorError) {
  starlark_bigint big(number::one << 1200);
  starlark_bigint small(number::one);
  Arena arena;
  error_handler error_callback;

  small.binary_percent(big, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkBigint, BinaryPercentZeroFloatError) {
  starlark_float f0(0.0);
  starlark_bigint small(number::one);
  Arena arena;
  error_handler error_callback;

  small.binary_percent(f0, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkBigint, BinaryPercentZeroIntError) {
  starlark_integer i0(0);
  starlark_bigint small(number::one);
  Arena arena;
  error_handler error_callback;

  small.binary_percent(i0, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkBigint, BinaryPercentZeroBigintError) {
  starlark_bigint b0(number::zero);
  starlark_bigint small(number::one);
  Arena arena;
  error_handler error_callback;

  small.binary_percent(b0, arena, &error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

}  // namespace
