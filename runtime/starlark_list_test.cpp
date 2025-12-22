// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <string>
#include <vector>

#include "runtime/starlark_bigint.hpp"
#include "runtime/starlark_bool.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_list.hpp"
#include "runtime/starlark_none.hpp"
#include "runtime/starlark_testing.hpp"
#include "runtime/starlark_tuple.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;
using ::starlark::runtime::starlark_bigint;
using ::starlark::runtime::starlark_bool;
using ::starlark::runtime::starlark_integer;
using ::starlark::runtime::starlark_list;
using ::starlark::runtime::starlark_none;
using ::starlark::runtime::starlark_obj;
using ::starlark::runtime::starlark_tuple;
using ::starlark::testing::error_handler;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::IsEmpty;
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
  error_handler error_callback;

  EXPECT_EQ("[]", list.str());
  list.add(&none, error_callback);
  EXPECT_EQ("[None]", list.str());
  list.add(&true_obj, error_callback);
  EXPECT_EQ("[None, True]", list.str());
  list.add(&one, error_callback);
  EXPECT_EQ("[None, True, 1]", list.str());
}

TEST(StarlarkList, StrRecursion) {
  // Bazel prints `[1, [1, ..., 1], 1]`, Python prints `[1, [...], 1]`.
  starlark_list list;
  starlark_integer one(1);
  error_handler error_callback;

  list.add(&one, error_callback);
  list.add(&list, error_callback);
  list.add(&one, error_callback);
  EXPECT_EQ("[1, [...], 1]", list.str());
}

TEST(StarlarkList, Truthy) {
  starlark_none none;
  starlark_list list;
  error_handler error_callback;

  EXPECT_FALSE(list.truthy());
  list.add(&none, error_callback);
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
  error_handler error_callback;

  list2.add(&none, error_callback);
  list3.add(&one, error_callback);
  list4.add(&none, error_callback);
  list4.add(&one, error_callback);
  list5.add(&one, error_callback);
  list5.add(&none, error_callback);

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
  error_handler error_callback;

  list_a.add(&list_b, error_callback);
  list_b.add(&list_a, error_callback);
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
  error_handler error_callback;

  list_a.add(&list_b, error_callback);
  list_b.add(&list_a, error_callback);
  EXPECT_EQ(-1, list_a.hash());
}

TEST(StarlarkList, HashRecursionFreezed) {
  starlark_list list_a;
  starlark_list list_b;
  error_handler error_callback;

  list_a.add(&list_b, error_callback);
  list_b.add(&list_a, error_callback);
  list_a.freeze();
  EXPECT_EQ(-1, list_a.hash());
}

TEST(StarlarkList, Unpack) {
  starlark_none none;
  starlark_integer one(1);
  starlark_list list;
  std::vector<starlark_obj*> stack;
  error_handler error_callback;

  list.unpack(0, stack, error_callback);
  EXPECT_THAT(stack, SizeIs(0));

  list.add(&one, error_callback);
  list.unpack(1, stack, error_callback);
  ASSERT_THAT(stack, SizeIs(1));
  EXPECT_THAT(stack[0], &one);

  stack.clear();
  list.add(&none, error_callback);
  list.unpack(2, stack, error_callback);
  ASSERT_THAT(stack, SizeIs(2));
  EXPECT_THAT(stack[0], &none);
  EXPECT_THAT(stack[1], &one);
}

TEST(StarlarkList, UnpackError) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_list list;
  error_handler error_callback;

  list.add(&zero, error_callback);
  list.add(&one, error_callback);
  {
    std::vector<starlark_obj*> consumer;
    error_handler error_callback;

    list.unpack(3, consumer, error_callback);
    ASSERT_THAT(consumer, IsEmpty());
    EXPECT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "ValueError: not enough values to unpack (expected 3, got 2)");
  }
  {
    std::vector<starlark_obj*> consumer;
    error_handler error_callback;

    list.unpack(1, consumer, error_callback);
    ASSERT_THAT(consumer, IsEmpty());
    EXPECT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "ValueError: too many values to unpack (expected 1, got 2)");
  }
}

