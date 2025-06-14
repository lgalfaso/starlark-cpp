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

using starlark::runtime::starlark_bigint;
using starlark::runtime::starlark_bool;
using starlark::runtime::starlark_built_in_function;
using starlark::runtime::starlark_bytes;
using starlark::runtime::starlark_dictionary;
using starlark::runtime::starlark_float;
using starlark::runtime::starlark_function;
using starlark::runtime::starlark_integer;
using starlark::runtime::starlark_list;
using starlark::runtime::starlark_none;
using starlark::runtime::starlark_range;
using starlark::runtime::starlark_set;
using starlark::runtime::starlark_string;
using starlark::runtime::starlark_struct;
using starlark::runtime::starlark_tuple;

namespace {

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

}  // namespace
