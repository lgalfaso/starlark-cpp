// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <string>

#include "compiler/starlark_bool.hpp"
#include "compiler/starlark_integer.hpp"
#include "compiler/starlark_none.hpp"

using starlark::compiler::starlark_bool;
using starlark::compiler::starlark_integer;
using starlark::compiler::starlark_none;

namespace {

TEST(StarlarkBool, Type) {
  EXPECT_EQ("bool", starlark_bool(true).type());
}

TEST(StarlarkBool, Str) {
  EXPECT_EQ("False", starlark_bool(false).str());
  EXPECT_EQ("True", starlark_bool(true).str());
}

TEST(StarlarkBool, Truthy) {
  EXPECT_FALSE(starlark_bool(false).truthy());
  EXPECT_TRUE(starlark_bool(true).truthy());
}

TEST(StarlarkBool, Equals) {
  EXPECT_TRUE(starlark_bool(false).equals(starlark_bool(false)));
  EXPECT_FALSE(starlark_bool(false).equals(starlark_bool(true)));
  EXPECT_TRUE(starlark_bool(true).equals(starlark_bool(true)));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_bool(false)));

  EXPECT_FALSE(starlark_bool(false).equals(starlark_none()));
  EXPECT_FALSE(starlark_bool(false).equals(starlark_integer(0)));

  EXPECT_FALSE(starlark_bool(true).equals(starlark_none()));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_integer(0)));
}

TEST(StarlarkBool, Hash) {
  EXPECT_EQ(0, starlark_bool(false).hash());
  EXPECT_EQ(1, starlark_bool(true).hash());
}

}  // namespace
