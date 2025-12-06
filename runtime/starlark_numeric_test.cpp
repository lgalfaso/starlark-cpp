// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <limits>

#include "runtime/starlark_numeric.hpp"

using ::starlark::bigint::number;
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

}

