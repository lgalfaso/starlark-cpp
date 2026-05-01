// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <map>
#include <string>
#include <vector>

#include "runtime/starlark_bigint.hpp"
#include "runtime/starlark_bool.hpp"
#include "runtime/starlark_bytes.hpp"
#include "runtime/starlark_dictionary.hpp"
#include "runtime/starlark_float.hpp"
#include "runtime/starlark_function.hpp"
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
using ::starlark::testing::error_handler;
using ::starlark::testing::starlark_testing_function;
using ::std::literals::string_view_literals::operator""sv;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::IsEmpty;
using ::testing::Lt;
using ::testing::SizeIs;

namespace {

starlark_obj* base_fn(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn&) {
  return nullptr;
}

TEST(StarlarkBool, Type) {
  EXPECT_EQ("bool", starlark_bool(true).type());
}

TEST(StarlarkBool, Primitive) {
  EXPECT_TRUE(starlark_bool(true).primitive());
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
  EXPECT_FALSE(starlark_bool(false).equals(starlark_bytes(""sv)));
  EXPECT_FALSE(starlark_bool(false).equals(starlark_built_in_function(nullptr, base_fn, "fn_name")));
  EXPECT_FALSE(starlark_bool(false).equals(starlark_dictionary()));
  EXPECT_FALSE(starlark_bool(false).equals(starlark_float(0.0)));
  EXPECT_FALSE(starlark_bool(false).equals(starlark_testing_function()));
  EXPECT_FALSE(starlark_bool(false).equals(starlark_integer(0)));
  EXPECT_FALSE(starlark_bool(false).equals(starlark_list(0)));
  EXPECT_FALSE(starlark_bool(false).equals(starlark_range(0, 1, 1)));
  EXPECT_FALSE(starlark_bool(false).equals(starlark_set()));
  EXPECT_FALSE(starlark_bool(false).equals(starlark_string(""sv)));
  EXPECT_FALSE(starlark_bool(false).equals(starlark_tuple(0)));

  EXPECT_FALSE(starlark_bool(true).equals(starlark_bigint(0)));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_none()));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_bytes(""sv)));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_built_in_function(nullptr, base_fn, "fn_name")));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_dictionary()));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_float(0.0)));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_testing_function()));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_integer(0)));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_list(0)));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_range(0, 1, 1)));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_set()));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_string(""sv)));
  EXPECT_FALSE(starlark_bool(true).equals(starlark_tuple(0)));
}

void cmp_helper(starlark::result::status_or<int> cmp, auto matcher) {
  ASSERT_TRUE(cmp.ok());
  EXPECT_THAT(*cmp, matcher);
}

TEST(StarlarkBool, Cmp) {
  error_handler error_callback;
  cmp_helper(starlark_bool(false).cmp(starlark_bool(false), "cmp", error_callback), Eq(0));
  cmp_helper(starlark_bool(false).cmp(starlark_bool(true), "cmp", error_callback), Lt(0));
  cmp_helper(starlark_bool(true).cmp(starlark_bool(true), "cmp", error_callback), Eq(0));
  cmp_helper(starlark_bool(true).cmp(starlark_bool(false), "cmp", error_callback), Gt(0));
  EXPECT_THAT(error_callback.messages, IsEmpty());
  auto cmp = starlark_bool(true).cmp(starlark_none(), "cmp", error_callback);
  ASSERT_FALSE(cmp.ok());
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'cmp' not supported between instances of 'bool' and 'NoneType'");
}

TEST(StarlarkBool, Hash) {
  EXPECT_EQ(0, starlark_bool(false).hash());
  EXPECT_EQ(1, starlark_bool(true).hash());
}

}  // namespace
