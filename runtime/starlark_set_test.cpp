// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include "runtime/starlark_bool.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_none.hpp"
#include "runtime/starlark_set.hpp"
#include "runtime/starlark_tuple.hpp"

using starlark::runtime::starlark_bool;
using starlark::runtime::starlark_integer;
using starlark::runtime::starlark_none;
using starlark::runtime::starlark_set;
using starlark::runtime::starlark_tuple;

namespace {

TEST(StarlarkSet, Type) {
  EXPECT_EQ("set", starlark_set().type());
}

TEST(StarlarkSet, Str) {
  starlark_none none;
  starlark_bool true_obj(true);
  starlark_integer one(1);
  EXPECT_EQ("set()", starlark_set().str());
  starlark_set set1;
  set1.add(&none, nullptr);
  EXPECT_EQ("set([None])", set1.str());
  set1.add(&true_obj, nullptr);
  EXPECT_EQ("set([None, True])", set1.str());
  set1.add(&one, nullptr);
  EXPECT_EQ("set([None, True, 1])", set1.str());
}

TEST(StarlarkSet, Truthy) {
  starlark_none none;
  starlark_set set1;
  EXPECT_FALSE(set1.truthy());
  set1.add(&none, nullptr);
  EXPECT_TRUE(set1.truthy());
}

TEST(StarlarkSet, Equals) {
  starlark_none none;
  starlark_set set1;
  starlark_set set2;
  starlark_set set3;
  set2.add(&none, nullptr);
  set3.add(&none, nullptr);
  EXPECT_TRUE(starlark_set().equals(set1));
  EXPECT_FALSE(set2.equals(set1));
  EXPECT_FALSE(set1.equals(set2));
  EXPECT_TRUE(set2.equals(set3));
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
  set_1.add(&none, nullptr);
  set_1.add(&zero, nullptr);
  set_1.add(&one, nullptr);
  set_1.add(&two, nullptr);
  set_1.add(&bool_true, nullptr);
  set_1.add(&bool_false, nullptr);
  set_2.add(&none, nullptr);
  set_2.add(&zero, nullptr);
  set_2.add(&one, nullptr);
  set_2.add(&two, nullptr);
  set_2.add(&bool_true, nullptr);
  set_2.add(&bool_false, nullptr);
  set_3.add(&bool_false, nullptr);
  set_3.add(&bool_true, nullptr);
  set_3.add(&two, nullptr);
  set_3.add(&one, nullptr);
  set_3.add(&zero, nullptr);
  set_3.add(&none, nullptr);
  EXPECT_TRUE(set_1.equals(set_2));
  EXPECT_TRUE(set_1.equals(set_3));
}

// TODO(lmirelmann): Test inner_freeze.
// TODO(lmirelmann): Test hash of freezed and unfreezed sets.
// TODO(lmirelmann): Test trying to insert to a freezed set including the error.

}  // namespace
