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
#include "runtime/starlark_testing.hpp"

using ::google::protobuf::Arena;
using ::starlark::runtime::starlark_bool;
using ::starlark::runtime::starlark_dictionary;
using ::starlark::runtime::starlark_integer;
using ::starlark::runtime::starlark_list;
using ::starlark::runtime::starlark_none;
using ::starlark::runtime::starlark_string;
using ::starlark::testing::error_handler;
using ::testing::IsEmpty;
using ::testing::SizeIs;

namespace {

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
  error_handler error_callback;

  EXPECT_EQ("{}", starlark_dictionary().str());
  starlark_dictionary dict;
  dict.insert(&s1, &none, error_callback);
  EXPECT_EQ("{\"1\": None}", dict.str());
  dict.insert(&s2, &true_obj, error_callback);
  EXPECT_EQ("{\"1\": None, \"2\": True}", dict.str());
  dict.insert(&s3, &one, error_callback);
  EXPECT_EQ("{\"1\": None, \"2\": True, \"3\": 1}", dict.str());
}

TEST(StarlarkDictionary, StrOrder) {
  starlark_string s1("1");
  starlark_string s2("2");
  starlark_string s3("3");
  starlark_none none;
  starlark_bool true_obj(true);
  starlark_integer one(1);
  starlark_dictionary dict_1;
  error_handler error_callback;

  dict_1.insert(&s1, &none, error_callback);
  dict_1.insert(&s2, &true_obj, error_callback);
  dict_1.insert(&s3, &one, error_callback);
  EXPECT_EQ("{\"1\": None, \"2\": True, \"3\": 1}", dict_1.str());
  starlark_dictionary dict_2;
  dict_2.insert(&s3, &one, error_callback);
  dict_2.insert(&s1, &none, error_callback);
  dict_2.insert(&s2, &true_obj, error_callback);
  EXPECT_EQ("{\"3\": 1, \"1\": None, \"2\": True}", dict_2.str());
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
  error_handler error_callback;

  map.insert(&s1, &none, error_callback);
  map.insert(&s2, &true_obj, error_callback);
  map.insert(&s3, &one, error_callback);
  map.insert(&s4, &map, error_callback);
  EXPECT_EQ("{\"1\": None, \"2\": True, \"3\": 1, \"4\": {...}}", map.str());
}

TEST(StarlarkDictionary, Truthy) {
  starlark_none none;
  starlark_dictionary dict;
  error_handler error_callback;

  EXPECT_FALSE(dict.truthy());
  dict.insert(&none, &none, error_callback);
  EXPECT_TRUE(dict.truthy());
}

