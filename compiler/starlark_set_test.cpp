// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include "compiler/starlark_set.hpp"
#include "compiler/starlark_bool.hpp"
#include "compiler/starlark_integer.hpp"
#include "compiler/starlark_none.hpp"

using starlark::compiler::starlark_bool;
using starlark::compiler::starlark_integer;
using starlark::compiler::starlark_none;
using starlark::compiler::starlark_set;

namespace {

TEST(StarlarkSet, Type) {
  EXPECT_EQ("set", starlark_set().type());
}

TEST(StarlarkSet, Str) {
  starlark_none none;
  starlark_bool true_obj(true);
  starlark_integer one(1);
  EXPECT_EQ("set()", starlark_set().str());
  EXPECT_EQ("set([None])", starlark_set().add(&none).str());
  EXPECT_EQ("set([None, True])", starlark_set().add(&none).add(&true_obj).str());
  EXPECT_EQ("set([None, True, 1])", starlark_set().add(&none).add(&true_obj).add(&one).str());
}

TEST(StarlarkSet, Truthy) {
  starlark_none none;
  EXPECT_FALSE(starlark_set().truthy());
  EXPECT_TRUE(starlark_set().add(&none).truthy());
}

}  // namespace
