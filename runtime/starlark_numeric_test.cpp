// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <limits>
#include <utility>

#include "runtime/starlark_numeric.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;
using ::starlark::runtime::create_integer;
using ::starlark::runtime::starlark_numeric;
using ::starlark::runtime::starlark_numeric_type;
using ::starlark::runtime::to_double;

namespace {

TEST(ToDouble, FromBigInt) {
  EXPECT_EQ(0.0, to_double(number::zero));
  EXPECT_EQ(1.0, to_double(number::one));
  EXPECT_EQ(-1.0, to_double(number::minus_one));
  EXPECT_EQ(1.8446744073709552e+19, to_double((number::one << 64) - number::one));
  EXPECT_EQ(3.6893488147419103e+19, to_double((number::one << 65) - number::one));
  EXPECT_EQ(-3.6893488147419103e+19, to_double((number::minus_one << 65) + number::one));
  EXPECT_EQ(-8.98846567431158e+307, to_double(number::minus_one << 1023));
  EXPECT_EQ(8.98846567431158e+307, to_double(number::one << 1023));
  EXPECT_EQ(-std::numeric_limits<double>::infinity(), to_double(number::minus_one << 1024));
  EXPECT_EQ(std::numeric_limits<double>::infinity(), to_double(number::one << 1024));
}

TEST(CreateIntegerFromBigInt, Downgrades) {
  auto test = [](starlark_numeric_type numeric_type, number&& value) {
    Arena arena;
    EXPECT_EQ(numeric_type, static_cast<starlark_numeric*>(create_integer(std::move(value), arena))->numeric_type());
  };
  test(starlark_numeric_type::kBigInt, (number::minus_one << 63) - number::one);
  test(starlark_numeric_type::kInt64, number::minus_one << 63);
  test(starlark_numeric_type::kInt64, (number::minus_one << 63) + number::one);
  test(starlark_numeric_type::kInt64, number(number::minus_one));
  test(starlark_numeric_type::kInt64, number(number::zero));
  test(starlark_numeric_type::kInt64, number(number::one));
  test(starlark_numeric_type::kInt64, (number::one << 63) - number::one);
  test(starlark_numeric_type::kBigInt, number::one << 63);
}

}  // namespace

