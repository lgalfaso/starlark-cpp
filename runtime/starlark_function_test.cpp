// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <map>
#include <string>
#include <vector>

#include "runtime/starlark_function.hpp"
#include "runtime/starlark_testing.hpp"

using ::google::protobuf::Arena;
using ::starlark::runtime::starlark_built_in_function;
using ::starlark::runtime::starlark_function;
using ::starlark::runtime::starlark_obj;
using ::starlark::testing::error_handler;

namespace {

class Fn {
 public:
  MOCK_METHOD(starlark_obj*, Call, (const std::vector<starlark_obj*>&, (const std::map<std::string, starlark_obj*>&)));
};

static Fn* fn_mock = nullptr;

starlark_obj* base_fn(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args) {
  if (fn_mock != nullptr) {
    return fn_mock->Call(pos_args, named_args);
  }
  return nullptr;
}

class FnTest : public ::testing::Test {
 protected:
  void SetUp() override {
    fn_mock = new Fn();
  }

  void TearDown() override {
    delete fn_mock;
    fn_mock = nullptr;
  }
};

TEST(StarlarkFunction, Type) {
  EXPECT_EQ("function", starlark_function().type());
  EXPECT_EQ("builtin_function_or_method", starlark_built_in_function(base_fn, "fn_name").type());
}

TEST(StarlarkFunction, Truthy) {
  EXPECT_TRUE(starlark_function().truthy());
  EXPECT_TRUE(starlark_built_in_function(base_fn, "fn_name").truthy());
}

TEST(StarlarkFunction, Str) {
  EXPECT_EQ("<built-in function fn_name>", starlark_built_in_function(base_fn, "fn_name").str());
}

TEST_F(FnTest, Call) {
  starlark_built_in_function fn(base_fn, "fn_name");
  Arena arena;
  error_handler error_callback;

  EXPECT_CALL(*fn_mock, Call(testing::_, testing::_))
      .WillOnce(testing::Return(nullptr));
  fn.call({}, {}, arena, error_callback);
  // TODO(lmirelmann): Check the return value.
}

// TODO(lmirelmann): Check the error case.

}  // namespace
