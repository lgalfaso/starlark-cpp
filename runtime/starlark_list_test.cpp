// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <string>
#include <vector>

#include "runtime/starlark_bigint.hpp"
#include "runtime/starlark_bool.hpp"
#include "runtime/starlark_float.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_list.hpp"
#include "runtime/starlark_none.hpp"
#include "runtime/starlark_string.hpp"
#include "runtime/starlark_testing.hpp"
#include "runtime/starlark_tuple.hpp"
#include "runtime/starlark_types.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;
using ::starlark::runtime::context;
using ::starlark::runtime::runtime_options;
using ::starlark::runtime::starlark_bigint;
using ::starlark::runtime::starlark_bool;
using ::starlark::runtime::starlark_float;
using ::starlark::runtime::starlark_integer;
using ::starlark::runtime::starlark_list;
using ::starlark::runtime::starlark_none;
using ::starlark::runtime::starlark_obj;
using ::starlark::runtime::starlark_string;
using ::starlark::runtime::starlark_tuple;
using ::starlark::runtime::starlark_types;
using ::starlark::testing::error_handler;
using ::std::literals::string_view_literals::operator""sv;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::IsEmpty;
using ::testing::Lt;
using ::testing::SizeIs;

namespace {

TEST(StarlarkList, Type) {
  EXPECT_EQ("list", starlark_list(0).type());
}

TEST(StarlarkList, Primitve) {
  EXPECT_FALSE(starlark_list(0).primitive());
}

TEST(StarlarkList, Str) {
  starlark_none none;
  starlark_bool true_obj(true);
  starlark_integer one(1);
  starlark_list list(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  EXPECT_EQ("[]", list.str());
  list.append(&none, ctx, error_callback);
  EXPECT_EQ("[None]", list.str());
  list.append(&true_obj, ctx, error_callback);
  EXPECT_EQ("[None, True]", list.str());
  list.append(&one, ctx, error_callback);
  EXPECT_EQ("[None, True, 1]", list.str());
}

TEST(StarlarkList, StrRecursion) {
  // Bazel prints `[1, [1, ..., 1], 1]`, Python prints `[1, [...], 1]`.
  starlark_list list(0);
  starlark_integer one(1);
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  list.append(&one, ctx, error_callback);
  list.append(&list, ctx, error_callback);
  list.append(&one, ctx, error_callback);
  EXPECT_EQ("[1, [...], 1]", list.str());
}

TEST(StarlarkList, Truthy) {
  starlark_none none;
  starlark_list list(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  EXPECT_FALSE(list.truthy());
  list.append(&none, ctx, error_callback);
  EXPECT_TRUE(list.truthy());
}

TEST(StarlarkList, Equals) {
  starlark_none none;
  starlark_integer one(1);
  starlark_list list1(0);
  starlark_list list2(0);
  starlark_list list3(0);
  starlark_list list4(0);
  starlark_list list5(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  list2.append(&none, ctx, error_callback);
  list3.append(&one, ctx, error_callback);
  list4.append(&none, ctx, error_callback);
  list4.append(&one, ctx, error_callback);
  list5.append(&one, ctx, error_callback);
  list5.append(&none, ctx, error_callback);

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
  starlark_list list_a(0);
  starlark_list list_b(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  list_a.append(&list_b, ctx, error_callback);
  list_b.append(&list_a, ctx, error_callback);
  EXPECT_TRUE(list_a.equals(list_b));
}

TEST(StarlarkList, HashWhenNotFreezed) {
  starlark_list list_a(0);
  EXPECT_EQ(-1, list_a.hash());
}

TEST(StarlarkList, HashWhenFreezed) {
  starlark_list list_a(0);
  list_a.freeze();
  EXPECT_EQ(-1, list_a.hash());
}

TEST(StarlarkList, HashRecursion) {
  starlark_list list_a(0);
  starlark_list list_b(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  list_a.append(&list_b, ctx, error_callback);
  list_b.append(&list_a, ctx, error_callback);
  EXPECT_EQ(-1, list_a.hash());
}

TEST(StarlarkList, HashRecursionFreezed) {
  starlark_list list_a(0);
  starlark_list list_b(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  list_a.append(&list_b, ctx, error_callback);
  list_b.append(&list_a, ctx, error_callback);
  list_a.freeze();
  EXPECT_EQ(-1, list_a.hash());
}

TEST(StarlarkList, Unpack) {
  starlark_none none;
  starlark_integer one(1);
  starlark_list list(0);
  std::vector<starlark_obj*> stack;
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  list.unpack(0, stack, ctx, error_callback);
  EXPECT_THAT(stack, SizeIs(0));

  list.append(&one, ctx, error_callback);
  list.unpack(1, stack, ctx, error_callback);
  ASSERT_THAT(stack, SizeIs(1));
  EXPECT_THAT(stack[0], &one);

  stack.clear();
  list.append(&none, ctx, error_callback);
  list.unpack(2, stack, ctx, error_callback);
  ASSERT_THAT(stack, SizeIs(2));
  EXPECT_THAT(stack[0], &none);
  EXPECT_THAT(stack[1], &one);
}

TEST(StarlarkList, UnpackError) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_list list(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  list.append(&zero, ctx, error_callback);
  list.append(&one, ctx, error_callback);
  {
    std::vector<starlark_obj*> consumer;
    error_handler error_callback;

    list.unpack(3, consumer, ctx, error_callback);
    ASSERT_THAT(consumer, IsEmpty());
    EXPECT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "not enough values to unpack (expected 3, got 2)");
  }
  {
    std::vector<starlark_obj*> consumer;
    error_handler error_callback;

    list.unpack(1, consumer, ctx, error_callback);
    ASSERT_THAT(consumer, IsEmpty());
    EXPECT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "too many values to unpack (expected 1, got 2)");
  }
}

void cmp_helper(starlark::result::status_or<int> cmp, auto matcher) {
  ASSERT_TRUE(cmp.ok());
  EXPECT_THAT(*cmp, matcher);
}

TEST(StarlarkList, Order) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_list list1(0);
  starlark_list list2(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  list2.append(&zero, ctx, error_callback);
  starlark_list list3(0);
  list3.append(&zero, ctx, error_callback);
  list3.append(&one, ctx, error_callback);
  starlark_list list4(0);
  list4.append(&one, ctx, error_callback);
  starlark_list list5(0);
  list5.append(&one, ctx, error_callback);
  list5.append(&zero, ctx, error_callback);

  cmp_helper(list1.cmp(list1, "cmp", error_callback), Eq(0));
  cmp_helper(list1.cmp(list2, "cmp", error_callback), Lt(0));
  cmp_helper(list1.cmp(list3, "cmp", error_callback), Lt(0));
  cmp_helper(list1.cmp(list4, "cmp", error_callback), Lt(0));
  cmp_helper(list1.cmp(list5, "cmp", error_callback), Lt(0));

  cmp_helper(list2.cmp(list1, "cmp", error_callback), Gt(0));
  cmp_helper(list2.cmp(list2, "cmp", error_callback), Eq(0));
  cmp_helper(list2.cmp(list3, "cmp", error_callback), Lt(0));
  cmp_helper(list2.cmp(list4, "cmp", error_callback), Lt(0));
  cmp_helper(list2.cmp(list5, "cmp", error_callback), Lt(0));

  cmp_helper(list3.cmp(list1, "cmp", error_callback), Gt(0));
  cmp_helper(list3.cmp(list2, "cmp", error_callback), Gt(0));
  cmp_helper(list3.cmp(list3, "cmp", error_callback), Eq(0));
  cmp_helper(list3.cmp(list4, "cmp", error_callback), Lt(0));
  cmp_helper(list3.cmp(list5, "cmp", error_callback), Lt(0));

  cmp_helper(list4.cmp(list1, "cmp", error_callback), Gt(0));
  cmp_helper(list4.cmp(list2, "cmp", error_callback), Gt(0));
  cmp_helper(list4.cmp(list3, "cmp", error_callback), Gt(0));
  cmp_helper(list4.cmp(list4, "cmp", error_callback), Eq(0));
  cmp_helper(list4.cmp(list5, "cmp", error_callback), Lt(0));

  cmp_helper(list5.cmp(list1, "cmp", error_callback), Gt(0));
  cmp_helper(list5.cmp(list2, "cmp", error_callback), Gt(0));
  cmp_helper(list5.cmp(list3, "cmp", error_callback), Gt(0));
  cmp_helper(list5.cmp(list4, "cmp", error_callback), Gt(0));
  cmp_helper(list5.cmp(list5, "cmp", error_callback), Eq(0));
}

TEST(StarlarkList, OrderCycle) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_list list(3);
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  list.append(&zero, ctx, error_callback);
  list.append(&list, ctx, error_callback);
  list.append(&one, ctx, error_callback);
  cmp_helper(list.cmp(list, "cmp", error_callback), Eq(0));
}

TEST(StarlarkList, OrderCycleBis) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_list list1(3);
  starlark_list list2(3);
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  list1.append(&zero, ctx, error_callback);
  list1.append(&list2, ctx, error_callback);
  list1.append(&one, ctx, error_callback);
  list2.append(&zero, ctx, error_callback);
  list2.append(&list1, ctx, error_callback);
  list2.append(&one, ctx, error_callback);
  cmp_helper(list1.cmp(list2, "cmp", error_callback), Eq(0));
}

TEST(StarlarkList, OrderError) {
  error_handler error_callback;
  starlark_integer one(1);
  starlark_list list(0);

  EXPECT_FALSE(list.cmp(one, "<", error_callback).ok());
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'<' not supported between instances of 'list' and 'int'");
}

TEST(StarlarkList, AddWithFreeze) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_integer one(1);
  starlark_list list(0);
  list.append(&one, ctx, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  list.freeze();
  list.append(&one, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: trying to mutate a frozen list value");
}

TEST(StarlarkList, AddWithMultipleFreeze) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_integer one(1);
  starlark_list list(0);
  list.append(&one, ctx, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  list.freeze();
  list.freeze();
  list.append(&one, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: trying to mutate a frozen list value");
}

TEST(StarlarkList, Membership) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_list list(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  list.append(&zero, ctx, error_callback);

  EXPECT_TRUE(list.binary_in(zero, error_callback));
  EXPECT_FALSE(list.binary_in(one, error_callback));
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkList, Call) {
  error_handler error_callback;
  starlark_list list(0);
  Arena arena;
  context ctx(arena);

  list.call({}, {}, ctx, error_callback);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'list' object is not callable");
}

TEST(StarlarkList, UnaryPlus) {
  error_handler error_callback;
  starlark_list list(0);
  Arena arena;
  context ctx(arena);

  list.unary_plus(ctx, error_callback);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: bad operand type for unary +: 'list'");
}

TEST(StarlarkList, UnaryMinus) {
  error_handler error_callback;
  starlark_list list(0);
  Arena arena;
  context ctx(arena);

  list.unary_minus(ctx, error_callback);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: bad operand type for unary -: 'list'");
}

TEST(StarlarkList, UnaryTilde) {
  error_handler error_callback;
  starlark_list list(0);
  Arena arena;
  context ctx(arena);

  list.unary_tilde(ctx, error_callback);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: bad operand type for unary ~: 'list'");
}

TEST(StarlarkList, BinaryPlus) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_list list_1(0);
  starlark_list list_2(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  list_1.append(&zero, ctx, error_callback);
  list_2.append(&one, ctx, error_callback);

  auto* result = list_1.binary_plus(list_2, ctx, error_callback);

  ASSERT_NE(result, nullptr);
  EXPECT_EQ(list_1.str(), "[0]");
  EXPECT_EQ(list_2.str(), "[1]");
  EXPECT_EQ(result->str(), "[0, 1]");
}

TEST(StarlarkList, BinaryPlusOverflowNoOverflow) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_list list_1(0);
  starlark_list list_2(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena, runtime_options{.max_sequence_size = 20});

  for (int i = 0; i < 10; ++i) {
    list_1.append(&zero, ctx, error_callback);
    list_2.append(&one, ctx, error_callback);
  }

  auto* result = list_1.binary_plus(list_2, ctx, error_callback);

  ASSERT_NE(result, nullptr);
  EXPECT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list_1.str(), "[0, 0, 0, 0, 0, 0, 0, 0, 0, 0]");
  EXPECT_EQ(list_2.str(), "[1, 1, 1, 1, 1, 1, 1, 1, 1, 1]");
  EXPECT_EQ(result->str(), "[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]");
}

TEST(StarlarkList, BinaryPlusOverflowOverflow) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_list list_1(0);
  starlark_list list_2(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena, runtime_options{.max_sequence_size = 19});

  for (int i = 0; i < 10; ++i) {
    list_1.append(&zero, ctx, error_callback);
    list_2.append(&one, ctx, error_callback);
  }

  auto* result = list_1.binary_plus(list_2, ctx, error_callback);

  ASSERT_EQ(result, nullptr);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: sequences must be at most 19 elements");
  EXPECT_EQ(list_1.str(), "[0, 0, 0, 0, 0, 0, 0, 0, 0, 0]");
  EXPECT_EQ(list_2.str(), "[1, 1, 1, 1, 1, 1, 1, 1, 1, 1]");
}

TEST(StarlarkList, BinaryPlusNotList) {
  starlark_list list(0);
  starlark_tuple tuple(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = list.binary_plus(tuple, ctx, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: can only concatenate list (not \"tuple\") to list");
}

TEST(StarlarkList, PlusEqualsAssign) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_list list_1(0);
  starlark_list list_2(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  list_1.append(&zero, ctx, error_callback);
  list_2.append(&one, ctx, error_callback);

  auto* result = list_1.plus_equals_assign(list_2, ctx, error_callback);

  ASSERT_NE(result, nullptr);
  EXPECT_EQ(list_1.str(), "[0, 1]");
  EXPECT_EQ(list_2.str(), "[1]");
  EXPECT_EQ(result->str(), "[0, 1]");
}

TEST(StarlarkList, PlusEqualsAssignOverflowNoOverflow) {
  starlark_list list_1(0);
  starlark_list list_2(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena, runtime_options{.max_sequence_size = 20});

  for (int i = 0; i < 8; ++i) {
    list_1.append(ctx.zero(), ctx, error_callback);
  }
  for (int i = 0; i < 12; ++i) {
    list_2.append(ctx.one(), ctx, error_callback);
  }

  auto* result = list_1.plus_equals_assign(list_2, ctx, error_callback);

  ASSERT_NE(result, nullptr);
  EXPECT_EQ(list_1.str(), "[0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkList, PlusEqualsAssignOverflowOverflow) {
  starlark_list list_1(0);
  starlark_list list_2(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena, runtime_options{.max_sequence_size = 19});

  for (int i = 0; i < 8; ++i) {
    list_1.append(ctx.zero(), ctx, error_callback);
  }
  for (int i = 0; i < 12; ++i) {
    list_2.append(ctx.one(), ctx, error_callback);
  }

  auto* result = list_1.plus_equals_assign(list_2, ctx, error_callback);

  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: sequences must be at most 19 elements");
}

TEST(StarlarkList, PlusEqualsAssignSelf) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_list list(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  list.append(&zero, ctx, error_callback);
  list.append(&one, ctx, error_callback);

  auto* result = list.plus_equals_assign(list, ctx, error_callback);

  ASSERT_NE(result, nullptr);
  EXPECT_EQ(list.str(), "[0, 1, 0, 1]");
  EXPECT_EQ(result->str(), "[0, 1, 0, 1]");
}

TEST(StarlarkList, PlusEqualsAssignNotList) {
  starlark_list list(0);
  starlark_tuple tuple(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = list.plus_equals_assign(tuple, ctx, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: can only concatenate list (not \"tuple\") to list");
}

TEST(StarlarkList, PlusEqualsAssignWhileIterating) {
  starlark_list list(0);
  starlark_integer zero(0);
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  list.append(&zero, ctx, error_callback);
  list.append(&one, ctx, error_callback);

  [[maybe_unused]] auto* it = list.get_iterator(true, ctx, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());
  list.plus_equals_assign(list, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("Error in append: list value is temporarily immutable due to active for-loop iteration", error_callback.messages[0]);
}

TEST(StarlarkList, StarEqualsAssign) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_bigint three(3);
  starlark_list list0(0);
  starlark_list list1(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  list1.append(&zero, ctx, error_callback);
  list1.append(&one, ctx, error_callback);

  auto* result_1 = list1.star_equals_assign(two, ctx, error_callback);
  auto* result_2 = list1.star_equals_assign(three, ctx, error_callback);
  auto* result_3 = list0.star_equals_assign(three, ctx, error_callback);
  auto* result_4 = list0.star_equals_assign(two, ctx, error_callback);

  EXPECT_EQ(list0.str(), "[]");
  EXPECT_EQ(list1.str(), "[0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1]");
  ASSERT_NE(result_1, nullptr);
  EXPECT_EQ(result_1->str(), "[0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1]");
  ASSERT_NE(result_2, nullptr);
  EXPECT_EQ(result_2->str(), "[0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1]");
  ASSERT_NE(result_3, nullptr);
  EXPECT_EQ(result_3->str(), "[]");
  ASSERT_NE(result_4, nullptr);
  EXPECT_EQ(result_4->str(), "[]");
}

TEST(StarlarkList, StarEqualsAssignOverflowNoOverflow) {
  starlark_integer param(10);
  starlark_list list(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena, runtime_options{.max_sequence_size = 20});

  list.append(ctx.zero(), ctx, error_callback);
  list.append(ctx.one(), ctx, error_callback);

  auto* result = list.star_equals_assign(param, ctx, error_callback);

  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->str(), "[0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1]");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkList, StarEqualsAssignOverflowOverflow) {
  starlark_integer param(10);
  starlark_list list(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena, runtime_options{.max_sequence_size = 19});

  list.append(ctx.zero(), ctx, error_callback);
  list.append(ctx.one(), ctx, error_callback);

  auto* result = list.star_equals_assign(param, ctx, error_callback);

  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: sequences must be at most 19 elements");
}

TEST(StarlarkList, StarEqualsAssignOverflowNoOverflowBigint) {
  starlark_bigint param(10);
  starlark_list list(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena, runtime_options{.max_sequence_size = 20});

  list.append(ctx.zero(), ctx, error_callback);
  list.append(ctx.one(), ctx, error_callback);

  auto* result = list.star_equals_assign(param, ctx, error_callback);

  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->str(), "[0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1]");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkList, StarEqualsAssignOverflowOverflowBigint) {
  starlark_bigint param(10);
  starlark_list list(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena, runtime_options{.max_sequence_size = 19});

  list.append(ctx.zero(), ctx, error_callback);
  list.append(ctx.one(), ctx, error_callback);

  auto* result = list.star_equals_assign(param, ctx, error_callback);

  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: sequences must be at most 19 elements");
}

TEST(StarlarkList, StarEqualsAssignNegativeAndZero) {
  starlark_bigint minus_two(number::minus_one() << 1);
  starlark_integer minus_one(-1);
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_list list1(0);
  starlark_list list2(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  list1.append(&zero, ctx, error_callback);
  list1.append(&one, ctx, error_callback);
  list2.append(&zero, ctx, error_callback);
  list2.append(&one, ctx, error_callback);

  auto* result_1 = list1.star_equals_assign(minus_one, ctx, error_callback);
  auto* result_2 = list2.star_equals_assign(minus_two, ctx, error_callback);

  EXPECT_EQ(list1.str(), "[]");
  EXPECT_EQ(list2.str(), "[]");
  ASSERT_NE(result_1, nullptr);
  EXPECT_EQ(result_1->str(), "[]");
  ASSERT_NE(result_2, nullptr);
  EXPECT_EQ(result_2->str(), "[]");
}

TEST(StarlarkList, StarEqualsAssignReverse) {
  starlark_bigint minus_two(number::minus_one() << 1);
  starlark_integer minus_one(-1);
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_bigint three(3);
  starlark_list list0(0);
  starlark_list list1(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  list1.append(&zero, ctx, error_callback);
  list1.append(&one, ctx, error_callback);

  auto* result_1 = two.star_equals_assign(list1, ctx, error_callback);
  auto* result_2 = three.star_equals_assign(list1, ctx, error_callback);
  auto* result_3 = three.star_equals_assign(list0, ctx, error_callback);
  auto* result_4 = minus_one.star_equals_assign(list1, ctx, error_callback);
  auto* result_5 = minus_two.star_equals_assign(list1, ctx, error_callback);

  EXPECT_EQ(list0.str(), "[]");
  EXPECT_EQ(list1.str(), "[0, 1]");
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

TEST(StarlarkList, StarEqualsAssignNotInt) {
  starlark_list list(0);
  starlark_tuple tuple(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = list.star_equals_assign(tuple, ctx, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: can't multiply sequence by non-int of type 'tuple'");
}

TEST(StarlarkList, StarEqualsAssignTooBig) {
  starlark_list list(0);
  starlark_bigint big(number::one() << 64);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  list.append(&big, ctx, error_callback);

  auto* result = list.star_equals_assign(big, ctx, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: sequences must be at most 2147483647 elements");
}

TEST(StarlarkList, StarEqualsAssignWhileIterating1) {
  starlark_list list(0);
  starlark_integer zero(0);
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  list.append(&zero, ctx, error_callback);
  list.append(&one, ctx, error_callback);

  [[maybe_unused]] auto* it = list.get_iterator(true, ctx, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());
  list.star_equals_assign(zero, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("Error in append: list value is temporarily immutable due to active for-loop iteration", error_callback.messages[0]);
}

TEST(StarlarkList, StarEqualsAssignWhileIterating2) {
  starlark_list list(0);
  starlark_bigint zero(0);
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  list.append(&zero, ctx, error_callback);
  list.append(&one, ctx, error_callback);

  [[maybe_unused]] auto* it = list.get_iterator(true, ctx, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());
  list.star_equals_assign(zero, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("Error in append: list value is temporarily immutable due to active for-loop iteration", error_callback.messages[0]);
}

TEST(StarlarkList, StarEqualsAssignWhileIterating3) {
  starlark_list list(0);
  starlark_bigint zero(0);
  starlark_integer one(1);
  starlark_tuple tuple(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  list.append(&zero, ctx, error_callback);
  list.append(&one, ctx, error_callback);

  [[maybe_unused]] auto* it = list.get_iterator(true, ctx, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());
  list.star_equals_assign(tuple, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: can't multiply sequence by non-int of type 'tuple'");
}

TEST(StarlarkList, BinaryStar) {
  starlark_bigint minus_two(number::minus_one() << 1);
  starlark_integer minus_one(-1);
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_bigint three(3);
  starlark_list list0(0);
  starlark_list list1(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  list1.append(&zero, ctx, error_callback);
  list1.append(&one, ctx, error_callback);

  auto* result_1 = list1.binary_star(two, ctx, error_callback);
  auto* result_2 = list1.binary_star(three, ctx, error_callback);
  auto* result_3 = list0.binary_star(three, ctx, error_callback);
  auto* result_4 = list1.binary_star(minus_one, ctx, error_callback);
  auto* result_5 = list1.binary_star(minus_two, ctx, error_callback);
  auto* result_6 = list0.binary_star(two, ctx, error_callback);

  EXPECT_EQ(list0.str(), "[]");
  EXPECT_EQ(list1.str(), "[0, 1]");
  EXPECT_EQ(result_1->str(), "[0, 1, 0, 1]");
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
  ASSERT_NE(result_6, nullptr);
  EXPECT_EQ(result_6->str(), "[]");
  ASSERT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkList, BinaryStarOverflowNoOverflow) {
  starlark_integer param(20);
  starlark_list list(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena, runtime_options{.max_sequence_size = 20});

  list.append(ctx.zero(), ctx, error_callback);

  auto* result = list.binary_star(param, ctx, error_callback);

  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->str(), "[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]");
  ASSERT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkList, BinaryStarOverflowOverflow) {
  starlark_integer param(21);
  starlark_list list(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena, runtime_options{.max_sequence_size = 20});

  list.append(ctx.zero(), ctx, error_callback);

  auto* result = list.binary_star(param, ctx, error_callback);

  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: sequences must be at most 20 elements");
}

TEST(StarlarkList, BinaryStarOverflowNoOverflowBigint) {
  starlark_bigint param(20);
  starlark_list list(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena, runtime_options{.max_sequence_size = 20});

  list.append(ctx.zero(), ctx, error_callback);

  auto* result = list.binary_star(param, ctx, error_callback);

  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->str(), "[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]");
  ASSERT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkList, BinaryStarOverflowOverflowBigint) {
  starlark_bigint param(21);
  starlark_list list(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena, runtime_options{.max_sequence_size = 20});

  list.append(ctx.zero(), ctx, error_callback);

  auto* result = list.binary_star(param, ctx, error_callback);

  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: sequences must be at most 20 elements");
}

TEST(StarlarkList, BinaryStarReverse) {
  starlark_bigint minus_two(number::minus_one() << 1);
  starlark_integer minus_one(-1);
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_bigint three(3);
  starlark_list list0(0);
  starlark_list list1(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  list1.append(&zero, ctx, error_callback);
  list1.append(&one, ctx, error_callback);

  auto* result_1 = two.binary_star(list1, ctx, error_callback);
  auto* result_2 = three.binary_star(list1, ctx, error_callback);
  auto* result_3 = three.binary_star(list0, ctx, error_callback);
  auto* result_4 = minus_one.binary_star(list1, ctx, error_callback);
  auto* result_5 = minus_two.binary_star(list1, ctx, error_callback);

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
  starlark_list list(0);
  starlark_tuple tuple(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = list.binary_star(tuple, ctx, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: can't multiply sequence by non-int of type 'tuple'");
}

TEST(StarlarkList, BinaryStarTooBig) {
  starlark_list list(0);
  starlark_bigint big(number::one() << 64);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  list.append(&big, ctx, error_callback);

  auto* result = list.binary_star(big, ctx, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: sequences must be at most 2147483647 elements");
}

TEST(StarlarkList, Len) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_list list0(0);
  starlark_list list1(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  list1.append(&zero, ctx, error_callback);
  list1.append(&one, ctx, error_callback);

  EXPECT_EQ(0, list0.len(true, error_callback));
  EXPECT_EQ(2, list1.len(true, error_callback));
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkList, GetIterator) {
  starlark_list list0(0);
  starlark_list list1(0);
  starlark_integer zero(0);
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  list1.append(&zero, ctx, error_callback);
  list1.append(&one, ctx, error_callback);

  auto* it0 = list0.get_iterator(true, ctx, error_callback);
  EXPECT_FALSE(it0->has_next());
  it0->end_iterator();

  auto* it1 = list1.get_iterator(true, ctx, error_callback);
  EXPECT_TRUE(it1->has_next());
  EXPECT_TRUE(it1->next()->equals(zero));
  EXPECT_TRUE(it1->has_next());
  EXPECT_TRUE(it1->next()->equals(one));
  EXPECT_FALSE(it1->has_next());
  it1->end_iterator();
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkList, MutationWhileIterating1) {
  starlark_list list(0);
  starlark_integer zero(0);
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  list.append(&zero, ctx, error_callback);
  list.append(&one, ctx, error_callback);

  [[maybe_unused]] auto* it = list.get_iterator(true, ctx, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());
  list.append(&zero, ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("Error in append: list value is temporarily immutable due to active for-loop iteration", error_callback.messages[0]);
}

TEST(StarlarkList, Subscript) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list(0);
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  list.append(&zero, ctx, error_callback);
  list.append(&one, ctx, error_callback);
  list.append(&two, ctx, error_callback);

  EXPECT_EQ(list.index(starlark_integer(-3), ctx, error_callback)->repr(), "0");
  EXPECT_EQ(list.index(starlark_bigint(-3), ctx, error_callback)->repr(), "0");
  EXPECT_EQ(list.index(starlark_integer(-2), ctx, error_callback)->repr(), "1");
  EXPECT_EQ(list.index(starlark_bigint(-2), ctx, error_callback)->repr(), "1");
  EXPECT_EQ(list.index(starlark_integer(-1), ctx, error_callback)->repr(), "2");
  EXPECT_EQ(list.index(starlark_bigint(-1), ctx, error_callback)->repr(), "2");
  EXPECT_EQ(list.index(starlark_integer(0), ctx, error_callback)->repr(), "0");
  EXPECT_EQ(list.index(starlark_bigint(0), ctx, error_callback)->repr(), "0");
  EXPECT_EQ(list.index(starlark_integer(1), ctx, error_callback)->repr(), "1");
  EXPECT_EQ(list.index(starlark_bigint(1), ctx, error_callback)->repr(), "1");
  EXPECT_EQ(list.index(starlark_integer(2), ctx, error_callback)->repr(), "2");
  EXPECT_EQ(list.index(starlark_bigint(2), ctx, error_callback)->repr(), "2");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkList, SubscriptOutOfRange1) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list(0);
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  list.append(&zero, ctx, error_callback);
  list.append(&one, ctx, error_callback);
  list.append(&two, ctx, error_callback);

  EXPECT_EQ(nullptr, list.index(starlark_integer(-4), ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("list index out of range", error_callback.messages[0]);
}

TEST(StarlarkList, SubscriptOutOfRange2) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list(0);
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  list.append(&zero, ctx, error_callback);
  list.append(&one, ctx, error_callback);
  list.append(&two, ctx, error_callback);

  EXPECT_EQ(nullptr, list.index(starlark_integer(3), ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("list index out of range", error_callback.messages[0]);
}

TEST(StarlarkList, SubscriptOutOfRange3) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list(0);
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  list.append(&zero, ctx, error_callback);
  list.append(&one, ctx, error_callback);
  list.append(&two, ctx, error_callback);

  EXPECT_EQ(nullptr, list.index(starlark_bigint(-4), ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("list index out of range", error_callback.messages[0]);
}

TEST(StarlarkList, SubscriptOutOfRange4) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list(0);
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  list.append(&zero, ctx, error_callback);
  list.append(&one, ctx, error_callback);
  list.append(&two, ctx, error_callback);

  EXPECT_EQ(nullptr, list.index(starlark_bigint(3), ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("list index out of range", error_callback.messages[0]);
}

TEST(StarlarkList, SubscriptOutOfRange5) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list(0);
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  list.append(&zero, ctx, error_callback);
  list.append(&one, ctx, error_callback);
  list.append(&two, ctx, error_callback);

  EXPECT_EQ(nullptr, list.index(starlark_bigint(number::one() << 64), ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("list index out of range", error_callback.messages[0]);
}

TEST(StarlarkList, SubscriptNotInt) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list(0);
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  list.append(&zero, ctx, error_callback);
  list.append(&one, ctx, error_callback);
  list.append(&two, ctx, error_callback);

  EXPECT_EQ(nullptr, list.index(starlark_float(1), ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("list indices must be integers or slices, not 'float'", error_callback.messages[0]);
}

TEST(StarlarkList, IndexAssign) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list(0);
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_integer three(3);
  starlark_integer four(4);
  list.append(&zero, ctx, error_callback);
  list.append(&one, ctx, error_callback);
  list.append(&two, ctx, error_callback);

  list.index_assign(starlark_integer(-3), three, error_callback);
  list.index_assign(*ctx.one(), four, error_callback);
  EXPECT_EQ("[3, 4, 2]", list.str());
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkList, IndexAssignOutOfRange1) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list(0);
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_integer three(3);
  list.append(&zero, ctx, error_callback);
  list.append(&one, ctx, error_callback);
  list.append(&two, ctx, error_callback);

  list.index_assign(starlark_integer(-4), three, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("list index out of range", error_callback.messages[0]);
}

TEST(StarlarkList, IndexAssignOutOfRange2) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list(0);
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_integer three(3);
  list.append(&zero, ctx, error_callback);
  list.append(&one, ctx, error_callback);
  list.append(&two, ctx, error_callback);

  list.index_assign(starlark_integer(4), three, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("list index out of range", error_callback.messages[0]);
}

TEST(StarlarkList, IndexAssignWhileIterating) {
  starlark_list list(0);
  starlark_integer zero(0);
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  list.append(&zero, ctx, error_callback);
  list.append(&one, ctx, error_callback);

  [[maybe_unused]] auto* it = list.get_iterator(true, ctx, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());
  list.index_assign(zero, one, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("Error in update: list value is temporarily immutable due to active for-loop iteration", error_callback.messages[0]);
}

TEST(StarlarkList, IndexAssignWithFreeze) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_integer one(1);
  starlark_list list(0);
  list.append(&one, ctx, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  list.freeze();
  list.index_assign(*ctx.zero(), one, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: trying to mutate a frozen list value");
}

TEST(StarlarkList, SliceRange) {
  Arena arena;
  context ctx(arena);
  auto test = [&ctx](const starlark_obj* start, const starlark_obj* end, const starlark_obj* stride,
      std::string_view expected_value0,
      std::string_view expected_value1,
      std::string_view expected_value2,
      std::string_view expected_value3,
      std::string_view expected_value4,
      std::string_view expected_value5) {
    error_handler error_callback;
    starlark_list list(0);
    starlark_integer two(2);
    starlark_integer three(3);
    starlark_integer four(4);

    auto* result0 = list.slice_range(*start, *end, *stride, ctx, error_callback);
    list.append(ctx.zero(), ctx, error_callback);
    auto* result1 = list.slice_range(*start, *end, *stride, ctx, error_callback);
    list.append(ctx.one(), ctx, error_callback);
    auto* result2 = list.slice_range(*start, *end, *stride, ctx, error_callback);
    list.append(&two, ctx, error_callback);
    auto* result3 = list.slice_range(*start, *end, *stride, ctx, error_callback);
    list.append(&three, ctx, error_callback);
    auto* result4 = list.slice_range(*start, *end, *stride, ctx, error_callback);
    list.append(&four, ctx, error_callback);
    auto* result5 = list.slice_range(*start, *end, *stride, ctx, error_callback);

    ASSERT_NE(nullptr, result0);
    ASSERT_NE(nullptr, result1);
    ASSERT_NE(nullptr, result2);
    ASSERT_NE(nullptr, result3);
    ASSERT_NE(nullptr, result4);
    ASSERT_NE(nullptr, result5);
    EXPECT_EQ(expected_value0, result0->str()) << "Start: " << start->str() << ", end: " << end->str() << ", stride: " << stride->str() << "\n";
    EXPECT_EQ(expected_value1, result1->str()) << "Start: " << start->str() << ", end: " << end->str() << ", stride: " << stride->str() << "\n";
    EXPECT_EQ(expected_value2, result2->str()) << "Start: " << start->str() << ", end: " << end->str() << ", stride: " << stride->str() << "\n";
    EXPECT_EQ(expected_value3, result3->str()) << "Start: " << start->str() << ", end: " << end->str() << ", stride: " << stride->str() << "\n";
    EXPECT_EQ(expected_value4, result4->str()) << "Start: " << start->str() << ", end: " << end->str() << ", stride: " << stride->str() << "\n";
    EXPECT_EQ(expected_value5, result5->str()) << "Start: " << start->str() << ", end: " << end->str() << ", stride: " << stride->str() << "\n";
    EXPECT_EQ("[0, 1, 2, 3, 4]", list.str());
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  /*
  ```python
  def tt(a):
      if a == None:
          return "ctx.none_value()"
      if a == -1:
          return "ctx.minus_one()"
      if a == 0:
          return "ctx.zero()"
      if a == 1:
          return "ctx.one()"

  def rr(a, b, c):
      return 'test({}, {}, {}, "{}", "{}", "{}", "{}", "{}", "{}");'.format(tt(a), tt(b), tt(c), *[str(list(range(x))[a:b:c]) for x in range(6)])

  "\n  ".join([rr(a,b,c) for a in (None, -1, 0, 1) for b in (None, -1, 0, 1) for c in (None, -1, 1)])
  ```
  */

  test(ctx.none_value(), ctx.none_value(), ctx.none_value(), "[]", "[0]", "[0, 1]", "[0, 1, 2]", "[0, 1, 2, 3]", "[0, 1, 2, 3, 4]");
  test(ctx.none_value(), ctx.none_value(), ctx.minus_one(), "[]", "[0]", "[1, 0]", "[2, 1, 0]", "[3, 2, 1, 0]", "[4, 3, 2, 1, 0]");
  test(ctx.none_value(), ctx.none_value(), ctx.one(), "[]", "[0]", "[0, 1]", "[0, 1, 2]", "[0, 1, 2, 3]", "[0, 1, 2, 3, 4]");
  test(ctx.none_value(), ctx.minus_one(), ctx.none_value(), "[]", "[]", "[0]", "[0, 1]", "[0, 1, 2]", "[0, 1, 2, 3]");
  test(ctx.none_value(), ctx.minus_one(), ctx.minus_one(), "[]", "[]", "[]", "[]", "[]", "[]");
  test(ctx.none_value(), ctx.minus_one(), ctx.one(), "[]", "[]", "[0]", "[0, 1]", "[0, 1, 2]", "[0, 1, 2, 3]");
  test(ctx.none_value(), ctx.zero(), ctx.none_value(), "[]", "[]", "[]", "[]", "[]", "[]");
  test(ctx.none_value(), ctx.zero(), ctx.minus_one(), "[]", "[]", "[1]", "[2, 1]", "[3, 2, 1]", "[4, 3, 2, 1]");
  test(ctx.none_value(), ctx.zero(), ctx.one(), "[]", "[]", "[]", "[]", "[]", "[]");
  test(ctx.none_value(), ctx.one(), ctx.none_value(), "[]", "[0]", "[0]", "[0]", "[0]", "[0]");
  test(ctx.none_value(), ctx.one(), ctx.minus_one(), "[]", "[]", "[]", "[2]", "[3, 2]", "[4, 3, 2]");
  test(ctx.none_value(), ctx.one(), ctx.one(), "[]", "[0]", "[0]", "[0]", "[0]", "[0]");

  test(ctx.minus_one(), ctx.none_value(), ctx.none_value(), "[]", "[0]", "[1]", "[2]", "[3]", "[4]");
  test(ctx.minus_one(), ctx.none_value(), ctx.minus_one(), "[]", "[0]", "[1, 0]", "[2, 1, 0]", "[3, 2, 1, 0]", "[4, 3, 2, 1, 0]");
  test(ctx.minus_one(), ctx.none_value(), ctx.one(), "[]", "[0]", "[1]", "[2]", "[3]", "[4]");
  test(ctx.minus_one(), ctx.minus_one(), ctx.none_value(), "[]", "[]", "[]", "[]", "[]", "[]");
  test(ctx.minus_one(), ctx.minus_one(), ctx.minus_one(), "[]", "[]", "[]", "[]", "[]", "[]");
  test(ctx.minus_one(), ctx.minus_one(), ctx.one(), "[]", "[]", "[]", "[]", "[]", "[]");
  test(ctx.minus_one(), ctx.zero(), ctx.none_value(), "[]", "[]", "[]", "[]", "[]", "[]");
  test(ctx.minus_one(), ctx.zero(), ctx.minus_one(), "[]", "[]", "[1]", "[2, 1]", "[3, 2, 1]", "[4, 3, 2, 1]");
  test(ctx.minus_one(), ctx.zero(), ctx.one(), "[]", "[]", "[]", "[]", "[]", "[]");
  test(ctx.minus_one(), ctx.one(), ctx.none_value(), "[]", "[0]", "[]", "[]", "[]", "[]");
  test(ctx.minus_one(), ctx.one(), ctx.minus_one(), "[]", "[]", "[]", "[2]", "[3, 2]", "[4, 3, 2]");
  test(ctx.minus_one(), ctx.one(), ctx.one(), "[]", "[0]", "[]", "[]", "[]", "[]");

  test(ctx.zero(), ctx.none_value(), ctx.none_value(), "[]", "[0]", "[0, 1]", "[0, 1, 2]", "[0, 1, 2, 3]", "[0, 1, 2, 3, 4]");
  test(ctx.zero(), ctx.none_value(), ctx.minus_one(), "[]", "[0]", "[0]", "[0]", "[0]", "[0]");
  test(ctx.zero(), ctx.none_value(), ctx.one(), "[]", "[0]", "[0, 1]", "[0, 1, 2]", "[0, 1, 2, 3]", "[0, 1, 2, 3, 4]");
  test(ctx.zero(), ctx.minus_one(), ctx.none_value(), "[]", "[]", "[0]", "[0, 1]", "[0, 1, 2]", "[0, 1, 2, 3]");
  test(ctx.zero(), ctx.minus_one(), ctx.minus_one(), "[]", "[]", "[]", "[]", "[]", "[]");
  test(ctx.zero(), ctx.minus_one(), ctx.one(), "[]", "[]", "[0]", "[0, 1]", "[0, 1, 2]", "[0, 1, 2, 3]");
  test(ctx.zero(), ctx.zero(), ctx.none_value(), "[]", "[]", "[]", "[]", "[]", "[]");
  test(ctx.zero(), ctx.zero(), ctx.minus_one(), "[]", "[]", "[]", "[]", "[]", "[]");
  test(ctx.zero(), ctx.zero(), ctx.one(), "[]", "[]", "[]", "[]", "[]", "[]");
  test(ctx.zero(), ctx.one(), ctx.none_value(), "[]", "[0]", "[0]", "[0]", "[0]", "[0]");
  test(ctx.zero(), ctx.one(), ctx.minus_one(), "[]", "[]", "[]", "[]", "[]", "[]");
  test(ctx.zero(), ctx.one(), ctx.one(), "[]", "[0]", "[0]", "[0]", "[0]", "[0]");

  test(ctx.one(), ctx.none_value(), ctx.none_value(), "[]", "[]", "[1]", "[1, 2]", "[1, 2, 3]", "[1, 2, 3, 4]");
  test(ctx.one(), ctx.none_value(), ctx.minus_one(), "[]", "[0]", "[1, 0]", "[1, 0]", "[1, 0]", "[1, 0]");
  test(ctx.one(), ctx.none_value(), ctx.one(), "[]", "[]", "[1]", "[1, 2]", "[1, 2, 3]", "[1, 2, 3, 4]");
  test(ctx.one(), ctx.minus_one(), ctx.none_value(), "[]", "[]", "[]", "[1]", "[1, 2]", "[1, 2, 3]");
  test(ctx.one(), ctx.minus_one(), ctx.minus_one(), "[]", "[]", "[]", "[]", "[]", "[]");
  test(ctx.one(), ctx.minus_one(), ctx.one(), "[]", "[]", "[]", "[1]", "[1, 2]", "[1, 2, 3]");
  test(ctx.one(), ctx.zero(), ctx.none_value(), "[]", "[]", "[]", "[]", "[]", "[]");
  test(ctx.one(), ctx.zero(), ctx.minus_one(), "[]", "[]", "[1]", "[1]", "[1]", "[1]");
  test(ctx.one(), ctx.zero(), ctx.one(), "[]", "[]", "[]", "[]", "[]", "[]");
  test(ctx.one(), ctx.one(), ctx.none_value(), "[]", "[]", "[]", "[]", "[]", "[]");
  test(ctx.one(), ctx.one(), ctx.minus_one(), "[]", "[]", "[]", "[]", "[]", "[]");
  test(ctx.one(), ctx.one(), ctx.one(), "[]", "[]", "[]", "[]", "[]", "[]");
}

TEST(StarlarkList, SliceRangeBoolStart) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_list list(0);
  starlark_integer two(2);
  starlark_integer three(3);
  starlark_integer four(4);
  list.append(ctx.zero(), ctx, error_callback);
  list.append(ctx.one(), ctx, error_callback);
  list.append(&two, ctx, error_callback);
  list.append(&three, ctx, error_callback);
  list.append(&four, ctx, error_callback);

  auto* result = list.slice_range(*ctx.true_value(), *ctx.none_value(), *ctx.none_value(), ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_EQ("[0, 1, 2, 3, 4]", list.str());
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "slice indices must be integers, not 'bool'");
}

TEST(StarlarkList, SliceRangeBoolEnd) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_list list(0);
  starlark_integer two(2);
  starlark_integer three(3);
  starlark_integer four(4);
  list.append(ctx.zero(), ctx, error_callback);
  list.append(ctx.one(), ctx, error_callback);
  list.append(&two, ctx, error_callback);
  list.append(&three, ctx, error_callback);
  list.append(&four, ctx, error_callback);

  auto* result = list.slice_range(*ctx.none_value(), *ctx.false_value(), *ctx.none_value(), ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_EQ("[0, 1, 2, 3, 4]", list.str());
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "slice indices must be integers, not 'bool'");
}

TEST(StarlarkList, SliceRangeBoolStride) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_list list(0);
  starlark_integer two(2);
  starlark_integer three(3);
  starlark_integer four(4);
  list.append(ctx.zero(), ctx, error_callback);
  list.append(ctx.one(), ctx, error_callback);
  list.append(&two, ctx, error_callback);
  list.append(&three, ctx, error_callback);
  list.append(&four, ctx, error_callback);

  auto* result = list.slice_range(*ctx.none_value(), *ctx.none_value(), *ctx.false_value(), ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_EQ("[0, 1, 2, 3, 4]", list.str());
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "slice indices must be integers, not 'bool'");
}

TEST(StarlarkList, SliceRangeZeroStride) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_list list(0);
  starlark_integer two(2);
  starlark_integer three(3);
  starlark_integer four(4);
  list.append(ctx.zero(), ctx, error_callback);
  list.append(ctx.one(), ctx, error_callback);
  list.append(&two, ctx, error_callback);
  list.append(&three, ctx, error_callback);
  list.append(&four, ctx, error_callback);

  auto* result = list.slice_range(*ctx.none_value(), *ctx.none_value(), *ctx.zero(), ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_EQ("[0, 1, 2, 3, 4]", list.str());
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: slice step cannot be zero");
}

TEST(StarlarkList, GetAttrError) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_list list(0);

  EXPECT_EQ(nullptr, list.get_attr(true, "count", ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "AttributeError: 'list' object has no attribute 'count'");
}

TEST(StarlarkList, GetAttrErrorWithSuggestion) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_list list(0);

  EXPECT_EQ(nullptr, list.get_attr(true, "appen", ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "AttributeError: 'list' object has no attribute 'appen'. Did you mean: 'append'?");
}

TEST(StarlarkList, Dot) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_list list(0);

  auto* result = list.dot("append", ctx, error_callback);
  EXPECT_NE(nullptr, result);
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkList, DotError) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_list list(0);

  EXPECT_EQ(nullptr, list.dot("count", ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "AttributeError: 'list' object has no attribute 'count'");
}

TEST(StarlarkList, DotErrorWithSuggestion) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_list list(0);

  EXPECT_EQ(nullptr, list.dot("appen", ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "AttributeError: 'list' object has no attribute 'appen'. Did you mean: 'append'?");
}

TEST(StarlarkList, DotAssigneadOnly) {
  error_handler error_callback;
  starlark_list list(0);

  list.dot_assign("append", list, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "AttributeError: 'list' object attribute 'append' is read-only");
}

TEST(StarlarkList, DotAssignError) {
  error_handler error_callback;
  starlark_list list(0);

  list.dot_assign("count", list, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "AttributeError: 'list' object has no attribute 'count'");
}

TEST(StarlarkList, DotAssignErrorWithSuggestion) {
  error_handler error_callback;
  starlark_list list(0);

  list.dot_assign("appen", list, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "AttributeError: 'list' object has no attribute 'appen'. Did you mean: 'append'?");
}

TEST(StarlarkList, Append) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  auto* method = list.dot("append", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list.str(), "[0]");
  EXPECT_EQ(result->type(), starlark_types::none_t);
}

TEST(StarlarkList, AppendOverflow) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena, runtime_options{.max_sequence_size = 20});
  starlark_list list(0);

  for (int i = 0; i < 20; ++i) {
    list.append(ctx.zero(), ctx, error_callback);
  }
  EXPECT_THAT(error_callback.messages, IsEmpty());
  list.append(ctx.zero(), ctx, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: sequences must be at most 20 elements");
}

TEST(StarlarkList, AppendWhileIterating) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  auto* method = list.dot("append", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  [[maybe_unused]] auto* it = list.get_iterator(true, ctx, error_callback);
  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "Error in append: list value is temporarily immutable due to active for-loop iteration");
  EXPECT_EQ(list.str(), "[]");
}

TEST(StarlarkList, AppendNoPositionalArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = list.dot("append", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: list.append() takes exactly one argument (0 given)");
  EXPECT_EQ(list.str(), "[]");
}

TEST(StarlarkList, AppendTwoPositionalArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  auto* method = list.dot("append", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: list.append() takes exactly one argument (2 given)");
  EXPECT_EQ(list.str(), "[]");
}

TEST(StarlarkList, AppendWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("zero", ctx.zero());
  auto* method = list.dot("append", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: list.append() takes no keyword arguments");
  EXPECT_EQ(list.str(), "[]");
}

TEST(StarlarkList, Clear) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list(0);
  list.append(ctx.zero(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = list.dot("clear", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list.str(), "[]");
  EXPECT_EQ(result->type(), starlark_types::none_t);
}

TEST(StarlarkList, ClearWithPositionalArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list(0);
  list.append(ctx.zero(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  auto* method = list.dot("clear", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: list.clear() takes no arguments (1 given)");
  EXPECT_EQ(list.str(), "[0]");
}

TEST(StarlarkList, ClearWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list(0);
  list.append(ctx.zero(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert("zero", ctx.zero());
  auto* method = list.dot("clear", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: list.clear() takes no keyword arguments");
  EXPECT_EQ(list.str(), "[0]");
}

TEST(StarlarkList, ClearWhileIterating) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list(0);
  list.append(ctx.zero(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = list.dot("clear", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  [[maybe_unused]] auto* it = list.get_iterator(true, ctx, error_callback);
  ASSERT_NE(nullptr, it);
  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "Error in delete: list value is temporarily immutable due to active for-loop iteration");
  EXPECT_EQ(list.str(), "[0]");
}

TEST(StarlarkList, Extend) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  starlark_list list2(0);
  list1.append(ctx.zero(), ctx, error_callback);
  list2.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&list2);
  auto* method = list1.dot("extend", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list1.str(), "[0, 1]");
  EXPECT_EQ(result->type(), starlark_types::none_t);
}

TEST(StarlarkList, ExtendSelf) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&list1);
  auto* method = list1.dot("extend", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list1.str(), "[0, 1, 0, 1, 0, 1, 0, 1]");
  EXPECT_EQ(result->type(), starlark_types::none_t);
}

TEST(StarlarkList, ExtendTuple) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  starlark_tuple tuple(0);
  list1.append(ctx.zero(), ctx, error_callback);
  tuple.add(ctx.one());

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = list1.dot("extend", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list1.str(), "[0, 1]");
  EXPECT_EQ(result->type(), starlark_types::none_t);
}

TEST(StarlarkList, ExtendWithNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  list1.append(ctx.zero(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = list1.dot("extend", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: list.extend() takes exactly one argument (0 given)");
  EXPECT_EQ(list1.str(), "[0]");
}

TEST(StarlarkList, ExtendWithNotIterable) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  list1.append(ctx.zero(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  auto* method = list1.dot("extend", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'int' object is not iterable");
  EXPECT_EQ(list1.str(), "[0]");
}

TEST(StarlarkList, ExtendWhileIterating) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  starlark_list list2(0);
  list1.append(ctx.zero(), ctx, error_callback);
  list2.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&list2);
  auto* method = list1.dot("extend", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  [[maybe_unused]] auto* it = list1.get_iterator(true, ctx, error_callback);
  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "Error in append: list value is temporarily immutable due to active for-loop iteration");
  EXPECT_EQ(list1.str(), "[0]");
}

TEST(StarlarkList, Index) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  auto* method = list1.dot("index", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "0");
}

TEST(StarlarkList, IndexNotFound) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.minus_one());
  auto* method = list1.dot("index", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "list.index(x): x not in list");
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
}

TEST(StarlarkList, IndexWithNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = list1.dot("index", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: index expected at least 1 argument, got 0");
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
}

TEST(StarlarkList, IndexWithStart) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.minus_one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.one());
  auto* method = list1.dot("index", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1, -1]");
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "2");
}

TEST(StarlarkList, IndexWithStartResultAtEnd) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.minus_one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.minus_one());
  pos_args.push_back(ctx.zero());
  auto* method = list1.dot("index", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1, -1]");
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "6");
}

TEST(StarlarkList, IndexWithStartAsNone) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.none_value());
  auto* method = list1.dot("index", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "0");
}

TEST(StarlarkList, IndexWithStartAsBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.false_value());
  auto* method = list1.dot("index", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "slice indices must be integers, not 'bool'");
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
}

TEST(StarlarkList, IndexWithStartAsBigint) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  starlark_bigint bigint_one(1);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(&bigint_one);
  auto* method = list1.dot("index", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "2");
}

TEST(StarlarkList, IndexWithStartAsBigintBig) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  starlark_bigint big(number::minus_one() << 100);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(&big);
  auto* method = list1.dot("index", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "0");
}

TEST(StarlarkList, IndexWithStartAndEnd) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.one());
  auto* method = list1.dot("index", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "0");
}

TEST(StarlarkList, IndexWithStartAndEndAsBigInt) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  starlark_bigint big(number::one() << 100);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(&big);
  auto* method = list1.dot("index", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "0");
}

TEST(StarlarkList, IndexWithStartAndEndAsBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.true_value());
  auto* method = list1.dot("index", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "slice indices must be integers, not 'bool'");
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
}

TEST(StarlarkList, IndexWithStartAndEndResutlAtEndAndNegativeEnd) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.minus_one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.minus_one());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.minus_one());
  auto* method = list1.dot("index", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "list.index(x): x not in list");
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1, -1]");
}

TEST(StarlarkList, IndexWithFourArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.minus_one());
  pos_args.push_back(ctx.zero());
  auto* method = list1.dot("index", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: index expected at most 3 argument, got 4");
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
}

TEST(StarlarkList, Insert) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  starlark_string str("abc"sv);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(&str);
  auto* method = list1.dot("insert", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list1.str(), "[\"abc\", 0, 1, 0, 0, 1, 1]");
  EXPECT_EQ(result->type(), starlark_types::none_t);
  EXPECT_EQ(result->str(), "None");
}

TEST(StarlarkList, InsertNegativeIndex) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  starlark_string str("abc"sv);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.minus_one());
  pos_args.push_back(&str);
  auto* method = list1.dot("insert", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, \"abc\", 1]");
  EXPECT_EQ(result->type(), starlark_types::none_t);
  EXPECT_EQ(result->str(), "None");
}

TEST(StarlarkList, InsertNegativeIndexWithClamp) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  starlark_string str("abc"sv);
  starlark_integer idx(-100);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&idx);
  pos_args.push_back(&str);
  auto* method = list1.dot("insert", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list1.str(), "[\"abc\", 0, 1, 0, 0, 1, 1]");
  EXPECT_EQ(result->type(), starlark_types::none_t);
  EXPECT_EQ(result->str(), "None");
}

TEST(StarlarkList, InsertPositiveIndex) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  starlark_string str("abc"sv);
  starlark_integer idx(1);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&idx);
  pos_args.push_back(&str);
  auto* method = list1.dot("insert", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list1.str(), "[0, \"abc\", 1, 0, 0, 1, 1]");
  EXPECT_EQ(result->type(), starlark_types::none_t);
  EXPECT_EQ(result->str(), "None");
}

TEST(StarlarkList, InsertPositiveIndexSizePlusOne) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  starlark_string str("abc"sv);
  starlark_integer idx(6);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&idx);
  pos_args.push_back(&str);
  auto* method = list1.dot("insert", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1, \"abc\"]");
  EXPECT_EQ(result->type(), starlark_types::none_t);
  EXPECT_EQ(result->str(), "None");
}

