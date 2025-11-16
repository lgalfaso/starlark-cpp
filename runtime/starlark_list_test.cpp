// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <vector>

#include "runtime/starlark_bool.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_list.hpp"
#include "runtime/starlark_none.hpp"

using ::starlark::runtime::starlark_bool;
using ::starlark::runtime::starlark_integer;
using ::starlark::runtime::starlark_list;
using ::starlark::runtime::starlark_none;
using ::starlark::runtime::starlark_obj;
using ::testing::SizeIs;

namespace {

TEST(StarlarkList, Type) {
  EXPECT_EQ("list", starlark_list().type());
}

TEST(StarlarkList, Str) {
  starlark_none none;
  starlark_bool true_obj(true);
  starlark_integer one(1);
  starlark_list list;
  EXPECT_EQ("[]", list.str());
  list.add(&none);
  EXPECT_EQ("[None]", list.str());
  list.add(&true_obj);
  EXPECT_EQ("[None, True]", list.str());
  list.add(&one);
  EXPECT_EQ("[None, True, 1]", list.str());
}

TEST(StarlarkList, StrRecursion) {
  // Bazel prints `[1, [1, ..., 1], 1]`, Python prints `[1, [...], 1]`.
  starlark_list list;
  starlark_integer one(1);
  list.add(&one);
  list.add(&list);
  list.add(&one);
  EXPECT_EQ("[1, [...], 1]", list.str());
}

TEST(StarlarkList, Truthy) {
  starlark_none none;
  starlark_list list;
  EXPECT_FALSE(list.truthy());
  list.add(&none);
  EXPECT_TRUE(list.truthy());
}

TEST(StarlarkList, Equals) {
  starlark_none none;
  starlark_integer one(1);
  starlark_list list1;
  starlark_list list2;
  starlark_list list3;
  starlark_list list4;
  starlark_list list5;
  list2.add(&none);
  list3.add(&one);
  list4.add(&none);
  list4.add(&one);
  list5.add(&one);
  list5.add(&none);

  EXPECT_TRUE(list1.equals(list1));
  EXPECT_FALSE(list2.equals(list1));
  EXPECT_FALSE(list3.equals(list1));
  EXPECT_FALSE(list4.equals(list1));
  EXPECT_FALSE(list5.equals(list1));

  EXPECT_FALSE(list1.equals(list2));
  EXPECT_TRUE(list2.equals(list2));
  EXPECT_FALSE(list3.equals(list2));
  EXPECT_FALSE(list4.equals(list2));
  EXPECT_FALSE(list5.equals(list2));

  EXPECT_FALSE(list1.equals(list3));
  EXPECT_FALSE(list2.equals(list3));
  EXPECT_TRUE(list3.equals(list3));
  EXPECT_FALSE(list4.equals(list3));
  EXPECT_FALSE(list5.equals(list3));

  EXPECT_FALSE(list1.equals(list4));
  EXPECT_FALSE(list2.equals(list4));
  EXPECT_FALSE(list3.equals(list4));
  EXPECT_TRUE(list4.equals(list4));
  EXPECT_FALSE(list5.equals(list4));

  EXPECT_FALSE(list1.equals(list5));
  EXPECT_FALSE(list2.equals(list5));
  EXPECT_FALSE(list3.equals(list5));
  EXPECT_FALSE(list4.equals(list5));
  EXPECT_TRUE(list5.equals(list5));
}

TEST(StarlarkList, EqualsRecursion) {
  starlark_list list_a;
  starlark_list list_b;
  list_a.add(&list_b);
  list_b.add(&list_a);
  EXPECT_TRUE(list_a.equals(list_b));
}

TEST(StarlarkList, SequenceSize) {
  starlark_none none;
  starlark_integer one(1);
  starlark_list list;
  EXPECT_EQ(0, list.sequence_size());
  list.add(&one);
  EXPECT_EQ(1, list.sequence_size());
  list.add(&none);
  EXPECT_EQ(2, list.sequence_size());
}

TEST(StarlarkList, Unpack) {
  starlark_none none;
  starlark_integer one(1);
  starlark_list list;
  std::vector<starlark_obj*> stack;

  list.unpack(0, stack, nullptr);
  EXPECT_THAT(stack, SizeIs(0));

  list.add(&one);
  list.unpack(1, stack, nullptr);
  ASSERT_THAT(stack, SizeIs(1));
  EXPECT_THAT(stack[0], &one);

  stack.clear();
  list.add(&none);
  list.unpack(2, stack, nullptr);
  ASSERT_THAT(stack, SizeIs(2));
  EXPECT_THAT(stack[0], &none);
  EXPECT_THAT(stack[1], &one);
}

// TODO(lmirelmann): Test unpack when the number of elements do not match.

}  // namespace
