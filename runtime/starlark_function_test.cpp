// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include "runtime/starlark_function.hpp"

using starlark::runtime::starlark_built_in_function;
using starlark::runtime::starlark_function;

namespace {

TEST(StarlarkFunction, Type) {
  EXPECT_EQ("function", starlark_function().type());
  EXPECT_EQ("builtin_function_or_method", starlark_built_in_function().type());
}

TEST(StarlarkFunction, Truthy) {
  EXPECT_TRUE(starlark_function().truthy());
  EXPECT_TRUE(starlark_built_in_function().truthy());
}

}  // namespace