TEST(StarlarkList, InsertPositiveIndexWithClamping) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  starlark_string str("abc"sv);
  starlark_integer idx(100);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&idx);
  pos_args.push_back(&str);
  auto* method = list1.dot("insert", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1, \"abc\"]");
  EXPECT_EQ(result->type(), starlark_types::none_t);
  EXPECT_EQ(result->str(), "None");
}

TEST(StarlarkList, InsertBigIntIndex) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  starlark_string str("abc"sv);
  starlark_bigint idx(number::one());
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&idx);
  pos_args.push_back(&str);
  auto* method = list1.dot("insert", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list1.str(), "[0, \"abc\", 1, 0, 0, 1, 1]");
  EXPECT_EQ(result->type(), starlark_types::none_t);
  EXPECT_EQ(result->str(), "None");
}

TEST(StarlarkList, InsertNegativeBigIntIndexWithClamping) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  starlark_string str("abc"sv);
  starlark_bigint idx(number::minus_one() << 100);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&idx);
  pos_args.push_back(&str);
  auto* method = list1.dot("insert", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list1.str(), "[\"abc\", 0, 1, 0, 0, 1, 1]");
  EXPECT_EQ(result->type(), starlark_types::none_t);
  EXPECT_EQ(result->str(), "None");
}

TEST(StarlarkList, InsertPositiveBigIntIndexWithClamping) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  starlark_string str("abc"sv);
  starlark_bigint idx(number::one() << 100);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&idx);
  pos_args.push_back(&str);
  auto* method = list1.dot("insert", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1, \"abc\"]");
  EXPECT_EQ(result->type(), starlark_types::none_t);
  EXPECT_EQ(result->str(), "None");
}

