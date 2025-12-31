// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "runtime/starlark_bigint.hpp"
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
  starlark_integer one(1);
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
  starlark_integer one(1);
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

}  // namespace
