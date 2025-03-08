// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include "compiler/starlark_bool.hpp"
#include "compiler/starlark_dictionary.hpp"
#include "compiler/starlark_integer.hpp"
#include "compiler/starlark_none.hpp"
#include "compiler/starlark_string.hpp"

using starlark::compiler::starlark_bool;
using starlark::compiler::starlark_dictionary;
using starlark::compiler::starlark_integer;
using starlark::compiler::starlark_none;
using starlark::compiler::starlark_string;

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

}  // namespace
