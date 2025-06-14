// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <string>

#include "runtime/starlark_function.hpp"
#include "runtime/starlark_bigint.hpp"
#include "runtime/starlark_bool.hpp"
#include "runtime/starlark_bytes.hpp"
#include "runtime/starlark_dictionary.hpp"
#include "runtime/starlark_float.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_list.hpp"
#include "runtime/starlark_none.hpp"
#include "runtime/starlark_range.hpp"
#include "runtime/starlark_set.hpp"
#include "runtime/starlark_string.hpp"
#include "runtime/starlark_struct.hpp"
#include "runtime/starlark_tuple.hpp"

using starlark::runtime::starlark_bigint;
using starlark::runtime::starlark_bool;
using starlark::runtime::starlark_built_in_function;
using starlark::runtime::starlark_bytes;
using starlark::runtime::starlark_dictionary;
using starlark::runtime::starlark_float;
using starlark::runtime::starlark_function;
using starlark::runtime::starlark_integer;
using starlark::runtime::starlark_list;
using starlark::runtime::starlark_none;
using starlark::runtime::starlark_range;
using starlark::runtime::starlark_set;
using starlark::runtime::starlark_string;
using starlark::runtime::starlark_struct;
using starlark::runtime::starlark_tuple;

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

  EXPECT_FALSE(starlark_bool(false).equals(starlark_bigint(0)));
  EXPECT_FALSE(starlark_bool(false).equals(starlark_none()));
  EXPECT_FALSE(starlark_bool(false).equals(starlark_bytes("")));
  EXPECT_FALSE(starlark_bool(false).equals(starlark_built_in_function()));
  EXPECT_FALSE(starlark_bool(false).equals(starlark_dictionary()));
  EXPECT_FALSE(starlark_bool(false).equals(starlark_float(0.0)));
  EXPECT_FALSE(starlark_bool(false).equals(starlark_function()));
  EXPECT_FALSE(starlark_bool(false).equals(starlark_integer(0)));
  EXPECT_FALSE(starlark_bool(false).equals(starlark_list()));
  EXPECT_FALSE(starlark_bool(false).equals(starlark_range()));
  EXPECT_FALSE(starlark_bool(false).equals(starlark_set()));
  EXPECT_FALSE(starlark_bool(false).equals(starlark_string("")));
  EXPECT_FALSE(starlark_bool(false).equals(starlark_struct()));
  EXPECT_FALSE(starlark_bool(false).equals(starlark_tuple()));

  EXPECT_FALSE(starlark_bool(true).equals(starlark_bigint(0)));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_none()));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_bytes("")));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_built_in_function()));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_dictionary()));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_float(0.0)));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_function()));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_integer(0)));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_list()));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_range()));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_set()));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_string("")));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_struct()));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_tuple()));
}

TEST(StarlarkBool, Hash) {
  EXPECT_EQ(0, starlark_bool(false).hash());
  EXPECT_EQ(1, starlark_bool(true).hash());
}

}  // namespace
