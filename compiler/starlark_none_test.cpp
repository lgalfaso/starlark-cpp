// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <string>

#include "compiler/starlark_none.hpp"

using starlark::compiler::starlark_none;

namespace {

TEST(StarlarkNone, Type) {
  EXPECT_EQ("NoneType", starlark_none{}.type());
}

TEST(StarlarkNone, Str) {
  EXPECT_EQ("None", starlark_none{}.str());
}

TEST(StarlarkNone, Truthy) {
  EXPECT_FALSE(starlark_none().truthy());
}

TEST(StarlarkNone, Equals) {
  EXPECT_TRUE(starlark_none().equals(starlark_none()));
}

TEST(StarlarkNone, Hash) {
  EXPECT_EQ(0xfca86420, starlark_none().hash());
}

}  // namespace