TEST(StarlarkList, Order) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_list list1;
  starlark_list list2;
  error_handler error_callback;

  list2.add(&zero, error_callback);
  starlark_list list3;
  list3.add(&zero, error_callback);
  list3.add(&one, error_callback);
  starlark_list list4;
  list4.add(&one, error_callback);
  starlark_list list5;
  list5.add(&one, error_callback);
  list5.add(&zero, error_callback);

  EXPECT_THAT(list1.cmp(list1, "cmp", error_callback), Eq(0));
  EXPECT_THAT(list1.cmp(list2, "cmp", error_callback), Lt(0));
  EXPECT_THAT(list1.cmp(list3, "cmp", error_callback), Lt(0));
  EXPECT_THAT(list1.cmp(list4, "cmp", error_callback), Lt(0));
  EXPECT_THAT(list1.cmp(list5, "cmp", error_callback), Lt(0));

  EXPECT_THAT(list2.cmp(list1, "cmp", error_callback), Gt(0));
  EXPECT_THAT(list2.cmp(list2, "cmp", error_callback), Eq(0));
  EXPECT_THAT(list2.cmp(list3, "cmp", error_callback), Lt(0));
  EXPECT_THAT(list2.cmp(list4, "cmp", error_callback), Lt(0));
  EXPECT_THAT(list2.cmp(list5, "cmp", error_callback), Lt(0));

  EXPECT_THAT(list3.cmp(list1, "cmp", error_callback), Gt(0));
  EXPECT_THAT(list3.cmp(list2, "cmp", error_callback), Gt(0));
  EXPECT_THAT(list3.cmp(list3, "cmp", error_callback), Eq(0));
  EXPECT_THAT(list3.cmp(list4, "cmp", error_callback), Lt(0));
  EXPECT_THAT(list3.cmp(list5, "cmp", error_callback), Lt(0));

  EXPECT_THAT(list4.cmp(list1, "cmp", error_callback), Gt(0));
  EXPECT_THAT(list4.cmp(list2, "cmp", error_callback), Gt(0));
  EXPECT_THAT(list4.cmp(list3, "cmp", error_callback), Gt(0));
  EXPECT_THAT(list4.cmp(list4, "cmp", error_callback), Eq(0));
  EXPECT_THAT(list4.cmp(list5, "cmp", error_callback), Lt(0));

  EXPECT_THAT(list5.cmp(list1, "cmp", error_callback), Gt(0));
  EXPECT_THAT(list5.cmp(list2, "cmp", error_callback), Gt(0));
  EXPECT_THAT(list5.cmp(list3, "cmp", error_callback), Gt(0));
  EXPECT_THAT(list5.cmp(list4, "cmp", error_callback), Gt(0));
  EXPECT_THAT(list5.cmp(list5, "cmp", error_callback), Eq(0));
}

