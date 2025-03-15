// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include "compiler/starlark_bool.hpp"
#include "compiler/starlark_integer.hpp"
#include "compiler/starlark_none.hpp"
#include "compiler/starlark_set.hpp"
#include "compiler/starlark_tuple.hpp"

using starlark::compiler::starlark_bool;
using starlark::compiler::starlark_integer;
using starlark::compiler::starlark_none;
using starlark::compiler::starlark_set;
using starlark::compiler::starlark_tuple;

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

TEST(StarlarkSet, Equals) {
  starlark_none none;
  EXPECT_TRUE(starlark_set().equals(starlark_set()));
  EXPECT_FALSE(starlark_set().add(&none).equals(starlark_set()));
  EXPECT_FALSE(starlark_set().equals(starlark_set().add(&none)));
  EXPECT_TRUE(starlark_set().add(&none).equals(starlark_set().add(&none)));
}

TEST(StarlarkSet, EqualsInDifferentOrder) {
  starlark_none none;
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_bool bool_true(true);
  starlark_bool bool_false(false);
  starlark_set set_1;
  starlark_set set_2;
  starlark_set set_3;
  set_1.add(&none).add(&zero).add(&one).add(&two).add(&bool_true).add(&bool_false);
  set_2.add(&none).add(&zero).add(&one).add(&two).add(&bool_true).add(&bool_false);
  set_3.add(&bool_false).add(&bool_true).add(&two).add(&one).add(&zero).add(&none);
  EXPECT_TRUE(set_1.equals(set_2));
  EXPECT_TRUE(set_1.equals(set_3));
}

}  // namespace
