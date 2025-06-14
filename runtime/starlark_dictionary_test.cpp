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
  EXPECT_EQ("{'1': None}", starlark_dictionary().insert(&s1, &none).str());
  EXPECT_EQ("{'1': None, '2': True}", starlark_dictionary().insert(&s1, &none).insert(&s2, &true_obj).str());
  EXPECT_EQ("{'1': None, '2': True, '3': 1}", starlark_dictionary().insert(&s1, &none).insert(&s2, &true_obj).insert(&s3, &one).str());
}

TEST(StarlarkDictionary, StrOrder) {
  starlark_string s1("1");
  starlark_string s2("2");
  starlark_string s3("3");
  starlark_none none;
  starlark_bool true_obj(true);
  starlark_integer one(1);
  EXPECT_EQ("{'1': None, '2': True, '3': 1}", starlark_dictionary().insert(&s1, &none).insert(&s2, &true_obj).insert(&s3, &one).str());
  EXPECT_EQ("{'3': 1, '1': None, '2': True}", starlark_dictionary().insert(&s3, &one).insert(&s1, &none).insert(&s2, &true_obj).str());
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
  map.insert(&s1, &none).insert(&s2, &true_obj).insert(&s3, &one).insert(&s4, &map);
  EXPECT_EQ("{'1': None, '2': True, '3': 1, '4': {...}}", map.str());
}

TEST(StarlarkDictionary, Truthy) {
  starlark_none none;
  EXPECT_FALSE(starlark_dictionary().truthy());
  EXPECT_TRUE(starlark_dictionary().insert(&none, &none).truthy());
}

TEST(StarlarkDictionary, Equals) {
  starlark_none none;
  starlark_bool bool_true(true);
  EXPECT_FALSE(starlark_dictionary().equals(none));
  EXPECT_TRUE(starlark_dictionary().equals(starlark_dictionary()));
  EXPECT_FALSE(starlark_dictionary().insert(&none, &none).equals(starlark_dictionary()));
  EXPECT_FALSE(starlark_dictionary().equals(starlark_dictionary().insert(&none, &none)));
  EXPECT_TRUE(starlark_dictionary().insert(&none, &none).equals(starlark_dictionary().insert(&none, &none)));
  EXPECT_FALSE(starlark_dictionary().insert(&none, &bool_true).equals(starlark_dictionary().insert(&none, &none)));
  EXPECT_FALSE(starlark_dictionary().insert(&bool_true, &none).equals(starlark_dictionary().insert(&none, &none)));
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
  dict_1.insert(&none, &none).insert(&zero, &zero).insert(&one, &one).insert(&two, &two).insert(&bool_true, &bool_true).insert(&bool_false, &bool_false);
  dict_2.insert(&none, &none).insert(&zero, &zero).insert(&one, &one).insert(&two, &two).insert(&bool_true, &bool_true).insert(&bool_false, &bool_false);
  dict_3.insert(&bool_false, &bool_false).insert(&bool_true, &bool_true).insert(&two, &two).insert(&one, &one).insert(&zero, &zero).insert(&none, &none);
  EXPECT_TRUE(dict_1.equals(dict_2));
  EXPECT_TRUE(dict_1.equals(dict_3));
}

TEST(StarlarkDictionary, Hash) {
  EXPECT_EQ(starlark_dictionary().hash(), -1);
}

}  // namespace
