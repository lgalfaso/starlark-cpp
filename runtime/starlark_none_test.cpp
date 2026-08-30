// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <map>
#include <string>
#include <vector>

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
#include "runtime/starlark_testing.hpp"
#include "runtime/starlark_tuple.hpp"

using ::google::protobuf::Arena;
using ::starlark::runtime::context;
using ::starlark::runtime::error_fn;
using ::starlark::runtime::starlark_bigint;
using ::starlark::runtime::starlark_bool;
using ::starlark::runtime::builtin_entrypoints;
using ::starlark::runtime::starlark_built_in_function;
using ::starlark::runtime::starlark_bytes;
using ::starlark::runtime::starlark_dictionary;
using ::starlark::runtime::starlark_float;
using ::starlark::runtime::starlark_function;
using ::starlark::runtime::starlark_integer;
using ::starlark::runtime::starlark_list;
using ::starlark::runtime::starlark_none;
using ::starlark::runtime::starlark_obj;
using ::starlark::runtime::starlark_range;
using ::starlark::runtime::starlark_set;
using ::starlark::runtime::starlark_string;
using ::starlark::runtime::starlark_tuple;
using ::starlark::testing::starlark_testing_function;
using ::std::literals::string_view_literals::operator""sv;

namespace {

starlark_obj* base_fn(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  return nullptr;
}

TEST(StarlarkNone, Type) {
  EXPECT_EQ("NoneType", starlark_none{}.type());
}

TEST(StarlarkNone, Primitive) {
  EXPECT_TRUE(starlark_none{}.primitive());
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
  EXPECT_FALSE(starlark_none().equals(starlark_bytes(""sv)));
  EXPECT_FALSE(starlark_none().equals(starlark_built_in_function(nullptr, builtin_entrypoints{.call = base_fn}, "fn_name")));
  EXPECT_FALSE(starlark_none().equals(starlark_dictionary()));
  EXPECT_FALSE(starlark_none().equals(starlark_float(0.0)));
  EXPECT_FALSE(starlark_none().equals(starlark_testing_function()));
  EXPECT_FALSE(starlark_none().equals(starlark_integer(0)));
  EXPECT_FALSE(starlark_none().equals(starlark_list(0)));
  EXPECT_FALSE(starlark_none().equals(starlark_range(0, 0, 1)));
  EXPECT_FALSE(starlark_none().equals(starlark_set()));
  EXPECT_FALSE(starlark_none().equals(starlark_string(""sv)));
  EXPECT_FALSE(starlark_none().equals(starlark_tuple(0)));
}

TEST(StarlarkNone, Hash) {
  EXPECT_EQ(0xfca86420, starlark_none().hash());
}

}  // namespace
