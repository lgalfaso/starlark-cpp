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
#include "runtime/starlark_string.hpp"
#include "runtime/starlark_testing.hpp"
#include "runtime/starlark_tuple.hpp"
#include "runtime/starlark_types.hpp"
#include "unicode/utf8_reader.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;
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
using ::starlark::runtime::starlark_string;
using ::starlark::runtime::starlark_tuple;
using ::starlark::runtime::starlark_types;
using ::starlark::testing::error_handler;
using ::testing::IsEmpty;
using ::testing::SizeIs;
using starlark::unicode::utf8_reader;

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

starlark_obj* base2_fn(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, Arena& arena, error_fn& error_callback) {
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

TEST(StarlarkFunction, Primitve) {
  EXPECT_FALSE(starlark_function().primitive());
  EXPECT_FALSE(starlark_built_in_function(base_fn, "fn_name").primitive());
}

TEST(StarlarkFunction, Truthy) {
  EXPECT_TRUE(starlark_function().truthy());
  EXPECT_TRUE(starlark_built_in_function(base_fn, "fn_name").truthy());
}

TEST(StarlarkFunction, Str) {
  EXPECT_EQ("<built-in function fn_name>", starlark_built_in_function(base_fn, "fn_name").str());
}

TEST(StarlarkFunction, Hash) {
  EXPECT_EQ(0, starlark_built_in_function(base_fn, "").hash());
  EXPECT_EQ(-5056436948751091085, starlark_built_in_function(base_fn, "fn_name").hash());
  EXPECT_EQ(-342786463226536281, starlark_built_in_function(base_fn, "some_fn").hash());
}

TEST(StarlarkFunction, Equals) {
  EXPECT_TRUE(starlark_built_in_function(base_fn, "fn_name").equals(starlark_built_in_function(base_fn, "fn_name")));
  EXPECT_FALSE(starlark_built_in_function(base_fn, "fn_name").equals(starlark_built_in_function(base2_fn, "fn_name")));
  EXPECT_FALSE(starlark_built_in_function(base_fn, "fn_name").equals(starlark_built_in_function(base_fn, "another_name")));
  EXPECT_FALSE(starlark_built_in_function(base_fn, "fn_name").equals(starlark_built_in_function(base2_fn, "another_name")));
  EXPECT_FALSE(starlark_built_in_function(base_fn, "fn_name").equals(starlark_list(0)));
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
  starlark_list list(0);
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
  starlark_list list(0);
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
  starlark_list list(0);
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

TEST(StarlarkAll, List) {
  starlark_integer zero(0);
  starlark_integer one(1);
  Arena arena;
  error_handler error_callback;

  starlark_list list1(0);
  std::vector<starlark_obj*> pos_args1;
  std::map<std::string, starlark_obj*> named_args1;
  pos_args1.push_back(&list1);

  starlark_list list2(0);
  list2.add(&one, error_callback);
  std::vector<starlark_obj*> pos_args2;
  std::map<std::string, starlark_obj*> named_args2;
  pos_args2.push_back(&list2);

  starlark_list list3(0);
  list3.add(&zero, error_callback);
  list3.add(&one, error_callback);
  std::vector<starlark_obj*> pos_args3;
  std::map<std::string, starlark_obj*> named_args3;
  pos_args3.push_back(&list3);

  EXPECT_EQ("True", starlark_fn_all(pos_args1, named_args1, arena, error_callback)->str());
  EXPECT_EQ("True", starlark_fn_all(pos_args2, named_args2, arena, error_callback)->str());
  EXPECT_EQ("False", starlark_fn_all(pos_args3, named_args3, arena, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkAll, Integer) {
  starlark_integer one(1);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_all(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: 'int' object is not iterable", error_callback.messages[0]);
}

TEST(StarlarkAll, NoPosArgs) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;

  EXPECT_EQ(nullptr, starlark_fn_all(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: all() takes exactly one argument (0 given)", error_callback.messages[0]);
}

TEST(StarlarkAll, MultiplePosArgs) {
  starlark_integer one(1);
  starlark_list list(0);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&list);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_all(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: all() takes exactly one argument (2 given)", error_callback.messages[0]);
}

TEST(StarlarkAll, NamedArguments) {
  starlark_integer one(1);
  starlark_list list(0);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  named_args["1"] = &one;
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_all(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: all() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkAny, List) {
  starlark_integer zero(0);
  starlark_integer one(1);
  Arena arena;
  error_handler error_callback;

  starlark_list list1(0);
  std::vector<starlark_obj*> pos_args1;
  std::map<std::string, starlark_obj*> named_args1;
  pos_args1.push_back(&list1);

  starlark_list list2(0);
  list2.add(&one, error_callback);
  std::vector<starlark_obj*> pos_args2;
  std::map<std::string, starlark_obj*> named_args2;
  pos_args2.push_back(&list2);

  starlark_list list3(0);
  list3.add(&zero, error_callback);
  list3.add(&one, error_callback);
  std::vector<starlark_obj*> pos_args3;
  std::map<std::string, starlark_obj*> named_args3;
  pos_args3.push_back(&list3);

  starlark_list list4(0);
  list4.add(&zero, error_callback);
  std::vector<starlark_obj*> pos_args4;
  std::map<std::string, starlark_obj*> named_args4;
  pos_args4.push_back(&list4);

  EXPECT_EQ("False", starlark_fn_any(pos_args1, named_args1, arena, error_callback)->str());
  EXPECT_EQ("True", starlark_fn_any(pos_args2, named_args2, arena, error_callback)->str());
  EXPECT_EQ("True", starlark_fn_any(pos_args3, named_args3, arena, error_callback)->str());
  EXPECT_EQ("False", starlark_fn_any(pos_args4, named_args4, arena, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkAny, Integer) {
  starlark_integer one(1);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_any(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: 'int' object is not iterable", error_callback.messages[0]);
}

TEST(StarlarkAny, NoPosArgs) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;

  EXPECT_EQ(nullptr, starlark_fn_any(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: any() takes exactly one argument (0 given)", error_callback.messages[0]);
}

TEST(StarlarkAny, MultiplePosArgs) {
  starlark_integer one(1);
  starlark_list list(0);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&list);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_any(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: any() takes exactly one argument (2 given)", error_callback.messages[0]);
}

TEST(StarlarkAny, NamedArguments) {
  starlark_integer one(1);
  starlark_list list(0);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  named_args["1"] = &one;
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_all(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: all() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkBool, List) {
  starlark_integer one(1);
  starlark_list list1(0);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args1;
  std::map<std::string, starlark_obj*> named_args1;
  pos_args1.push_back(&list1);

  starlark_list list2(0);
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
  starlark_list list(0);
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
  starlark_list list(0);
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

TEST(StarlarkBytes, FromBytesOrString) {
  Arena arena;
  error_handler error_callback;

  {
    starlark_string str("abc");
    std::vector<starlark_obj*> pos_args;
    std::map<std::string, starlark_obj*> named_args;
    pos_args.push_back(&str);

    auto* result = starlark_fn_bytes(pos_args, named_args, arena, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->repr(), "b\"abc\"");
  }
  {
    starlark_bytes bytes("def");
    std::vector<starlark_obj*> pos_args;
    std::map<std::string, starlark_obj*> named_args;
    pos_args.push_back(&bytes);

    auto* result = starlark_fn_bytes(pos_args, named_args, arena, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->repr(), "b\"def\"");
  }
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkBytes, FromStringInvalidUnicodeSequence) {
  Arena arena;
  error_handler error_callback;

  starlark_string str("abc\xf0\x{f1}def");
  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&str);

  auto* result = starlark_fn_bytes(pos_args, named_args, arena, error_callback);
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
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args1;
  std::map<std::string, starlark_obj*> named_args1;
  pos_args1.push_back(&list1);

  starlark_list list2(0);
  list2.add(&zero, error_callback);
  list2.add(&one, error_callback);
  list2.add(&max_minus_one, error_callback);
  list2.add(&max_byte, error_callback);
  std::vector<starlark_obj*> pos_args2;
  std::map<std::string, starlark_obj*> named_args2;
  pos_args2.push_back(&list2);

  auto* result1 = starlark_fn_bytes(pos_args1, named_args1, arena, error_callback);
  auto* result2 = starlark_fn_bytes(pos_args2, named_args2, arena, error_callback);
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
  starlark_bigint big(number::one << 64);

  auto test = [](starlark_obj* value) {
    Arena arena;
    error_handler error_callback;
    starlark_list list(0);
    list.add(value, error_callback);
    std::vector<starlark_obj*> pos_args;
    std::map<std::string, starlark_obj*> named_args;
    pos_args.push_back(&list);

    EXPECT_EQ(nullptr, starlark_fn_bytes(pos_args, named_args, arena, error_callback)) << value->str();
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
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;

  EXPECT_EQ(nullptr, starlark_fn_bytes(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: bytes() takes exactly one argument (0 given)", error_callback.messages[0]);
}

TEST(StarlarkBytes, ListWithNone) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  starlark_list list(0);
  starlark_none none;
  list.add(&none, error_callback);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_bytes(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: 'NoneType' object cannot be interpreted as an integer", error_callback.messages[0]);
}

TEST(StarlarkBytes, None) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  starlark_none none;
  pos_args.push_back(&none);

  EXPECT_EQ(nullptr, starlark_fn_bytes(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: cannot convert 'NoneType' object to bytes", error_callback.messages[0]);
}

TEST(StarlarkBytes, MultiplePosArgs) {
  starlark_bytes bytes("def");
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&bytes);
  pos_args.push_back(&bytes);

  EXPECT_EQ(nullptr, starlark_fn_bytes(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: bytes() takes exactly one argument (2 given)", error_callback.messages[0]);
}

TEST(StarlarkBytes, NamedArguments) {
  starlark_bytes bytes("def");
  starlark_integer one(1);
  starlark_list list(0);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&bytes);
  named_args["1"] = &one;
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_bytes(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: bytes() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkChr, FromInt) {
  Arena arena;
  error_handler error_callback;

  for (int i = 0; i <= 0x10FFFF; ++i) {
    std::vector<starlark_obj*> pos_args;
    std::map<std::string, starlark_obj*> named_args;
    starlark_integer value(i);
    pos_args.push_back(&value);

    auto* result = starlark_fn_chr(pos_args, named_args, arena, error_callback);
    ASSERT_NE(nullptr, result);
    ASSERT_EQ(starlark_types::string_t, result->type());
    EXPECT_EQ(i, utf8_reader(result->str(), false, false).peek_code_point());
  }
  for (int i = 0; i <= 0x10FFFF; ++i) {
    std::vector<starlark_obj*> pos_args;
    std::map<std::string, starlark_obj*> named_args;
    starlark_bigint value(i);
    pos_args.push_back(&value);

    auto* result = starlark_fn_chr(pos_args, named_args, arena, error_callback);
    ASSERT_NE(nullptr, result);
    ASSERT_EQ(starlark_types::string_t, result->type());
    EXPECT_EQ(i, utf8_reader(result->str(), false, false).peek_code_point());
  }
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkChr, FromFloat) {
  starlark_float one(1);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_chr(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: 'float' object cannot be interpreted as an integer", error_callback.messages[0]);
}

TEST(StarlarkChr, OutOfRange1) {
  starlark_integer value(-1);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&value);

  EXPECT_EQ(nullptr, starlark_fn_chr(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("ValueError: Unicode code point must be in range(0, 0x110000)", error_callback.messages[0]);
}

TEST(StarlarkChr, OutOfRange2) {
  starlark_bigint value(-1);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&value);

  EXPECT_EQ(nullptr, starlark_fn_chr(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("ValueError: Unicode code point must be in range(0, 0x110000)", error_callback.messages[0]);
}

TEST(StarlarkChr, OutOfRange3) {
  starlark_integer value(0x110000);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&value);

  EXPECT_EQ(nullptr, starlark_fn_chr(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("ValueError: Unicode code point must be in range(0, 0x110000)", error_callback.messages[0]);
}

TEST(StarlarkChr, OutOfRange4) {
  starlark_bigint value(0x110000);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&value);

  EXPECT_EQ(nullptr, starlark_fn_chr(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("ValueError: Unicode code point must be in range(0, 0x110000)", error_callback.messages[0]);
}

TEST(StarlarkChr, NamedArguments) {
  starlark_bytes bytes("def");
  starlark_integer one(1);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&one);
  named_args["1"] = &bytes;

  EXPECT_EQ(nullptr, starlark_fn_chr(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: chr() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkDict, Empty) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;

  auto* result = starlark_fn_dict(pos_args, named_args, arena, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "{}");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkDict, FromDict) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  starlark_dictionary dict;
  starlark_none none;
  starlark_integer one(1);
  dict.insert(&none, &none, error_callback);
  pos_args.push_back(&dict);

  auto* result = starlark_fn_dict(pos_args, named_args, arena, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "{None: None}");
  // Check that this is a copy.
  dict.insert(&none, &one, error_callback);
  EXPECT_EQ(result->repr(), "{None: None}");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkDict, FromIterable) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  starlark_list list(0);
  starlark_tuple tuple(0);
  starlark_none none;
  starlark_integer one(1);
  tuple.add(&none);
  tuple.add(&one);
  list.add(&tuple, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_dict(pos_args, named_args, arena, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "{None: 1}");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkDict, FromNamedArguments) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  starlark_integer one(1);
  starlark_integer two(2);
  named_args["one"] = &one;
  named_args["two"] = &two;

  auto* result = starlark_fn_dict(pos_args, named_args, arena, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "{\"one\": 1, \"two\": 2}");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkDict, FromInteger) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  starlark_integer one(1);
  pos_args.push_back(&one);

  auto* result = starlark_fn_dict(pos_args, named_args, arena, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'int' object is not iterable");
}

TEST(StarlarkDict, FromNonIterable) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  starlark_list list(0);
  starlark_integer one(1);
  list.add(&one, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_dict(pos_args, named_args, arena, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'int' object is not iterable");
}

TEST(StarlarkDict, FromNonHashable) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  starlark_list list1(0);
  starlark_list list2(0);
  starlark_tuple tuple(0);
  starlark_integer one(1);
  tuple.add(&list2);
  tuple.add(&one);
  list1.add(&tuple, error_callback);
  pos_args.push_back(&list1);

  auto* result = starlark_fn_dict(pos_args, named_args, arena, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: cannot use 'list' as a dict key (unhashable type: 'list')");
}

TEST(StarlarkDict, MultiplePositionalArguments) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  starlark_list list(0);
  pos_args.push_back(&list);
  pos_args.push_back(&list);

  auto* result = starlark_fn_dict(pos_args, named_args, arena, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: dict expected at most 1 argument, got 2");
}

TEST(StarlarkDict, FromIterableWithWrongNumberOfElements1) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  starlark_list list(0);
  starlark_tuple tuple1(0);
  starlark_tuple tuple2(0);
  starlark_none none;
  starlark_integer one(1);
  tuple1.add(&none);
  tuple1.add(&one);
  list.add(&tuple1, error_callback);
  list.add(&tuple2, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_dict(pos_args, named_args, arena, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: dictionary update sequence element #1 has length 0; 2 is required");
}

TEST(StarlarkDict, FromIterableWithWrongNumberOfElements2) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  starlark_list list(0);
  starlark_tuple tuple1(0);
  starlark_tuple tuple2(0);
  starlark_none none;
  starlark_integer one(1);
  tuple1.add(&none);
  tuple1.add(&one);
  tuple2.add(&none);
  list.add(&tuple1, error_callback);
  list.add(&tuple2, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_dict(pos_args, named_args, arena, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: dictionary update sequence element #1 has length 1; 2 is required");
}

TEST(StarlarkDict, FromIterableWithWrongNumberOfElements3) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
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
  list.add(&tuple1, error_callback);
  list.add(&tuple2, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_dict(pos_args, named_args, arena, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: dictionary update sequence element #1 has length 3; 2 is required");
}

TEST(StarlarkEnumerate, FromIterable) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  starlark_list list(0);
  starlark_string s_one("one");
  starlark_string s_two("two");
  starlark_string s_three("three");
  list.add(&s_one, error_callback);
  list.add(&s_two, error_callback);
  list.add(&s_three, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_enumerate(pos_args, named_args, arena, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "[(0, \"one\"), (1, \"two\"), (2, \"three\")]");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkEnumerate, FromIterableWithStart) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  starlark_list list(0);
  starlark_string s_one("one");
  starlark_string s_two("two");
  starlark_string s_three("three");
  list.add(&s_one, error_callback);
  list.add(&s_two, error_callback);
  list.add(&s_three, error_callback);
  pos_args.push_back(&list);
  starlark_integer start(100);
  named_args["start"] = &start;

  auto* result = starlark_fn_enumerate(pos_args, named_args, arena, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "[(100, \"one\"), (101, \"two\"), (102, \"three\")]");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkEnumerate, InvalidStart) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  starlark_list list(0);
  starlark_string s_one("one");
  starlark_string s_two("two");
  starlark_string s_three("three");
  list.add(&s_one, error_callback);
  list.add(&s_two, error_callback);
  list.add(&s_three, error_callback);
  pos_args.push_back(&list);
  starlark_string start("100");
  named_args["start"] = &start;

  auto* result = starlark_fn_enumerate(pos_args, named_args, arena, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: parameter 'start' cannot be interpreted as an integer (string).");
}

TEST(StarlarkEnumerate, InvalidNamedArgument) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  starlark_list list(0);
  starlark_string s_one("one");
  starlark_string s_two("two");
  starlark_string s_three("three");
  list.add(&s_one, error_callback);
  list.add(&s_two, error_callback);
  list.add(&s_three, error_callback);
  pos_args.push_back(&list);
  starlark_string end("100");
  named_args["end"] = &end;

  auto* result = starlark_fn_enumerate(pos_args, named_args, arena, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "Unknown named argument 'end'.");
}

TEST(StarlarkEnumerate, TooFewPosArguments) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;

  auto* result = starlark_fn_enumerate(pos_args, named_args, arena, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: enumerate() takes exactly one argument (0 given)");
}

TEST(StarlarkEnumerate, TooManyPosArguments) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  starlark_list list(0);
  starlark_string s_one("one");
  starlark_string s_two("two");
  starlark_string s_three("three");
  list.add(&s_one, error_callback);
  list.add(&s_two, error_callback);
  list.add(&s_three, error_callback);
  pos_args.push_back(&list);
  pos_args.push_back(&list);

  auto* result = starlark_fn_enumerate(pos_args, named_args, arena, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: enumerate() takes exactly one argument (2 given)");
}

TEST(StarlarkEnumerate, NotIterable) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  starlark_string s_one("one");
  pos_args.push_back(&s_one);

  auto* result = starlark_fn_enumerate(pos_args, named_args, arena, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'string' object is not iterable");
}

TEST(StarlarkFail, Message) {
  Arena arena;
  error_handler error_callback;
  starlark_string str("some error message");
  starlark_list list(0);
  starlark_integer one(1);

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&str);
  pos_args.push_back(&list);
  pos_args.push_back(&one);

  auto* result = starlark_fn_fail(pos_args, named_args, arena, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "Error: some error message [] 1");
}

TEST(StarlarkFail, NamedArgs) {
  Arena arena;
  error_handler error_callback;
  starlark_integer one(1);

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  named_args["1"] = &one;

  EXPECT_EQ(nullptr, starlark_fn_fail(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: fail() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkFloat, FromFloat) {
  auto test = [](double fvalue, std::string_view repr) {
    starlark_float value(fvalue);
    Arena arena;
    error_handler error_callback;

    std::vector<starlark_obj*> pos_args;
    std::map<std::string, starlark_obj*> named_args;
    pos_args.push_back(&value);

    EXPECT_EQ(repr, starlark_fn_float(pos_args, named_args, arena, error_callback)->repr());
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
    error_handler error_callback;

    std::vector<starlark_obj*> pos_args;
    std::map<std::string, starlark_obj*> named_args;
    pos_args.push_back(&value);

    EXPECT_EQ(repr, starlark_fn_float(pos_args, named_args, arena, error_callback)->repr());
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
    error_handler error_callback;

    std::vector<starlark_obj*> pos_args;
    std::map<std::string, starlark_obj*> named_args;
    pos_args.push_back(&value);

    EXPECT_EQ(repr, starlark_fn_float(pos_args, named_args, arena, error_callback)->repr());
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
    error_handler error_callback;

    std::vector<starlark_obj*> pos_args;
    std::map<std::string, starlark_obj*> named_args;
    pos_args.push_back(&value);

    EXPECT_EQ(repr, starlark_fn_float(pos_args, named_args, arena, error_callback)->repr());
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test("1", "1.0");
  test("-1", "-1.0");
  test("1e308", "1e+308");
  test("Infinity", "inf");
  test("-Infinity", "-inf");
  test("NaN", "nan");
}

TEST(StarlarkFloat, FromBool) {
  auto test = [](bool bvalue, std::string_view repr) {
    starlark_bool value(bvalue);
    Arena arena;
    error_handler error_callback;

    std::vector<starlark_obj*> pos_args;
    std::map<std::string, starlark_obj*> named_args;
    pos_args.push_back(&value);

    EXPECT_EQ(repr, starlark_fn_float(pos_args, named_args, arena, error_callback)->repr());
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test(false, "0.0");
  test(true, "1.0");
}

TEST(StarlarkFloat, FromList) {
  starlark_list value(0);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&value);

  auto* result = starlark_fn_float(pos_args, named_args, arena, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: float() argument must be a string or a real number, not 'list'");
}

TEST(StarlarkFloat, BigintOverflow) {
  starlark_bigint value(number::one << 2000);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&value);

  auto* result = starlark_fn_float(pos_args, named_args, arena, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to float");
}

TEST(StarlarkFloat, StringOverflow) {
  starlark_string value("2e308");
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&value);

  auto* result = starlark_fn_float(pos_args, named_args, arena, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: floating-point number too large");
}

TEST(StarlarkFloat, InvalidString) {
  starlark_string value("1a");
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&value);

  auto* result = starlark_fn_float(pos_args, named_args, arena, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: could not convert string to float: '1a'");
}

TEST(StarlarkFloat, MultiplePosArgs) {
  starlark_integer one(1);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&one);
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_float(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: float() takes exactly one argument (2 given)", error_callback.messages[0]);
}

TEST(StarlarkFloat, NamedArguments) {
  starlark_integer one(1);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  named_args["1"] = &one;
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_float(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: float() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkHash, String) {
  starlark_string str1("");
  starlark_string str2("abc");
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args1;
  std::map<std::string, starlark_obj*> named_args1;
  pos_args1.push_back(&str1);
  std::vector<starlark_obj*> pos_args2;
  std::map<std::string, starlark_obj*> named_args2;
  pos_args2.push_back(&str2);

  EXPECT_EQ("0", starlark_fn_hash(pos_args1, named_args1, arena, error_callback)->str());
  EXPECT_EQ("6041520446639342335", starlark_fn_hash(pos_args2, named_args2, arena, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkHash, Bytes) {
  starlark_bytes bytes1("");
  starlark_bytes bytes2("abc");
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args1;
  std::map<std::string, starlark_obj*> named_args1;
  pos_args1.push_back(&bytes1);
  std::vector<starlark_obj*> pos_args2;
  std::map<std::string, starlark_obj*> named_args2;
  pos_args2.push_back(&bytes2);

  EXPECT_EQ("0", starlark_fn_hash(pos_args1, named_args1, arena, error_callback)->str());
  EXPECT_EQ("-8236155743588961689", starlark_fn_hash(pos_args2, named_args2, arena, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkHash, Bool) {
  starlark_bool true_obj(true);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&true_obj);

  EXPECT_EQ(nullptr, starlark_fn_hash(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: in call to hash(), got value of type 'bool', want 'string' or 'bytes'", error_callback.messages[0]);
}

TEST(StarlarkHash, NoPosArgs) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;

  EXPECT_EQ(nullptr, starlark_fn_hash(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: hash() takes exactly one argument (0 given)", error_callback.messages[0]);
}

TEST(StarlarkHash, MultiplePosArgs) {
  starlark_string str("");
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&str);
  pos_args.push_back(&str);

  EXPECT_EQ(nullptr, starlark_fn_hash(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: hash() takes exactly one argument (2 given)", error_callback.messages[0]);
}

TEST(StarlarkHash, NamedArguments) {
  starlark_string str("");
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  named_args["1"] = &str;
  pos_args.push_back(&str);

  EXPECT_EQ(nullptr, starlark_fn_hash(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: hash() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkInt, FromInt) {
  starlark_integer one(1);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&one);

  EXPECT_EQ("1", starlark_fn_int(pos_args, named_args, arena, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkInt, FromIntWithBase) {
  starlark_integer one(1);
  starlark_integer two(2);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&one);
  pos_args.push_back(&two);

  EXPECT_EQ(nullptr, starlark_fn_int(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: int() can't convert non-string with explicit base", error_callback.messages[0]);
}

TEST(StarlarkInt, FromFloat) {
  starlark_float value(1e70);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&value);

  EXPECT_EQ("10000000000000000725314363815292351261583744096465219555182101554790400", starlark_fn_int(pos_args, named_args, arena, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkInt, FromFloatInfinity) {
  starlark_float value(std::numeric_limits<double>::infinity());
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&value);

  EXPECT_EQ(nullptr, starlark_fn_int(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("OverflowError: cannot convert float infinity to integer", error_callback.messages[0]);
}

TEST(StarlarkInt, FromFloatNaN) {
  starlark_float value(std::numeric_limits<double>::quiet_NaN());
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&value);

  EXPECT_EQ(nullptr, starlark_fn_int(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("ValueError: cannot convert float NaN to integer", error_callback.messages[0]);
}

TEST(StarlarkInt, FromFloatWithBase) {
  starlark_float value(1e70);
  starlark_integer two(2);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&value);
  pos_args.push_back(&two);

  EXPECT_EQ(nullptr, starlark_fn_int(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: int() can't convert non-string with explicit base", error_callback.messages[0]);
}

TEST(StarlarkInt, FromBool) {
  starlark_bool true_value(true);
  starlark_bool false_value(false);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args1;
  std::map<std::string, starlark_obj*> named_args1;
  pos_args1.push_back(&true_value);
  std::vector<starlark_obj*> pos_args2;
  std::map<std::string, starlark_obj*> named_args2;
  pos_args2.push_back(&false_value);

  EXPECT_EQ("1", starlark_fn_int(pos_args1, named_args1, arena, error_callback)->str());
  EXPECT_EQ("0", starlark_fn_int(pos_args2, named_args2, arena, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkInt, FromBoolWithBase) {
  starlark_bool value(true);
  starlark_integer two(2);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&value);
  pos_args.push_back(&two);

  EXPECT_EQ(nullptr, starlark_fn_int(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: int() can't convert non-string with explicit base", error_callback.messages[0]);
}

TEST(StarlarkInt, FromString) {
  auto test = [](std::string value, std::string_view expected) {
    Arena arena;
    error_handler error_callback;

    starlark_string str(value);
    std::vector<starlark_obj*> pos_args;
    std::map<std::string, starlark_obj*> named_args;
    pos_args.push_back(&str);

    auto* result = starlark_fn_int(pos_args, named_args, arena, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->str(), expected);
  };

  test("-0123", "-123");
  test("-123", "-123");
  test("0", "0");
  test("123", "123");
  test("0123", "123");
}

TEST(StarlarkInt, FromStringWithBase) {
  auto test = [](std::string value, int base, std::string_view expected) {
    Arena arena;
    error_handler error_callback;

    starlark_string str(value);
    starlark_integer ibase(base);
    starlark_bigint bbase(base);
    std::vector<starlark_obj*> pos_args1;
    std::map<std::string, starlark_obj*> named_args1;
    std::vector<starlark_obj*> pos_args2;
    std::map<std::string, starlark_obj*> named_args2;
    pos_args1.push_back(&str);
    pos_args1.push_back(&ibase);
    pos_args2.push_back(&str);
    pos_args2.push_back(&bbase);

    auto* result1 = starlark_fn_int(pos_args1, named_args1, arena, error_callback);
    auto* result2 = starlark_fn_int(pos_args2, named_args2, arena, error_callback);
    ASSERT_NE(nullptr, result1);
    ASSERT_NE(nullptr, result2);

    EXPECT_EQ(result1->str(), expected);
    EXPECT_EQ(result2->str(), expected);
  };

  test("123", 10, "123");
  test("123", 0, "123");
  test("0123", 10, "123");
  test("0x123", 16, "291");
  test("123", 16, "291");
  test("0x123", 0, "291");
  test("0o123", 8, "83");
  test("123", 8, "83");
  test("0o123", 0, "83");
  test("0b101", 2, "5");
  test("101", 2, "5");
  test("0b101", 0, "5");
}

template <typename T>
void test_invalid_base(int base) {
  starlark_string str("1");
  T ibase(base);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&str);
  pos_args.push_back(&ibase);

  EXPECT_EQ(nullptr, starlark_fn_int(pos_args, named_args, arena, error_callback));
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
  starlark_string str("1");
  starlark_list list(0);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&str);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_int(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: 'list' object cannot be interpreted as an integer", error_callback.messages[0]);
}

TEST(StarlarkInt, FromStringNotAbleToParseInFull) {
  starlark_string str("123abc");
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&str);

  EXPECT_EQ(nullptr, starlark_fn_int(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("ValueError: invalid literal for int() with base 10: '123abc'", error_callback.messages[0]);
}

TEST(StarlarkInt, FromList) {
  starlark_list list(0);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_int(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: int() argument must be a string, int, bool or a real number, not 'list'", error_callback.messages[0]);
}

TEST(StarlarkInt, TooFewPosArgs) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;

  EXPECT_EQ(nullptr, starlark_fn_int(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: int() takes one or two argument (0 given)", error_callback.messages[0]);
}

TEST(StarlarkInt, TooManyPosArgs) {
  starlark_string str("");
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&str);
  pos_args.push_back(&str);
  pos_args.push_back(&str);

  EXPECT_EQ(nullptr, starlark_fn_int(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: int() takes one or two argument (3 given)", error_callback.messages[0]);
}

TEST(StarlarkInt, NamedArguments) {
  starlark_string str("1");
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  named_args["1"] = &str;
  pos_args.push_back(&str);

  EXPECT_EQ(nullptr, starlark_fn_int(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: int() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkLen, List) {
  starlark_integer one(1);
  starlark_list list1(0);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args1;
  std::map<std::string, starlark_obj*> named_args1;
  pos_args1.push_back(&list1);

  starlark_list list2(0);
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
  starlark_list list(0);
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
  starlark_list list(0);
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

TEST(StarlarkList, Tuple) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_tuple tuple1(0);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args1;
  std::map<std::string, starlark_obj*> named_args1;
  pos_args1.push_back(&tuple1);

  starlark_tuple tuple2(0);
  tuple2.add(&zero);
  tuple2.add(&one);
  std::vector<starlark_obj*> pos_args2;
  std::map<std::string, starlark_obj*> named_args2;
  pos_args2.push_back(&tuple2);

  EXPECT_EQ("[]", starlark_fn_list(pos_args1, named_args1, arena, error_callback)->str());
  EXPECT_EQ("[0, 1]", starlark_fn_list(pos_args2, named_args2, arena, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkList, Integer) {
  starlark_integer one(1);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_list(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: 'int' object is not iterable", error_callback.messages[0]);
}

TEST(StarlarkList, NoPosArgs) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;

  EXPECT_EQ("[]", starlark_fn_list(pos_args, named_args, arena, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkList, MultiplePosArgs) {
  starlark_list list(0);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&list);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_list(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: list expected at most 1 argument, got 2", error_callback.messages[0]);
}

TEST(StarlarkList, NamedArguments) {
  starlark_integer one(1);
  starlark_list list(0);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  named_args["1"] = &one;
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_list(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: list() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkOrd, FromString) {
  starlark_string str("😃");
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&str);

  EXPECT_EQ("128515", starlark_fn_ord(pos_args, named_args, arena, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkOrd, FromBytes) {
  starlark_bytes bytes("\xFF");
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&bytes);

  EXPECT_EQ("255", starlark_fn_ord(pos_args, named_args, arena, error_callback)->str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkOrd, ShortString) {
  starlark_string str("");
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&str);

  EXPECT_EQ(nullptr, starlark_fn_ord(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: ord() expected a character, but string of length 0 found", error_callback.messages[0]);
}

TEST(StarlarkOrd, LongString) {
  starlark_string str("ab");
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&str);

  EXPECT_EQ(nullptr, starlark_fn_ord(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: ord() expected a character, but string of length 2 found", error_callback.messages[0]);
}

TEST(StarlarkOrd, ShortBytes) {
  starlark_bytes bytes("");
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&bytes);

  EXPECT_EQ(nullptr, starlark_fn_ord(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: ord() expected a character, but bytes of length 0 found", error_callback.messages[0]);
}

TEST(StarlarkOrd, LongBytes) {
  starlark_bytes bytes("ab");
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&bytes);

  EXPECT_EQ(nullptr, starlark_fn_ord(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: ord() expected a character, but bytes of length 2 found", error_callback.messages[0]);
}

TEST(StarlarkOrd, List) {
  starlark_list list(0);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_ord(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: ord() expected bytes of length 1 or string with one character, but 'list' found", error_callback.messages[0]);
}

TEST(StarlarkOrd, MultiplePosArgs) {
  starlark_bytes bytes("\xFF");
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&bytes);
  pos_args.push_back(&bytes);

  EXPECT_EQ(nullptr, starlark_fn_ord(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: ord() takes exactly one argument (2 given)", error_callback.messages[0]);
}

TEST(StarlarkOrd, NamedArguments) {
  starlark_integer one(1);
  starlark_bytes bytes("\xFF");
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  named_args["1"] = &one;
  pos_args.push_back(&bytes);

  EXPECT_EQ(nullptr, starlark_fn_ord(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: ord() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkRange, OneArgument) {
  starlark_integer one(1);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&one);

  auto* result = starlark_fn_range(pos_args, named_args, arena, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->str(), "range(1)");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkRange, OneArgumentBigInt) {
  starlark_bigint big(100);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&big);

  auto* result = starlark_fn_range(pos_args, named_args, arena, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->str(), "range(100)");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkRange, OneArgumentBigIntTooBig) {
  starlark_bigint big(number::one << 63);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&big);

  auto* result = starlark_fn_range(pos_args, named_args, arena, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to int64");
}

TEST(StarlarkRange, OneInvalidArgument) {
  starlark_list list(0);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&list);

  auto* result = starlark_fn_range(pos_args, named_args, arena, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'list' object cannot be interpreted as an integer");
}

TEST(StarlarkRange, TwoArguments) {
  starlark_integer one(1);
  starlark_integer ten(10);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&one);
  pos_args.push_back(&ten);

  auto* result = starlark_fn_range(pos_args, named_args, arena, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->str(), "range(1, 10)");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkRange, TwoInvalidArguments) {
  starlark_integer one(1);
  starlark_list list(0);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&one);
  pos_args.push_back(&list);

  auto* result = starlark_fn_range(pos_args, named_args, arena, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'list' object cannot be interpreted as an integer");
}

TEST(StarlarkRange, TwoArgumentsOverflow) {
  starlark_integer minus_one(-1);
  starlark_integer max_int64(std::numeric_limits<int64_t>::max());
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&minus_one);
  pos_args.push_back(&max_int64);

  auto* result = starlark_fn_range(pos_args, named_args, arena, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "OverflowError: int too large to convert to int64");
}

TEST(StarlarkRange, ThreeArguments) {
  starlark_integer one(1);
  starlark_integer ten(10);
  starlark_integer minus_one(-1);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&one);
  pos_args.push_back(&ten);
  pos_args.push_back(&minus_one);

  auto* result = starlark_fn_range(pos_args, named_args, arena, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->str(), "range(1, 10, -1)");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkRange, ThreeInvalidArguments) {
  starlark_integer one(1);
  starlark_list list(0);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&one);
  pos_args.push_back(&one);
  pos_args.push_back(&list);

  auto* result = starlark_fn_range(pos_args, named_args, arena, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'list' object cannot be interpreted as an integer");
}

TEST(StarlarkRange, ZeroStep) {
  starlark_integer one(1);
  starlark_integer ten(10);
  starlark_integer zero(0);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&one);
  pos_args.push_back(&ten);
  pos_args.push_back(&zero);

  auto* result = starlark_fn_range(pos_args, named_args, arena, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: range() arg 3 must not be zero");
}

TEST(StarlarkRange, TooFewPosArguments) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;

  EXPECT_EQ(nullptr, starlark_fn_range(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: range expected at least 1 argument, got 0", error_callback.messages[0]);
}

TEST(StarlarkRange, TooManyPosArguments) {
  starlark_integer one(1);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&one);
  pos_args.push_back(&one);
  pos_args.push_back(&one);
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_range(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: range expected at most 3 argument, got 4", error_callback.messages[0]);
}

TEST(StarlarkRange, NamedArguments) {
  starlark_integer one(1);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  named_args["1"] = &one;
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_range(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: range() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkRepr, String) {
  Arena arena;
  error_handler error_callback;
  starlark_string str("abc");

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&str);

  auto* result = starlark_fn_repr(pos_args, named_args, arena, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->as_string(), "\"abc\"");
}

TEST(StarlarkRepr, TooFewPosArguments) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;

  EXPECT_EQ(nullptr, starlark_fn_repr(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: repr() takes exactly one argument (0 given)", error_callback.messages[0]);
}

TEST(StarlarkRepr, TooManyPosArguments) {
  starlark_integer one(1);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&one);
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_repr(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: repr() takes exactly one argument (2 given)", error_callback.messages[0]);
}

TEST(StarlarkRepr, NamedArguments) {
  starlark_integer one(1);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  named_args["1"] = &one;
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_repr(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: repr() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkSet, NoArguments) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;

  auto* result = starlark_fn_set(pos_args, named_args, arena, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "set()");
}

TEST(StarlarkSet, OneArguments) {
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_list list(3);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  list.add(&one, error_callback);
  list.add(&two, error_callback);
  list.add(&one, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_set(pos_args, named_args, arena, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "set([1, 2])");
}

TEST(StarlarkSet, ElementNotHashable) {
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_list list(3);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  list.add(&one, error_callback);
  list.add(&two, error_callback);
  list.add(&list, error_callback);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_set(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: cannot use 'list' as a set element (unhashable type: 'list')", error_callback.messages[0]);
}

TEST(StarlarkSet, String) {
  Arena arena;
  error_handler error_callback;
  starlark_string str("abc");

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&str);

  EXPECT_EQ(nullptr, starlark_fn_set(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: 'string' object is not iterable", error_callback.messages[0]);
}

TEST(StarlarkSet, TooManyPosArguments) {
  starlark_list list(0);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&list);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_set(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: set expected at most 1 argument, got 2", error_callback.messages[0]);
}

TEST(StarlarkSet, NamedArguments) {
  starlark_list list(0);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  named_args["1"] = &list;
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_set(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: set() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkStr, String) {
  Arena arena;
  error_handler error_callback;
  starlark_string str("abc");

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&str);

  auto* result = starlark_fn_str(pos_args, named_args, arena, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->as_string(), "abc");
}

TEST(StarlarkStr, TooFewPosArguments) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;

  EXPECT_EQ(nullptr, starlark_fn_str(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: str() takes exactly one argument (0 given)", error_callback.messages[0]);
}

TEST(StarlarkStr, TooManyPosArguments) {
  starlark_integer one(1);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&one);
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_str(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: str() takes exactly one argument (2 given)", error_callback.messages[0]);
}

TEST(StarlarkStr, NamedArguments) {
  starlark_integer one(1);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  named_args["1"] = &one;
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_str(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: str() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkType, NoArguments) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;

  auto* result = starlark_fn_tuple(pos_args, named_args, arena, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "()");
}

TEST(StarlarkTuple, OneArguments) {
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_list list(3);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  list.add(&one, error_callback);
  list.add(&two, error_callback);
  list.add(&one, error_callback);
  pos_args.push_back(&list);

  auto* result = starlark_fn_tuple(pos_args, named_args, arena, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->repr(), "(1, 2, 1)");
}

TEST(StarlarkTuple, String) {
  Arena arena;
  error_handler error_callback;
  starlark_string str("abc");

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&str);

  EXPECT_EQ(nullptr, starlark_fn_tuple(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: 'string' object is not iterable", error_callback.messages[0]);
}

TEST(StarlarkTuple, TooManyPosArguments) {
  starlark_list list(0);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&list);
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_tuple(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: tuple expected at most 1 argument, got 2", error_callback.messages[0]);
}

TEST(StarlarkTuple, NamedArguments) {
  starlark_list list(0);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  named_args["1"] = &list;
  pos_args.push_back(&list);

  EXPECT_EQ(nullptr, starlark_fn_tuple(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: tuple() takes no keyword arguments", error_callback.messages[0]);
}

TEST(StarlarkType, String) {
  Arena arena;
  error_handler error_callback;
  starlark_string str("abc");

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&str);

  auto* result = starlark_fn_type(pos_args, named_args, arena, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->as_string(), "string");
}

TEST(StarlarkType, TooFewPosArguments) {
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;

  EXPECT_EQ(nullptr, starlark_fn_type(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: type() takes exactly one argument (0 given)", error_callback.messages[0]);
}

TEST(StarlarkType, TooManyPosArguments) {
  starlark_integer one(1);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  pos_args.push_back(&one);
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_type(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: type() takes exactly one argument (2 given)", error_callback.messages[0]);
}

TEST(StarlarkType, NamedArguments) {
  starlark_integer one(1);
  Arena arena;
  error_handler error_callback;

  std::vector<starlark_obj*> pos_args;
  std::map<std::string, starlark_obj*> named_args;
  named_args["1"] = &one;
  pos_args.push_back(&one);

  EXPECT_EQ(nullptr, starlark_fn_type(pos_args, named_args, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: type() takes no keyword arguments", error_callback.messages[0]);
}

}  // namespace
