// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include "runtime/starlark_struct.hpp"

using starlark::runtime::starlark_struct;

namespace {

TEST(StarlarkStruct, Type) {
  EXPECT_EQ("struct", starlark_struct().type());
}

TEST(StarlarkStruct, Truthy) {
  EXPECT_TRUE(starlark_struct().truthy());
}

}  // namespace
