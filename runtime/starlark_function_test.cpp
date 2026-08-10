// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <map>
#include <string>
#include <utility>
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
#include "runtime/starlark_numeric.hpp"
#include "runtime/starlark_range.hpp"
#include "runtime/starlark_string.hpp"
#include "runtime/starlark_testing.hpp"
#include "runtime/starlark_tuple.hpp"
#include "runtime/starlark_types.hpp"
#include "unicode/utf8_reader.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;
using ::starlark::bigint::parse_number;
using ::starlark::runtime::context;
using ::starlark::runtime::create_float;
using ::starlark::runtime::create_integer;
using ::starlark::runtime::error_fn;
using ::starlark::runtime::runtime_options;
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
using ::starlark::runtime::starlark_string;
using ::starlark::runtime::starlark_tuple;
using ::starlark::runtime::starlark_types;
using ::starlark::testing::error_handler;
using ::starlark::testing::starlark_testing_function;
using ::std::literals::string_view_literals::operator""sv;
using ::testing::IsEmpty;
using ::testing::SizeIs;
using starlark::unicode::utf8_reader;

namespace {

class Fn {
 public:
  MOCK_METHOD(starlark_obj*, Call, (starlark_obj* this_obj, const starlark_obj::pos_args_t&, const starlark_obj::named_args_t&, context&, error_fn&));
};

static Fn* fn_mock = nullptr;

starlark_obj* base_fn(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (fn_mock != nullptr) {
    return fn_mock->Call(this_obj, pos_args, named_args, ctx, error_callback);
  }
  return nullptr;
}

starlark_obj* base2_fn(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (fn_mock != nullptr) {
    return fn_mock->Call(this_obj, pos_args, named_args, ctx, error_callback);
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
  EXPECT_EQ("function", starlark_testing_function().type());
  EXPECT_EQ("builtin_function_or_method", starlark_built_in_function(nullptr, base_fn, "fn_name").type());
}

TEST(StarlarkFunction, Primitve) {
  EXPECT_FALSE(starlark_testing_function().primitive());
  EXPECT_FALSE(starlark_built_in_function(nullptr, base_fn, "fn_name").primitive());
}

TEST(StarlarkFunction, Truthy) {
  EXPECT_TRUE(starlark_testing_function().truthy());
  EXPECT_TRUE(starlark_built_in_function(nullptr, base_fn, "fn_name").truthy());
}

TEST(StarlarkFunction, Str) {
  starlark_list list(0);
  EXPECT_EQ("<built-in function fn_name>", starlark_built_in_function(nullptr, base_fn, "fn_name").str());
  EXPECT_EQ("<built-in method fn_name of list value>", starlark_built_in_function(&list, base_fn, "fn_name").str());
  EXPECT_EQ("<function foo from //:test.star>", starlark_testing_function("foo", "//:test.star").str());
}

TEST(StarlarkFunction, Hash) {
  EXPECT_EQ(0, starlark_built_in_function(nullptr, base_fn, "").hash());
  EXPECT_EQ(-5056436948751091085, starlark_built_in_function(nullptr, base_fn, "fn_name").hash());
  EXPECT_EQ(-342786463226536281, starlark_built_in_function(nullptr, base_fn, "some_fn").hash());
  EXPECT_EQ(0, starlark_testing_function("").hash());
  EXPECT_EQ(-8419484683692405967, starlark_testing_function("fn_name").hash());
  EXPECT_EQ(-4071090497886085033, starlark_testing_function("some_fn").hash());
}

TEST(StarlarkFunction, Equals) {
  starlark_list list1(0);
  starlark_list list2(0);
  starlark_integer zero(0);
  starlark_integer one(1);
  EXPECT_TRUE(starlark_built_in_function(nullptr, base_fn, "fn_name").equals(starlark_built_in_function(nullptr, base_fn, "fn_name")));
  EXPECT_FALSE(starlark_built_in_function(nullptr, base_fn, "fn_name").equals(starlark_built_in_function(nullptr, base2_fn, "fn_name")));
  EXPECT_FALSE(starlark_built_in_function(nullptr, base_fn, "fn_name").equals(starlark_built_in_function(nullptr, base_fn, "another_name")));
  EXPECT_FALSE(starlark_built_in_function(nullptr, base_fn, "fn_name").equals(starlark_built_in_function(nullptr, base2_fn, "another_name")));
  EXPECT_FALSE(starlark_built_in_function(nullptr, base_fn, "fn_name").equals(starlark_list(0)));
  EXPECT_FALSE(starlark_built_in_function(&list1, base_fn, "fn_name").equals(starlark_built_in_function(nullptr, base_fn, "fn_name")));
  EXPECT_FALSE(starlark_built_in_function(&list1, base_fn, "fn_name").equals(starlark_built_in_function(&list2, base_fn, "fn_name")));
  EXPECT_TRUE(starlark_built_in_function(&list1, base_fn, "fn_name").equals(starlark_built_in_function(&list1, base_fn, "fn_name")));
  EXPECT_FALSE(starlark_built_in_function(&zero, base_fn, "fn_name").equals(starlark_built_in_function(&list1, base_fn, "fn_name")));
  EXPECT_FALSE(starlark_built_in_function(&zero, base_fn, "fn_name").equals(starlark_built_in_function(&one, base_fn, "fn_name")));
  EXPECT_TRUE(starlark_built_in_function(&zero, base_fn, "fn_name").equals(starlark_built_in_function(&zero, base_fn, "fn_name")));
}

TEST_F(FnTest, Call) {
  starlark_built_in_function fn(nullptr, base_fn, "fn_name");
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_CALL(*fn_mock, Call(testing::_, testing::_, testing::_, testing::_, testing::_))
      .WillOnce(testing::Return(ctx.one()));
  auto* result = fn.call({}, {}, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ("1", result->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

starlark_obj* fn_error(starlark_obj* this_obj, const starlark_obj::pos_args_t&, const starlark_obj::named_args_t&, context&, error_fn& error_callback) {
  error_callback.add_error("Error message");
  return nullptr;
}

TEST_F(FnTest, CallWithError) {
  starlark_built_in_function fn(nullptr, base_fn, "fn_name");
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_CALL(*fn_mock, Call(testing::_, testing::_, testing::_, testing::_, testing::_))
      .WillOnce(testing::Invoke(fn_error));
  auto* result = fn.call({}, {}, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "Error message");
}

TEST(StarlarkAbs, Numeric) {
  auto itest = [](auto&& value, std::string_view result) {
    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    Arena arena;
    context ctx(arena);
    error_handler error_callback;
    pos_args.push_back(create_integer(std::forward<decltype(value)>(value), ctx));

    EXPECT_EQ(result, starlark_fn_abs(nullptr, pos_args, named_args, ctx, error_callback)->str());
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  auto ftest = [](double value, std::string_view result) {
    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    Arena arena;
    context ctx(arena);
    error_handler error_callback;
    pos_args.push_back(create_float(value, ctx));

    EXPECT_EQ(result, starlark_fn_abs(nullptr, pos_args, named_args, ctx, error_callback)->str());
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  itest(std::numeric_limits<int64_t>::min(), "9223372036854775808");
  itest(-1, "1");
  itest(0, "0");
  itest(1, "1");
  itest(std::numeric_limits<int64_t>::max(), "9223372036854775807");
  itest(number::minus_one() << 64, "18446744073709551616");
  itest(number(number::minus_one()), "1");
  itest(number(number::zero()), "0");
  itest(number(number::one()), "1");
  itest(number::one() << 64, "18446744073709551616");
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
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_abs(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("in call to abs(), got value of type 'list', want 'int' or 'float'", error_callback.messages[0]);
}

TEST(StarlarkAbs, NoPosArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  EXPECT_EQ(nullptr, starlark_fn_abs(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("abs() takes exactly one argument (0 given)", error_callback.messages[0]);
}

TEST(StarlarkAbs, MultiplePosArgs) {
  starlark_integer one(1);
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&list);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_abs(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("abs() takes exactly one argument (2 given)", error_callback.messages[0]);
}

TEST(StarlarkAbs, NamedArguments) {
  std::string s_one("1");
  starlark_integer one(1);
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_one, &one);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_abs(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("abs() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkAll, List) {
  starlark_integer zero(0);
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_list list1(0);
  starlark_obj::pos_args_t pos_args1;
  starlark_obj::named_args_t named_args1;
  pos_args1.push_back(&list1);

  starlark_list list2(0);
  list2.append(&one, ctx, error_callback);
  starlark_obj::pos_args_t pos_args2;
  starlark_obj::named_args_t named_args2;
  pos_args2.push_back(&list2);

  starlark_list list3(0);
  list3.append(&zero, ctx, error_callback);
  list3.append(&one, ctx, error_callback);
  starlark_obj::pos_args_t pos_args3;
  starlark_obj::named_args_t named_args3;
  pos_args3.push_back(&list3);

  EXPECT_EQ("True", starlark_fn_all(nullptr, pos_args1, named_args1, ctx, error_callback)->str());
  EXPECT_EQ("True", starlark_fn_all(nullptr, pos_args2, named_args2, ctx, error_callback)->str());
  EXPECT_EQ("False", starlark_fn_all(nullptr, pos_args3, named_args3, ctx, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkAll, Integer) {
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_all(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("'int' object is not iterable", error_callback.messages[0]);
}

TEST(StarlarkAll, NoPosArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  EXPECT_EQ(nullptr, starlark_fn_all(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("all() takes exactly one argument (0 given)", error_callback.messages[0]);
}

TEST(StarlarkAll, MultiplePosArgs) {
  starlark_integer one(1);
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&list);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_all(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("all() takes exactly one argument (2 given)", error_callback.messages[0]);
}

TEST(StarlarkAll, NamedArguments) {
  std::string s_one("1");
  starlark_integer one(1);
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_one, &one);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_all(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("all() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkAny, List) {
  starlark_integer zero(0);
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_list list1(0);
  starlark_obj::pos_args_t pos_args1;
  starlark_obj::named_args_t named_args1;
  pos_args1.push_back(&list1);

  starlark_list list2(0);
  list2.append(&one, ctx, error_callback);
  starlark_obj::pos_args_t pos_args2;
  starlark_obj::named_args_t named_args2;
  pos_args2.push_back(&list2);

  starlark_list list3(0);
  list3.append(&zero, ctx, error_callback);
  list3.append(&one, ctx, error_callback);
  starlark_obj::pos_args_t pos_args3;
  starlark_obj::named_args_t named_args3;
  pos_args3.push_back(&list3);

  starlark_list list4(0);
  list4.append(&zero, ctx, error_callback);
  starlark_obj::pos_args_t pos_args4;
  starlark_obj::named_args_t named_args4;
  pos_args4.push_back(&list4);

  EXPECT_EQ("False", starlark_fn_any(nullptr, pos_args1, named_args1, ctx, error_callback)->str());
  EXPECT_EQ("True", starlark_fn_any(nullptr, pos_args2, named_args2, ctx, error_callback)->str());
  EXPECT_EQ("True", starlark_fn_any(nullptr, pos_args3, named_args3, ctx, error_callback)->str());
  EXPECT_EQ("False", starlark_fn_any(nullptr, pos_args4, named_args4, ctx, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkAny, Integer) {
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_any(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("'int' object is not iterable", error_callback.messages[0]);
}

TEST(StarlarkAny, NoPosArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  EXPECT_EQ(nullptr, starlark_fn_any(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("any() takes exactly one argument (0 given)", error_callback.messages[0]);
}

TEST(StarlarkAny, MultiplePosArgs) {
  starlark_integer one(1);
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&list);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_any(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("any() takes exactly one argument (2 given)", error_callback.messages[0]);
}

TEST(StarlarkAny, NamedArguments) {
  std::string s_one("1");
  starlark_integer one(1);
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_one, &one);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_any(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("any() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkBool, List) {
  starlark_integer one(1);
  starlark_list list1(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args1;
  starlark_obj::named_args_t named_args1;
  pos_args1.push_back(&list1);

  starlark_list list2(0);
  list2.append(&one, ctx, error_callback);
  starlark_obj::pos_args_t pos_args2;
  starlark_obj::named_args_t named_args2;
  pos_args2.push_back(&list2);

  EXPECT_EQ("True", starlark_fn_bool(nullptr, pos_args2, named_args2, ctx, error_callback)->str());
  EXPECT_EQ("False", starlark_fn_bool(nullptr, pos_args1, named_args1, ctx, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkBool, NoPosArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  auto* result = starlark_fn_bool(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bool_t);
  EXPECT_EQ(result->truthy(), false);
  ASSERT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkBool, MultiplePosArgs) {
  starlark_integer one(1);
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&list);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_bool(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("bool expected at most 1 argument, got 2", error_callback.messages[0]);
}

TEST(StarlarkBool, NamedArguments) {
  std::string s_one("1");
  starlark_integer one(1);
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_one, &one);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_bool(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("bool() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkBytes, FromBytesOrString) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  {
    starlark_string str("abc"sv);
    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&str);

    auto* result = starlark_fn_bytes(nullptr, pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->repr(), "b\"abc\"");
  }
  {
    starlark_bytes bytes("def"sv);
    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&bytes);

    auto* result = starlark_fn_bytes(nullptr, pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->repr(), "b\"def\"");
  }
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkBytes, FromStringInvalidUnicodeSequence) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_string str("abc\xf0\x{f1}def"sv);
  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);

  auto* result = starlark_fn_bytes(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "b\"abc\\xef\\xbf\\xbd\\xef\\xbf\\xbddef\"");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkBytes, FromStringOverflowNoOverflow) {
  Arena arena;
  context ctx(arena, runtime_options{.max_string_length = 20});
  error_handler error_callback;

  starlark_string str("abc\xf0\x{f1}def01234567"sv);
  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);

  auto* result = starlark_fn_bytes(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "b\"abc\\xef\\xbf\\xbd\\xef\\xbf\\xbddef01234567\"");

  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkBytes, FromStringOverflowOverflow) {
  Arena arena;
  context ctx(arena, runtime_options{.max_string_length = 20});
  error_handler error_callback;

  // The only way to make the new bytes longer is to use invalid sequences that will
  // force the output to be larger. If the logic were to change, then this condition
  // can be removed.
  starlark_string str("abc\xf0\x{f1}def012345678"sv);
  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);

  auto* result = starlark_fn_bytes(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: bytes must be at most 20 elements");
}

TEST(StarlarkBytes, List) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer max_minus_one(254);
  starlark_bigint max_byte(255);
  starlark_list list1(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args1;
  starlark_obj::named_args_t named_args1;
  pos_args1.push_back(&list1);

  starlark_list list2(0);
  list2.append(&zero, ctx, error_callback);
  list2.append(&one, ctx, error_callback);
  list2.append(&max_minus_one, ctx, error_callback);
  list2.append(&max_byte, ctx, error_callback);
  starlark_obj::pos_args_t pos_args2;
  starlark_obj::named_args_t named_args2;
  pos_args2.push_back(&list2);

  auto* result1 = starlark_fn_bytes(nullptr, pos_args1, named_args1, ctx, error_callback);
  auto* result2 = starlark_fn_bytes(nullptr, pos_args2, named_args2, ctx, error_callback);
  ASSERT_NE(nullptr, result1);
  EXPECT_EQ("b\"\"", result1->str());
  ASSERT_NE(nullptr, result2) << error_callback.messages.front();
  EXPECT_EQ("b\"\\x00\\x01\\xfe\\xff\"", result2->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkBytes, OutOfRange) {
  starlark_integer iminus_one(-1);
  starlark_bigint bminus_one(-1);
  starlark_integer imax_plus_one(256);
  starlark_bigint bmax_plus_one(256);
  starlark_bigint big(number::one() << 64);

  auto test = [](starlark_obj* value) {
    Arena arena;
    context ctx(arena);
    error_handler error_callback;
    starlark_list list(0);
    list.append(value, ctx, error_callback);
    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&list);

    EXPECT_EQ(nullptr, starlark_fn_bytes(nullptr, pos_args, named_args, ctx, error_callback)) << value->str();
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ("bytes must be in range(0, 256)", error_callback.messages[0]);
  };
  test(&iminus_one);
  test(&bminus_one);
  test(&imax_plus_one);
  test(&bmax_plus_one);
  test(&big);
}

TEST(StarlarkBytes, NoPosArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  auto* result = starlark_fn_bytes(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->type(), starlark_types::bytes_t);
  EXPECT_EQ(result->str(), "b\"\"");
  ASSERT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkBytes, ListWithNone) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_list list(0);
  starlark_none none;
  list.append(&none, ctx, error_callback);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_bytes(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("'NoneType' object cannot be interpreted as an integer", error_callback.messages[0]);
}

TEST(StarlarkBytes, None) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_none none;
  pos_args.push_back(&none);

  EXPECT_EQ(nullptr, starlark_fn_bytes(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("cannot convert 'NoneType' object to bytes", error_callback.messages[0]);
}

TEST(StarlarkBytes, MultiplePosArgs) {
  starlark_bytes bytes("def"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&bytes);
  pos_args.push_back(&bytes);

  EXPECT_EQ(nullptr, starlark_fn_bytes(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("bytes expected at most 1 argument, got 2", error_callback.messages[0]);
}

TEST(StarlarkBytes, UnknownNamedArguments) {
  starlark_bytes bytes("def"sv);
  std::string s_one("1");
  starlark_integer one(1);
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&bytes);
  named_args.insert(s_one, &one);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_bytes(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("unknown named argument '1'", error_callback.messages[0]);
}

TEST(StarlarkBytes, SourceAsNamedArgument) {
  starlark_string str("123"sv);
  std::string s_source("source");
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_source, &str);

  auto* result = starlark_fn_bytes(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->type(), starlark_types::bytes_t);
  EXPECT_EQ(result->str(), "b\"123\"");
  ASSERT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkBytes, SourceAsNamedAndPositionalArgument) {
  starlark_string str("123"sv);
  std::string s_source("source");
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  named_args.insert(s_source, &str);

  EXPECT_EQ(nullptr, starlark_fn_bytes(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: bytes() got multiple values for argument 'source'", error_callback.messages[0]);
}

TEST(StarlarkChr, FromInt) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  for (int i = 0; i <= 0x10FFFF; ++i) {
    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    starlark_integer value(i);
    pos_args.push_back(&value);

    auto* result = starlark_fn_chr(nullptr, pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    ASSERT_EQ(starlark_types::string_t, result->type());
    EXPECT_EQ(i, utf8_reader(result->str(), false, false).read_code_point());
  }
  for (int i = 0; i <= 0x10FFFF; ++i) {
    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    starlark_bigint value(i);
    pos_args.push_back(&value);

    auto* result = starlark_fn_chr(nullptr, pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    ASSERT_EQ(starlark_types::string_t, result->type());
    EXPECT_EQ(i, utf8_reader(result->str(), false, false).read_code_point());
  }
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkChr, FromFloat) {
  starlark_float one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_chr(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("'float' object cannot be interpreted as an integer", error_callback.messages[0]);
}

TEST(StarlarkChr, OutOfRange1) {
  starlark_integer value(-1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&value);

  EXPECT_EQ(nullptr, starlark_fn_chr(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("Unicode code point must be in range(0, 0x110000)", error_callback.messages[0]);
}

TEST(StarlarkChr, OutOfRange2) {
  starlark_bigint value(-1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&value);

  EXPECT_EQ(nullptr, starlark_fn_chr(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("Unicode code point must be in range(0, 0x110000)", error_callback.messages[0]);
}

TEST(StarlarkChr, OutOfRange3) {
  starlark_integer value(0x110000);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&value);

  EXPECT_EQ(nullptr, starlark_fn_chr(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("Unicode code point must be in range(0, 0x110000)", error_callback.messages[0]);
}

TEST(StarlarkChr, OutOfRange4) {
  starlark_bigint value(0x110000);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&value);

  EXPECT_EQ(nullptr, starlark_fn_chr(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("Unicode code point must be in range(0, 0x110000)", error_callback.messages[0]);
}

TEST(StarlarkChr, NamedArguments) {
  starlark_bytes bytes("def"sv);
  std::string s_one("1");
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&one);
  named_args.insert(s_one, &one);

  EXPECT_EQ(nullptr, starlark_fn_chr(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("chr() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkDict, Empty) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  auto* result = starlark_fn_dict(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "{}");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkDict, FromDict) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_dictionary dict;
  starlark_none none;
  starlark_integer one(1);
  dict.insert(&none, &none, error_callback);
  pos_args.push_back(&dict);

  auto* result = starlark_fn_dict(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "{None: None}");
  // Check that this is a copy.
  dict.insert(&none, &one, error_callback);
  EXPECT_EQ(result->repr(), "{None: None}");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkDict, FromIterable) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_list list(0);
  starlark_tuple tuple(0);
  starlark_none none;
  starlark_integer one(1);
  tuple.add(&none);
  tuple.add(&one);
  list.append(&tuple, ctx, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_dict(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "{None: 1}");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkDict, FromNamedArguments) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  std::string s_one("one");
  std::string s_two("two");

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_integer one(1);
  starlark_integer two(2);
  named_args.insert(s_one, &one);
  named_args.insert(s_two, &two);

  auto* result = starlark_fn_dict(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "{\"one\": 1, \"two\": 2}");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkDict, FromInteger) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_integer one(1);
  pos_args.push_back(&one);

  auto* result = starlark_fn_dict(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'int' object is not iterable");
}

TEST(StarlarkDict, FromNonIterable) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_list list(0);
  starlark_integer one(1);
  list.append(&one, ctx, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_dict(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'int' object is not iterable");
}

TEST(StarlarkDict, FromNonHashable) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_list list1(0);
  starlark_list list2(0);
  starlark_tuple tuple(0);
  starlark_integer one(1);
  tuple.add(&list2);
  tuple.add(&one);
  list1.append(&tuple, ctx, error_callback);
  pos_args.push_back(&list1);

  auto* result = starlark_fn_dict(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "cannot use 'list' as a dict key (unhashable type: 'list')");
}

TEST(StarlarkDict, MultiplePositionalArguments) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_list list(0);
  pos_args.push_back(&list);
  pos_args.push_back(&list);

  auto* result = starlark_fn_dict(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "dict expected at most 1 argument, got 2");
}

TEST(StarlarkDict, FromIterableWithWrongNumberOfElements1) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_list list(0);
  starlark_tuple tuple1(0);
  starlark_tuple tuple2(0);
  starlark_none none;
  starlark_integer one(1);
  tuple1.add(&none);
  tuple1.add(&one);
  list.append(&tuple1, ctx, error_callback);
  list.append(&tuple2, ctx, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_dict(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "dictionary update sequence element #1 has length 0; 2 is required");
}

TEST(StarlarkDict, FromIterableWithWrongNumberOfElements2) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_list list(0);
  starlark_tuple tuple1(0);
  starlark_tuple tuple2(0);
  starlark_none none;
  starlark_integer one(1);
  tuple1.add(&none);
  tuple1.add(&one);
  tuple2.add(&none);
  list.append(&tuple1, ctx, error_callback);
  list.append(&tuple2, ctx, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_dict(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "dictionary update sequence element #1 has length 1; 2 is required");
}

TEST(StarlarkDict, FromIterableWithWrongNumberOfElements3) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_list list(0);
  starlark_tuple tuple1(0);
  starlark_tuple tuple2(0);
  starlark_none none;
  starlark_integer one(1);
  tuple1.add(&none);
  tuple1.add(&one);
  tuple2.add(&none);
  tuple2.add(&none);
  tuple2.add(&none);
  list.append(&tuple1, ctx, error_callback);
  list.append(&tuple2, ctx, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_dict(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "dictionary update sequence element #1 has length 3; 2 is required");
}

TEST(StarlarkDir, ReturnsTheAttributes) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_list list(0);
  pos_args.push_back(&list);

  auto* result = starlark_fn_dir(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->str(), "[\"append\", \"clear\", \"extend\", \"index\", \"insert\", \"pop\", \"remove\"]");
  ASSERT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkDir, TooFewPosArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  auto* result = starlark_fn_dir(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "dir() takes exactly one argument (0 given)");
}

TEST(StarlarkDir, TooManyPosArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_list list(0);
  pos_args.push_back(&list);
  pos_args.push_back(&list);

  auto* result = starlark_fn_dir(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "dir() takes exactly one argument (2 given)");
}

TEST(StarlarkDir, NamedArguments) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  std::string s_one("one");
  starlark_integer one(1);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_list list(0);
  pos_args.push_back(&list);
  named_args.insert(s_one, &one);

  auto* result = starlark_fn_dir(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "dir() takes no keyword arguments");
}

TEST(StarlarkEnumerate, FromIterable) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_list list(0);
  starlark_string s_one("one"sv);
  starlark_string s_two("two"sv);
  starlark_string s_three("three"sv);
  list.append(&s_one, ctx, error_callback);
  list.append(&s_two, ctx, error_callback);
  list.append(&s_three, ctx, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_enumerate(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "[(0, \"one\"), (1, \"two\"), (2, \"three\")]");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkEnumerate, FromIterableWithIterableAsNamedArgument) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_list list(0);
  std::string s_iterable("iterable");
  std::string s_start("start");
  starlark_string s_one("one"sv);
  starlark_string s_two("two"sv);
  starlark_string s_three("three"sv);
  list.append(&s_one, ctx, error_callback);
  list.append(&s_two, ctx, error_callback);
  list.append(&s_three, ctx, error_callback);
  starlark_integer start(100);
  named_args.insert(s_iterable, &list);
  named_args.insert(s_start, &start);

  auto* result = starlark_fn_enumerate(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "[(100, \"one\"), (101, \"two\"), (102, \"three\")]");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkEnumerate, FromIterableWithStartAsNamedArgument) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_list list(0);
  std::string s_start("start");
  starlark_string s_one("one"sv);
  starlark_string s_two("two"sv);
  starlark_string s_three("three"sv);
  list.append(&s_one, ctx, error_callback);
  list.append(&s_two, ctx, error_callback);
  list.append(&s_three, ctx, error_callback);
  pos_args.push_back(&list);
  starlark_integer start(100);
  named_args.insert(s_start, &start);

  auto* result = starlark_fn_enumerate(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "[(100, \"one\"), (101, \"two\"), (102, \"three\")]");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkEnumerate, FromIterableWithStartAsPositionalArgument) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_list list(0);
  starlark_string s_one("one"sv);
  starlark_string s_two("two"sv);
  starlark_string s_three("three"sv);
  list.append(&s_one, ctx, error_callback);
  list.append(&s_two, ctx, error_callback);
  list.append(&s_three, ctx, error_callback);
  pos_args.push_back(&list);
  starlark_integer start(100);
  pos_args.push_back(&start);

  auto* result = starlark_fn_enumerate(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "[(100, \"one\"), (101, \"two\"), (102, \"three\")]");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkEnumerate, InvalidStartAsNamedArgument) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_list list(0);
  std::string s_start("start");
  starlark_string s_one("one"sv);
  starlark_string s_two("two"sv);
  starlark_string s_three("three"sv);
  list.append(&s_one, ctx, error_callback);
  list.append(&s_two, ctx, error_callback);
  list.append(&s_three, ctx, error_callback);
  pos_args.push_back(&list);
  starlark_string start("100"sv);
  named_args.insert(s_start, &start);

  auto* result = starlark_fn_enumerate(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "parameter 'start' cannot be interpreted as an integer (string)");
}

TEST(StarlarkEnumerate, InvalidStartAsPositionalArgument) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_list list(0);
  starlark_string s_one("one"sv);
  starlark_string s_two("two"sv);
  starlark_string s_three("three"sv);
  list.append(&s_one, ctx, error_callback);
  list.append(&s_two, ctx, error_callback);
  list.append(&s_three, ctx, error_callback);
  pos_args.push_back(&list);
  starlark_string start("100"sv);
  pos_args.push_back(&start);

  auto* result = starlark_fn_enumerate(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "parameter 'start' cannot be interpreted as an integer (string)");
}

TEST(StarlarkEnumerate, IterableAsNamedArgumentAndPositionalArgument) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_list list(0);
  std::string s_iterable("iterable");
  starlark_string s_one("one"sv);
  starlark_string s_two("two"sv);
  starlark_string s_three("three"sv);
  starlark_integer start(100);
  list.append(&s_one, ctx, error_callback);
  list.append(&s_two, ctx, error_callback);
  list.append(&s_three, ctx, error_callback);
  pos_args.push_back(&list);
  pos_args.push_back(&start);
  named_args.insert(s_iterable, &list);

  auto* result = starlark_fn_enumerate(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: enumerate() got multiple values for argument 'iterable'");
}

TEST(StarlarkEnumerate, StartAsNamedArgumentAndPositionalArgument) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_list list(0);
  std::string s_start("start");
  starlark_string s_one("one"sv);
  starlark_string s_two("two"sv);
  starlark_string s_three("three"sv);
  list.append(&s_one, ctx, error_callback);
  list.append(&s_two, ctx, error_callback);
  list.append(&s_three, ctx, error_callback);
  pos_args.push_back(&list);
  starlark_integer start(100);
  named_args.insert(s_start, &start);
  pos_args.push_back(&start);

  auto* result = starlark_fn_enumerate(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: enumerate() got multiple values for argument 'start'");
}

TEST(StarlarkEnumerate, InvalidNamedArgument) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_list list(0);
  starlark_string s_one("one"sv);
  starlark_string s_two("two"sv);
  starlark_string s_three("three"sv);
  std::string s_end("end");
  list.append(&s_one, ctx, error_callback);
  list.append(&s_two, ctx, error_callback);
  list.append(&s_three, ctx, error_callback);
  pos_args.push_back(&list);
  starlark_string end("100"sv);
  named_args.insert(s_end, &end);

  auto* result = starlark_fn_enumerate(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "unknown named argument 'end'");
}

TEST(StarlarkEnumerate, TooFewPosArguments) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  auto* result = starlark_fn_enumerate(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: enumerate() missing required argument 'iterable'");
}

TEST(StarlarkEnumerate, TwoPosArguments) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_list list(0);
  starlark_string s_one("one"sv);
  starlark_string s_two("two"sv);
  starlark_string s_three("three"sv);
  list.append(&s_one, ctx, error_callback);
  list.append(&s_two, ctx, error_callback);
  list.append(&s_three, ctx, error_callback);
  pos_args.push_back(&list);
  pos_args.push_back(ctx.one());

  auto* result = starlark_fn_enumerate(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "[(1, \"one\"), (2, \"two\"), (3, \"three\")]");
  ASSERT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkEnumerate, ThreePosArguments) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_list list(0);
  starlark_string s_one("one"sv);
  starlark_string s_two("two"sv);
  starlark_string s_three("three"sv);
  list.append(&s_one, ctx, error_callback);
  list.append(&s_two, ctx, error_callback);
  list.append(&s_three, ctx, error_callback);
  pos_args.push_back(&list);
  pos_args.push_back(&list);
  pos_args.push_back(&list);

  auto* result = starlark_fn_enumerate(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "enumerate expected at most 2 argument, got 3");
}

TEST(StarlarkEnumerate, NotIterable) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_string s_one("one"sv);
  pos_args.push_back(&s_one);

  auto* result = starlark_fn_enumerate(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'string' object is not iterable");
}

TEST(StarlarkFail, Message) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_string str("some error message"sv);
  starlark_list list(0);
  starlark_integer one(1);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  pos_args.push_back(&list);
  pos_args.push_back(&one);

  auto* result = starlark_fn_fail(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "Error: some error message [] 1");
}

TEST(StarlarkFail, NamedArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  std::string s_one("1");
  starlark_integer one(1);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_one, &one);

  EXPECT_EQ(nullptr, starlark_fn_fail(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("fail() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkFloat, NoArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  auto result = starlark_fn_float(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::float_t);
  EXPECT_EQ("0.0", result->repr());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkFloat, FromFloat) {
  auto test = [](double fvalue, std::string_view repr) {
    starlark_float value(fvalue);
    Arena arena;
    context ctx(arena);
    error_handler error_callback;

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&value);

    EXPECT_EQ(repr, starlark_fn_float(nullptr, pos_args, named_args, ctx, error_callback)->repr());
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test(1.25, "1.25");
  test(std::numeric_limits<double>::infinity(), "inf");
  test(-std::numeric_limits<double>::infinity(), "-inf");
  test(std::numeric_limits<double>::quiet_NaN(), "nan");
}

TEST(StarlarkFloat, FromInteger) {
  auto test = [](int64_t ivalue, std::string_view repr) {
    starlark_integer value(ivalue);
    Arena arena;
    context ctx(arena);
    error_handler error_callback;

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&value);

    EXPECT_EQ(repr, starlark_fn_float(nullptr, pos_args, named_args, ctx, error_callback)->repr());
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test(1, "1.0");
  test(std::numeric_limits<int64_t>::max(), "9.223372036854776e+18");
  test(std::numeric_limits<int64_t>::min(), "-9.223372036854776e+18");
}

TEST(StarlarkFloat, FromBigint) {
  auto test = [](int64_t ivalue, std::string_view repr) {
    starlark_bigint value(ivalue);
    Arena arena;
    context ctx(arena);
    error_handler error_callback;

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&value);

    EXPECT_EQ(repr, starlark_fn_float(nullptr, pos_args, named_args, ctx, error_callback)->repr());
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test(1, "1.0");
  test(std::numeric_limits<int64_t>::max(), "9.223372036854776e+18");
  test(std::numeric_limits<int64_t>::min(), "-9.223372036854776e+18");
}

TEST(StarlarkFloat, FromBigintEdgeCases) {
  auto test = [](std::string_view ivalue, double fvalue) {
    starlark_bigint value{parse_number(ivalue, nullptr, 0)};
    Arena arena;
    context ctx(arena);
    error_handler error_callback;

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&value);

    EXPECT_EQ(fvalue, starlark_fn_float(nullptr, pos_args, named_args, ctx, error_callback)->as_float());
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test("43629857643785634295372846", 4.362985764378564e+25);
  test("0x120b7da93b9910f00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", 0x1.20b7da93b9911p+520);
  test("0x120b7da93b9910e00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", 0x1.20b7da93b9911p+520);
  test("0x120b7da93b9910d00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", 0x1.20b7da93b9911p+520);
  test("0x120b7da93b9910c00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", 0x1.20b7da93b9911p+520);
  test("0x120b7da93b9910b00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", 0x1.20b7da93b9911p+520);
  test("0x120b7da93b9910a00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", 0x1.20b7da93b9911p+520);
  test("0x120b7da93b9910900000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", 0x1.20b7da93b9911p+520);
  test("0x120b7da93b9910800000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001", 0x1.20b7da93b9911p+520);
  test("0x120b7da93b9910800000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", 0x1.20b7da93b9910p+520);
  test("0x120b7da93b99107ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff", 0x1.20b7da93b9910p+520);
}

TEST(StarlarkFloat, FromString) {
  auto test = [](std::string_view svalue, std::string_view repr) {
    starlark_string value(svalue);
    Arena arena;
    context ctx(arena);
    error_handler error_callback;

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&value);

    EXPECT_EQ(repr, starlark_fn_float(nullptr, pos_args, named_args, ctx, error_callback)->repr());
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test("1", "1.0");
  test("-1", "-1.0");
  test("1e308", "1e+308");
  test("Infinity", "inf");
  test("-Infinity", "-inf");
  test("NaN", "nan");
  test("-NaN", "nan");
  test("+NaN", "nan");
}

TEST(StarlarkFloat, FromBool) {
  auto test = [](bool bvalue, std::string_view repr) {
    starlark_bool value(bvalue);
    Arena arena;
    context ctx(arena);
    error_handler error_callback;

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&value);

    EXPECT_EQ(repr, starlark_fn_float(nullptr, pos_args, named_args, ctx, error_callback)->repr());
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test(false, "0.0");
  test(true, "1.0");
}

TEST(StarlarkFloat, FromList) {
  starlark_list value(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&value);

  auto* result = starlark_fn_float(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "float() argument must be a string or a real number, not 'list'");
}

TEST(StarlarkFloat, BigintOverflow) {
  starlark_bigint value(number::one() << 2000);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&value);

  auto* result = starlark_fn_float(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "int too large to convert to float");
}

TEST(StarlarkFloat, StringOverflow) {
  starlark_string value("2e308"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&value);

  auto* result = starlark_fn_float(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "floating-point number too large");
}

TEST(StarlarkFloat, InvalidString) {
  starlark_string value("1a"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&value);

  auto* result = starlark_fn_float(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "could not convert string to float: '1a'");
}

TEST(StarlarkFloat, MultiplePosArgs) {
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&one);
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_float(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("float expected at most 1 argument, got 2", error_callback.messages[0]);
}

TEST(StarlarkFloat, NamedArguments) {
  std::string s_one("1");
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_one, &one);
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_float(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("float() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkGetattr, CheckAttribute) {
  starlark_list list(0);
  starlark_string attribute("append"sv);
  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  pos_args.push_back(&list);
  pos_args.push_back(&attribute);
  auto* result = starlark_fn_getattr(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);

  pos_args.clear();
  named_args.clear();
  pos_args.push_back(&list);
  auto* call_result = result->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, call_result);
  EXPECT_EQ(call_result->str(), "None");
  EXPECT_EQ(list.str(), "[[...]]");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkGetattr, AttributeDoesNotExist) {
  starlark_list list(0);
  starlark_string attribute("appen"sv);
  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  pos_args.push_back(&list);
  pos_args.push_back(&attribute);
  auto* result = starlark_fn_getattr(nullptr, pos_args, named_args, ctx, error_callback);

  ASSERT_EQ(nullptr, result);
  EXPECT_EQ(list.str(), "[]");
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("'list' object has no attribute 'appen'. Did you mean: 'append'?", error_callback.messages[0]);
}

TEST(StarlarkGetattr, AttributeDoesNotExistWithDefault) {
  starlark_list list(0);
  starlark_list list2(0);
  starlark_string attribute("appen"sv);
  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  pos_args.push_back(&list);
  pos_args.push_back(&attribute);
  pos_args.push_back(&list2);
  auto* result = starlark_fn_getattr(nullptr, pos_args, named_args, ctx, error_callback);

  ASSERT_NE(nullptr, result);
  EXPECT_EQ(&list2, result);
  EXPECT_EQ(list.str(), "[]");
  ASSERT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkGetattr, AttributeNotString) {
  starlark_list list(0);
  starlark_bytes attribute("append"sv);
  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  pos_args.push_back(&list);
  pos_args.push_back(&attribute);
  auto* result = starlark_fn_getattr(nullptr, pos_args, named_args, ctx, error_callback);

  ASSERT_EQ(nullptr, result);
  EXPECT_EQ(list.str(), "[]");
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("attribute name must be string, not 'bytes'", error_callback.messages[0]);
}

TEST(StarlarkGetattr, OneParam) {
  starlark_list list(0);
  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  pos_args.push_back(&list);
  auto* result = starlark_fn_getattr(nullptr, pos_args, named_args, ctx, error_callback);

  ASSERT_EQ(nullptr, result);
  EXPECT_EQ(list.str(), "[]");
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("getattr expected at least 2 argument, got 1", error_callback.messages[0]);
}

TEST(StarlarkGetattr, FourParams) {
  starlark_list list(0);
  starlark_string attribute("append"sv);
  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  pos_args.push_back(&list);
  pos_args.push_back(&attribute);
  pos_args.push_back(&attribute);
  pos_args.push_back(&attribute);
  auto* result = starlark_fn_getattr(nullptr, pos_args, named_args, ctx, error_callback);

  ASSERT_EQ(nullptr, result);
  EXPECT_EQ(list.str(), "[]");
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("getattr expected at most 3 argument, got 4", error_callback.messages[0]);
}

TEST(StarlarkGetattr, NamedArguments) {
  starlark_list list(0);
  starlark_string attribute("append"sv);
  std::string s_one("one");
  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  pos_args.push_back(&list);
  pos_args.push_back(&attribute);
  named_args.insert(s_one, ctx.one());

  auto* result = starlark_fn_getattr(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("getattr() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkHasattr, CheckAttribute) {
  auto test = [](starlark_obj& element, std::string_view attr, bool expected) {
    starlark_string attribute(attr);
    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    Arena arena;
    context ctx(arena);
    error_handler error_callback;

    pos_args.push_back(&element);
    pos_args.push_back(&attribute);

    auto* result = starlark_fn_hasattr(nullptr, pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->truthy(), expected) << "Key: '" << attr << "'";
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  starlark_string str(""sv);
  test(str, "count", true);
  test(str, "coun", false);
  test(str, "zzz", false);
  test(str, "", false);
}

TEST(StarlarkHasattr, WrongAttributeType) {
  starlark_string str(""sv);
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_hasattr(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("attribute name must be string, not 'int'", error_callback.messages[0]);
}

TEST(StarlarkHasattr, TooFewPosArgs) {
  starlark_string str(""sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);

  EXPECT_EQ(nullptr, starlark_fn_hasattr(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("hasattr expected 2 arguments, got 1", error_callback.messages[0]);
}

TEST(StarlarkHasattr, TooManyPosArgs) {
  starlark_string str(""sv);
  starlark_string attr("count"sv);
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  pos_args.push_back(&attr);
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_hasattr(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("hasattr expected 2 arguments, got 3", error_callback.messages[0]);
}

TEST(StarlarkHasattr, NamedArguments) {
  std::string s_one("1");
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_one, &one);
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_hasattr(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("hasattr() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkHash, String) {
  starlark_string str1(""sv);
  starlark_string str2("abc"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args1;
  starlark_obj::named_args_t named_args1;
  pos_args1.push_back(&str1);
  starlark_obj::pos_args_t pos_args2;
  starlark_obj::named_args_t named_args2;
  pos_args2.push_back(&str2);

  EXPECT_EQ("0", starlark_fn_hash(nullptr, pos_args1, named_args1, ctx, error_callback)->str());
  EXPECT_EQ("6041520446639342335", starlark_fn_hash(nullptr, pos_args2, named_args2, ctx, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkHash, Bytes) {
  starlark_bytes bytes1(""sv);
  starlark_bytes bytes2("abc"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args1;
  starlark_obj::named_args_t named_args1;
  pos_args1.push_back(&bytes1);
  starlark_obj::pos_args_t pos_args2;
  starlark_obj::named_args_t named_args2;
  pos_args2.push_back(&bytes2);

  EXPECT_EQ("0", starlark_fn_hash(nullptr, pos_args1, named_args1, ctx, error_callback)->str());
  EXPECT_EQ("-8236155743588961689", starlark_fn_hash(nullptr, pos_args2, named_args2, ctx, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkHash, Bool) {
  starlark_bool true_obj(true);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&true_obj);

  EXPECT_EQ(nullptr, starlark_fn_hash(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("in call to hash(), got value of type 'bool', want 'string' or 'bytes'", error_callback.messages[0]);
}

TEST(StarlarkHash, NoPosArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  EXPECT_EQ(nullptr, starlark_fn_hash(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("hash() takes exactly one argument (0 given)", error_callback.messages[0]);
}

TEST(StarlarkHash, MultiplePosArgs) {
  starlark_string str(""sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  pos_args.push_back(&str);

  EXPECT_EQ(nullptr, starlark_fn_hash(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("hash() takes exactly one argument (2 given)", error_callback.messages[0]);
}

TEST(StarlarkHash, NamedArguments) {
  std::string s_one("1");
  starlark_string str(""sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_one, &str);
  pos_args.push_back(&str);

  EXPECT_EQ(nullptr, starlark_fn_hash(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("hash() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkInt, FromInt) {
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&one);

  EXPECT_EQ("1", starlark_fn_int(nullptr, pos_args, named_args, ctx, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkInt, FromIntWithBaseAsPositionalArgument) {
  starlark_integer one(1);
  starlark_integer two(2);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&one);
  pos_args.push_back(&two);

  EXPECT_EQ(nullptr, starlark_fn_int(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: int() can't convert non-string with explicit base", error_callback.messages[0]);
}

TEST(StarlarkInt, FromIntWithBaseAsNamedArgument) {
  starlark_integer one(1);
  starlark_integer two(2);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&one);
  std::string s_base("base"sv);
  named_args.insert(s_base, &two);

  EXPECT_EQ(nullptr, starlark_fn_int(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: int() can't convert non-string with explicit base", error_callback.messages[0]);
}

TEST(StarlarkInt, FromFloat) {
  starlark_float value(1e70);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&value);

  EXPECT_EQ("10000000000000000725314363815292351261583744096465219555182101554790400", starlark_fn_int(nullptr, pos_args, named_args, ctx, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkInt, FromFloatInfinity) {
  starlark_float value(std::numeric_limits<double>::infinity());
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&value);

  EXPECT_EQ(nullptr, starlark_fn_int(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("cannot convert float infinity to integer", error_callback.messages[0]);
}

TEST(StarlarkInt, FromFloatNaN) {
  starlark_float value(std::numeric_limits<double>::quiet_NaN());
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&value);

  EXPECT_EQ(nullptr, starlark_fn_int(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("cannot convert float NaN to integer", error_callback.messages[0]);
}

TEST(StarlarkInt, FromFloatWithBaseWithPositionalArgument) {
  starlark_float value(1e70);
  starlark_integer two(2);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&value);
  pos_args.push_back(&two);

  EXPECT_EQ(nullptr, starlark_fn_int(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: int() can't convert non-string with explicit base", error_callback.messages[0]);
}

TEST(StarlarkInt, FromFloatWithBaseWithNamedArgument) {
  starlark_float value(1e70);
  starlark_integer two(2);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&value);
  std::string s_base("base"sv);
  named_args.insert(s_base, &two);

  EXPECT_EQ(nullptr, starlark_fn_int(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: int() can't convert non-string with explicit base", error_callback.messages[0]);
}

TEST(StarlarkInt, FromBool) {
  starlark_bool true_value(true);
  starlark_bool false_value(false);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args1;
  starlark_obj::named_args_t named_args1;
  pos_args1.push_back(&true_value);
  starlark_obj::pos_args_t pos_args2;
  starlark_obj::named_args_t named_args2;
  pos_args2.push_back(&false_value);

  EXPECT_EQ("1", starlark_fn_int(nullptr, pos_args1, named_args1, ctx, error_callback)->str());
  EXPECT_EQ("0", starlark_fn_int(nullptr, pos_args2, named_args2, ctx, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkInt, FromBoolWithBaseAsPositionalArgument) {
  starlark_bool value(true);
  starlark_integer two(2);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&value);
  pos_args.push_back(&two);

  EXPECT_EQ(nullptr, starlark_fn_int(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: int() can't convert non-string with explicit base", error_callback.messages[0]);
}

TEST(StarlarkInt, FromBoolWithBaseAsNamedArgument) {
  starlark_bool value(true);
  starlark_integer two(2);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&value);
  std::string s_base("base"sv);
  named_args.insert(s_base, &two);

  EXPECT_EQ(nullptr, starlark_fn_int(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: int() can't convert non-string with explicit base", error_callback.messages[0]);
}

TEST(StarlarkInt, FromString) {
  auto test = [](std::string value, std::string_view expected) {
    Arena arena;
    context ctx(arena);
    error_handler error_callback;

    starlark_string str(value);
    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&str);

    auto* result = starlark_fn_int(nullptr, pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result) << value;
    EXPECT_EQ(result->str(), expected);
  };

  test("-0123", "-123");
  test("+0123", "123");
  test("-123", "-123");
  test("0", "0");
  test("123", "123");
  test("0123", "123");
}

TEST(StarlarkInt, FromStringWithBase) {
  auto test = [](std::string value, int base, std::string_view expected) {
    Arena arena;
    context ctx(arena);
    error_handler error_callback;

    std::string s_base("base");
    starlark_string str(value);
    starlark_integer ibase(base);
    starlark_bigint bbase(base);
    starlark_obj::pos_args_t pos_args1;
    starlark_obj::named_args_t named_args1;
    starlark_obj::pos_args_t pos_args2;
    starlark_obj::named_args_t named_args2;
    starlark_obj::pos_args_t pos_args3;
    starlark_obj::named_args_t named_args3;
    starlark_obj::pos_args_t pos_args4;
    starlark_obj::named_args_t named_args4;
    pos_args1.push_back(&str);
    pos_args1.push_back(&ibase);
    pos_args2.push_back(&str);
    pos_args2.push_back(&bbase);
    pos_args3.push_back(&str);
    named_args3.insert(s_base, &ibase);
    pos_args4.push_back(&str);
    named_args4.insert(s_base, &bbase);

    auto* result1 = starlark_fn_int(nullptr, pos_args1, named_args1, ctx, error_callback);
    auto* result2 = starlark_fn_int(nullptr, pos_args2, named_args2, ctx, error_callback);
    auto* result3 = starlark_fn_int(nullptr, pos_args3, named_args3, ctx, error_callback);
    auto* result4 = starlark_fn_int(nullptr, pos_args4, named_args4, ctx, error_callback);
    EXPECT_THAT(error_callback.messages, IsEmpty());
    ASSERT_NE(nullptr, result1) << value;
    ASSERT_NE(nullptr, result2);
    ASSERT_NE(nullptr, result3);
    ASSERT_NE(nullptr, result4);

    EXPECT_EQ(result1->str(), expected);
    EXPECT_EQ(result2->str(), expected);
    EXPECT_EQ(result3->str(), expected);
    EXPECT_EQ(result4->str(), expected);
  };

  test("123", 10, "123");
  test("123", 0, "123");
  test("0123", 10, "123");
  test("0x123", 16, "291");
  test("+0x123", 16, "291");
  test("-0x123", 16, "-291");
  test("123", 16, "291");
  test("0x123", 0, "291");
  test("0o123", 8, "83");
  test("-0o123", 8, "-83");
  test("+0o123", 8, "83");
  test("123", 8, "83");
  test("0o123", 0, "83");
  test("0b101", 2, "5");
  test("+0b101", 2, "5");
  test("-0b101", 2, "-5");
  test("101", 2, "5");
  test("0b101", 0, "5");
}

template <typename T>
void test_invalid_base(int base, bool positional) {
  starlark_string str("1"sv);
  T ibase(base);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  std::string s_base("base"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  if (positional) {
    pos_args.push_back(&ibase);
  } else {
    named_args.insert(s_base, &ibase);
  }

  EXPECT_EQ(nullptr, starlark_fn_int(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("int() base must be >= 2 and <= 36, or 0", error_callback.messages[0]);
}

TEST(StarlarkInt, FromStringInvalidBase) {
  test_invalid_base<starlark_integer>(-1, false);
  test_invalid_base<starlark_integer>(-1, true);
  test_invalid_base<starlark_bigint>(-1, false);
  test_invalid_base<starlark_bigint>(-1, true);
  test_invalid_base<starlark_integer>(1, false);
  test_invalid_base<starlark_integer>(1, true);
  test_invalid_base<starlark_bigint>(1, false);
  test_invalid_base<starlark_bigint>(1, true);
  test_invalid_base<starlark_integer>(37, false);
  test_invalid_base<starlark_integer>(37, true);
  test_invalid_base<starlark_bigint>(37, false);
  test_invalid_base<starlark_bigint>(37, true);
  test_invalid_base<starlark_integer>(100, false);
  test_invalid_base<starlark_integer>(100, true);
  test_invalid_base<starlark_bigint>(100, false);
  test_invalid_base<starlark_bigint>(100, true);
}

TEST(StarlarkInt, FromStringBaseNotIntAsPositionalArgument) {
  starlark_string str("1"sv);
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_int(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("'list' object cannot be interpreted as an integer", error_callback.messages[0]);
}

TEST(StarlarkInt, FromStringBaseNotIntAsNamedArgument) {
  starlark_string str("1"sv);
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  std::string s_base("base");

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  named_args.insert(s_base, &list);

  EXPECT_EQ(nullptr, starlark_fn_int(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("parameter 'base' cannot be interpreted as an integer (list)", error_callback.messages[0]);
}

TEST(StarlarkInt, FromStringNotAbleToParseInFull) {
  starlark_string str("123abc"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);

  EXPECT_EQ(nullptr, starlark_fn_int(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("invalid literal for int() with base 10: '123abc'", error_callback.messages[0]);
}

TEST(StarlarkInt, FromStringEmpty) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.empty_string());

  EXPECT_EQ(nullptr, starlark_fn_int(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("invalid literal for int() with base 10: ''", error_callback.messages[0]);
}

TEST(StarlarkInt, FromList) {
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_int(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("int() argument must be a string, int, bool or a real number, not 'list'", error_callback.messages[0]);
}

TEST(StarlarkInt, NoPosArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  auto* result = starlark_fn_int(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "0");
  ASSERT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkInt, NoPosArgsAndBase) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_integer base(10);
  std::string s_base("base");
  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_base, &base);

  auto* result = starlark_fn_int(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: int() missing string argument", error_callback.messages[0]);
}

TEST(StarlarkInt, BaseAsNamedAndPositionalArgument) {
  starlark_string str("123"sv);
  starlark_integer base(10);
  std::string s_base("base");
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  pos_args.push_back(&base);
  named_args.insert(s_base, &base);

  EXPECT_EQ(nullptr, starlark_fn_int(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: int() got multiple values for argument 'base'", error_callback.messages[0]);
}

TEST(StarlarkInt, ThreePosArgs) {
  starlark_string str(""sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  pos_args.push_back(&str);
  pos_args.push_back(&str);

  EXPECT_EQ(nullptr, starlark_fn_int(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("int() takes one or two argument (3 given)", error_callback.messages[0]);
}

TEST(StarlarkInt, NamedArguments) {
  std::string s_one("1");
  starlark_string str("1"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_one, &str);
  pos_args.push_back(&str);

  EXPECT_EQ(nullptr, starlark_fn_int(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("unknown named argument '1'", error_callback.messages[0]);
}

TEST(StarlarkLen, List) {
  starlark_integer one(1);
  starlark_list list1(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args1;
  starlark_obj::named_args_t named_args1;
  pos_args1.push_back(&list1);

  starlark_list list2(0);
  list2.append(&one, ctx, error_callback);
  starlark_obj::pos_args_t pos_args2;
  starlark_obj::named_args_t named_args2;
  pos_args2.push_back(&list2);

  EXPECT_EQ("1", starlark_fn_len(nullptr, pos_args2, named_args2, ctx, error_callback)->str());
  EXPECT_EQ("0", starlark_fn_len(nullptr, pos_args1, named_args1, ctx, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkLen, Integer) {
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_len(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("object of type 'int' has no len()", error_callback.messages[0]);
}

TEST(StarlarkLen, NoPosArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  EXPECT_EQ(nullptr, starlark_fn_len(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("len() takes exactly one argument (0 given)", error_callback.messages[0]);
}

TEST(StarlarkLen, MultiplePosArgs) {
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&list);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_len(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("len() takes exactly one argument (2 given)", error_callback.messages[0]);
}

TEST(StarlarkLen, NamedArguments) {
  std::string s_one("1");
  starlark_integer one(1);
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_one, &one);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_len(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("len() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkList, Tuple) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_tuple tuple1(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args1;
  starlark_obj::named_args_t named_args1;
  pos_args1.push_back(&tuple1);

  starlark_tuple tuple2(0);
  tuple2.add(&zero);
  tuple2.add(&one);
  starlark_obj::pos_args_t pos_args2;
  starlark_obj::named_args_t named_args2;
  pos_args2.push_back(&tuple2);

  EXPECT_EQ("[]", starlark_fn_list(nullptr, pos_args1, named_args1, ctx, error_callback)->str());
  EXPECT_EQ("[0, 1]", starlark_fn_list(nullptr, pos_args2, named_args2, ctx, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkList, Integer) {
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_list(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("'int' object is not iterable", error_callback.messages[0]);
}

TEST(StarlarkList, NoPosArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  EXPECT_EQ("[]", starlark_fn_list(nullptr, pos_args, named_args, ctx, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkList, MultiplePosArgs) {
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&list);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_list(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("list expected at most 1 argument, got 2", error_callback.messages[0]);
}

TEST(StarlarkList, NamedArguments) {
  std::string s_one("1");
  starlark_integer one(1);
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_one, &one);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_list(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("list() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkMax, NoPosArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  EXPECT_EQ(nullptr, starlark_fn_max(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("max expected at least 1 argument, got 0", error_callback.messages[0]);
}

TEST(StarlarkMax, OnePosArgsEmpty) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_list list(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&list);

  auto* result = starlark_fn_max(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: max() iterable argument is empty");
}

TEST(StarlarkMax, OnePosArgOneElement) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_list list(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  list.append(ctx.one(), ctx, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_max(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(starlark_types::int_t, result->type());
  EXPECT_EQ(result->as_int64(), 1);
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkMax, OnePosArgManyElements) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_list list(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  list.append(ctx.zero(), ctx, error_callback);
  list.append(ctx.one(), ctx, error_callback);
  list.append(ctx.minus_one(), ctx, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_max(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(starlark_types::int_t, result->type());
  EXPECT_EQ(result->as_int64(), 1);
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkMax, OnePosArgsNotComparable) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_list list(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  list.append(ctx.one(), ctx, error_callback);
  list.append(ctx.empty_string(), ctx, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_max(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'<' not supported between instances of 'int' and 'string'");
}

TEST(StarlarkMax, OnePosArgsNotIterable) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.one());

  auto* result = starlark_fn_max(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'int' object is not iterable");
}

TEST(StarlarkMax, OnePosArgsWithKey) {
  std::string s_key("key");
  starlark_list list(0);
  starlark_string one("one"sv);
  starlark_string two("two"sv);
  starlark_string three("three"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  list.append(&one, ctx, error_callback);
  list.append(&two, ctx, error_callback);
  list.append(&three, ctx, error_callback);
  pos_args.push_back(&list);
  named_args.insert(s_key, create_function(ctx, nullptr, starlark::runtime::starlark_fn_len, "len"));

  auto* result = starlark_fn_max(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(starlark_types::string_t, result->type());
  EXPECT_EQ(result->as_string(), "three");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkMax, OnePosArgsWithKeyAsNone) {
  std::string s_key("key");
  starlark_list list(0);
  starlark_string one("one"sv);
  starlark_string two("two"sv);
  starlark_string three("three"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  list.append(&one, ctx, error_callback);
  list.append(&two, ctx, error_callback);
  list.append(&three, ctx, error_callback);
  pos_args.push_back(&list);
  named_args.insert(s_key, ctx.none_value());

  auto* result = starlark_fn_max(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(starlark_types::string_t, result->type());
  EXPECT_EQ(result->as_string(), "two");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkMax, OnePosArgsWithKeyErrorInKeyCall_1) {
  std::string s_key("key");
  starlark_list list(0);
  starlark_string two("two"sv);
  starlark_string three("three"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  list.append(ctx.one(), ctx, error_callback);
  list.append(&two, ctx, error_callback);
  list.append(&three, ctx, error_callback);
  pos_args.push_back(&list);
  named_args.insert(s_key, create_function(ctx, nullptr, starlark::runtime::starlark_fn_len, "len"));

  auto* result = starlark_fn_max(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "object of type 'int' has no len()");
}

TEST(StarlarkMax, OnePosArgsWithKeyErrorInKeyCall_2) {
  std::string s_key("key");
  starlark_list list(0);
  starlark_string two("two"sv);
  starlark_string three("three"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  list.append(&two, ctx, error_callback);
  list.append(ctx.one(), ctx, error_callback);
  list.append(&three, ctx, error_callback);
  pos_args.push_back(&list);
  named_args.insert(s_key, create_function(ctx, nullptr, starlark::runtime::starlark_fn_len, "len"));

  auto* result = starlark_fn_max(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "object of type 'int' has no len()");
}

TEST(StarlarkMax, OnePosArgsResultOfKeyAreNotComparable) {
  std::string s_key("key");
  starlark_tuple tuple(0);
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  tuple.add(&list);
  tuple.add(&list);
  pos_args.push_back(&tuple);
  named_args.insert(s_key, create_function(ctx, nullptr, starlark::runtime::starlark_fn_set, "set"));

  auto* result = starlark_fn_max(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'<' not supported between instances of 'set' and 'set'");
}

TEST(StarlarkMax, TwoPosArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.one());

  auto* result = starlark_fn_max(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(starlark_types::int_t, result->type());
  EXPECT_EQ(result->as_int64(), 1);
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkMax, TwoPosArgsNotComparable) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.false_value());

  auto* result = starlark_fn_max(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'<' not supported between instances of 'int' and 'bool'");
}

TEST(StarlarkMax, ManyPosArgsWithKey) {
  std::string s_key("key");
  starlark_string one("one"sv);
  starlark_string two("two"sv);
  starlark_string three("three"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&one);
  pos_args.push_back(&two);
  pos_args.push_back(&three);
  named_args.insert(s_key, create_function(ctx, nullptr, starlark::runtime::starlark_fn_len, "len"));

  auto* result = starlark_fn_max(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(starlark_types::string_t, result->type());
  EXPECT_EQ(result->as_string(), "three");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkMax, ManyPosArgsWithKeyAsNone) {
  std::string s_key("key");
  starlark_string one("one"sv);
  starlark_string two("two"sv);
  starlark_string three("three"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&one);
  pos_args.push_back(&two);
  pos_args.push_back(&three);
  named_args.insert(s_key, ctx.none_value());

  auto* result = starlark_fn_max(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(starlark_types::string_t, result->type());
  EXPECT_EQ(result->as_string(), "two");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkMax, TwoPosArgsWithKeyErrorInKeyCall_1) {
  std::string s_key("key");
  starlark_string two("two"sv);
  starlark_string three("three"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.one());
  pos_args.push_back(&two);
  pos_args.push_back(&three);
  named_args.insert(s_key, create_function(ctx, nullptr, starlark::runtime::starlark_fn_len, "len"));

  auto* result = starlark_fn_max(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "object of type 'int' has no len()");
}

TEST(StarlarkMax, TwoPosArgsWithKeyErrorInKeyCall_2) {
  std::string s_key("key");
  starlark_string two("two"sv);
  starlark_string three("three"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&two);
  pos_args.push_back(ctx.one());
  pos_args.push_back(&three);
  named_args.insert(s_key, create_function(ctx, nullptr, starlark::runtime::starlark_fn_len, "len"));

  auto* result = starlark_fn_max(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "object of type 'int' has no len()");
}

TEST(StarlarkMax, TwoPosArgsResultOfKeyAreNotComparable) {
  std::string s_key("key");
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&list);
  pos_args.push_back(&list);
  named_args.insert(s_key, create_function(ctx, nullptr, starlark::runtime::starlark_fn_set, "set"));

  auto* result = starlark_fn_max(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'<' not supported between instances of 'set' and 'set'");
}

TEST(StarlarkMax, UnknownNamedArguments) {
  std::string s_one("1");
  starlark_integer one(1);
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_one, &one);
  pos_args.push_back(&list);
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.one());

  EXPECT_EQ(nullptr, starlark_fn_max(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("unknown named argument '1'", error_callback.messages[0]);
}

TEST(StarlarkMin, NoPosArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  EXPECT_EQ(nullptr, starlark_fn_min(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("min expected at least 1 argument, got 0", error_callback.messages[0]);
}

TEST(StarlarkMin, OnePosArgsEmpty) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_list list(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&list);

  auto* result = starlark_fn_min(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: min() iterable argument is empty");
}

TEST(StarlarkMin, OnePosArgOneElement) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_list list(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  list.append(ctx.one(), ctx, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_min(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(starlark_types::int_t, result->type());
  EXPECT_EQ(result->as_int64(), 1);
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkMin, OnePosArgManyElements) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_list list(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  list.append(ctx.zero(), ctx, error_callback);
  list.append(ctx.minus_one(), ctx, error_callback);
  list.append(ctx.one(), ctx, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_min(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(starlark_types::int_t, result->type());
  EXPECT_EQ(result->as_int64(), -1);
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkMin, OnePosArgsNotComparable) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_list list(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  list.append(ctx.one(), ctx, error_callback);
  list.append(ctx.empty_string(), ctx, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_min(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'<' not supported between instances of 'int' and 'string'");
}

TEST(StarlarkMin, OnePosArgsNotIterable) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.one());

  auto* result = starlark_fn_min(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'int' object is not iterable");
}

TEST(StarlarkMin, OnePosArgsWithKey) {
  std::string s_key("key");
  starlark_list list(0);
  starlark_string one("one"sv);
  starlark_string two("two"sv);
  starlark_string three("three"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  list.append(&three, ctx, error_callback);
  list.append(&one, ctx, error_callback);
  list.append(&two, ctx, error_callback);
  pos_args.push_back(&list);
  named_args.insert(s_key, create_function(ctx, nullptr, starlark::runtime::starlark_fn_len, "len"));

  auto* result = starlark_fn_min(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(starlark_types::string_t, result->type());
  EXPECT_EQ(result->as_string(), "one");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkMin, OnePosArgsWithKeyAsNone) {
  std::string s_key("key");
  starlark_list list(0);
  starlark_string one("one"sv);
  starlark_string two("two"sv);
  starlark_string three("three"sv);
  starlark_string four("four"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  list.append(&three, ctx, error_callback);
  list.append(&four, ctx, error_callback);
  list.append(&one, ctx, error_callback);
  list.append(&two, ctx, error_callback);
  pos_args.push_back(&list);
  named_args.insert(s_key, ctx.none_value());

  auto* result = starlark_fn_min(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(starlark_types::string_t, result->type());
  EXPECT_EQ(result->as_string(), "four");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkMin, OnePosArgsWithKeyErrorInKeyCall_1) {
  std::string s_key("key");
  starlark_list list(0);
  starlark_string two("two"sv);
  starlark_string three("three"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  list.append(ctx.one(), ctx, error_callback);
  list.append(&two, ctx, error_callback);
  list.append(&three, ctx, error_callback);
  pos_args.push_back(&list);
  named_args.insert(s_key, create_function(ctx, nullptr, starlark::runtime::starlark_fn_len, "len"));

  auto* result = starlark_fn_min(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "object of type 'int' has no len()");
}

TEST(StarlarkMin, OnePosArgsWithKeyErrorInKeyCall_2) {
  std::string s_key("key");
  starlark_list list(0);
  starlark_string two("two"sv);
  starlark_string three("three"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  list.append(&two, ctx, error_callback);
  list.append(ctx.one(), ctx, error_callback);
  list.append(&three, ctx, error_callback);
  pos_args.push_back(&list);
  named_args.insert(s_key, create_function(ctx, nullptr, starlark::runtime::starlark_fn_len, "len"));

  auto* result = starlark_fn_min(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "object of type 'int' has no len()");
}

TEST(StarlarkMin, OnePosArgsResultOfKeyAreNotComparable) {
  std::string s_key("key");
  starlark_tuple tuple(0);
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  tuple.add(&list);
  tuple.add(&list);
  pos_args.push_back(&tuple);
  named_args.insert(s_key, create_function(ctx, nullptr, starlark::runtime::starlark_fn_set, "set"));

  auto* result = starlark_fn_min(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'<' not supported between instances of 'set' and 'set'");
}

TEST(StarlarkMin, TwoPosArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.one());
  pos_args.push_back(ctx.minus_one());

  auto* result = starlark_fn_min(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(starlark_types::int_t, result->type());
  EXPECT_EQ(result->as_int64(), -1);
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkMin, TwoPosArgsNotComparable) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.false_value());

  auto* result = starlark_fn_min(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'<' not supported between instances of 'int' and 'bool'");
}

TEST(StarlarkMin, ManyPosArgsWithKey) {
  std::string s_key("key");
  starlark_string one("one"sv);
  starlark_string two("two"sv);
  starlark_string three("three"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&three);
  pos_args.push_back(&one);
  pos_args.push_back(&two);
  named_args.insert(s_key, create_function(ctx, nullptr, starlark::runtime::starlark_fn_len, "len"));

  auto* result = starlark_fn_min(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(starlark_types::string_t, result->type());
  EXPECT_EQ(result->as_string(), "one");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkMin, ManyPosArgsWithKeyAsNone) {
  std::string s_key("key");
  starlark_string one("one"sv);
  starlark_string two("two"sv);
  starlark_string three("three"sv);
  starlark_string four("four"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&three);
  pos_args.push_back(&four);
  pos_args.push_back(&one);
  pos_args.push_back(&two);
  named_args.insert(s_key, ctx.none_value());

  auto* result = starlark_fn_min(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(starlark_types::string_t, result->type());
  EXPECT_EQ(result->as_string(), "four");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkMin, TwoPosArgsWithKeyErrorInKeyCall_1) {
  std::string s_key("key");
  starlark_string two("two"sv);
  starlark_string three("three"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.one());
  pos_args.push_back(&two);
  pos_args.push_back(&three);
  named_args.insert(s_key, create_function(ctx, nullptr, starlark::runtime::starlark_fn_len, "len"));

  auto* result = starlark_fn_min(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "object of type 'int' has no len()");
}

TEST(StarlarkMin, TwoPosArgsWithKeyErrorInKeyCall_2) {
  std::string s_key("key");
  starlark_string two("two"sv);
  starlark_string three("three"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&two);
  pos_args.push_back(ctx.one());
  pos_args.push_back(&three);
  named_args.insert(s_key, create_function(ctx, nullptr, starlark::runtime::starlark_fn_len, "len"));

  auto* result = starlark_fn_min(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "object of type 'int' has no len()");
}

TEST(StarlarkMin, TwoPosArgsResultOfKeyAreNotComparable) {
  std::string s_key("key");
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&list);
  pos_args.push_back(&list);
  named_args.insert(s_key, create_function(ctx, nullptr, starlark::runtime::starlark_fn_set, "set"));

  auto* result = starlark_fn_min(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'<' not supported between instances of 'set' and 'set'");
}

TEST(StarlarkMin, UnknownNamedArguments) {
  std::string s_one("1");
  starlark_integer one(1);
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_one, &one);
  pos_args.push_back(&list);
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.one());

  EXPECT_EQ(nullptr, starlark_fn_min(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("unknown named argument '1'", error_callback.messages[0]);
}

TEST(StarlarkOrd, NoPosArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  EXPECT_EQ(nullptr, starlark_fn_ord(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("ord() takes exactly one argument (0 given)", error_callback.messages[0]);
}

TEST(StarlarkOrd, FromString) {
  starlark_string str("😃"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);

  EXPECT_EQ("128515", starlark_fn_ord(nullptr, pos_args, named_args, ctx, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkOrd, FromBytes) {
  starlark_bytes bytes("\xFF"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&bytes);

  EXPECT_EQ("255", starlark_fn_ord(nullptr, pos_args, named_args, ctx, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkOrd, ShortString) {
  starlark_string str(""sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);

  EXPECT_EQ(nullptr, starlark_fn_ord(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("ord() expected a character, but string of length 0 found", error_callback.messages[0]);
}

TEST(StarlarkOrd, LongString) {
  starlark_string str("ab"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);

  EXPECT_EQ(nullptr, starlark_fn_ord(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("ord() expected a character, but string of length 2 found", error_callback.messages[0]);
}

TEST(StarlarkOrd, ShortBytes) {
  starlark_bytes bytes(""sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&bytes);

  EXPECT_EQ(nullptr, starlark_fn_ord(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("ord() expected a character, but bytes of length 0 found", error_callback.messages[0]);
}

TEST(StarlarkOrd, LongBytes) {
  starlark_bytes bytes("ab"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&bytes);

  EXPECT_EQ(nullptr, starlark_fn_ord(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("ord() expected a character, but bytes of length 2 found", error_callback.messages[0]);
}

TEST(StarlarkOrd, List) {
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_ord(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("ord() expected bytes of length 1 or string with one character, but 'list' found", error_callback.messages[0]);
}

TEST(StarlarkOrd, MultiplePosArgs) {
  starlark_bytes bytes("\xFF"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&bytes);
  pos_args.push_back(&bytes);

  EXPECT_EQ(nullptr, starlark_fn_ord(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("ord() takes exactly one argument (2 given)", error_callback.messages[0]);
}

TEST(StarlarkOrd, NamedArguments) {
  std::string s_one("1");
  starlark_integer one(1);
  starlark_bytes bytes("\xFF"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_one, &one);
  pos_args.push_back(&bytes);

  EXPECT_EQ(nullptr, starlark_fn_ord(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("ord() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkPrint, NoArguments) {
  std::basic_ostringstream<char> out;
  Arena arena;
  context ctx(arena, runtime_options{.out = out});
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  auto* result = starlark_fn_print(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);
  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(out.str(), "\n");
}

TEST(StarlarkPrint, OneArguments) {
  std::basic_ostringstream<char> out;
  Arena arena;
  context ctx(arena, runtime_options{.out = out});
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());

  auto* result = starlark_fn_print(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);
  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(out.str(), "0\n");
}

TEST(StarlarkPrint, TwoArguments) {
  std::basic_ostringstream<char> out;
  Arena arena;
  context ctx(arena, runtime_options{.out = out});
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.one());

  auto* result = starlark_fn_print(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);
  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(out.str(), "0 1\n");
}

TEST(StarlarkPrint, NamedArguments) {
  std::string s_sep("sep");
  starlark_string sep(", "sv);
  std::basic_ostringstream<char> out;
  Arena arena;
  context ctx(arena, runtime_options{.out = out});
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_sep, &sep);
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.one());

  auto* result = starlark_fn_print(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);
  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(out.str(), "0, 1\n");
}

TEST(StarlarkPrint, NamedArgumentsSepAsNone) {
  std::string s_sep("sep");
  std::basic_ostringstream<char> out;
  Arena arena;
  context ctx(arena, runtime_options{.out = out});
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_sep, ctx.none_value());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.one());

  auto* result = starlark_fn_print(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "parameter 'sep' cannot be interpreted as an string (NoneType)");
}

TEST(StarlarkPrint, NamedArgumentsEnd) {
  std::string s_end("end");
  starlark_string end(", "sv);
  std::basic_ostringstream<char> out;
  Arena arena;
  context ctx(arena, runtime_options{.out = out});
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_end, &end);
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.one());

  auto* result = starlark_fn_print(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "unknown named argument 'end'");
}

TEST(StarlarkRange, OneArgument) {
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&one);

  auto* result = starlark_fn_range(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->str(), "range(1)");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkRange, OneArgumentBigInt) {
  starlark_bigint big(100);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&big);

  auto* result = starlark_fn_range(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->str(), "range(100)");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkRange, OneArgumentBigIntTooBig) {
  starlark_bigint big(number::one() << 63);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&big);

  auto* result = starlark_fn_range(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "int too large to convert to int64");
}

TEST(StarlarkRange, OneInvalidArgument) {
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&list);

  auto* result = starlark_fn_range(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'list' object cannot be interpreted as an integer");
}

TEST(StarlarkRange, TwoArguments) {
  starlark_integer one(1);
  starlark_integer ten(10);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&one);
  pos_args.push_back(&ten);

  auto* result = starlark_fn_range(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->str(), "range(1, 10)");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkRange, TwoInvalidArguments) {
  starlark_integer one(1);
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&one);
  pos_args.push_back(&list);

  auto* result = starlark_fn_range(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'list' object cannot be interpreted as an integer");
}

TEST(StarlarkRange, TwoArgumentsOverflow) {
  starlark_integer minus_one(-1);
  starlark_integer max_int64(std::numeric_limits<int64_t>::max());
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&minus_one);
  pos_args.push_back(&max_int64);

  auto* result = starlark_fn_range(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "int too large to convert to int64");
}

TEST(StarlarkRange, ThreeArguments) {
  starlark_integer one(1);
  starlark_integer ten(10);
  starlark_integer minus_one(-1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&one);
  pos_args.push_back(&ten);
  pos_args.push_back(&minus_one);

  auto* result = starlark_fn_range(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->str(), "range(1, 10, -1)");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkRange, ThreeInvalidArguments) {
  starlark_integer one(1);
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&one);
  pos_args.push_back(&one);
  pos_args.push_back(&list);

  auto* result = starlark_fn_range(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'list' object cannot be interpreted as an integer");
}

TEST(StarlarkRange, ZeroStep) {
  starlark_integer one(1);
  starlark_integer ten(10);
  starlark_integer zero(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&one);
  pos_args.push_back(&ten);
  pos_args.push_back(&zero);

  auto* result = starlark_fn_range(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "range() arg 3 must not be zero");
}

TEST(StarlarkRange, TooFewPosArguments) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  EXPECT_EQ(nullptr, starlark_fn_range(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("range expected at least 1 argument, got 0", error_callback.messages[0]);
}

TEST(StarlarkRange, TooManyPosArguments) {
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&one);
  pos_args.push_back(&one);
  pos_args.push_back(&one);
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_range(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("range expected at most 3 argument, got 4", error_callback.messages[0]);
}

TEST(StarlarkRange, NamedArguments) {
  std::string s_one("1");
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_one, &one);
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_range(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("range() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkRepr, String) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);

  auto* result = starlark_fn_repr(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->as_string(), "\"abc\"");
}

TEST(StarlarkRepr, TooFewPosArguments) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  EXPECT_EQ(nullptr, starlark_fn_repr(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("repr() takes exactly one argument (0 given)", error_callback.messages[0]);
}

TEST(StarlarkRepr, TooManyPosArguments) {
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&one);
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_repr(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("repr() takes exactly one argument (2 given)", error_callback.messages[0]);
}

TEST(StarlarkRepr, NamedArguments) {
  std::string s_one("1");
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_one, &one);
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_repr(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("repr() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkReversed, Tuple) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_tuple tuple1(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args1;
  starlark_obj::named_args_t named_args1;
  pos_args1.push_back(&tuple1);

  starlark_tuple tuple2(0);
  tuple2.add(&zero);
  tuple2.add(&one);
  starlark_obj::pos_args_t pos_args2;
  starlark_obj::named_args_t named_args2;
  pos_args2.push_back(&tuple2);

  EXPECT_EQ("[]", starlark_fn_reversed(nullptr, pos_args1, named_args1, ctx, error_callback)->str());
  EXPECT_EQ("[1, 0]", starlark_fn_reversed(nullptr, pos_args2, named_args2, ctx, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkReversed, Integer) {
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_reversed(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("'int' object is not iterable", error_callback.messages[0]);
}

TEST(StarlarkReversed, NoPosArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  EXPECT_EQ(nullptr, starlark_fn_reversed(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("reversed() takes exactly one argument (0 given)", error_callback.messages[0]);
}

TEST(StarlarkReversed, MultiplePosArgs) {
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&list);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_reversed(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("reversed() takes exactly one argument (2 given)", error_callback.messages[0]);
}

TEST(StarlarkReversed, NamedArguments) {
  std::string s_one("1");
  starlark_integer one(1);
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_one, &one);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_reversed(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("reversed() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkSet, NoArguments) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  auto* result = starlark_fn_set(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "set()");
}

TEST(StarlarkSet, OneArguments) {
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_list list(3);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  list.append(&one, ctx, error_callback);
  list.append(&two, ctx, error_callback);
  list.append(&one, ctx, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_set(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "set([1, 2])");
}

TEST(StarlarkSet, ElementNotHashable) {
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_list list(3);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  list.append(&one, ctx, error_callback);
  list.append(&two, ctx, error_callback);
  list.append(&list, ctx, error_callback);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_set(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("cannot use 'list' as a set element (unhashable type: 'list')", error_callback.messages[0]);
}

TEST(StarlarkSet, String) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);

  EXPECT_EQ(nullptr, starlark_fn_set(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("'string' object is not iterable", error_callback.messages[0]);
}

TEST(StarlarkSet, TooManyPosArguments) {
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&list);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_set(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("set expected at most 1 argument, got 2", error_callback.messages[0]);
}

TEST(StarlarkSet, NamedArguments) {
  std::string s_one("1");
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_one, &list);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_set(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("set() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkSorted, NoArguments) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  EXPECT_EQ(nullptr, starlark_fn_sorted(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("sorted() takes exactly one argument (0 given)", error_callback.messages[0]);
}

TEST(StarlarkSorted, OneArgumentsEmptyTuple) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_tuple tuple(0);
  starlark_obj::pos_args_t pos_args;
  pos_args.push_back(&tuple);
  starlark_obj::named_args_t named_args;
  auto* result = starlark_fn_sorted(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->str(), "[]");
}

TEST(StarlarkSorted, OneArgumentsElementIsEditableAfterSorting) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_list list(0);
  starlark_obj::pos_args_t pos_args;
  pos_args.push_back(&list);
  starlark_obj::named_args_t named_args;
  auto* result = starlark_fn_sorted(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->str(), "[]");
  list.append(ctx.one(), ctx, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkSorted, OneArguments) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_list list(0);
  starlark_string one("one"sv);
  starlark_string two("two"sv);
  starlark_string three("three"sv);
  starlark_string four("four"sv);
  starlark_string five("five"sv);
  starlark_string six("six"sv);
  starlark_string seven("seven"sv);
  list.append(&one, ctx, error_callback);
  list.append(&two, ctx, error_callback);
  list.append(&three, ctx, error_callback);
  list.append(&four, ctx, error_callback);
  list.append(&five, ctx, error_callback);
  list.append(&six, ctx, error_callback);
  list.append(&seven, ctx, error_callback);
  std::string s_key("key");
  std::string s_reverse("reverse");

  starlark_obj::pos_args_t pos_args;
  pos_args.push_back(&list);
  {
    starlark_obj::named_args_t named_args;
    auto* result = starlark_fn_sorted(nullptr, pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->str(), "[\"five\", \"four\", \"one\", \"seven\", \"six\", \"three\", \"two\"]");
    EXPECT_EQ(list.str(), "[\"one\", \"two\", \"three\", \"four\", \"five\", \"six\", \"seven\"]");
  }
  {
    starlark_obj::named_args_t named_args;
    named_args.insert(s_key, ctx.none_value());
    auto* result = starlark_fn_sorted(nullptr, pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->str(), "[\"five\", \"four\", \"one\", \"seven\", \"six\", \"three\", \"two\"]");
    EXPECT_EQ(list.str(), "[\"one\", \"two\", \"three\", \"four\", \"five\", \"six\", \"seven\"]");
  }
  {
    starlark_obj::named_args_t named_args;
    named_args.insert(s_key, create_function(ctx, nullptr, starlark::runtime::starlark_fn_len, "len"));
    auto* result = starlark_fn_sorted(nullptr, pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->str(), "[\"one\", \"two\", \"six\", \"four\", \"five\", \"three\", \"seven\"]");
    EXPECT_EQ(list.str(), "[\"one\", \"two\", \"three\", \"four\", \"five\", \"six\", \"seven\"]");
  }
  {
    starlark_obj::named_args_t named_args;
    named_args.insert(s_reverse, ctx.false_value());
    auto* result = starlark_fn_sorted(nullptr, pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->str(), "[\"five\", \"four\", \"one\", \"seven\", \"six\", \"three\", \"two\"]");
    EXPECT_EQ(list.str(), "[\"one\", \"two\", \"three\", \"four\", \"five\", \"six\", \"seven\"]");
  }
  {
    starlark_obj::named_args_t named_args;
    named_args.insert(s_reverse, ctx.false_value());
    named_args.insert(s_key, ctx.none_value());
    auto* result = starlark_fn_sorted(nullptr, pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->str(), "[\"five\", \"four\", \"one\", \"seven\", \"six\", \"three\", \"two\"]");
    EXPECT_EQ(list.str(), "[\"one\", \"two\", \"three\", \"four\", \"five\", \"six\", \"seven\"]");
  }
  {
    starlark_obj::named_args_t named_args;
    named_args.insert(s_reverse, ctx.false_value());
    named_args.insert(s_key, create_function(ctx, nullptr, starlark::runtime::starlark_fn_len, "len"));
    auto* result = starlark_fn_sorted(nullptr, pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->str(), "[\"one\", \"two\", \"six\", \"four\", \"five\", \"three\", \"seven\"]");
    EXPECT_EQ(list.str(), "[\"one\", \"two\", \"three\", \"four\", \"five\", \"six\", \"seven\"]");
  }
  {
    starlark_obj::named_args_t named_args;
    named_args.insert(s_reverse, ctx.true_value());
    auto* result = starlark_fn_sorted(nullptr, pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->str(), "[\"two\", \"three\", \"six\", \"seven\", \"one\", \"four\", \"five\"]");
    EXPECT_EQ(list.str(), "[\"one\", \"two\", \"three\", \"four\", \"five\", \"six\", \"seven\"]");
  }
  {
    starlark_obj::named_args_t named_args;
    named_args.insert(s_reverse, ctx.true_value());
    named_args.insert(s_key, ctx.none_value());
    auto* result = starlark_fn_sorted(nullptr, pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->str(), "[\"two\", \"three\", \"six\", \"seven\", \"one\", \"four\", \"five\"]");
    EXPECT_EQ(list.str(), "[\"one\", \"two\", \"three\", \"four\", \"five\", \"six\", \"seven\"]");
  }
  {
    starlark_obj::named_args_t named_args;
    named_args.insert(s_reverse, ctx.true_value());
    named_args.insert(s_key, create_function(ctx, nullptr, starlark::runtime::starlark_fn_len, "len"));
    auto* result = starlark_fn_sorted(nullptr, pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->str(), "[\"seven\", \"three\", \"five\", \"four\", \"six\", \"two\", \"one\"]");
    EXPECT_EQ(list.str(), "[\"one\", \"two\", \"three\", \"four\", \"five\", \"six\", \"seven\"]");
  }

  ASSERT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkSorted, OneArgumentsReverseNone) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_tuple tuple(0);
  std::string s_reverse("reverse");

  starlark_obj::pos_args_t pos_args;
  pos_args.push_back(&tuple);
  starlark_obj::named_args_t named_args;
  named_args.insert(s_reverse, ctx.none_value());

  EXPECT_EQ(nullptr, starlark_fn_sorted(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: sorted() argument reverse must be bool, not NoneType", error_callback.messages[0]);
}

TEST(StarlarkSorted, OneArgumentsUnknownNamedArgument) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_tuple tuple(0);
  std::string s_foo("foo");

  starlark_obj::pos_args_t pos_args;
  pos_args.push_back(&tuple);
  starlark_obj::named_args_t named_args;
  named_args.insert(s_foo, ctx.none_value());

  EXPECT_EQ(nullptr, starlark_fn_sorted(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("unknown named argument 'foo'", error_callback.messages[0]);
}

TEST(StarlarkSorted, OneArgumentsNotIterable) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  pos_args.push_back(ctx.zero());
  starlark_obj::named_args_t named_args;

  EXPECT_EQ(nullptr, starlark_fn_sorted(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("'int' object is not iterable", error_callback.messages[0]);
}

TEST(StarlarkSorted, OneArgumentsNotComparable) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_tuple tuple(0);

  tuple.add(ctx.zero());
  tuple.add(ctx.false_value());
  starlark_obj::pos_args_t pos_args;
  pos_args.push_back(&tuple);
  starlark_obj::named_args_t named_args;

  EXPECT_EQ(nullptr, starlark_fn_sorted(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("'<' not supported between instances of 'bool' and 'int'", error_callback.messages[0]);
}

TEST(StarlarkSorted, OneArgumentsManyElementsNotComparable) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_tuple tuple(0);

  tuple.add(ctx.zero());
  tuple.add(ctx.false_value());
  tuple.add(ctx.one());
  tuple.add(ctx.true_value());
  starlark_obj::pos_args_t pos_args;
  pos_args.push_back(&tuple);
  starlark_obj::named_args_t named_args;

  EXPECT_EQ(nullptr, starlark_fn_sorted(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("'<' not supported between instances of 'bool' and 'int'", error_callback.messages[0]);
}

TEST(StarlarkSorted, TwoArguments) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_list list(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&list);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_sorted(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("sorted() takes exactly one argument (2 given)", error_callback.messages[0]);
}

TEST(StarlarkStr, String) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);

  auto* result = starlark_fn_str(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->as_string(), "abc");
}

TEST(StarlarkStr, NoPosArguments) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  auto* result = starlark_fn_str(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->type(), starlark_types::string_t);
  EXPECT_EQ(result->str(), "");
  ASSERT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkStr, TooManyPosArguments) {
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&one);
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_str(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("str expected at most 1 argument, got 2", error_callback.messages[0]);
}

TEST(StarlarkStr, NamedArguments) {
  std::string s_object("object");
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_object, &one);

  auto* result = starlark_fn_str(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(result, nullptr) << error_callback.messages[0];
  EXPECT_EQ(result->type(), starlark_types::string_t);
  EXPECT_EQ(result->str(), "1");
  ASSERT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkStr, ObjectAsNamedAndPositionalArgument) {
  starlark_string str("123"sv);
  std::string s_object("object");
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  named_args.insert(s_object, &str);

  EXPECT_EQ(nullptr, starlark_fn_str(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: str() got multiple values for argument 'object'", error_callback.messages[0]);
}

TEST(StarlarkStr, UnknownNamedArguments) {
  std::string s_one("1");
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_one, &one);
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_str(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("unknown named argument '1'", error_callback.messages[0]);
}

TEST(StarlarkTuple, NoArguments) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  auto* result = starlark_fn_tuple(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "()");
}

TEST(StarlarkTuple, OneArguments) {
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_list list(3);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  list.append(&one, ctx, error_callback);
  list.append(&two, ctx, error_callback);
  list.append(&one, ctx, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_tuple(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "(1, 2, 1)");
}

TEST(StarlarkTuple, String) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);

  EXPECT_EQ(nullptr, starlark_fn_tuple(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("'string' object is not iterable", error_callback.messages[0]);
}

TEST(StarlarkTuple, TooManyPosArguments) {
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&list);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_tuple(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("tuple expected at most 1 argument, got 2", error_callback.messages[0]);
}

TEST(StarlarkTuple, NamedArguments) {
  std::string s_one("1");
  starlark_list list(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_one, &list);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_tuple(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("tuple() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkType, String) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);

  auto* result = starlark_fn_type(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->as_string(), "string");
}

TEST(StarlarkType, TooFewPosArguments) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  EXPECT_EQ(nullptr, starlark_fn_type(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("type() takes exactly one argument (0 given)", error_callback.messages[0]);
}

TEST(StarlarkType, TooManyPosArguments) {
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&one);
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_type(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("type() takes exactly one argument (2 given)", error_callback.messages[0]);
}

TEST(StarlarkType, NamedArguments) {
  std::string s_one("1");
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_one, &one);
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_type(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("type() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkZip, NoArguments) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  auto* result = starlark_fn_zip(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "[]");
}

TEST(StarlarkZip, OneArgument) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_range range(0, 3, 1);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&range);

  auto* result = starlark_fn_zip(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "[(0,), (1,), (2,)]");
}

TEST(StarlarkZip, TwoArgument) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_range range1(0, 3, 1);
  starlark_range range2(100, 200, 1);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&range1);
  pos_args.push_back(&range2);

  auto* result = starlark_fn_zip(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "[(0, 100), (1, 101), (2, 102)]");
}

TEST(StarlarkZip, ThreeArgument) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_range range1(0, 3, 1);
  starlark_range range2(100, 200, 1);
  starlark_range range3(1000, 1002, 1);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&range1);
  pos_args.push_back(&range2);
  pos_args.push_back(&range3);

  auto* result = starlark_fn_zip(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "[(0, 100, 1000), (1, 101, 1001)]");
}

TEST(StarlarkZip, NonIterable) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);

  EXPECT_EQ(nullptr, starlark_fn_zip(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("'string' object is not iterable", error_callback.messages[0]);
}

TEST(StarlarkZip, NamedArguments) {
  std::string s_one("1");
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert(s_one, &one);
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_zip(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("zip() takes no keyword arguments", error_callback.messages[0]);
}

}  // namespace
