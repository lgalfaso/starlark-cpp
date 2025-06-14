// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include "runtime/starlark_bool.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_list.hpp"
#include "runtime/starlark_none.hpp"

using starlark::runtime::starlark_bool;
using starlark::runtime::starlark_integer;
using starlark::runtime::starlark_list;
using starlark::runtime::starlark_none;

namespace {

TEST(StarlarkList, Type) {
  EXPECT_EQ("list", starlark_list().type());
}

TEST(StarlarkList, Str) {
  starlark_none none;
  starlark_bool true_obj(true);
  starlark_integer one(1);
  EXPECT_EQ("[]", starlark_list().str());
  EXPECT_EQ("[None]", starlark_list().add(&none).str());
  EXPECT_EQ("[None, True]", starlark_list().add(&none).add(&true_obj).str());
  EXPECT_EQ("[None, True, 1]", starlark_list().add(&none).add(&true_obj).add(&one).str());
}

TEST(StarlarkList, StrRecursion) {
  // Bazel prints `[1, [1, ..., 1], 1]`, Python prints `[1, [...], 1]`.
  starlark_list list;
  starlark_integer one(1);
  list.add(&one).add(&list).add(&one);
  EXPECT_EQ("[1, [...], 1]", list.str());
}

TEST(StarlarkList, Truthy) {
  starlark_none none;
  EXPECT_FALSE(starlark_list().truthy());
  EXPECT_TRUE(starlark_list().add(&none).truthy());
}

TEST(StarlarkList, Equals) {
  starlark_none none;
  starlark_integer one(1);
  EXPECT_TRUE(starlark_list().equals(starlark_list()));
  EXPECT_FALSE(starlark_list().add(&none).equals(starlark_list()));
  EXPECT_FALSE(starlark_list().add(&one).equals(starlark_list()));
  EXPECT_FALSE(starlark_list().add(&none).add(&one).equals(starlark_list()));
  EXPECT_FALSE(starlark_list().add(&one).add(&none).equals(starlark_list()));

  EXPECT_FALSE(starlark_list().equals(starlark_list().add(&none)));
  EXPECT_TRUE(starlark_list().add(&none).equals(starlark_list().add(&none)));
  EXPECT_FALSE(starlark_list().add(&one).equals(starlark_list().add(&none)));
  EXPECT_FALSE(starlark_list().add(&none).add(&one).equals(starlark_list().add(&none)));
  EXPECT_FALSE(starlark_list().add(&one).add(&none).equals(starlark_list().add(&none)));

  EXPECT_FALSE(starlark_list().equals(starlark_list().add(&one)));
  EXPECT_FALSE(starlark_list().add(&none).equals(starlark_list().add(&one)));
  EXPECT_TRUE(starlark_list().add(&one).equals(starlark_list().add(&one)));
  EXPECT_FALSE(starlark_list().add(&none).add(&one).equals(starlark_list().add(&one)));
  EXPECT_FALSE(starlark_list().add(&one).add(&none).equals(starlark_list().add(&one)));

  EXPECT_FALSE(starlark_list().equals(starlark_list().add(&none).add(&one)));
  EXPECT_FALSE(starlark_list().add(&none).equals(starlark_list().add(&none).add(&one)));
  EXPECT_FALSE(starlark_list().add(&one).equals(starlark_list().add(&none).add(&one)));
  EXPECT_TRUE(starlark_list().add(&none).add(&one).equals(starlark_list().add(&none).add(&one)));
  EXPECT_FALSE(starlark_list().add(&one).add(&none).equals(starlark_list().add(&none).add(&one)));
}

TEST(StarlarkList, EqualsRecursion) {
  starlark_list list_a;
  starlark_list list_b;
  list_a.add(&list_b);
  list_b.add(&list_a);
  EXPECT_TRUE(list_a.equals(list_b));
}

}  // namespace
