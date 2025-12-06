// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <string>
#include <vector>

#include "runtime/starlark_bool.hpp"
#include "runtime/starlark_dictionary.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_list.hpp"
#include "runtime/starlark_none.hpp"
#include "runtime/starlark_string.hpp"

using ::starlark::runtime::error_fn;
using ::starlark::runtime::starlark_bool;
using ::starlark::runtime::starlark_dictionary;
using ::starlark::runtime::starlark_integer;
using ::starlark::runtime::starlark_list;
using ::starlark::runtime::starlark_none;
using ::starlark::runtime::starlark_string;
using ::testing::SizeIs;

namespace {

struct error_handler : public error_fn {
  void add_error(std::string_view error_msg) override {
    messages.push_back(std::string(error_msg));
  }

  std::vector<std::string> messages;
};

TEST(StarlarkDictionary, Type) {
  EXPECT_EQ("dict", starlark_dictionary().type());
}

TEST(StarlarkDictionary, Str) {
  starlark_string s1("1");
  starlark_string s2("2");
  starlark_string s3("3");
  starlark_none none;
  starlark_bool true_obj(true);
  starlark_integer one(1);
  EXPECT_EQ("{}", starlark_dictionary().str());
  starlark_dictionary dict;
  dict.insert(&s1, &none, nullptr);
  EXPECT_EQ("{'1': None}", dict.str());
  dict.insert(&s2, &true_obj, nullptr);
  EXPECT_EQ("{'1': None, '2': True}", dict.str());
  dict.insert(&s3, &one, nullptr);
  EXPECT_EQ("{'1': None, '2': True, '3': 1}", dict.str());
}

TEST(StarlarkDictionary, StrOrder) {
  starlark_string s1("1");
  starlark_string s2("2");
  starlark_string s3("3");
  starlark_none none;
  starlark_bool true_obj(true);
  starlark_integer one(1);
  starlark_dictionary dict_1;
  dict_1.insert(&s1, &none, nullptr);
  dict_1.insert(&s2, &true_obj, nullptr);
  dict_1.insert(&s3, &one, nullptr);
  EXPECT_EQ("{'1': None, '2': True, '3': 1}", dict_1.str());
  starlark_dictionary dict_2;
  dict_2.insert(&s3, &one, nullptr);
  dict_2.insert(&s1, &none, nullptr);
  dict_2.insert(&s2, &true_obj, nullptr);
  EXPECT_EQ("{'3': 1, '1': None, '2': True}", dict_2.str());
}

TEST(StarlarkDictionary, StrContainsItself) {
  starlark_string s1("1");
  starlark_string s2("2");
  starlark_string s3("3");
  starlark_string s4("4");
  starlark_none none;
  starlark_bool true_obj(true);
  starlark_integer one(1);
  starlark_dictionary map;
  map.insert(&s1, &none, nullptr);
  map.insert(&s2, &true_obj, nullptr);
  map.insert(&s3, &one, nullptr);
  map.insert(&s4, &map, nullptr);
  EXPECT_EQ("{'1': None, '2': True, '3': 1, '4': {...}}", map.str());
}

TEST(StarlarkDictionary, Truthy) {
  starlark_none none;
  starlark_dictionary dict;
  EXPECT_FALSE(dict.truthy());
  dict.insert(&none, &none, nullptr);
  EXPECT_TRUE(dict.truthy());
}

TEST(StarlarkDictionary, Equals) {
  starlark_none none;
  starlark_bool bool_true(true);
  EXPECT_FALSE(starlark_dictionary().equals(none));
  EXPECT_TRUE(starlark_dictionary().equals(starlark_dictionary()));
  starlark_dictionary dict_1;
  dict_1.insert(&none, &none, nullptr);
  starlark_dictionary dict_2;
  dict_2.insert(&none, &none, nullptr);
  EXPECT_FALSE(dict_1.equals(starlark_dictionary()));
  EXPECT_FALSE(starlark_dictionary().equals(dict_1));
  EXPECT_TRUE(dict_1.equals(dict_2));
  starlark_dictionary dict_3;
  dict_3.insert(&none, &bool_true, nullptr);
  EXPECT_FALSE(dict_3.equals(dict_1));
  starlark_dictionary dict_4;
  dict_4.insert(&bool_true, &none, nullptr);
  EXPECT_FALSE(dict_4.equals(dict_1));
}

TEST(StarlarkDictionary, EqualsInDifferentOrder) {
  starlark_none none;
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_bool bool_true(true);
  starlark_bool bool_false(false);
  starlark_dictionary dict_1;
  starlark_dictionary dict_2;
  starlark_dictionary dict_3;
  dict_1.insert(&none, &none, nullptr);
  dict_1.insert(&zero, &zero, nullptr);
  dict_1.insert(&one, &one, nullptr);
  dict_1.insert(&two, &two, nullptr);
  dict_1.insert(&bool_true, &bool_true, nullptr);
  dict_1.insert(&bool_false, &bool_false, nullptr);
  dict_2.insert(&none, &none, nullptr);
  dict_2.insert(&zero, &zero, nullptr);
  dict_2.insert(&one, &one, nullptr);
  dict_2.insert(&two, &two, nullptr);
  dict_2.insert(&bool_true, &bool_true, nullptr);
  dict_2.insert(&bool_false, &bool_false, nullptr);
  dict_3.insert(&bool_false, &bool_false, nullptr);
  dict_3.insert(&bool_true, &bool_true, nullptr);
  dict_3.insert(&two, &two, nullptr);
  dict_3.insert(&one, &one, nullptr);
  dict_3.insert(&zero, &zero, nullptr);
  dict_3.insert(&none, &none, nullptr);
  EXPECT_TRUE(dict_1.equals(dict_2));
  EXPECT_TRUE(dict_1.equals(dict_3));
}

TEST(StarlarkDictionary, Hash) {
  starlark_dictionary dict;
  EXPECT_EQ(dict.hash(), -1);
  dict.freeze();
  EXPECT_EQ(dict.hash(), -1);
}

TEST(StarlarkDictionary, InsertingUsingUnhashableKey) {
  starlark_dictionary dict;
  starlark_list list;
  starlark_none none;
  error_handler error_callback;

  EXPECT_FALSE(dict.insert(&list, &none, &error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: cannot use 'list' as a dict key (unhashable type: 'list')", error_callback.messages[0]);
}

TEST(StarlarkDictionary, InsertReturnValue) {
  starlark_dictionary dict;
  starlark_none none;
  starlark_integer zero(0);
  EXPECT_TRUE(dict.insert(&none, &none, nullptr));
  EXPECT_FALSE(dict.insert(&none, &none, nullptr));
  EXPECT_FALSE(dict.insert(&none, &zero, nullptr));
}

TEST(StarlarkDictionary, Freeze) {
  starlark_dictionary dict1;
  starlark_dictionary dict2;
  starlark_none none;
  EXPECT_TRUE(dict1.insert(&none, &dict2, nullptr));
  dict1.freeze();
  EXPECT_FALSE(dict2.insert(&none, &none, nullptr));
}

TEST(StarlarkDictionary, InsertFreezed) {
  starlark_dictionary dict;
  starlark_none none;
  error_handler error_callback;

  dict.freeze();

  EXPECT_FALSE(dict.insert(&none, &none, &error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: trying to mutate a frozen dict value");
}

TEST(StarlarkDictionary, BinaryPipe) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_integer three(3);
  starlark_string s_zero("zero");
  starlark_string s_one("one");
  starlark_string s_two("two");
  starlark_string s_three("three");
  starlark_string s_four("four");
  starlark_dictionary dict_1;
  starlark_dictionary dict_2;
  google::protobuf::Arena arena;
  error_handler error_callback;

  dict_1.insert(&zero, &s_zero, nullptr);
  dict_1.insert(&one, &s_one, nullptr);
  dict_2.insert(&zero, &s_four, nullptr);
  dict_2.insert(&two, &s_two, nullptr);
  dict_2.insert(&three, &s_three, nullptr);

  auto* dict_3 = dict_1.binary_pipe(dict_2,  arena, &error_callback);
  ASSERT_NE(dict_3, nullptr);
  EXPECT_EQ(dict_3->str(), "{0: 'four', 1: 'one', 2: 'two', 3: 'three'}");
  EXPECT_EQ(dict_1.str(), "{0: 'zero', 1: 'one'}");
  EXPECT_EQ(dict_2.str(), "{0: 'four', 2: 'two', 3: 'three'}");
}

TEST(StarlarkDictionary, BinaryPipeWithNonDict) {
  starlark_dictionary dict;
  starlark_list list;
  google::protobuf::Arena arena;
  error_handler error_callback;

  auto* result = dict.binary_pipe(list, arena, &error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for |: 'dict' and 'list'");
}

}  // namespace
