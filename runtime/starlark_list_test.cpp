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
using ::testing::Eq;
using ::testing::Gt;
using ::testing::Lt;
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
  list.add(&none, nullptr);
  EXPECT_EQ("[None]", list.str());
  list.add(&true_obj, nullptr);
  EXPECT_EQ("[None, True]", list.str());
  list.add(&one, nullptr);
  EXPECT_EQ("[None, True, 1]", list.str());
}

TEST(StarlarkList, StrRecursion) {
  // Bazel prints `[1, [1, ..., 1], 1]`, Python prints `[1, [...], 1]`.
  starlark_list list;
  starlark_integer one(1);
  list.add(&one, nullptr);
  list.add(&list, nullptr);
  list.add(&one, nullptr);
  EXPECT_EQ("[1, [...], 1]", list.str());
}

TEST(StarlarkList, Truthy) {
  starlark_none none;
  starlark_list list;
  EXPECT_FALSE(list.truthy());
  list.add(&none, nullptr);
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
  list2.add(&none, nullptr);
  list3.add(&one, nullptr);
  list4.add(&none, nullptr);
  list4.add(&one, nullptr);
  list5.add(&one, nullptr);
  list5.add(&none, nullptr);

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
  list_a.add(&list_b, nullptr);
  list_b.add(&list_a, nullptr);
  EXPECT_TRUE(list_a.equals(list_b));
}

TEST(StarlarkList, HashWhenNotFreezed) {
  starlark_list list_a;
  EXPECT_EQ(-1, list_a.hash());
}

TEST(StarlarkList, HashWhenFreezed) {
  starlark_list list_a;
  list_a.freeze();
  EXPECT_EQ(-1, list_a.hash());
}

TEST(StarlarkList, HashRecursion) {
  starlark_list list_a;
  starlark_list list_b;
  list_a.add(&list_b, nullptr);
  list_b.add(&list_a, nullptr);
  EXPECT_EQ(-1, list_a.hash());
}

TEST(StarlarkList, HashRecursionFreezed) {
  starlark_list list_a;
  starlark_list list_b;
  list_a.add(&list_b, nullptr);
  list_b.add(&list_a, nullptr);
  list_a.freeze();
  EXPECT_EQ(-1, list_a.hash());
}

TEST(StarlarkList, Unpack) {
  starlark_none none;
  starlark_integer one(1);
  starlark_list list;
  std::vector<starlark_obj*> stack;

  list.unpack(0, stack, nullptr);
  EXPECT_THAT(stack, SizeIs(0));

  list.add(&one, nullptr);
  list.unpack(1, stack, nullptr);
  ASSERT_THAT(stack, SizeIs(1));
  EXPECT_THAT(stack[0], &one);

  stack.clear();
  list.add(&none, nullptr);
  list.unpack(2, stack, nullptr);
  ASSERT_THAT(stack, SizeIs(2));
  EXPECT_THAT(stack[0], &none);
  EXPECT_THAT(stack[1], &one);
}

TEST(StarlarkList, Order) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_list list1;
  starlark_list list2;
  list2.add(&zero, nullptr);
  starlark_list list3;
  list3.add(&zero, nullptr);
  list3.add(&one, nullptr);
  starlark_list list4;
  list4.add(&one, nullptr);
  starlark_list list5;
  list5.add(&one, nullptr);
  list5.add(&zero, nullptr);

  EXPECT_THAT(list1.cmp(list1, "cmp", nullptr), Eq(0));
  EXPECT_THAT(list1.cmp(list2, "cmp", nullptr), Lt(0));
  EXPECT_THAT(list1.cmp(list3, "cmp", nullptr), Lt(0));
  EXPECT_THAT(list1.cmp(list4, "cmp", nullptr), Lt(0));
  EXPECT_THAT(list1.cmp(list5, "cmp", nullptr), Lt(0));

  EXPECT_THAT(list2.cmp(list1, "cmp", nullptr), Gt(0));
  EXPECT_THAT(list2.cmp(list2, "cmp", nullptr), Eq(0));
  EXPECT_THAT(list2.cmp(list3, "cmp", nullptr), Lt(0));
  EXPECT_THAT(list2.cmp(list4, "cmp", nullptr), Lt(0));
  EXPECT_THAT(list2.cmp(list5, "cmp", nullptr), Lt(0));

  EXPECT_THAT(list3.cmp(list1, "cmp", nullptr), Gt(0));
  EXPECT_THAT(list3.cmp(list2, "cmp", nullptr), Gt(0));
  EXPECT_THAT(list3.cmp(list3, "cmp", nullptr), Eq(0));
  EXPECT_THAT(list3.cmp(list4, "cmp", nullptr), Lt(0));
  EXPECT_THAT(list3.cmp(list5, "cmp", nullptr), Lt(0));

  EXPECT_THAT(list4.cmp(list1, "cmp", nullptr), Gt(0));
  EXPECT_THAT(list4.cmp(list2, "cmp", nullptr), Gt(0));
  EXPECT_THAT(list4.cmp(list3, "cmp", nullptr), Gt(0));
  EXPECT_THAT(list4.cmp(list4, "cmp", nullptr), Eq(0));
  EXPECT_THAT(list4.cmp(list5, "cmp", nullptr), Lt(0));

  EXPECT_THAT(list5.cmp(list1, "cmp", nullptr), Gt(0));
  EXPECT_THAT(list5.cmp(list2, "cmp", nullptr), Gt(0));
  EXPECT_THAT(list5.cmp(list3, "cmp", nullptr), Gt(0));
  EXPECT_THAT(list5.cmp(list4, "cmp", nullptr), Gt(0));
  EXPECT_THAT(list5.cmp(list5, "cmp", nullptr), Eq(0));
}

// TODO(lmirelmann): Test unpack when the number of elements do not match.
// TODO(lmirelmann): Test trying to add to a freezed list including the error message.

}  // namespace
