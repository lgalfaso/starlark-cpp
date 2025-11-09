// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include "runtime/starlark_bool.hpp"
#include "runtime/starlark_dictionary.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_none.hpp"
#include "runtime/starlark_string.hpp"

using starlark::runtime::starlark_bool;
using starlark::runtime::starlark_dictionary;
using starlark::runtime::starlark_integer;
using starlark::runtime::starlark_none;
using starlark::runtime::starlark_string;

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
  EXPECT_EQ("{}", starlark_dictionary().str());
  starlark_dictionary dict;
  dict.insert(&s1, &none);
  EXPECT_EQ("{'1': None}", dict.str());
  dict.insert(&s2, &true_obj);
  EXPECT_EQ("{'1': None, '2': True}", dict.str());
  dict.insert(&s3, &one);
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
  dict_1.insert(&s1, &none);
  dict_1.insert(&s2, &true_obj);
  dict_1.insert(&s3, &one);
  EXPECT_EQ("{'1': None, '2': True, '3': 1}", dict_1.str());
  starlark_dictionary dict_2;
  dict_2.insert(&s3, &one);
  dict_2.insert(&s1, &none);
  dict_2.insert(&s2, &true_obj);
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
  map.insert(&s1, &none);
  map.insert(&s2, &true_obj);
  map.insert(&s3, &one);
  map.insert(&s4, &map);
  EXPECT_EQ("{'1': None, '2': True, '3': 1, '4': {...}}", map.str());
}

TEST(StarlarkDictionary, Truthy) {
  starlark_none none;
  starlark_dictionary dict;
  EXPECT_FALSE(dict.truthy());
  dict.insert(&none, &none);
  EXPECT_TRUE(dict.truthy());
}

TEST(StarlarkDictionary, Equals) {
  starlark_none none;
  starlark_bool bool_true(true);
  EXPECT_FALSE(starlark_dictionary().equals(none));
  EXPECT_TRUE(starlark_dictionary().equals(starlark_dictionary()));
  starlark_dictionary dict_1;
  dict_1.insert(&none, &none);
  starlark_dictionary dict_2;
  dict_2.insert(&none, &none);
  EXPECT_FALSE(dict_1.equals(starlark_dictionary()));
  EXPECT_FALSE(starlark_dictionary().equals(dict_1));
  EXPECT_TRUE(dict_1.equals(dict_2));
  starlark_dictionary dict_3;
  dict_3.insert(&none, &bool_true);
  EXPECT_FALSE(dict_3.equals(dict_1));
  starlark_dictionary dict_4;
  dict_4.insert(&bool_true, &none);
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
  dict_1.insert(&none, &none);
  dict_1.insert(&zero, &zero);
  dict_1.insert(&one, &one);
  dict_1.insert(&two, &two);
  dict_1.insert(&bool_true, &bool_true);
  dict_1.insert(&bool_false, &bool_false);
  dict_2.insert(&none, &none);
  dict_2.insert(&zero, &zero);
  dict_2.insert(&one, &one);
  dict_2.insert(&two, &two);
  dict_2.insert(&bool_true, &bool_true);
  dict_2.insert(&bool_false, &bool_false);
  dict_3.insert(&bool_false, &bool_false);
  dict_3.insert(&bool_true, &bool_true);
  dict_3.insert(&two, &two);
  dict_3.insert(&one, &one);
  dict_3.insert(&zero, &zero);
  dict_3.insert(&none, &none);
  EXPECT_TRUE(dict_1.equals(dict_2));
  EXPECT_TRUE(dict_1.equals(dict_3));
}

TEST(StarlarkDictionary, Hash) {
  EXPECT_EQ(starlark_dictionary().hash(), -1);
}

// TODO(lmirelmann): Test the return value of `insert`.

}  // namespace
