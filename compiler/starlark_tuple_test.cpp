// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include "compiler/starlark_bool.hpp"
#include "compiler/starlark_integer.hpp"
#include "compiler/starlark_none.hpp"
#include "compiler/starlark_tuple.hpp"

using starlark::compiler::starlark_bool;
using starlark::compiler::starlark_integer;
using starlark::compiler::starlark_none;
using starlark::compiler::starlark_tuple;

namespace {

TEST(StarlarkTuple, Type) {
  EXPECT_EQ("tuple", starlark_tuple().type());
}

TEST(StarlarkTuple, Str) {
  starlark_none none;
  starlark_bool true_obj(true);
  starlark_integer one(1);
  EXPECT_EQ("()", starlark_tuple().str());
  EXPECT_EQ("(None,)", starlark_tuple().add(&none).str());
  EXPECT_EQ("(None, True)", starlark_tuple().add(&none).add(&true_obj).str());
  EXPECT_EQ("(None, True, 1)", starlark_tuple().add(&none).add(&true_obj).add(&one).str());
}

TEST(StarlarkTuple, Truthy) {
  starlark_none none;
  EXPECT_FALSE(starlark_tuple().truthy());
  EXPECT_TRUE(starlark_tuple().add(&none).truthy());
}

TEST(StarlarkTuple, Equals) {
  starlark_none none;
  starlark_integer one(1);
  EXPECT_TRUE(starlark_tuple().equals(starlark_tuple()));
  EXPECT_FALSE(starlark_tuple().add(&none).equals(starlark_tuple()));
  EXPECT_FALSE(starlark_tuple().add(&one).equals(starlark_tuple()));
  EXPECT_FALSE(starlark_tuple().add(&none).add(&one).equals(starlark_tuple()));
  EXPECT_FALSE(starlark_tuple().add(&one).add(&none).equals(starlark_tuple()));

  EXPECT_FALSE(starlark_tuple().equals(starlark_tuple().add(&none)));
  EXPECT_TRUE(starlark_tuple().add(&none).equals(starlark_tuple().add(&none)));
  EXPECT_FALSE(starlark_tuple().add(&one).equals(starlark_tuple().add(&none)));
  EXPECT_FALSE(starlark_tuple().add(&none).add(&one).equals(starlark_tuple().add(&none)));
  EXPECT_FALSE(starlark_tuple().add(&one).add(&none).equals(starlark_tuple().add(&none)));

  EXPECT_FALSE(starlark_tuple().equals(starlark_tuple().add(&one)));
  EXPECT_FALSE(starlark_tuple().add(&none).equals(starlark_tuple().add(&one)));
  EXPECT_TRUE(starlark_tuple().add(&one).equals(starlark_tuple().add(&one)));
  EXPECT_FALSE(starlark_tuple().add(&none).add(&one).equals(starlark_tuple().add(&one)));
  EXPECT_FALSE(starlark_tuple().add(&one).add(&none).equals(starlark_tuple().add(&one)));

  EXPECT_FALSE(starlark_tuple().equals(starlark_tuple().add(&none).add(&one)));
  EXPECT_FALSE(starlark_tuple().add(&none).equals(starlark_tuple().add(&none).add(&one)));
  EXPECT_FALSE(starlark_tuple().add(&one).equals(starlark_tuple().add(&none).add(&one)));
  EXPECT_TRUE(starlark_tuple().add(&none).add(&one).equals(starlark_tuple().add(&none).add(&one)));
  EXPECT_FALSE(starlark_tuple().add(&one).add(&none).equals(starlark_tuple().add(&none).add(&one)));
}

TEST(StarlarkTuple, EqualsRecursion) {
  starlark_tuple tuple_a;
  starlark_tuple tuple_b;
  tuple_a.add(&tuple_b);
  tuple_b.add(&tuple_a);
  EXPECT_TRUE(tuple_a.equals(tuple_b));
}

}  // namespace
