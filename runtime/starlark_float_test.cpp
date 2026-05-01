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
#include "runtime/starlark_testing.hpp"
#include "runtime/starlark_tuple.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;
using ::starlark::bigint::parse_number;
using ::starlark::runtime::context;
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
using ::starlark::runtime::starlark_tuple;
using ::starlark::testing::error_handler;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::IsEmpty;
using ::testing::Lt;
using ::testing::SizeIs;

namespace {

TEST(StarlarkFloat, Type) {
  EXPECT_EQ("float", starlark_float(1).type());
}

TEST(StarlarkFloat, Primitve) {
  EXPECT_TRUE(starlark_float(1).primitive());
}

TEST(StarlarkFloat, Str) {
  // Follow Python results as much as possible.
  EXPECT_EQ("0.0", starlark_float(0.0).str());
  EXPECT_EQ("-0.0", starlark_float(-0.0).str());
  EXPECT_EQ("1.234", starlark_float(1.234).str());
  EXPECT_EQ("1.0", starlark_float(1.0).str());
  EXPECT_EQ("10.0", starlark_float(10.0).str());
  EXPECT_EQ("100.0", starlark_float(100.0).str());
  EXPECT_EQ("1000.0", starlark_float(1000.0).str());
  EXPECT_EQ("10000.0", starlark_float(10000.0).str());
  EXPECT_EQ("100000.0", starlark_float(100000.0).str());
  EXPECT_EQ("1000000.0", starlark_float(1000000.0).str());
  EXPECT_EQ("10000000.0", starlark_float(10000000.0).str());
  EXPECT_EQ("100000000.0", starlark_float(100000000.0).str());
  EXPECT_EQ("1000000000.0", starlark_float(1000000000.0).str());
  EXPECT_EQ("10000000000.0", starlark_float(10000000000.0).str());
  EXPECT_EQ("100000000000.0", starlark_float(100000000000.0).str());
  EXPECT_EQ("1000000000000.0", starlark_float(1000000000000.0).str());
  EXPECT_EQ("10000000000000.0", starlark_float(10000000000000.0).str());
  EXPECT_EQ("100000000000000.0", starlark_float(100000000000000.0).str());
  EXPECT_EQ("1000000000000000.0", starlark_float(1000000000000000.0).str());
  EXPECT_EQ("1e+16", starlark_float(10000000000000000.0).str());
  EXPECT_EQ("1e+17", starlark_float(100000000000000000.0).str());
  EXPECT_EQ("1234567890123456.0", starlark_float(1234567890123456.0).str());
  EXPECT_EQ("1.2345678901234568e+16", starlark_float(12345678901234567.0).str());
  EXPECT_EQ("1.0000000000000004e+18", starlark_float(1000000000000000321.0).str());
  EXPECT_EQ("-1.234", starlark_float(-1.234).str());
  EXPECT_EQ("-1.0", starlark_float(-1.0).str());
  EXPECT_EQ("-10.0", starlark_float(-10.0).str());
  EXPECT_EQ("-100.0", starlark_float(-100.0).str());
  EXPECT_EQ("-1000.0", starlark_float(-1000.0).str());
  EXPECT_EQ("-10000.0", starlark_float(-10000.0).str());
  EXPECT_EQ("-100000.0", starlark_float(-100000.0).str());
  EXPECT_EQ("-1000000.0", starlark_float(-1000000.0).str());
  EXPECT_EQ("-10000000.0", starlark_float(-10000000.0).str());
  EXPECT_EQ("-100000000.0", starlark_float(-100000000.0).str());
  EXPECT_EQ("-1000000000.0", starlark_float(-1000000000.0).str());
  EXPECT_EQ("-10000000000.0", starlark_float(-10000000000.0).str());
  EXPECT_EQ("-100000000000.0", starlark_float(-100000000000.0).str());
  EXPECT_EQ("-1000000000000.0", starlark_float(-1000000000000.0).str());
  EXPECT_EQ("-10000000000000.0", starlark_float(-10000000000000.0).str());
  EXPECT_EQ("-100000000000000.0", starlark_float(-100000000000000.0).str());
  EXPECT_EQ("-1000000000000000.0", starlark_float(-1000000000000000.0).str());
  EXPECT_EQ("-1e+16", starlark_float(-10000000000000000.0).str());
  EXPECT_EQ("-1e+17", starlark_float(-100000000000000000.0).str());
  EXPECT_EQ("-1234567890123456.0", starlark_float(-1234567890123456.0).str());
  EXPECT_EQ("-1.2345678901234568e+16", starlark_float(-12345678901234567.0).str());
  EXPECT_EQ("-1.0000000000000004e+18", starlark_float(-1000000000000000321.0).str());
  EXPECT_EQ("nan", starlark_float(std::numeric_limits<double>::quiet_NaN()).str());
  EXPECT_EQ("inf", starlark_float(std::numeric_limits<double>::infinity()).str());
  EXPECT_EQ("-inf", starlark_float(-std::numeric_limits<double>::infinity()).str());
}

TEST(StarlarkFloat, Truthy) {
  EXPECT_TRUE(starlark_float(-std::numeric_limits<double>::infinity()).truthy());
  EXPECT_TRUE(starlark_float(-1.0).truthy());
  EXPECT_FALSE(starlark_float(-0.0).truthy());
  EXPECT_FALSE(starlark_float(0.0).truthy());
  EXPECT_TRUE(starlark_float(1.0).truthy());
  EXPECT_TRUE(starlark_float(std::numeric_limits<double>::infinity()).truthy());
  EXPECT_TRUE(starlark_float(std::numeric_limits<double>::quiet_NaN()).truthy());
}

TEST(StarlarkFloat, Equals) {
  EXPECT_TRUE(starlark_float(-1).equals(starlark_integer(-1)));
  EXPECT_TRUE(starlark_float(-1).equals(starlark_bigint(-1)));
  EXPECT_TRUE(starlark_float(-1).equals(starlark_float(-1)));

  EXPECT_TRUE(starlark_float(0).equals(starlark_integer(0)));
  EXPECT_TRUE(starlark_float(0).equals(starlark_bigint(0)));
  EXPECT_TRUE(starlark_float(0).equals(starlark_float(0)));

  EXPECT_TRUE(starlark_float(1).equals(starlark_integer(1)));
  EXPECT_TRUE(starlark_float(1).equals(starlark_bigint(1)));
  EXPECT_TRUE(starlark_float(1).equals(starlark_float(1)));

  EXPECT_FALSE(starlark_float(1).equals(starlark_integer(0)));
  EXPECT_FALSE(starlark_float(1).equals(starlark_bigint(0)));
  EXPECT_FALSE(starlark_float(1).equals(starlark_float(0)));

  EXPECT_FALSE(starlark_float(1).equals(starlark_integer(-1)));
  EXPECT_FALSE(starlark_float(1).equals(starlark_bigint(-1)));
  EXPECT_FALSE(starlark_float(1).equals(starlark_float(-1)));

  EXPECT_FALSE(starlark_float(1.1).equals(starlark_integer(1)));
  EXPECT_FALSE(starlark_float(1.1).equals(starlark_bigint(1)));
  EXPECT_TRUE(starlark_float(-0.0).equals(starlark_float(0.0)));
  // Starlark mandates that `NaN == NaN`.
  EXPECT_TRUE(starlark_float(NAN).equals(starlark_float(NAN)));

  EXPECT_FALSE(starlark_float(0).equals(starlark_bool(false)));
}

TEST(StarlarkFloat, EqualsExact) {
  EXPECT_FALSE(starlark_float((1L<<53)+1).equals(starlark_integer((1L<<53)+1)));
  EXPECT_FALSE(starlark_float((1L<<53)+1).equals(starlark_bigint((1L<<53)+1)));
  EXPECT_TRUE(starlark_float(std::numeric_limits<int64_t>::min()).equals(starlark_integer(std::numeric_limits<int64_t>::min())));
  EXPECT_TRUE(starlark_float(std::numeric_limits<int64_t>::min()).equals(starlark_bigint(std::numeric_limits<int64_t>::min())));
  EXPECT_FALSE(starlark_float(std::numeric_limits<int64_t>::max()).equals(starlark_integer(std::numeric_limits<int64_t>::max())));
  EXPECT_FALSE(starlark_float(std::numeric_limits<int64_t>::max()).equals(starlark_bigint(std::numeric_limits<int64_t>::max())));
  EXPECT_FALSE(starlark_float(std::numeric_limits<double>::infinity()).equals(starlark_integer(std::numeric_limits<int64_t>::max())));
  EXPECT_FALSE(starlark_float(std::numeric_limits<double>::infinity()).equals(starlark_bigint(number::one() << 2000)));
}

TEST(StarlarkFloat, EqualsVsBigInt) {
  EXPECT_TRUE(starlark_float(1e50).equals(starlark_bigint(starlark::bigint::parse_number("100000000000000007629769841091887003294964970946560", nullptr, 0))));
}

TEST(StarlarkFloat, Hash) {
  EXPECT_EQ(starlark_float(1e50).hash(), 1387127493139725924);
  EXPECT_EQ(starlark_bigint(starlark::bigint::parse_number("100000000000000007629769841091887003294964970946560", nullptr, 0)).hash(), 1387127493139725924);
}

void cmp_helper(starlark::result::status_or<int> cmp, auto matcher) {
  ASSERT_TRUE(cmp.ok());
  EXPECT_THAT(*cmp, matcher);
}

TEST(StarlarkFloat, OrderVsFloat) {
  error_handler error_callback;

  cmp_helper(starlark_float(-std::numeric_limits<double>::infinity()).cmp(starlark_float(-1e50), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(-1e50).cmp(starlark_float(-1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(-1).cmp(starlark_float(-1e-50), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(-1e-50).cmp(starlark_float(0.0), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(0.0).cmp(starlark_float(1e-50), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(1e-50).cmp(starlark_float(1.0), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(1.0).cmp(starlark_float(1e50), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(1e50).cmp(starlark_float(std::numeric_limits<double>::infinity()), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(std::numeric_limits<double>::infinity()).cmp(starlark_float(NAN), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(NAN).cmp(starlark_float(std::numeric_limits<double>::infinity()), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(NAN).cmp(starlark_float(NAN), "cmp", error_callback), Eq(0));
}

TEST(StarlarkFloat, OrderVsInteger) {
  error_handler error_callback;

  cmp_helper(starlark_float(-std::numeric_limits<double>::infinity()).cmp(starlark_integer(0), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(std::numeric_limits<double>::infinity()).cmp(starlark_integer(0), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(NAN).cmp(starlark_integer(0), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(-1).cmp(starlark_integer(0), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(1).cmp(starlark_integer(0), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(0).cmp(starlark_integer(-1), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(0).cmp(starlark_integer(1), "cmp", error_callback), Lt(0));

  cmp_helper(starlark_float(-1).cmp(starlark_integer(-1), "cmp", error_callback), Eq(0));
  cmp_helper(starlark_float(-1).cmp(starlark_integer(1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(1).cmp(starlark_integer(-1), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(1).cmp(starlark_integer(1), "cmp", error_callback), Eq(0));


  cmp_helper(starlark_float(0).cmp(starlark_integer(0), "cmp", error_callback), Eq(0));

  cmp_helper(starlark_float(-2).cmp(starlark_integer(-2), "cmp", error_callback), Eq(0));
  cmp_helper(starlark_float(-2).cmp(starlark_integer(-1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(-2).cmp(starlark_integer(1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(-2).cmp(starlark_integer(2), "cmp", error_callback), Lt(0));

  cmp_helper(starlark_float(-1).cmp(starlark_integer(-2), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(-1).cmp(starlark_integer(-1), "cmp", error_callback), Eq(0));
  cmp_helper(starlark_float(-1).cmp(starlark_integer(1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(-1).cmp(starlark_integer(2), "cmp", error_callback), Lt(0));

  cmp_helper(starlark_float(1).cmp(starlark_integer(-2), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(1).cmp(starlark_integer(-1), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(1).cmp(starlark_integer(1), "cmp", error_callback), Eq(0));
  cmp_helper(starlark_float(1).cmp(starlark_integer(2), "cmp", error_callback), Lt(0));

  cmp_helper(starlark_float(2).cmp(starlark_integer(-2), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(2).cmp(starlark_integer(-1), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(2).cmp(starlark_integer(1), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(2).cmp(starlark_integer(2), "cmp", error_callback), Eq(0));

  cmp_helper(starlark_float(-1.25).cmp(starlark_integer(-1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(1.25).cmp(starlark_integer(1), "cmp", error_callback), Gt(0));

  cmp_helper(starlark_float((1L << 53) + 1).cmp(starlark_integer((1L << 53) + 1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float((1L << 53) + (1L << 49) + 1).cmp(starlark_integer((1L << 53) + (1L << 50) + 1), "cmp", error_callback), Lt(0));
}

TEST(StarlarkFloat, OrderVsBigInt) {
  error_handler error_callback;

  cmp_helper(starlark_float(-std::numeric_limits<double>::infinity()).cmp(starlark_bigint(0), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(std::numeric_limits<double>::infinity()).cmp(starlark_bigint(0), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(NAN).cmp(starlark_bigint(0), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(-1).cmp(starlark_bigint(0), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(1).cmp(starlark_bigint(0), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(0).cmp(starlark_bigint(-1), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(0).cmp(starlark_bigint(1), "cmp", error_callback), Lt(0));

  cmp_helper(starlark_float(-1).cmp(starlark_bigint(-1), "cmp", error_callback), Eq(0));
  cmp_helper(starlark_float(-1).cmp(starlark_bigint(1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(1).cmp(starlark_bigint(-1), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(1).cmp(starlark_bigint(1), "cmp", error_callback), Eq(0));


  cmp_helper(starlark_float(0).cmp(starlark_bigint(0), "cmp", error_callback), Eq(0));

  cmp_helper(starlark_float(-2).cmp(starlark_bigint(-2), "cmp", error_callback), Eq(0));
  cmp_helper(starlark_float(-2).cmp(starlark_bigint(-1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(-2).cmp(starlark_bigint(1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(-2).cmp(starlark_bigint(2), "cmp", error_callback), Lt(0));

  cmp_helper(starlark_float(-1).cmp(starlark_bigint(-2), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(-1).cmp(starlark_bigint(-1), "cmp", error_callback), Eq(0));
  cmp_helper(starlark_float(-1).cmp(starlark_bigint(1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(-1).cmp(starlark_bigint(2), "cmp", error_callback), Lt(0));

  cmp_helper(starlark_float(1).cmp(starlark_bigint(-2), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(1).cmp(starlark_bigint(-1), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(1).cmp(starlark_bigint(1), "cmp", error_callback), Eq(0));
  cmp_helper(starlark_float(1).cmp(starlark_bigint(2), "cmp", error_callback), Lt(0));

  cmp_helper(starlark_float(2).cmp(starlark_bigint(-2), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(2).cmp(starlark_bigint(-1), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(2).cmp(starlark_bigint(1), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(2).cmp(starlark_bigint(2), "cmp", error_callback), Eq(0));


  cmp_helper(starlark_float(1e50).cmp(starlark_bigint(parse_number("100100000000000007629769841091887003294964970946560", nullptr, 0)), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(1e50).cmp(starlark_bigint(parse_number("100000000000000006629769841091887003294964970946560", nullptr, 0)), "cmp", error_callback), Gt(0));

  cmp_helper(starlark_float(-1.25).cmp(starlark_bigint(-1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(1.25).cmp(starlark_bigint(1), "cmp", error_callback), Gt(0));

  cmp_helper(starlark_float(1e50).cmp(starlark_bigint(parse_number("100000000000000007629769841091887003294964970946559", nullptr, 0)), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(1e50).cmp(starlark_bigint(parse_number("100000000000000007629769841091887003294964970946560", nullptr, 0)), "cmp", error_callback), Eq(0));
  cmp_helper(starlark_float(1e50).cmp(starlark_bigint(parse_number("100000000000000007629769841091887003294964970946561", nullptr, 0)), "cmp", error_callback), Lt(0));
}

TEST(StarlarkFloat, OrderVsBool) {
  error_handler error_callback;
  starlark_bool obj_true(true);

  auto cmp = starlark_float(1).cmp(obj_true, "<", error_callback);
  ASSERT_FALSE(cmp.ok());
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: '<' not supported between instances of 'float' and 'bool'");
}

TEST(StarlarkFloat, OrderExact) {
  error_handler error_callback;

  cmp_helper(starlark_float((1L<<53)+1).cmp(starlark_integer((1L<<53)+1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float((1L<<53)+1).cmp(starlark_bigint((1L<<53)+1), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_float(std::numeric_limits<int64_t>::min()).cmp(starlark_integer(std::numeric_limits<int64_t>::min()), "cmp", error_callback), Eq(0));
  cmp_helper(starlark_float(std::numeric_limits<int64_t>::min()).cmp(starlark_bigint(std::numeric_limits<int64_t>::min()), "cmp", error_callback), Eq(0));
  cmp_helper(starlark_float(std::numeric_limits<int64_t>::max()).cmp(starlark_integer(std::numeric_limits<int64_t>::max()), "cmp", error_callback), Gt(0));
  cmp_helper(starlark_float(std::numeric_limits<int64_t>::max()).cmp(starlark_bigint(std::numeric_limits<int64_t>::max()), "cmp", error_callback), Gt(0));
  ASSERT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkFloat, PlusEqualsAssign) {
  starlark_float f1(1.0);
  starlark_float f2(2.0);
  starlark_integer i1(3);
  starlark_bigint b1(number::one() << 2);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result1 = f1.plus_equals_assign(f2, ctx, error_callback);
  auto* result2 = f1.plus_equals_assign(i1, ctx, error_callback);
  auto* result3 = f1.plus_equals_assign(b1, ctx, error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("3.0", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("4.0", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("5.0", result3->str());
}

TEST(StarlarkFloat, PlusEqualsAssignError) {
  starlark_float f1(0);
  starlark_bool true_obj(true);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.plus_equals_assign(true_obj, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for +=: 'float' and 'bool'");
}

TEST(StarlarkFloat, BinaryPlus) {
  starlark_float f1(1.0);
  starlark_float f2(2.0);
  starlark_integer i1(3);
  starlark_bigint b1(number::one() << 2);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result1 = f1.binary_plus(f2, ctx, error_callback);
  auto* result2 = f1.binary_plus(i1, ctx, error_callback);
  auto* result3 = f1.binary_plus(b1, ctx, error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("3.0", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("4.0", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("5.0", result3->str());
}

TEST(StarlarkFloat, BinaryPlusError) {
  starlark_float f1(0);
  starlark_bool true_obj(true);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_plus(true_obj, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for +: 'float' and 'bool'");
}

TEST(StarlarkFloat, BinaryPlusOverflowError) {
  starlark_bigint big(number::one() << 1200);
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_plus(big, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to float");
}

TEST(StarlarkFloat, MinusEqualsAssign) {
  starlark_float f1(2.0);
  starlark_float f2(3.0);
  starlark_integer i1(4);
  starlark_bigint b1(number::one() << 3);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result1 = f1.minus_equals_assign(f2, ctx, error_callback);
  auto* result2 = f1.minus_equals_assign(i1, ctx, error_callback);
  auto* result3 = f1.minus_equals_assign(b1, ctx, error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("-1.0", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("-2.0", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("-6.0", result3->str());
}

TEST(StarlarkFloat, MinusEqualsAssignError) {
  starlark_float f1(0);
  starlark_bool true_obj(true);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.minus_equals_assign(true_obj, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for -=: 'float' and 'bool'");
}

TEST(StarlarkFloat, BinaryMinus) {
  starlark_float f1(2.0);
  starlark_float f2(3.0);
  starlark_integer i1(4);
  starlark_bigint b1(number::one() << 3);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result1 = f1.binary_minus(f2, ctx, error_callback);
  auto* result2 = f1.binary_minus(i1, ctx, error_callback);
  auto* result3 = f1.binary_minus(b1, ctx, error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("-1.0", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("-2.0", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("-6.0", result3->str());
}

TEST(StarlarkFloat, BinaryMinusError) {
  starlark_float f1(0);
  starlark_bool true_obj(true);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_minus(true_obj, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for -: 'float' and 'bool'");
}

TEST(StarlarkFloat, BinaryMinusOverflowError) {
  starlark_bigint big(number::one() << 1200);
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_minus(big, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to float");
}

TEST(StarlarkFloat, StarEqualsAssign) {
  starlark_float f1(2.0);
  starlark_float f2(3.0);
  starlark_integer i1(4);
  starlark_bigint b1(number::one() << 3);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result1 = f1.star_equals_assign(f2, ctx, error_callback);
  auto* result2 = f1.star_equals_assign(i1, ctx, error_callback);
  auto* result3 = f1.star_equals_assign(b1, ctx, error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("6.0", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("8.0", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("16.0", result3->str());
}

TEST(StarlarkFloat, StarEqualsAssignError) {
  starlark_float f1(0);
  starlark_bool true_obj(true);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.star_equals_assign(true_obj, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for *=: 'float' and 'bool'");
}

TEST(StarlarkFloat, BinaryStar) {
  starlark_float f1(2.0);
  starlark_float f2(3.0);
  starlark_integer i1(4);
  starlark_bigint b1(number::one() << 3);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result1 = f1.binary_star(f2, ctx, error_callback);
  auto* result2 = f1.binary_star(i1, ctx, error_callback);
  auto* result3 = f1.binary_star(b1, ctx, error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("6.0", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("8.0", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("16.0", result3->str());
}

TEST(StarlarkFloat, BinaryStarError) {
  starlark_float f1(0);
  starlark_bool true_obj(true);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_star(true_obj, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for *: 'float' and 'bool'");
}

TEST(StarlarkFloat, BinaryStarOverflowError) {
  starlark_bigint big(number::one() << 1200);
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_star(big, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to float");
}

TEST(StarlarkFloat, SlashEqualsAssign) {
  starlark_float f1(10.0);
  starlark_float f2(-10.0);
  starlark_float f3(2.0);
  starlark_integer i1(4);
  starlark_bigint b1(number::one() << 2);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result1 = f1.slash_equals_assign(f3, ctx, error_callback);
  auto* result2 = f1.slash_equals_assign(i1, ctx, error_callback);
  auto* result3 = f1.slash_equals_assign(b1, ctx, error_callback);
  auto* result4 = f2.slash_equals_assign(f3, ctx, error_callback);
  auto* result5 = f2.slash_equals_assign(i1, ctx, error_callback);
  auto* result6 = f2.slash_equals_assign(b1, ctx, error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("5.0", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("2.5", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("2.5", result3->str());
  ASSERT_NE(result4, nullptr);
  EXPECT_EQ("-5.0", result4->str());
  ASSERT_NE(result5, nullptr);
  EXPECT_EQ("-2.5", result5->str());
  ASSERT_NE(result6, nullptr);
  EXPECT_EQ("-2.5", result6->str());
}

TEST(StarlarkFloat, SlashEqualsAssignError) {
  starlark_float f1(0);
  starlark_bool true_obj(true);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.slash_equals_assign(true_obj, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for /=: 'float' and 'bool'");
}

TEST(StarlarkFloat, BinarySlash) {
  starlark_float f1(10.0);
  starlark_float f2(-10.0);
  starlark_float f3(2.0);
  starlark_integer i1(4);
  starlark_bigint b1(number::one() << 2);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result1 = f1.binary_slash(f3, ctx, error_callback);
  auto* result2 = f1.binary_slash(i1, ctx, error_callback);
  auto* result3 = f1.binary_slash(b1, ctx, error_callback);
  auto* result4 = f2.binary_slash(f3, ctx, error_callback);
  auto* result5 = f2.binary_slash(i1, ctx, error_callback);
  auto* result6 = f2.binary_slash(b1, ctx, error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("5.0", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("2.5", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("2.5", result3->str());
  ASSERT_NE(result4, nullptr);
  EXPECT_EQ("-5.0", result4->str());
  ASSERT_NE(result5, nullptr);
  EXPECT_EQ("-2.5", result5->str());
  ASSERT_NE(result6, nullptr);
  EXPECT_EQ("-2.5", result6->str());
}

TEST(StarlarkFloat, BinarySlashError) {
  starlark_float f1(0);
  starlark_bool true_obj(true);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_slash(true_obj, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for /: 'float' and 'bool'");
}

TEST(StarlarkFloat, BinarySlashOverflowError) {
  starlark_bigint big(number::one() << 1200);
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_slash(big, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to float");
}

TEST(StarlarkFloat, BinarySlashZeroFloatError) {
  starlark_float f0(0.0);
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_slash(f0, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkFloat, BinarySlashZeroIntError) {
  starlark_integer i0(0);
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_slash(i0, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkFloat, BinarySlashZeroBigintError) {
  starlark_bigint b0(number::zero());
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_slash(b0, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkFloat, SlashSlashEqualsAssign) {
  starlark_float f1(10.0);
  starlark_float f2(-10.0);
  starlark_float f3(2.0);
  starlark_integer i1(4);
  starlark_bigint b1(number::one() << 2);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result1 = f1.slash_slash_equals_assign(f3, ctx, error_callback);
  auto* result2 = f1.slash_slash_equals_assign(i1, ctx, error_callback);
  auto* result3 = f1.slash_slash_equals_assign(b1, ctx, error_callback);
  auto* result4 = f2.slash_slash_equals_assign(f3, ctx, error_callback);
  auto* result5 = f2.slash_slash_equals_assign(i1, ctx, error_callback);
  auto* result6 = f2.slash_slash_equals_assign(b1, ctx, error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("5.0", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("2.0", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("2.0", result3->str());
  ASSERT_NE(result4, nullptr);
  EXPECT_EQ("-5.0", result4->str());
  ASSERT_NE(result5, nullptr);
  EXPECT_EQ("-3.0", result5->str());
  ASSERT_NE(result6, nullptr);
  EXPECT_EQ("-3.0", result6->str());
}

TEST(StarlarkFloat, SlashSlashEqualsAssignError) {
  starlark_float f1(0);
  starlark_bool true_obj(true);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.slash_slash_equals_assign(true_obj, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for //=: 'float' and 'bool'");
}

TEST(StarlarkFloat, BinarySlashSlash) {
  starlark_float f1(10.0);
  starlark_float f2(-10.0);
  starlark_float f3(2.0);
  starlark_integer i1(4);
  starlark_bigint b1(number::one() << 2);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result1 = f1.binary_slash_slash(f3, ctx, error_callback);
  auto* result2 = f1.binary_slash_slash(i1, ctx, error_callback);
  auto* result3 = f1.binary_slash_slash(b1, ctx, error_callback);
  auto* result4 = f2.binary_slash_slash(f3, ctx, error_callback);
  auto* result5 = f2.binary_slash_slash(i1, ctx, error_callback);
  auto* result6 = f2.binary_slash_slash(b1, ctx, error_callback);

  ASSERT_NE(result1, nullptr);
  EXPECT_EQ("5.0", result1->str());
  ASSERT_NE(result2, nullptr);
  EXPECT_EQ("2.0", result2->str());
  ASSERT_NE(result3, nullptr);
  EXPECT_EQ("2.0", result3->str());
  ASSERT_NE(result4, nullptr);
  EXPECT_EQ("-5.0", result4->str());
  ASSERT_NE(result5, nullptr);
  EXPECT_EQ("-3.0", result5->str());
  ASSERT_NE(result6, nullptr);
  EXPECT_EQ("-3.0", result6->str());
}

TEST(StarlarkFloat, BinarySlashSlashError) {
  starlark_float f1(0);
  starlark_bool true_obj(true);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_slash_slash(true_obj, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for //: 'float' and 'bool'");
}

TEST(StarlarkFloat, BinarySlashSlashOverflowError) {
  starlark_bigint big(number::one() << 1200);
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_slash_slash(big, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to float");
}

TEST(StarlarkFloat, BinarySlashSlashZeroFloatError) {
  starlark_float f0(0.0);
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_slash_slash(f0, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkFloat, BinarySlashSlashZeroIntError) {
  starlark_integer i0(0);
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_slash_slash(i0, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkFloat, BinarySlashSlashZeroBigintError) {
  starlark_bigint b0(number::zero());
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_slash_slash(b0, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkFloat, BinaryPercent) {
  auto test = [](double n, int64_t d, std::string_view r) {
    starlark_float num(n);
    starlark_float denf(d);
    starlark_integer deni(d);
    starlark_bigint denb(d);
    Arena arena;
    context ctx(arena);
    error_handler error_callback;

    auto* resultf1 = num.binary_percent(denf, ctx, error_callback);
    auto* resulti1 = num.binary_percent(deni, ctx, error_callback);
    auto* resultb1 = num.binary_percent(denb, ctx, error_callback);
    auto* resultf2 = num.percent_equals_assign(denf, ctx, error_callback);
    auto* resulti2 = num.percent_equals_assign(deni, ctx, error_callback);
    auto* resultb2 = num.percent_equals_assign(denb, ctx, error_callback);

    EXPECT_THAT(error_callback.messages, SizeIs(0));
    EXPECT_NE(resultf1, nullptr);
    EXPECT_NE(resulti1, nullptr);
    EXPECT_NE(resultb1, nullptr);
    EXPECT_EQ(r, resultf1->str());
    EXPECT_EQ(r, resulti1->str());
    EXPECT_EQ(r, resultb1->str());
    EXPECT_NE(resultf2, nullptr);
    EXPECT_NE(resulti2, nullptr);
    EXPECT_NE(resultb2, nullptr);
    EXPECT_EQ(r, resultf2->str());
    EXPECT_EQ(r, resulti2->str());
    EXPECT_EQ(r, resultb2->str());
  };
  /*
  ```python
  for a in range(-10, 11):
      for b in range(-10, 11):
          if b!=0:
              print('  test({}, {}, "{}");'.format(float(a), float(b), (float(a) % float(b))))
  ```
  */

  test(-10.0, -10.0, "-0.0");
  test(-10.0, -9.0, "-1.0");
  test(-10.0, -8.0, "-2.0");
  test(-10.0, -7.0, "-3.0");
  test(-10.0, -6.0, "-4.0");
  test(-10.0, -5.0, "-0.0");
  test(-10.0, -4.0, "-2.0");
  test(-10.0, -3.0, "-1.0");
  test(-10.0, -2.0, "-0.0");
  test(-10.0, -1.0, "-0.0");
  test(-10.0, 1.0, "0.0");
  test(-10.0, 2.0, "0.0");
  test(-10.0, 3.0, "2.0");
  test(-10.0, 4.0, "2.0");
  test(-10.0, 5.0, "0.0");
  test(-10.0, 6.0, "2.0");
  test(-10.0, 7.0, "4.0");
  test(-10.0, 8.0, "6.0");
  test(-10.0, 9.0, "8.0");
  test(-10.0, 10.0, "0.0");
  test(-9.0, -10.0, "-9.0");
  test(-9.0, -9.0, "-0.0");
  test(-9.0, -8.0, "-1.0");
  test(-9.0, -7.0, "-2.0");
  test(-9.0, -6.0, "-3.0");
  test(-9.0, -5.0, "-4.0");
  test(-9.0, -4.0, "-1.0");
  test(-9.0, -3.0, "-0.0");
  test(-9.0, -2.0, "-1.0");
  test(-9.0, -1.0, "-0.0");
  test(-9.0, 1.0, "0.0");
  test(-9.0, 2.0, "1.0");
  test(-9.0, 3.0, "0.0");
  test(-9.0, 4.0, "3.0");
  test(-9.0, 5.0, "1.0");
  test(-9.0, 6.0, "3.0");
  test(-9.0, 7.0, "5.0");
  test(-9.0, 8.0, "7.0");
  test(-9.0, 9.0, "0.0");
  test(-9.0, 10.0, "1.0");
  test(-8.0, -10.0, "-8.0");
  test(-8.0, -9.0, "-8.0");
  test(-8.0, -8.0, "-0.0");
  test(-8.0, -7.0, "-1.0");
  test(-8.0, -6.0, "-2.0");
  test(-8.0, -5.0, "-3.0");
  test(-8.0, -4.0, "-0.0");
  test(-8.0, -3.0, "-2.0");
  test(-8.0, -2.0, "-0.0");
  test(-8.0, -1.0, "-0.0");
  test(-8.0, 1.0, "0.0");
  test(-8.0, 2.0, "0.0");
  test(-8.0, 3.0, "1.0");
  test(-8.0, 4.0, "0.0");
  test(-8.0, 5.0, "2.0");
  test(-8.0, 6.0, "4.0");
  test(-8.0, 7.0, "6.0");
  test(-8.0, 8.0, "0.0");
  test(-8.0, 9.0, "1.0");
  test(-8.0, 10.0, "2.0");
  test(-7.0, -10.0, "-7.0");
  test(-7.0, -9.0, "-7.0");
  test(-7.0, -8.0, "-7.0");
  test(-7.0, -7.0, "-0.0");
  test(-7.0, -6.0, "-1.0");
  test(-7.0, -5.0, "-2.0");
  test(-7.0, -4.0, "-3.0");
  test(-7.0, -3.0, "-1.0");
  test(-7.0, -2.0, "-1.0");
  test(-7.0, -1.0, "-0.0");
  test(-7.0, 1.0, "0.0");
  test(-7.0, 2.0, "1.0");
  test(-7.0, 3.0, "2.0");
  test(-7.0, 4.0, "1.0");
  test(-7.0, 5.0, "3.0");
  test(-7.0, 6.0, "5.0");
  test(-7.0, 7.0, "0.0");
  test(-7.0, 8.0, "1.0");
  test(-7.0, 9.0, "2.0");
  test(-7.0, 10.0, "3.0");
  test(-6.0, -10.0, "-6.0");
  test(-6.0, -9.0, "-6.0");
  test(-6.0, -8.0, "-6.0");
  test(-6.0, -7.0, "-6.0");
  test(-6.0, -6.0, "-0.0");
  test(-6.0, -5.0, "-1.0");
  test(-6.0, -4.0, "-2.0");
  test(-6.0, -3.0, "-0.0");
  test(-6.0, -2.0, "-0.0");
  test(-6.0, -1.0, "-0.0");
  test(-6.0, 1.0, "0.0");
  test(-6.0, 2.0, "0.0");
  test(-6.0, 3.0, "0.0");
  test(-6.0, 4.0, "2.0");
  test(-6.0, 5.0, "4.0");
  test(-6.0, 6.0, "0.0");
  test(-6.0, 7.0, "1.0");
  test(-6.0, 8.0, "2.0");
  test(-6.0, 9.0, "3.0");
  test(-6.0, 10.0, "4.0");
  test(-5.0, -10.0, "-5.0");
  test(-5.0, -9.0, "-5.0");
  test(-5.0, -8.0, "-5.0");
  test(-5.0, -7.0, "-5.0");
  test(-5.0, -6.0, "-5.0");
  test(-5.0, -5.0, "-0.0");
  test(-5.0, -4.0, "-1.0");
  test(-5.0, -3.0, "-2.0");
  test(-5.0, -2.0, "-1.0");
  test(-5.0, -1.0, "-0.0");
  test(-5.0, 1.0, "0.0");
  test(-5.0, 2.0, "1.0");
  test(-5.0, 3.0, "1.0");
  test(-5.0, 4.0, "3.0");
  test(-5.0, 5.0, "0.0");
  test(-5.0, 6.0, "1.0");
  test(-5.0, 7.0, "2.0");
  test(-5.0, 8.0, "3.0");
  test(-5.0, 9.0, "4.0");
  test(-5.0, 10.0, "5.0");
  test(-4.0, -10.0, "-4.0");
  test(-4.0, -9.0, "-4.0");
  test(-4.0, -8.0, "-4.0");
  test(-4.0, -7.0, "-4.0");
  test(-4.0, -6.0, "-4.0");
  test(-4.0, -5.0, "-4.0");
  test(-4.0, -4.0, "-0.0");
  test(-4.0, -3.0, "-1.0");
  test(-4.0, -2.0, "-0.0");
  test(-4.0, -1.0, "-0.0");
  test(-4.0, 1.0, "0.0");
  test(-4.0, 2.0, "0.0");
  test(-4.0, 3.0, "2.0");
  test(-4.0, 4.0, "0.0");
  test(-4.0, 5.0, "1.0");
  test(-4.0, 6.0, "2.0");
  test(-4.0, 7.0, "3.0");
  test(-4.0, 8.0, "4.0");
  test(-4.0, 9.0, "5.0");
  test(-4.0, 10.0, "6.0");
  test(-3.0, -10.0, "-3.0");
  test(-3.0, -9.0, "-3.0");
  test(-3.0, -8.0, "-3.0");
  test(-3.0, -7.0, "-3.0");
  test(-3.0, -6.0, "-3.0");
  test(-3.0, -5.0, "-3.0");
  test(-3.0, -4.0, "-3.0");
  test(-3.0, -3.0, "-0.0");
  test(-3.0, -2.0, "-1.0");
  test(-3.0, -1.0, "-0.0");
  test(-3.0, 1.0, "0.0");
  test(-3.0, 2.0, "1.0");
  test(-3.0, 3.0, "0.0");
  test(-3.0, 4.0, "1.0");
  test(-3.0, 5.0, "2.0");
  test(-3.0, 6.0, "3.0");
  test(-3.0, 7.0, "4.0");
  test(-3.0, 8.0, "5.0");
  test(-3.0, 9.0, "6.0");
  test(-3.0, 10.0, "7.0");
  test(-2.0, -10.0, "-2.0");
  test(-2.0, -9.0, "-2.0");
  test(-2.0, -8.0, "-2.0");
  test(-2.0, -7.0, "-2.0");
  test(-2.0, -6.0, "-2.0");
  test(-2.0, -5.0, "-2.0");
  test(-2.0, -4.0, "-2.0");
  test(-2.0, -3.0, "-2.0");
  test(-2.0, -2.0, "-0.0");
  test(-2.0, -1.0, "-0.0");
  test(-2.0, 1.0, "0.0");
  test(-2.0, 2.0, "0.0");
  test(-2.0, 3.0, "1.0");
  test(-2.0, 4.0, "2.0");
  test(-2.0, 5.0, "3.0");
  test(-2.0, 6.0, "4.0");
  test(-2.0, 7.0, "5.0");
  test(-2.0, 8.0, "6.0");
  test(-2.0, 9.0, "7.0");
  test(-2.0, 10.0, "8.0");
  test(-1.0, -10.0, "-1.0");
  test(-1.0, -9.0, "-1.0");
  test(-1.0, -8.0, "-1.0");
  test(-1.0, -7.0, "-1.0");
  test(-1.0, -6.0, "-1.0");
  test(-1.0, -5.0, "-1.0");
  test(-1.0, -4.0, "-1.0");
  test(-1.0, -3.0, "-1.0");
  test(-1.0, -2.0, "-1.0");
  test(-1.0, -1.0, "-0.0");
  test(-1.0, 1.0, "0.0");
  test(-1.0, 2.0, "1.0");
  test(-1.0, 3.0, "2.0");
  test(-1.0, 4.0, "3.0");
  test(-1.0, 5.0, "4.0");
  test(-1.0, 6.0, "5.0");
  test(-1.0, 7.0, "6.0");
  test(-1.0, 8.0, "7.0");
  test(-1.0, 9.0, "8.0");
  test(-1.0, 10.0, "9.0");
  test(0.0, -10.0, "-0.0");
  test(0.0, -9.0, "-0.0");
  test(0.0, -8.0, "-0.0");
  test(0.0, -7.0, "-0.0");
  test(0.0, -6.0, "-0.0");
  test(0.0, -5.0, "-0.0");
  test(0.0, -4.0, "-0.0");
  test(0.0, -3.0, "-0.0");
  test(0.0, -2.0, "-0.0");
  test(0.0, -1.0, "-0.0");
  test(0.0, 1.0, "0.0");
  test(0.0, 2.0, "0.0");
  test(0.0, 3.0, "0.0");
  test(0.0, 4.0, "0.0");
  test(0.0, 5.0, "0.0");
  test(0.0, 6.0, "0.0");
  test(0.0, 7.0, "0.0");
  test(0.0, 8.0, "0.0");
  test(0.0, 9.0, "0.0");
  test(0.0, 10.0, "0.0");
  test(1.0, -10.0, "-9.0");
  test(1.0, -9.0, "-8.0");
  test(1.0, -8.0, "-7.0");
  test(1.0, -7.0, "-6.0");
  test(1.0, -6.0, "-5.0");
  test(1.0, -5.0, "-4.0");
  test(1.0, -4.0, "-3.0");
  test(1.0, -3.0, "-2.0");
  test(1.0, -2.0, "-1.0");
  test(1.0, -1.0, "-0.0");
  test(1.0, 1.0, "0.0");
  test(1.0, 2.0, "1.0");
  test(1.0, 3.0, "1.0");
  test(1.0, 4.0, "1.0");
  test(1.0, 5.0, "1.0");
  test(1.0, 6.0, "1.0");
  test(1.0, 7.0, "1.0");
  test(1.0, 8.0, "1.0");
  test(1.0, 9.0, "1.0");
  test(1.0, 10.0, "1.0");
  test(2.0, -10.0, "-8.0");
  test(2.0, -9.0, "-7.0");
  test(2.0, -8.0, "-6.0");
  test(2.0, -7.0, "-5.0");
  test(2.0, -6.0, "-4.0");
  test(2.0, -5.0, "-3.0");
  test(2.0, -4.0, "-2.0");
  test(2.0, -3.0, "-1.0");
  test(2.0, -2.0, "-0.0");
  test(2.0, -1.0, "-0.0");
  test(2.0, 1.0, "0.0");
  test(2.0, 2.0, "0.0");
  test(2.0, 3.0, "2.0");
  test(2.0, 4.0, "2.0");
  test(2.0, 5.0, "2.0");
  test(2.0, 6.0, "2.0");
  test(2.0, 7.0, "2.0");
  test(2.0, 8.0, "2.0");
  test(2.0, 9.0, "2.0");
  test(2.0, 10.0, "2.0");
  test(3.0, -10.0, "-7.0");
  test(3.0, -9.0, "-6.0");
  test(3.0, -8.0, "-5.0");
  test(3.0, -7.0, "-4.0");
  test(3.0, -6.0, "-3.0");
  test(3.0, -5.0, "-2.0");
  test(3.0, -4.0, "-1.0");
  test(3.0, -3.0, "-0.0");
  test(3.0, -2.0, "-1.0");
  test(3.0, -1.0, "-0.0");
  test(3.0, 1.0, "0.0");
  test(3.0, 2.0, "1.0");
  test(3.0, 3.0, "0.0");
  test(3.0, 4.0, "3.0");
  test(3.0, 5.0, "3.0");
  test(3.0, 6.0, "3.0");
  test(3.0, 7.0, "3.0");
  test(3.0, 8.0, "3.0");
  test(3.0, 9.0, "3.0");
  test(3.0, 10.0, "3.0");
  test(4.0, -10.0, "-6.0");
  test(4.0, -9.0, "-5.0");
  test(4.0, -8.0, "-4.0");
  test(4.0, -7.0, "-3.0");
  test(4.0, -6.0, "-2.0");
  test(4.0, -5.0, "-1.0");
  test(4.0, -4.0, "-0.0");
  test(4.0, -3.0, "-2.0");
  test(4.0, -2.0, "-0.0");
  test(4.0, -1.0, "-0.0");
  test(4.0, 1.0, "0.0");
  test(4.0, 2.0, "0.0");
  test(4.0, 3.0, "1.0");
  test(4.0, 4.0, "0.0");
  test(4.0, 5.0, "4.0");
  test(4.0, 6.0, "4.0");
  test(4.0, 7.0, "4.0");
  test(4.0, 8.0, "4.0");
  test(4.0, 9.0, "4.0");
  test(4.0, 10.0, "4.0");
  test(5.0, -10.0, "-5.0");
  test(5.0, -9.0, "-4.0");
  test(5.0, -8.0, "-3.0");
  test(5.0, -7.0, "-2.0");
  test(5.0, -6.0, "-1.0");
  test(5.0, -5.0, "-0.0");
  test(5.0, -4.0, "-3.0");
  test(5.0, -3.0, "-1.0");
  test(5.0, -2.0, "-1.0");
  test(5.0, -1.0, "-0.0");
  test(5.0, 1.0, "0.0");
  test(5.0, 2.0, "1.0");
  test(5.0, 3.0, "2.0");
  test(5.0, 4.0, "1.0");
  test(5.0, 5.0, "0.0");
  test(5.0, 6.0, "5.0");
  test(5.0, 7.0, "5.0");
  test(5.0, 8.0, "5.0");
  test(5.0, 9.0, "5.0");
  test(5.0, 10.0, "5.0");
  test(6.0, -10.0, "-4.0");
  test(6.0, -9.0, "-3.0");
  test(6.0, -8.0, "-2.0");
  test(6.0, -7.0, "-1.0");
  test(6.0, -6.0, "-0.0");
  test(6.0, -5.0, "-4.0");
  test(6.0, -4.0, "-2.0");
  test(6.0, -3.0, "-0.0");
  test(6.0, -2.0, "-0.0");
  test(6.0, -1.0, "-0.0");
  test(6.0, 1.0, "0.0");
  test(6.0, 2.0, "0.0");
  test(6.0, 3.0, "0.0");
  test(6.0, 4.0, "2.0");
  test(6.0, 5.0, "1.0");
  test(6.0, 6.0, "0.0");
  test(6.0, 7.0, "6.0");
  test(6.0, 8.0, "6.0");
  test(6.0, 9.0, "6.0");
  test(6.0, 10.0, "6.0");
  test(7.0, -10.0, "-3.0");
  test(7.0, -9.0, "-2.0");
  test(7.0, -8.0, "-1.0");
  test(7.0, -7.0, "-0.0");
  test(7.0, -6.0, "-5.0");
  test(7.0, -5.0, "-3.0");
  test(7.0, -4.0, "-1.0");
  test(7.0, -3.0, "-2.0");
  test(7.0, -2.0, "-1.0");
  test(7.0, -1.0, "-0.0");
  test(7.0, 1.0, "0.0");
  test(7.0, 2.0, "1.0");
  test(7.0, 3.0, "1.0");
  test(7.0, 4.0, "3.0");
  test(7.0, 5.0, "2.0");
  test(7.0, 6.0, "1.0");
  test(7.0, 7.0, "0.0");
  test(7.0, 8.0, "7.0");
  test(7.0, 9.0, "7.0");
  test(7.0, 10.0, "7.0");
  test(8.0, -10.0, "-2.0");
  test(8.0, -9.0, "-1.0");
  test(8.0, -8.0, "-0.0");
  test(8.0, -7.0, "-6.0");
  test(8.0, -6.0, "-4.0");
  test(8.0, -5.0, "-2.0");
  test(8.0, -4.0, "-0.0");
  test(8.0, -3.0, "-1.0");
  test(8.0, -2.0, "-0.0");
  test(8.0, -1.0, "-0.0");
  test(8.0, 1.0, "0.0");
  test(8.0, 2.0, "0.0");
  test(8.0, 3.0, "2.0");
  test(8.0, 4.0, "0.0");
  test(8.0, 5.0, "3.0");
  test(8.0, 6.0, "2.0");
  test(8.0, 7.0, "1.0");
  test(8.0, 8.0, "0.0");
  test(8.0, 9.0, "8.0");
  test(8.0, 10.0, "8.0");
  test(9.0, -10.0, "-1.0");
  test(9.0, -9.0, "-0.0");
  test(9.0, -8.0, "-7.0");
  test(9.0, -7.0, "-5.0");
  test(9.0, -6.0, "-3.0");
  test(9.0, -5.0, "-1.0");
  test(9.0, -4.0, "-3.0");
  test(9.0, -3.0, "-0.0");
  test(9.0, -2.0, "-1.0");
  test(9.0, -1.0, "-0.0");
  test(9.0, 1.0, "0.0");
  test(9.0, 2.0, "1.0");
  test(9.0, 3.0, "0.0");
  test(9.0, 4.0, "1.0");
  test(9.0, 5.0, "4.0");
  test(9.0, 6.0, "3.0");
  test(9.0, 7.0, "2.0");
  test(9.0, 8.0, "1.0");
  test(9.0, 9.0, "0.0");
  test(9.0, 10.0, "9.0");
  test(10.0, -10.0, "-0.0");
  test(10.0, -9.0, "-8.0");
  test(10.0, -8.0, "-6.0");
  test(10.0, -7.0, "-4.0");
  test(10.0, -6.0, "-2.0");
  test(10.0, -5.0, "-0.0");
  test(10.0, -4.0, "-2.0");
  test(10.0, -3.0, "-2.0");
  test(10.0, -2.0, "-0.0");
  test(10.0, -1.0, "-0.0");
  test(10.0, 1.0, "0.0");
  test(10.0, 2.0, "0.0");
  test(10.0, 3.0, "1.0");
  test(10.0, 4.0, "2.0");
  test(10.0, 5.0, "0.0");
  test(10.0, 6.0, "4.0");
  test(10.0, 7.0, "3.0");
  test(10.0, 8.0, "2.0");
  test(10.0, 9.0, "1.0");
  test(10.0, 10.0, "0.0");
}

TEST(StarlarkFloat, BinaryPercentInfinity) {
  starlark_float zero(0);
  starlark_float one(1);
  starlark_float inf(std::numeric_limits<double>::infinity());
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_EQ("0.0", zero.binary_percent(inf, ctx, error_callback)->str());
  EXPECT_EQ("1.0", one.binary_percent(inf, ctx, error_callback)->str());
  EXPECT_EQ("nan", inf.binary_percent(one, ctx, error_callback)->str());
  EXPECT_EQ("nan", inf.binary_percent(inf, ctx, error_callback)->str());
}

TEST(StarlarkFloat, BinaryPercentError) {
  starlark_float f1(0);
  starlark_bool true_obj(true);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_percent(true_obj, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for %: 'float' and 'bool'");
}

TEST(StarlarkFloat, BinaryPercentOverflowError) {
  starlark_bigint big(number::one() << 1200);
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_percent(big, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to float");
}

TEST(StarlarkFloat, BinaryPercentZeroFloatError) {
  starlark_float f0(0.0);
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_percent(f0, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkFloat, BinaryPercentZeroIntError) {
  starlark_integer i0(0);
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_percent(i0, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkFloat, BinaryPercentZeroBigintError) {
  starlark_bigint b0(number::zero());
  starlark_float f1(1.0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.binary_percent(b0, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ZeroDivisionError: division by zero");
}

TEST(StarlarkFloat, PercentEqualsAssignError) {
  starlark_float f1(0);
  starlark_bool true_obj(true);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  f1.percent_equals_assign(true_obj, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for %=: 'float' and 'bool'");
}

}  // namespace
