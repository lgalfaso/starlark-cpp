// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <map>
#include <string>
#include <vector>

#include "runtime/starlark_function.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_list.hpp"
#include "runtime/starlark_testing.hpp"

using ::google::protobuf::Arena;
using ::testing::IsEmpty;
using ::testing::SizeIs;
using ::starlark::bigint::number;
using ::starlark::runtime::create_float;
using ::starlark::runtime::create_integer;
using ::starlark::runtime::error_fn;
using ::starlark::runtime::starlark_built_in_function;
using ::starlark::runtime::starlark_function;
using ::starlark::runtime::starlark_integer;
using ::starlark::runtime::starlark_list;
using ::starlark::runtime::starlark_obj;
using ::starlark::testing::error_handler;

namespace {

class Fn {
 public:
  MOCK_METHOD(starlark_obj*, Call, (const std::vector<starlark_obj*>&, (const std::map<std::string, starlark_obj*>&), Arena&, error_fn&));
};

static Fn* fn_mock = nullptr;

starlark_obj* base_fn(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, Arena& arena, error_fn& error_callback) {
  if (fn_mock != nullptr) {
    return fn_mock->Call(pos_args, named_args, arena, error_callback);
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

  EXPECT_CALL(*fn_mock, Call(testing::_, testing::_, testing::_, testing::_))
      .WillOnce(testing::Return(nullptr));
  fn.call({}, {}, arena, error_callback);
  // TODO(lmirelmann): Check the return value.
}

// TODO(lmirelmann): Check the error case.

TEST(StarlarkAbs, Numeric) {
  auto itest = [](auto&& value, std::string_view result) {
    std::vector<starlark_obj*> pos_args;
    std::map<std::string, starlark_obj*> named_args;
    Arena arena;
    error_handler error_callback;
    pos_args.push_back(create_integer(std::forward<decltype(value)>(value), arena));

    EXPECT_EQ(result, starlark_fn_abs(pos_args, named_args, arena, error_callback)->str());
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  auto ftest = [](double value, std::string_view result) {
    std::vector<starlark_obj*> pos_args;
    std::map<std::string, starlark_obj*> named_args;
    Arena arena;
    error_handler error_callback;
    pos_args.push_back(create_float(value, arena));

    EXPECT_EQ(result, starlark_fn_abs(pos_args, named_args, arena, error_callback)->str());
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  itest(std::numeric_limits<int64_t>::min(), "9223372036854775808");
  itest(-1, "1");
  itest(0, "0");
  itest(1, "1");
  itest(std::numeric_limits<int64_t>::max(), "9223372036854775807");
  itest(number::minus_one << 64, "18446744073709551616");
  itest(number(number::minus_one), "1");
  itest(number(number::zero), "0");
  itest(number(number::one), "1");
  itest(number::one << 64, "18446744073709551616");
  ftest(std::copysign(std::numeric_limits<double>::quiet_NaN(), -1), "nan");
  ftest(std::numeric_limits<double>::quiet_NaN(), "nan");
  ftest(-std::numeric_limits<double>::infinity(), "inf");
  ftest(std::numeric_limits<double>::infinity(), "inf");
  ftest(-std::numeric_limits<double>::max(), "1.7976931348623157e+308");
  ftest(std::numeric_limits<double>::max(), "1.7976931348623157e+308");
  ftest(-std::numeric_limits<double>::min(), "2.2250738585072014e-308");
  ftest(std::numeric_limits<double>::min(), "2.2250738585072014e-308");
  ftest(-1.0, "1.0");
  ftest(std::copysign(0.0, -1), "0.0");
  ftest(0.0, "0.0");
  ftest(1.0, "1.0");
}

TEST(StarlarkAbs, List) {
  starlark_list list;
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_abs(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: bad operand type for abs(): 'list'", error_callback.messages[0]);
}

TEST(StarlarkAbs, NoPosArgs) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;

  EXPECT_EQ(nullptr, starlark_fn_abs(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: abs() takes exactly one argument (0 given)", error_callback.messages[0]);
}

TEST(StarlarkAbs, MultiplePosArgs) {
  starlark_integer one(1);
  starlark_list list;
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&list);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_abs(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: abs() takes exactly one argument (2 given)", error_callback.messages[0]);
}

TEST(StarlarkAbs, NamedArguments) {
  starlark_integer one(1);
  starlark_list list;
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  named_args["1"] = &one;
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_abs(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: abs() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkBool, List) {
  starlark_integer one(1);
  starlark_list list1;
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args1;
  std::map<std::string, starlark_obj*> named_args1;
  pos_args1.push_back(&list1);

  starlark_list list2;
  list2.add(&one, error_callback);
  std::vector<starlark_obj*> pos_args2;
  std::map<std::string, starlark_obj*> named_args2;
  pos_args2.push_back(&list2);

  EXPECT_EQ("True", starlark_fn_bool(pos_args2, named_args2, arena, error_callback)->str());
  EXPECT_EQ("False", starlark_fn_bool(pos_args1, named_args1, arena, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkBool, NoPosArgs) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;

  EXPECT_EQ(nullptr, starlark_fn_bool(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: bool() takes exactly one argument (0 given)", error_callback.messages[0]);
}

TEST(StarlarkBool, MultiplePosArgs) {
  starlark_integer one(1);
  starlark_list list;
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&list);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_bool(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: bool() takes exactly one argument (2 given)", error_callback.messages[0]);
}

TEST(StarlarkBool, NamedArguments) {
  starlark_integer one(1);
  starlark_list list;
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  named_args["1"] = &one;
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_bool(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: bool() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkLen, List) {
  starlark_integer one(1);
  starlark_list list1;
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args1;
  std::map<std::string, starlark_obj*> named_args1;
  pos_args1.push_back(&list1);

  starlark_list list2;
  list2.add(&one, error_callback);
  std::vector<starlark_obj*> pos_args2;
  std::map<std::string, starlark_obj*> named_args2;
  pos_args2.push_back(&list2);

  EXPECT_EQ("1", starlark_fn_len(pos_args2, named_args2, arena, error_callback)->str());
  EXPECT_EQ("0", starlark_fn_len(pos_args1, named_args1, arena, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkLen, Integer) {
  starlark_integer one(1);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_len(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: object of type 'int' has no len()", error_callback.messages[0]);
}

TEST(StarlarkLen, NoPosArgs) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;

  EXPECT_EQ(nullptr, starlark_fn_len(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: len() takes exactly one argument (0 given)", error_callback.messages[0]);
}

TEST(StarlarkLen, MultiplePosArgs) {
  starlark_integer one(1);
  starlark_list list;
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&list);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_len(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: len() takes exactly one argument (2 given)", error_callback.messages[0]);
}

TEST(StarlarkLen, NamedArguments) {
  starlark_integer one(1);
  starlark_list list;
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  named_args["1"] = &one;
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_len(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: len() takes no keyword arguments", error_callback.messages[0]);
}

}  // namespace
