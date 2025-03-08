// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include "compiler/starlark_range.hpp"

using starlark::compiler::starlark_range;

namespace {

TEST(StarlarkRange, Type) {
  EXPECT_EQ("range", starlark_range().type());
}

TEST(StarlarkRange, Truthy) {
  // TODO(lmirelmann): Implement
}

}  // namespace
