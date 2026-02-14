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
using ::starlark::runtime::context;
using ::starlark::runtime::create_float;
using ::starlark::runtime::create_integer;
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
using ::starlark::runtime::starlark_string;
using ::starlark::runtime::starlark_tuple;
using ::starlark::runtime::starlark_types;
using ::starlark::testing::error_handler;
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
  EXPECT_EQ("function", starlark_function().type());
  EXPECT_EQ("builtin_function_or_method", starlark_built_in_function(nullptr, base_fn, "fn_name").type());
}

TEST(StarlarkFunction, Primitve) {
  EXPECT_FALSE(starlark_function().primitive());
  EXPECT_FALSE(starlark_built_in_function(nullptr, base_fn, "fn_name").primitive());
}

TEST(StarlarkFunction, Truthy) {
  EXPECT_TRUE(starlark_function().truthy());
  EXPECT_TRUE(starlark_built_in_function(nullptr, base_fn, "fn_name").truthy());
}

TEST(StarlarkFunction, Str) {
  EXPECT_EQ("<built-in function fn_name>", starlark_built_in_function(nullptr, base_fn, "fn_name").str());
}

TEST(StarlarkFunction, Hash) {
  EXPECT_EQ(0, starlark_built_in_function(nullptr, base_fn, "").hash());
  EXPECT_EQ(-5056436948751091085, starlark_built_in_function(nullptr, base_fn, "fn_name").hash());
  EXPECT_EQ(-342786463226536281, starlark_built_in_function(nullptr, base_fn, "some_fn").hash());
}

TEST(StarlarkFunction, Equals) {
  EXPECT_TRUE(starlark_built_in_function(nullptr, base_fn, "fn_name").equals(starlark_built_in_function(nullptr, base_fn, "fn_name")));
  EXPECT_FALSE(starlark_built_in_function(nullptr, base_fn, "fn_name").equals(starlark_built_in_function(nullptr, base2_fn, "fn_name")));
  EXPECT_FALSE(starlark_built_in_function(nullptr, base_fn, "fn_name").equals(starlark_built_in_function(nullptr, base_fn, "another_name")));
  EXPECT_FALSE(starlark_built_in_function(nullptr, base_fn, "fn_name").equals(starlark_built_in_function(nullptr, base2_fn, "another_name")));
  EXPECT_FALSE(starlark_built_in_function(nullptr, base_fn, "fn_name").equals(starlark_list(0)));
}

TEST_F(FnTest, Call) {
  starlark_built_in_function fn(nullptr, base_fn, "fn_name");
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  EXPECT_CALL(*fn_mock, Call(testing::_, testing::_, testing::_, testing::_, testing::_))
      .WillOnce(testing::Return(nullptr));
  fn.call({}, {}, ctx, error_callback);
  // TODO(lmirelmann): Check the return value.
}

// TODO(lmirelmann): Check the error case.

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
  EXPECT_EQ("TypeError: bad operand type for abs(): 'list'", error_callback.messages[0]);
}