TEST(StarlarkList, InsertWhileIterating) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  starlark_string str("abc"sv);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(&str);
  auto* method = list1.dot("insert", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  [[maybe_unused]] auto* it = list1.get_iterator(true, ctx, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
  EXPECT_EQ(error_callback.messages[0], "Error in append: list value is temporarily immutable due to active for-loop iteration");
}

TEST(StarlarkList, InsertNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = list1.dot("insert", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  [[maybe_unused]] auto* it = list1.get_iterator(true, ctx, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
  EXPECT_EQ(error_callback.messages[0], "TypeError: list.insert expected 2 arguments, got 0");
}

TEST(StarlarkList, InsertOneArgument) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  starlark_string str("abc"sv);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  auto* method = list1.dot("insert", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  [[maybe_unused]] auto* it = list1.get_iterator(true, ctx, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
  EXPECT_EQ(error_callback.messages[0], "TypeError: list.insert expected 2 arguments, got 1");
}

TEST(StarlarkList, InsertThreeArgument) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  starlark_string str("abc"sv);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(&str);
  pos_args.push_back(&str);
  auto* method = list1.dot("insert", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  [[maybe_unused]] auto* it = list1.get_iterator(true, ctx, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
  EXPECT_EQ(error_callback.messages[0], "TypeError: list.insert expected 2 arguments, got 3");
}

TEST(StarlarkList, InsertBoolIndex) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  starlark_string str("abc"sv);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.false_value());
  pos_args.push_back(&str);
  auto* method = list1.dot("insert", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  [[maybe_unused]] auto* it = list1.get_iterator(true, ctx, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
  EXPECT_EQ(error_callback.messages[0], "'bool' object cannot be interpreted as an integer");
}

TEST(StarlarkList, Pop) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = list1.dot("pop", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1]");
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "1");
}

TEST(StarlarkList, PopIndex) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.one());
  auto* method = list1.dot("pop", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list1.str(), "[0, 0, 0, 1, 1]");
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "1");
}

