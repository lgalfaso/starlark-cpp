// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <limits>
#include <string>

#include "compiler/starlark_bool.hpp"
#include "compiler/starlark_integer.hpp"
#include "compiler/starlark_none.hpp"

using starlark::compiler::starlark_bool;
using starlark::compiler::starlark_integer;
using starlark::compiler::starlark_none;

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
  EXPECT_TRUE(starlark_integer(0).equals(starlark_integer(0)));
  EXPECT_TRUE(starlark_integer(1).equals(starlark_integer(1)));
  EXPECT_FALSE(starlark_integer(1).equals(starlark_integer(0)));
  EXPECT_FALSE(starlark_integer(1).equals(starlark_integer(-1)));
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
}

}  // namespace
