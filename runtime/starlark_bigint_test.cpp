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

}  // namespace