TEST(StarlarkAbs, NoPosArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  EXPECT_EQ(nullptr, starlark_fn_abs(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: abs() takes exactly one argument (0 given)", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: abs() takes exactly one argument (2 given)", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: abs() takes no keyword arguments", error_callback.messages[0]);
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
  list2.append(&one, error_callback);
  starlark_obj::pos_args_t pos_args2;
  starlark_obj::named_args_t named_args2;
  pos_args2.push_back(&list2);

  starlark_list list3(0);
  list3.append(&zero, error_callback);
  list3.append(&one, error_callback);
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
  EXPECT_EQ("TypeError: 'int' object is not iterable", error_callback.messages[0]);
}

TEST(StarlarkAll, NoPosArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  EXPECT_EQ(nullptr, starlark_fn_all(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: all() takes exactly one argument (0 given)", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: all() takes exactly one argument (2 given)", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: all() takes no keyword arguments", error_callback.messages[0]);
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
  list2.append(&one, error_callback);
  starlark_obj::pos_args_t pos_args2;
  starlark_obj::named_args_t named_args2;
  pos_args2.push_back(&list2);

  starlark_list list3(0);
  list3.append(&zero, error_callback);
  list3.append(&one, error_callback);
  starlark_obj::pos_args_t pos_args3;
  starlark_obj::named_args_t named_args3;
  pos_args3.push_back(&list3);

  starlark_list list4(0);
  list4.append(&zero, error_callback);
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
  EXPECT_EQ("TypeError: 'int' object is not iterable", error_callback.messages[0]);
}

TEST(StarlarkAny, NoPosArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  EXPECT_EQ(nullptr, starlark_fn_any(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: any() takes exactly one argument (0 given)", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: any() takes exactly one argument (2 given)", error_callback.messages[0]);
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

  EXPECT_EQ(nullptr, starlark_fn_all(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: all() takes no keyword arguments", error_callback.messages[0]);
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
  list2.append(&one, error_callback);
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

  EXPECT_EQ(nullptr, starlark_fn_bool(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: bool() takes exactly one argument (0 given)", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: bool() takes exactly one argument (2 given)", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: bool() takes no keyword arguments", error_callback.messages[0]);
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
  list2.append(&zero, error_callback);
  list2.append(&one, error_callback);
  list2.append(&max_minus_one, error_callback);
  list2.append(&max_byte, error_callback);
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
    list.append(value, error_callback);
    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&list);

    EXPECT_EQ(nullptr, starlark_fn_bytes(nullptr, pos_args, named_args, ctx, error_callback)) << value->str();
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ("ValueError: bytes must be in range(0, 256)", error_callback.messages[0]);
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

  EXPECT_EQ(nullptr, starlark_fn_bytes(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: bytes() takes exactly one argument (0 given)", error_callback.messages[0]);
}

TEST(StarlarkBytes, ListWithNone) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_list list(0);
  starlark_none none;
  list.append(&none, error_callback);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_bytes(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: 'NoneType' object cannot be interpreted as an integer", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: cannot convert 'NoneType' object to bytes", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: bytes() takes exactly one argument (2 given)", error_callback.messages[0]);
}

TEST(StarlarkBytes, NamedArguments) {
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
  EXPECT_EQ("TypeError: bytes() takes no keyword arguments", error_callback.messages[0]);
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
    EXPECT_EQ(i, utf8_reader(result->str(), false, false).peek_code_point());
  }
  for (int i = 0; i <= 0x10FFFF; ++i) {
    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    starlark_bigint value(i);
    pos_args.push_back(&value);

    auto* result = starlark_fn_chr(nullptr, pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    ASSERT_EQ(starlark_types::string_t, result->type());
    EXPECT_EQ(i, utf8_reader(result->str(), false, false).peek_code_point());
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
  EXPECT_EQ("TypeError: 'float' object cannot be interpreted as an integer", error_callback.messages[0]);
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
  EXPECT_EQ("ValueError: Unicode code point must be in range(0, 0x110000)", error_callback.messages[0]);
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
  EXPECT_EQ("ValueError: Unicode code point must be in range(0, 0x110000)", error_callback.messages[0]);
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
  EXPECT_EQ("ValueError: Unicode code point must be in range(0, 0x110000)", error_callback.messages[0]);
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
  EXPECT_EQ("ValueError: Unicode code point must be in range(0, 0x110000)", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: chr() takes no keyword arguments", error_callback.messages[0]);
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
  list.append(&tuple, error_callback);
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
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'int' object is not iterable");
}

TEST(StarlarkDict, FromNonIterable) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_list list(0);
  starlark_integer one(1);
  list.append(&one, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_dict(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'int' object is not iterable");
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
  list1.append(&tuple, error_callback);
  pos_args.push_back(&list1);

  auto* result = starlark_fn_dict(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: cannot use 'list' as a dict key (unhashable type: 'list')");
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
  EXPECT_EQ(error_callback.messages[0], "TypeError: dict expected at most 1 argument, got 2");
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
  list.append(&tuple1, error_callback);
  list.append(&tuple2, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_dict(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: dictionary update sequence element #1 has length 0; 2 is required");
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
  list.append(&tuple1, error_callback);
  list.append(&tuple2, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_dict(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: dictionary update sequence element #1 has length 1; 2 is required");
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
  list.append(&tuple1, error_callback);
  list.append(&tuple2, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_dict(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: dictionary update sequence element #1 has length 3; 2 is required");
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
  EXPECT_EQ(error_callback.messages[0], "TypeError: dir() takes exactly one argument (0 given)");
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
  EXPECT_EQ(error_callback.messages[0], "TypeError: dir() takes exactly one argument (2 given)");
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
  EXPECT_EQ(error_callback.messages[0], "TypeError: dir() takes no keyword arguments");
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
  list.append(&s_one, error_callback);
  list.append(&s_two, error_callback);
  list.append(&s_three, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_enumerate(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "[(0, \"one\"), (1, \"two\"), (2, \"three\")]");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkEnumerate, FromIterableWithStart) {
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
  list.append(&s_one, error_callback);
  list.append(&s_two, error_callback);
  list.append(&s_three, error_callback);
  pos_args.push_back(&list);
  starlark_integer start(100);
  named_args.insert(s_start, &start);

  auto* result = starlark_fn_enumerate(nullptr, pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "[(100, \"one\"), (101, \"two\"), (102, \"three\")]");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkEnumerate, InvalidStart) {
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
  list.append(&s_one, error_callback);
  list.append(&s_two, error_callback);
  list.append(&s_three, error_callback);
  pos_args.push_back(&list);
  starlark_string start("100"sv);
  named_args.insert(s_start, &start);

  auto* result = starlark_fn_enumerate(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: parameter 'start' cannot be interpreted as an integer (string).");
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
  list.append(&s_one, error_callback);
  list.append(&s_two, error_callback);
  list.append(&s_three, error_callback);
  pos_args.push_back(&list);
  starlark_string end("100"sv);
  named_args.insert(s_end, &end);

  auto* result = starlark_fn_enumerate(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "Unknown named argument 'end'.");
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
  EXPECT_EQ(error_callback.messages[0], "TypeError: enumerate() takes exactly one argument (0 given)");
}

TEST(StarlarkEnumerate, TooManyPosArguments) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  starlark_list list(0);
  starlark_string s_one("one"sv);
  starlark_string s_two("two"sv);
  starlark_string s_three("three"sv);
  list.append(&s_one, error_callback);
  list.append(&s_two, error_callback);
  list.append(&s_three, error_callback);
  pos_args.push_back(&list);
  pos_args.push_back(&list);

  auto* result = starlark_fn_enumerate(nullptr, pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: enumerate() takes exactly one argument (2 given)");
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
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'string' object is not iterable");
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
  EXPECT_EQ("TypeError: fail() takes no keyword arguments", error_callback.messages[0]);
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
  test(std::numeric_limits<int64_t>::max(), "9.2233720368547758e+18");
  test(std::numeric_limits<int64_t>::min(), "-9.2233720368547758e+18");
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
  test(std::numeric_limits<int64_t>::max(), "9.2233720368547758e+18");
  test(std::numeric_limits<int64_t>::min(), "-9.2233720368547758e+18");
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
  EXPECT_EQ(error_callback.messages[0], "TypeError: float() argument must be a string or a real number, not 'list'");
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
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to float");
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
  EXPECT_EQ(error_callback.messages[0], "OverflowError: floating-point number too large");
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
  EXPECT_EQ(error_callback.messages[0], "ValueError: could not convert string to float: '1a'");
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
  EXPECT_EQ("TypeError: float() takes exactly one argument (2 given)", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: float() takes no keyword arguments", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: attribute name must be string, not 'int'", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: hasattr expected 2 arguments, got 1", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: hasattr expected 2 arguments, got 3", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: hasattr() takes no keyword arguments", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: in call to hash(), got value of type 'bool', want 'string' or 'bytes'", error_callback.messages[0]);
}

TEST(StarlarkHash, NoPosArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  EXPECT_EQ(nullptr, starlark_fn_hash(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: hash() takes exactly one argument (0 given)", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: hash() takes exactly one argument (2 given)", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: hash() takes no keyword arguments", error_callback.messages[0]);
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

TEST(StarlarkInt, FromIntWithBase) {
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
  EXPECT_EQ("OverflowError: cannot convert float infinity to integer", error_callback.messages[0]);
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
  EXPECT_EQ("ValueError: cannot convert float NaN to integer", error_callback.messages[0]);
}

TEST(StarlarkInt, FromFloatWithBase) {
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

TEST(StarlarkInt, FromBoolWithBase) {
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

    starlark_string str(value);
    starlark_integer ibase(base);
    starlark_bigint bbase(base);
    starlark_obj::pos_args_t pos_args1;
    starlark_obj::named_args_t named_args1;
    starlark_obj::pos_args_t pos_args2;
    starlark_obj::named_args_t named_args2;
    pos_args1.push_back(&str);
    pos_args1.push_back(&ibase);
    pos_args2.push_back(&str);
    pos_args2.push_back(&bbase);

    auto* result1 = starlark_fn_int(nullptr, pos_args1, named_args1, ctx, error_callback);
    auto* result2 = starlark_fn_int(nullptr, pos_args2, named_args2, ctx, error_callback);
    ASSERT_NE(nullptr, result1) << value;
    ASSERT_NE(nullptr, result2);

    EXPECT_EQ(result1->str(), expected);
    EXPECT_EQ(result2->str(), expected);
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
void test_invalid_base(int base) {
  starlark_string str("1"sv);
  T ibase(base);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  pos_args.push_back(&ibase);

  EXPECT_EQ(nullptr, starlark_fn_int(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("ValueError: int() base must be >= 2 and <= 36, or 0", error_callback.messages[0]);
}

TEST(StarlarkInt, FromStringInvalidBase) {
  test_invalid_base<starlark_integer>(-1);
  test_invalid_base<starlark_bigint>(-1);
  test_invalid_base<starlark_integer>(1);
  test_invalid_base<starlark_bigint>(1);
  test_invalid_base<starlark_integer>(37);
  test_invalid_base<starlark_bigint>(37);
  test_invalid_base<starlark_integer>(100);
  test_invalid_base<starlark_bigint>(100);
}

TEST(StarlarkInt, FromStringBaseNotInt) {
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
  EXPECT_EQ("TypeError: 'list' object cannot be interpreted as an integer", error_callback.messages[0]);
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
  EXPECT_EQ("ValueError: invalid literal for int() with base 10: '123abc'", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: int() argument must be a string, int, bool or a real number, not 'list'", error_callback.messages[0]);
}

TEST(StarlarkInt, TooFewPosArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  EXPECT_EQ(nullptr, starlark_fn_int(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: int() takes one or two argument (0 given)", error_callback.messages[0]);
}

TEST(StarlarkInt, TooManyPosArgs) {
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
  EXPECT_EQ("TypeError: int() takes one or two argument (3 given)", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: int() takes no keyword arguments", error_callback.messages[0]);
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
  list2.append(&one, error_callback);
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
  EXPECT_EQ("TypeError: object of type 'int' has no len()", error_callback.messages[0]);
}

TEST(StarlarkLen, NoPosArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  EXPECT_EQ(nullptr, starlark_fn_len(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: len() takes exactly one argument (0 given)", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: len() takes exactly one argument (2 given)", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: len() takes no keyword arguments", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: 'int' object is not iterable", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: list expected at most 1 argument, got 2", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: list() takes no keyword arguments", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: ord() expected a character, but string of length 0 found", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: ord() expected a character, but string of length 2 found", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: ord() expected a character, but bytes of length 0 found", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: ord() expected a character, but bytes of length 2 found", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: ord() expected bytes of length 1 or string with one character, but 'list' found", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: ord() takes exactly one argument (2 given)", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: ord() takes no keyword arguments", error_callback.messages[0]);
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
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to int64");
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
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'list' object cannot be interpreted as an integer");
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
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'list' object cannot be interpreted as an integer");
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
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to int64");
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
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'list' object cannot be interpreted as an integer");
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
  EXPECT_EQ(error_callback.messages[0], "ValueError: range() arg 3 must not be zero");
}

TEST(StarlarkRange, TooFewPosArguments) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  EXPECT_EQ(nullptr, starlark_fn_range(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: range expected at least 1 argument, got 0", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: range expected at most 3 argument, got 4", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: range() takes no keyword arguments", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: repr() takes exactly one argument (0 given)", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: repr() takes exactly one argument (2 given)", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: repr() takes no keyword arguments", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: 'int' object is not iterable", error_callback.messages[0]);
}

TEST(StarlarkReversed, NoPosArgs) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  EXPECT_EQ(nullptr, starlark_fn_reversed(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: reversed() takes exactly one argument (0 given)", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: reversed() takes exactly one argument (2 given)", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: reversed() takes no keyword arguments", error_callback.messages[0]);
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
  list.append(&one, error_callback);
  list.append(&two, error_callback);
  list.append(&one, error_callback);
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
  list.append(&one, error_callback);
  list.append(&two, error_callback);
  list.append(&list, error_callback);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_set(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: cannot use 'list' as a set element (unhashable type: 'list')", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: 'string' object is not iterable", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: set expected at most 1 argument, got 2", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: set() takes no keyword arguments", error_callback.messages[0]);
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

TEST(StarlarkStr, TooFewPosArguments) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;

  EXPECT_EQ(nullptr, starlark_fn_str(nullptr, pos_args, named_args, ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: str() takes exactly one argument (0 given)", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: str() takes exactly one argument (2 given)", error_callback.messages[0]);
}

TEST(StarlarkStr, NamedArguments) {
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
  EXPECT_EQ("TypeError: str() takes no keyword arguments", error_callback.messages[0]);
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
  list.append(&one, error_callback);
  list.append(&two, error_callback);
  list.append(&one, error_callback);
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
  EXPECT_EQ("TypeError: 'string' object is not iterable", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: tuple expected at most 1 argument, got 2", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: tuple() takes no keyword arguments", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: type() takes exactly one argument (0 given)", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: type() takes exactly one argument (2 given)", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: type() takes no keyword arguments", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: 'string' object is not iterable", error_callback.messages[0]);
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
  EXPECT_EQ("TypeError: zip() takes no keyword arguments", error_callback.messages[0]);
}

}  // namespace