TEST(StarlarkList, PopNegativeIndex) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  starlark_integer idx(-3);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&idx);
  auto* method = list1.dot("pop", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list1.str(), "[0, 1, 0, 1, 1]");
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "0");
}

TEST(StarlarkList, PopTooSmallIndex) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  starlark_integer idx(-7);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&idx);
  auto* method = list1.dot("pop", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "pop index out of range");
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
}

TEST(StarlarkList, PopTooBigIndex) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  starlark_integer idx(6);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&idx);
  auto* method = list1.dot("pop", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "pop index out of range");
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
}

TEST(StarlarkList, PopWhileIterating) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = list1.dot("pop", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  [[maybe_unused]] auto* it = list1.get_iterator(true, ctx, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "Error in delete: list value is temporarily immutable due to active for-loop iteration");
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
}

TEST(StarlarkList, PopEmpty) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = list1.dot("pop", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "pop index out of range");
  EXPECT_EQ(list1.str(), "[]");
}

TEST(StarlarkList, PopBigInt) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  starlark_bigint idx(5);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&idx);
  auto* method = list1.dot("pop", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "1");
  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1]");
}

TEST(StarlarkList, PopBigIntTooSmall) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  starlark_bigint idx(number::minus_one() << 100);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&idx);
  auto* method = list1.dot("pop", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "pop index out of range");
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
}

