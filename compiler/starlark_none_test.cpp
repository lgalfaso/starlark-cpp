// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <string>

#include "compiler/starlark_function.hpp"
#include "compiler/starlark_bigint.hpp"
#include "compiler/starlark_bool.hpp"
#include "compiler/starlark_bytes.hpp"
#include "compiler/starlark_dictionary.hpp"
#include "compiler/starlark_float.hpp"
#include "compiler/starlark_integer.hpp"
#include "compiler/starlark_list.hpp"
#include "compiler/starlark_none.hpp"
#include "compiler/starlark_range.hpp"
#include "compiler/starlark_set.hpp"
#include "compiler/starlark_string.hpp"
#include "compiler/starlark_struct.hpp"
#include "compiler/starlark_tuple.hpp"

using starlark::compiler::starlark_bigint;
using starlark::compiler::starlark_bool;
using starlark::compiler::starlark_built_in_function;
using starlark::compiler::starlark_bytes;
using starlark::compiler::starlark_dictionary;
using starlark::compiler::starlark_float;
using starlark::compiler::starlark_function;
using starlark::compiler::starlark_integer;
using starlark::compiler::starlark_list;
using starlark::compiler::starlark_none;
using starlark::compiler::starlark_range;
using starlark::compiler::starlark_set;
using starlark::compiler::starlark_string;
using starlark::compiler::starlark_struct;
using starlark::compiler::starlark_tuple;

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
  EXPECT_FALSE(starlark_none().equals(starlark_bigint(0)));
  EXPECT_FALSE(starlark_none().equals(starlark_bool(false)));
  EXPECT_FALSE(starlark_none().equals(starlark_bytes("")));
  EXPECT_FALSE(starlark_none().equals(starlark_built_in_function()));
  EXPECT_FALSE(starlark_none().equals(starlark_dictionary()));
  EXPECT_FALSE(starlark_none().equals(starlark_float(0.0)));
  EXPECT_FALSE(starlark_none().equals(starlark_function()));
  EXPECT_FALSE(starlark_none().equals(starlark_integer(0)));
  EXPECT_FALSE(starlark_none().equals(starlark_list()));
  EXPECT_FALSE(starlark_none().equals(starlark_range()));
  EXPECT_FALSE(starlark_none().equals(starlark_set()));
  EXPECT_FALSE(starlark_none().equals(starlark_string("")));
  EXPECT_FALSE(starlark_none().equals(starlark_struct()));
  EXPECT_FALSE(starlark_none().equals(starlark_tuple()));
}

TEST(StarlarkNone, Hash) {
  EXPECT_EQ(0xfca86420, starlark_none().hash());
}

}  // namespace