TEST(StarlarkList, OrderError) {
  error_handler error_callback;
  starlark_integer one(1);
  starlark_list list;

  EXPECT_FALSE(list.cmp(one, "<", error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: '<' not supported between instances of 'list' and 'int'");
}

TEST(StarlarkList, AddWithFreeze) {
  error_handler error_callback;
  starlark_integer one(1);
  starlark_list list;
  list.add(&one, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  list.freeze();
  list.add(&one, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: trying to mutate a frozen list value");
}

TEST(StarlarkList, AddWithMultipleFreeze) {
  error_handler error_callback;
  starlark_integer one(1);
  starlark_list list;
  list.add(&one, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  list.freeze();
  list.freeze();
  list.add(&one, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: trying to mutate a frozen list value");
}

TEST(StarlarkList, Membership) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_list list;
  error_handler error_callback;

  list.add(&zero, error_callback);

  EXPECT_TRUE(list.binary_in(zero, error_callback));
  EXPECT_FALSE(list.binary_in(one, error_callback));
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkList, Call) {
  error_handler error_callback;
  starlark_list list;
  Arena arena;

  list.call({}, {}, arena, error_callback);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'list' object is not callable");
}

TEST(StarlarkList, UnaryPlus) {
  error_handler error_callback;
  starlark_list list;
  Arena arena;

  list.unary_plus(arena, error_callback);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: bad operand type for unary +: 'list'");
}

TEST(StarlarkList, UnaryMinus) {
  error_handler error_callback;
  starlark_list list;
  Arena arena;

  list.unary_minus(arena, error_callback);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: bad operand type for unary -: 'list'");
}

TEST(StarlarkList, UnaryTilde) {
  error_handler error_callback;
  starlark_list list;
  Arena arena;

  list.unary_tilde(arena, error_callback);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: bad operand type for unary ~: 'list'");
}

TEST(StarlarkList, BinaryPlus) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_list list_1;
  starlark_list list_2;
  error_handler error_callback;

  list_1.add(&zero, error_callback);
  list_2.add(&one, error_callback);
  Arena arena;

  auto* result = list_1.binary_plus(list_2, arena, error_callback);

  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->str(), "[0, 1]");
}

TEST(StarlarkList, BinaryPlusNotList) {
  starlark_list list;
  starlark_tuple tuple;
  Arena arena;
  error_handler error_callback;

  auto* result = list.binary_plus(tuple, arena, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: can only concatenate list (not \"tuple\") to list");
}

TEST(StarlarkList, BinaryStar) {
  starlark_bigint minus_two(number::minus_one << 1);
  starlark_integer minus_one(-1);
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_bigint three(3);
  starlark_list list0;
  starlark_list list1;
  error_handler error_callback;

  list1.add(&zero, error_callback);
  list1.add(&one, error_callback);
  Arena arena;

  auto* result_1 = list1.binary_star(two, arena, error_callback);
  auto* result_2 = list1.binary_star(three, arena, error_callback);
  auto* result_3 = list0.binary_star(three, arena, error_callback);
  auto* result_4 = list1.binary_star(minus_one, arena, error_callback);
  auto* result_5 = list1.binary_star(minus_two, arena, error_callback);

  ASSERT_NE(result_1, nullptr);
  EXPECT_EQ(result_1->str(), "[0, 1, 0, 1]");
  ASSERT_NE(result_2, nullptr);
  EXPECT_EQ(result_2->str(), "[0, 1, 0, 1, 0, 1]");
  ASSERT_NE(result_3, nullptr);
  EXPECT_EQ(result_3->str(), "[]");
  ASSERT_NE(result_4, nullptr);
  EXPECT_EQ(result_4->str(), "[]");
  ASSERT_NE(result_5, nullptr);
  EXPECT_EQ(result_5->str(), "[]");
}

TEST(StarlarkList, BinaryStarReverse) {
  starlark_bigint minus_two(number::minus_one << 1);
  starlark_integer minus_one(-1);
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_bigint three(3);
  starlark_list list0;
  starlark_list list1;
  error_handler error_callback;

  list1.add(&zero, error_callback);
  list1.add(&one, error_callback);
  Arena arena;

  auto* result_1 = two.binary_star(list1, arena, error_callback);
  auto* result_2 = three.binary_star(list1, arena, error_callback);
  auto* result_3 = three.binary_star(list0, arena, error_callback);
  auto* result_4 = minus_one.binary_star(list1, arena, error_callback);
  auto* result_5 = minus_two.binary_star(list1, arena, error_callback);

  ASSERT_NE(result_1, nullptr);
  EXPECT_EQ(result_1->str(), "[0, 1, 0, 1]");
  ASSERT_NE(result_2, nullptr);
  EXPECT_EQ(result_2->str(), "[0, 1, 0, 1, 0, 1]");
  ASSERT_NE(result_3, nullptr);
  EXPECT_EQ(result_3->str(), "[]");
  ASSERT_NE(result_4, nullptr);
  EXPECT_EQ(result_4->str(), "[]");
  ASSERT_NE(result_5, nullptr);
  EXPECT_EQ(result_5->str(), "[]");
}

TEST(StarlarkList, BinaryStarNotInt) {
  starlark_list list;
  starlark_tuple tuple;
  Arena arena;
  error_handler error_callback;

  auto* result = list.binary_star(tuple, arena, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: can't multiply sequence by non-int of type 'tuple'");
}

TEST(StarlarkList, BinaryStarTooBig) {
  starlark_list list;
  starlark_bigint big(number::one << 64);
  Arena arena;
  error_handler error_callback;
  list.add(&big, error_callback);

  auto* result = list.binary_star(big, arena, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: sequences must be at most 2147483647 elements");
}


TEST(StarlarkList, Len) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_list list0;
  starlark_list list1;
  error_handler error_callback;

  list1.add(&zero, error_callback);
  list1.add(&one, error_callback);

  EXPECT_EQ(0, list0.len(error_callback));
  EXPECT_EQ(2, list1.len(error_callback));
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

}  // namespace
