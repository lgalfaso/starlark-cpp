// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <limits>

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

TEST(StarlarkFloat, Type) {
  EXPECT_EQ("float", starlark_float(1).type());
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

  EXPECT_FALSE(starlark_float(1.1).equals(starlark_bigint(1)));
  EXPECT_TRUE(starlark_float(-0.0).equals(starlark_float(0.0)));
  // Starlark mandates that `NaN == NaN`.
  EXPECT_TRUE(starlark_float(NAN).equals(starlark_float(NAN)));
}

TEST(StarlarkFloat, EqualsVsBigInt) {
  EXPECT_TRUE(starlark_float(1e50).equals(starlark_bigint(starlark::bigint::parse_number("100000000000000007629769841091887003294964970946560", nullptr))));
}

TEST(StarlarkFloat, Hash) {
  EXPECT_EQ(starlark_float(1e50).hash(), 1387127493139725924);
  EXPECT_EQ(starlark_bigint(starlark::bigint::parse_number("100000000000000007629769841091887003294964970946560", nullptr)).hash(), 1387127493139725924);
}

TEST(StarlarkFloat, Order) {
  EXPECT_THAT(starlark_float(-std::numeric_limits<double>::infinity()).cmp(starlark_float(-1e50), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_float(-1e50).cmp(starlark_float(-1), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_float(-1).cmp(starlark_float(-1e-50), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_float(-1e-50).cmp(starlark_float(0.0), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_float(0.0).cmp(starlark_float(1e-50), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_float(1e-50).cmp(starlark_float(1.0), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_float(1.0).cmp(starlark_float(1e50), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_float(1e50).cmp(starlark_float(std::numeric_limits<double>::infinity()), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_float(std::numeric_limits<double>::infinity()).cmp(starlark_float(NAN), "cmp", nullptr), Lt(0));
}

TEST(StarlarkFloat, OrderWithBigInt) {
  EXPECT_THAT(starlark_float(-std::numeric_limits<double>::infinity()).cmp(starlark_bigint(number::zero), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_float(std::numeric_limits<double>::infinity()).cmp(starlark_bigint(number::zero), "cmp", nullptr), Gt(0));
  EXPECT_THAT(starlark_float(-1).cmp(starlark_bigint(number::zero), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_float(1).cmp(starlark_bigint(number::zero), "cmp", nullptr), Gt(0));
  EXPECT_THAT(starlark_float(0).cmp(starlark_bigint(-number::one), "cmp", nullptr), Gt(0));
  EXPECT_THAT(starlark_float(0).cmp(starlark_bigint(number::one), "cmp", nullptr), Lt(0));

  EXPECT_THAT(starlark_float(-1).cmp(starlark_bigint(-number::one), "cmp", nullptr), Eq(0));
  EXPECT_THAT(starlark_float(-1).cmp(starlark_bigint(number::one), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_float(1).cmp(starlark_bigint(-number::one), "cmp", nullptr), Gt(0));
  EXPECT_THAT(starlark_float(1).cmp(starlark_bigint(number::one), "cmp", nullptr), Eq(0));


  EXPECT_THAT(starlark_float(0).cmp(starlark_bigint(parse_number("0", nullptr)), "cmp", nullptr), Eq(0));

  EXPECT_THAT(starlark_float(-2).cmp(starlark_bigint(parse_number("-2", nullptr)), "cmp", nullptr), Eq(0));
  EXPECT_THAT(starlark_float(-2).cmp(starlark_bigint(parse_number("-1", nullptr)), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_float(-2).cmp(starlark_bigint(parse_number("1", nullptr)), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_float(-2).cmp(starlark_bigint(parse_number("2", nullptr)), "cmp", nullptr), Lt(0));

  EXPECT_THAT(starlark_float(-1).cmp(starlark_bigint(parse_number("-2", nullptr)), "cmp", nullptr), Gt(0));
  EXPECT_THAT(starlark_float(-1).cmp(starlark_bigint(parse_number("-1", nullptr)), "cmp", nullptr), Eq(0));
  EXPECT_THAT(starlark_float(-1).cmp(starlark_bigint(parse_number("1", nullptr)), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_float(-1).cmp(starlark_bigint(parse_number("2", nullptr)), "cmp", nullptr), Lt(0));

  EXPECT_THAT(starlark_float(1).cmp(starlark_bigint(parse_number("-2", nullptr)), "cmp", nullptr), Gt(0));
  EXPECT_THAT(starlark_float(1).cmp(starlark_bigint(parse_number("-1", nullptr)), "cmp", nullptr), Gt(0));
  EXPECT_THAT(starlark_float(1).cmp(starlark_bigint(parse_number("1", nullptr)), "cmp", nullptr), Eq(0));
  EXPECT_THAT(starlark_float(1).cmp(starlark_bigint(parse_number("2", nullptr)), "cmp", nullptr), Lt(0));

  EXPECT_THAT(starlark_float(2).cmp(starlark_bigint(parse_number("-2", nullptr)), "cmp", nullptr), Gt(0));
  EXPECT_THAT(starlark_float(2).cmp(starlark_bigint(parse_number("-1", nullptr)), "cmp", nullptr), Gt(0));
  EXPECT_THAT(starlark_float(2).cmp(starlark_bigint(parse_number("1", nullptr)), "cmp", nullptr), Gt(0));
  EXPECT_THAT(starlark_float(2).cmp(starlark_bigint(parse_number("2", nullptr)), "cmp", nullptr), Eq(0));


  EXPECT_THAT(starlark_float(1e50).cmp(starlark_bigint(parse_number("100100000000000007629769841091887003294964970946560", nullptr)), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_float(1e50).cmp(starlark_bigint(parse_number("100000000000000006629769841091887003294964970946560", nullptr)), "cmp", nullptr), Gt(0));

  EXPECT_THAT(starlark_float(-1.25).cmp(starlark_bigint(-number::one), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_float(1.25).cmp(starlark_bigint(number::one), "cmp", nullptr), Gt(0));

  EXPECT_THAT(starlark_float(1e50).cmp(starlark_bigint(parse_number("100000000000000007629769841091887003294964970946559", nullptr)), "cmp", nullptr), Gt(0));
  EXPECT_EQ(starlark_float(1e50).cmp(starlark_bigint(parse_number("100000000000000007629769841091887003294964970946560", nullptr)), "cmp", nullptr), 0);
  EXPECT_THAT(starlark_float(1e50).cmp(starlark_bigint(parse_number("100000000000000007629769841091887003294964970946561", nullptr)), "cmp", nullptr), Lt(0));
}

}  // namespace