TEST(StarlarkList, PopBigIntTooBig) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  starlark_bigint idx(number::one() << 100);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&idx);
  auto* method = list1.dot("pop", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "pop index out of range");
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
}

TEST(StarlarkList, PopBoolIndex) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.true_value());
  auto* method = list1.dot("pop", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'bool' object cannot be interpreted as an integer");
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
}

TEST(StarlarkList, PopTwoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  auto* method = list1.dot("pop", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: list.pop expected at most 1 argument, got 2");
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
}

TEST(StarlarkList, Remove) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.one());
  auto* method = list1.dot("remove", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(list1.str(), "[0, 0, 0, 1, 1]");
  EXPECT_EQ(result->type(), starlark_types::none_t);
  EXPECT_EQ(result->str(), "None");
}

TEST(StarlarkList, RemoveNoMatch) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.minus_one());
  auto* method = list1.dot("remove", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "list.remove(x): x not in list");
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
}

TEST(StarlarkList, RemoveWhileIterating) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.one());
  auto* method = list1.dot("remove", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  [[maybe_unused]] auto* it = list1.get_iterator(true, ctx, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "Error in delete: list value is temporarily immutable due to active for-loop iteration");
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
}

TEST(StarlarkList, RemoveNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = list1.dot("remove", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: list.remove() takes exactly one argument (0 given)");
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
}

TEST(StarlarkList, RemoveTwoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_list list1(0);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.zero(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);
  list1.append(ctx.one(), ctx, error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  auto* method = list1.dot("remove", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: list.remove() takes exactly one argument (2 given)");
  EXPECT_EQ(list1.str(), "[0, 1, 0, 0, 1, 1]");
}

}  // namespace