TEST(StarlarkDictionary, Equals) {
  starlark_none none;
  starlark_bool bool_true(true);
  error_handler error_callback;

  EXPECT_FALSE(starlark_dictionary().equals(none));
  EXPECT_TRUE(starlark_dictionary().equals(starlark_dictionary()));
  starlark_dictionary dict_1;
  dict_1.insert(&none, &none, error_callback);
  starlark_dictionary dict_2;
  dict_2.insert(&none, &none, error_callback);
  EXPECT_FALSE(dict_1.equals(starlark_dictionary()));
  EXPECT_FALSE(starlark_dictionary().equals(dict_1));
  EXPECT_TRUE(dict_1.equals(dict_2));
  starlark_dictionary dict_3;
  dict_3.insert(&none, &bool_true, error_callback);
  EXPECT_FALSE(dict_3.equals(dict_1));
  starlark_dictionary dict_4;
  dict_4.insert(&bool_true, &none, error_callback);
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
  error_handler error_callback;

  dict_1.insert(&none, &none, error_callback);
  dict_1.insert(&zero, &zero, error_callback);
  dict_1.insert(&one, &one, error_callback);
  dict_1.insert(&two, &two, error_callback);
  dict_1.insert(&bool_true, &bool_true, error_callback);
  dict_1.insert(&bool_false, &bool_false, error_callback);
  dict_2.insert(&none, &none, error_callback);
  dict_2.insert(&zero, &zero, error_callback);
  dict_2.insert(&one, &one, error_callback);
  dict_2.insert(&two, &two, error_callback);
  dict_2.insert(&bool_true, &bool_true, error_callback);
  dict_2.insert(&bool_false, &bool_false, error_callback);
  dict_3.insert(&bool_false, &bool_false, error_callback);
  dict_3.insert(&bool_true, &bool_true, error_callback);
  dict_3.insert(&two, &two, error_callback);
  dict_3.insert(&one, &one, error_callback);
  dict_3.insert(&zero, &zero, error_callback);
  dict_3.insert(&none, &none, error_callback);
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

  EXPECT_FALSE(dict.insert(&list, &none, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("TypeError: cannot use 'list' as a dict key (unhashable type: 'list')", error_callback.messages[0]);
}

TEST(StarlarkDictionary, InsertReturnValue) {
  starlark_dictionary dict;
  starlark_none none;
  starlark_integer zero(0);
  error_handler error_callback;

  EXPECT_TRUE(dict.insert(&none, &none, error_callback));
  EXPECT_FALSE(dict.insert(&none, &none, error_callback));
  EXPECT_FALSE(dict.insert(&none, &zero, error_callback));
}

TEST(StarlarkDictionary, Freeze) {
  starlark_dictionary dict1;
  starlark_dictionary dict2;
  starlark_none none;
  error_handler error_callback;

  EXPECT_TRUE(dict1.insert(&none, &dict2, error_callback));
  dict1.freeze();
  EXPECT_FALSE(dict2.insert(&none, &none, error_callback));
}

TEST(StarlarkDictionary, InsertFreezed) {
  starlark_dictionary dict;
  starlark_none none;
  error_handler error_callback;

  dict.freeze();

  EXPECT_FALSE(dict.insert(&none, &none, error_callback));
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
  Arena arena;
  error_handler error_callback;

  dict_1.insert(&zero, &s_zero, error_callback);
  dict_1.insert(&one, &s_one, error_callback);
  dict_2.insert(&zero, &s_four, error_callback);
  dict_2.insert(&two, &s_two, error_callback);
  dict_2.insert(&three, &s_three, error_callback);

  auto* dict_3 = dict_1.binary_pipe(dict_2,  arena, error_callback);
  ASSERT_NE(dict_3, nullptr);
  EXPECT_EQ(dict_3->str(), "{0: \"four\", 1: \"one\", 2: \"two\", 3: \"three\"}");
  EXPECT_EQ(dict_1.str(), "{0: \"zero\", 1: \"one\"}");
  EXPECT_EQ(dict_2.str(), "{0: \"four\", 2: \"two\", 3: \"three\"}");
}

TEST(StarlarkDictionary, BinaryPipeWithNonDict) {
  starlark_dictionary dict;
  starlark_list list;
  Arena arena;
  error_handler error_callback;

  auto* result = dict.binary_pipe(list, arena, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for |: 'dict' and 'list'");
}

TEST(StarlarkDictionary, Len) {
  starlark_dictionary dict_1;
  starlark_dictionary dict_2;
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_integer three(3);
  starlark_string s_zero("zero");
  starlark_string s_one("one");
  starlark_string s_two("two");
  starlark_string s_three("three");
  error_handler error_callback;

  dict_2.insert(&zero, &s_zero, error_callback);
  dict_2.insert(&one, &s_one, error_callback);
  dict_2.insert(&two, &s_two, error_callback);
  dict_2.insert(&three, &s_three, error_callback);

  EXPECT_EQ(0, dict_1.len(error_callback));
  EXPECT_EQ(4, dict_2.len(error_callback));
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkDictionary, GetIterator) {
  starlark_dictionary dictionary0;
  starlark_dictionary dictionary1;
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_string s_zero("key0");
  starlark_string s_one("key1");
  Arena arena;
  error_handler error_callback;
  dictionary1.insert(&s_zero, &zero, error_callback);
  dictionary1.insert(&s_one, &one, error_callback);

  auto* it0 = dictionary0.get_iterator(arena, error_callback);
  EXPECT_FALSE(it0->has_next());
  it0->end_iterator();

  auto* it1 = dictionary1.get_iterator(arena, error_callback);
  EXPECT_TRUE(it1->has_next());
  EXPECT_TRUE(it1->next()->equals(s_zero));
  EXPECT_TRUE(it1->has_next());
  EXPECT_TRUE(it1->next()->equals(s_one));
  EXPECT_FALSE(it1->has_next());
  it1->end_iterator();
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkDictionary, MutationWhileIterating) {
  starlark_dictionary dictionary;
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_string s_zero("key0");
  starlark_string s_one("key1");
  Arena arena;
  error_handler error_callback;
  dictionary.insert(&s_zero, &zero, error_callback);

  [[maybe_unused]] auto* it = dictionary.get_iterator(arena, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());
  dictionary.insert(&s_one, &one, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("Error in append: dictionary value is temporarily immutable due to active for-loop iteration", error_callback.messages[0]);
}

TEST(StarlarkDictionary, Index) {
  error_handler error_callback;
  Arena arena;
  starlark_dictionary dictionary;
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_string s_zero("key0");
  starlark_string s_one("key1");
  dictionary.insert(&s_zero, &zero, error_callback);
  dictionary.insert(&s_one, &one, error_callback);

  EXPECT_EQ(dictionary.index(s_zero, arena, error_callback)->repr(), "0");
  EXPECT_EQ(dictionary.index(s_one, arena, error_callback)->repr(), "1");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkDictionary, KeyError) {
  error_handler error_callback;
  Arena arena;
  starlark_dictionary dictionary;
  starlark_integer zero(0);
  starlark_string s_zero("key0");
  starlark_string s_one("key1");
  dictionary.insert(&s_zero, &zero, error_callback);

  EXPECT_EQ(nullptr, dictionary.index(s_one, arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("KeyError: \"key1\"", error_callback.messages[0]);
}

}  // namespace
