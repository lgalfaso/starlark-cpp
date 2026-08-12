// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <string>
#include <vector>

#include "runtime/starlark_bool.hpp"
#include "runtime/starlark_float.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_list.hpp"
#include "runtime/starlark_none.hpp"
#include "runtime/starlark_set.hpp"
#include "runtime/starlark_testing.hpp"
#include "runtime/starlark_tuple.hpp"
#include "runtime/starlark_types.hpp"

using ::google::protobuf::Arena;
using ::starlark::runtime::context;
using ::starlark::runtime::starlark_bool;
using ::starlark::runtime::starlark_float;
using ::starlark::runtime::starlark_integer;
using ::starlark::runtime::starlark_list;
using ::starlark::runtime::starlark_none;
using ::starlark::runtime::starlark_obj;
using ::starlark::runtime::starlark_set;
using ::starlark::runtime::starlark_tuple;
using ::starlark::runtime::starlark_types;
using ::starlark::testing::error_handler;
using ::testing::ElementsAre;
using ::testing::IsEmpty;
using ::testing::SizeIs;

namespace {

TEST(StarlarkSet, Type) {
  EXPECT_EQ("set", starlark_set().type());
}

TEST(StarlarkSet, Primitve) {
  EXPECT_FALSE(starlark_set().primitive());
}

TEST(StarlarkSet, Str) {
  starlark_none none;
  starlark_bool true_obj(true);
  starlark_integer one(1);
  error_handler error_callback;

  EXPECT_EQ("set()", starlark_set().str());
  starlark_set set1;
  set1.add(&none, error_callback);
  EXPECT_EQ("set([None])", set1.str());
  set1.add(&true_obj, error_callback);
  EXPECT_EQ("set([None, True])", set1.str());
  set1.add(&one, error_callback);
  EXPECT_EQ("set([None, True, 1])", set1.str());
}

TEST(StarlarkSet, Truthy) {
  starlark_none none;
  starlark_set set1;
  EXPECT_FALSE(set1.truthy());
  error_handler error_callback;

  set1.add(&none, error_callback);
  EXPECT_TRUE(set1.truthy());
}

TEST(StarlarkSet, Dir) {
  starlark_set set1;
  EXPECT_THAT(set1.dir(), ElementsAre(
      "add",          "clear",               "difference",           "difference_update",           "discard",
      "intersection", "intersection_update", "isdisjoint",           "issubset",                    "issuperset",
      "pop",          "remove",              "symmetric_difference", "symmetric_difference_update", "union",
      "update"));
}

TEST(StarlarkSet, AddingAnEqualsElementIsANoop) {
  starlark_float f_one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  starlark_set set1;
  set1.add(&f_one, error_callback);
  set1.add(ctx.one(), error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ("set([1.0])", set1.str());
}

TEST(StarlarkSet, Equals) {
  starlark_none none;
  starlark_set set1;
  starlark_set set2;
  starlark_set set3;
  error_handler error_callback;

  set2.add(&none, error_callback);
  set3.add(&none, error_callback);
  EXPECT_TRUE(starlark_set().equals(set1));
  EXPECT_FALSE(set2.equals(set1));
  EXPECT_FALSE(set1.equals(set2));
  EXPECT_TRUE(set2.equals(set3));
  EXPECT_FALSE(set2.equals(none));
  starlark_bool true_obj(true);
  starlark_bool false_obj(false);
  starlark_set set4;
  starlark_set set5;
  set4.add(&none, error_callback);
  set4.add(&true_obj, error_callback);
  set5.add(&none, error_callback);
  set5.add(&false_obj, error_callback);
  EXPECT_FALSE(set4.equals(set5));
}

TEST(StarlarkSet, EqualsInDifferentOrder) {
  starlark_none none;
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_bool bool_true(true);
  starlark_bool bool_false(false);
  starlark_set set_1;
  starlark_set set_2;
  starlark_set set_3;
  error_handler error_callback;

  set_1.add(&none, error_callback);
  set_1.add(&zero, error_callback);
  set_1.add(&one, error_callback);
  set_1.add(&two, error_callback);
  set_1.add(&bool_true, error_callback);
  set_1.add(&bool_false, error_callback);
  set_2.add(&none, error_callback);
  set_2.add(&zero, error_callback);
  set_2.add(&one, error_callback);
  set_2.add(&two, error_callback);
  set_2.add(&bool_true, error_callback);
  set_2.add(&bool_false, error_callback);
  set_3.add(&bool_false, error_callback);
  set_3.add(&bool_true, error_callback);
  set_3.add(&two, error_callback);
  set_3.add(&one, error_callback);
  set_3.add(&zero, error_callback);
  set_3.add(&none, error_callback);
  EXPECT_TRUE(set_1.equals(set_2));
  EXPECT_TRUE(set_1.equals(set_3));
}

TEST(StarlarkDictionary, Unpack) {
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_set set;
  std::vector<starlark_obj*> stack;
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  set.unpack(0, stack, ctx, error_callback);
  EXPECT_THAT(stack, SizeIs(0));

  set.add(&one, error_callback);
  set.unpack(1, stack, ctx, error_callback);
  ASSERT_THAT(stack, SizeIs(1));
  EXPECT_THAT(stack[0], &one);

  stack.clear();
  set.add(&two, error_callback);
  set.unpack(2, stack, ctx, error_callback);
  ASSERT_THAT(stack, SizeIs(2));
  EXPECT_THAT(stack[0], &two);
  EXPECT_THAT(stack[1], &one);
}

TEST(StarlarkDictionary, UnpackError) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_set set;
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  set.add(&zero, error_callback);
  set.add(&one, error_callback);
  {
    std::vector<starlark_obj*> consumer;
    error_handler error_callback;

    set.unpack(3, consumer, ctx, error_callback);
    ASSERT_THAT(consumer, IsEmpty());
    EXPECT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "not enough values to unpack (expected 3, got 2)");
  }
  {
    std::vector<starlark_obj*> consumer;
    error_handler error_callback;

    set.unpack(1, consumer, ctx, error_callback);
    ASSERT_THAT(consumer, IsEmpty());
    EXPECT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "too many values to unpack (expected 1, got 2)");
  }
}

TEST(StarlarkSet, Membership) {
  starlark_set set;
  starlark_integer zero(0);
  starlark_integer one(1);
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  set.add(&zero, error_callback);

  EXPECT_TRUE(set.binary_in(zero, error_callback));
  EXPECT_FALSE(set.binary_in(one, error_callback));
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkSet, MembershipNotHashable) {
  starlark_set set;
  starlark_list list(0);
  error_handler error_callback;
  Arena arena;
  context ctx(arena);

  EXPECT_FALSE(set.binary_in(list, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_THAT(error_callback.messages[0], "cannot use 'list' as a set element (unhashable type: 'list')");
}

TEST(StarlarkSet, Freeze) {
  starlark_set set;
  starlark_none none;
  starlark_integer zero(0);
  error_handler error_callback;

  set.add(&none, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set.hash(), -1);
  set.freeze();
  EXPECT_EQ(set.hash(), -1);
  set.add(&zero, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "trying to mutate a frozen set value");
}

TEST(StarlarkSet, AddUnhashable) {
  starlark_set set_1;
  starlark_set set_2;
  error_handler error_callback;

  set_1.add(&set_2, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "cannot use 'set' as a set element (unhashable type: 'set')");
}

TEST(StarlarkSet, BinaryPipe) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_integer three(3);
  starlark_set set_1;
  starlark_set set_2;
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  set_1.add(&zero, error_callback);
  set_1.add(&one, error_callback);
  set_2.add(&zero, error_callback);
  set_2.add(&two, error_callback);
  set_2.add(&three, error_callback);

  auto* set_3 = set_1.binary_pipe(set_2,  ctx, error_callback);
  ASSERT_NE(set_3, nullptr);
  EXPECT_EQ(set_3->str(), "set([0, 1, 2, 3])");
  EXPECT_EQ(set_1.str(), "set([0, 1])");
  EXPECT_EQ(set_2.str(), "set([0, 2, 3])");
}

TEST(StarlarkSet, BinaryPipeWithNonSet) {
  starlark_set set;
  starlark_tuple tuple(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = set.binary_pipe(tuple, ctx, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "unsupported operand type(s) for |: 'set' and 'tuple'");
}

TEST(StarlarkSet, PipeEqualsAssign) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_integer three(3);
  starlark_set set_1;
  starlark_set set_2;
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  set_1.add(&zero, error_callback);
  set_1.add(&one, error_callback);
  set_2.add(&zero, error_callback);
  set_2.add(&two, error_callback);
  set_2.add(&three, error_callback);

  auto* set_3 = set_1.pipe_equals_assign(set_2,  ctx, error_callback);
  ASSERT_NE(set_3, nullptr);
  EXPECT_EQ(set_3->str(), "set([0, 1, 2, 3])");
  EXPECT_EQ(set_1.str(), "set([0, 1, 2, 3])");
  EXPECT_EQ(set_2.str(), "set([0, 2, 3])");
}

TEST(StarlarkSet, PipeEqualsAssignSelf) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_set set;
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  set.add(&zero, error_callback);
  set.add(&one, error_callback);

  auto* result = set.pipe_equals_assign(set,  ctx, error_callback);
  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->str(), "set([0, 1])");
  EXPECT_EQ(set.str(), "set([0, 1])");
}

TEST(StarlarkSet, PipeEqualsAssignWhileIterating) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_set set;
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  set.add(&zero, error_callback);
  set.add(&one, error_callback);

  [[maybe_unused]] auto* it = set.get_iterator(true, ctx, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());
  auto* result = set.pipe_equals_assign(set,  ctx, error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("cannot perform merge, set value is temporarily immutable due to active for-loop iteration", error_callback.messages[0]);
}

TEST(StarlarkSet, PipeEqualsAssignWithNonSet) {
  starlark_set set;
  starlark_tuple tuple(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = set.pipe_equals_assign(tuple, ctx, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "unsupported operand type(s) for |=: 'set' and 'tuple'");
}

TEST(StarlarkSet, BinaryAnd) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_integer three(3);
  starlark_set set_1;
  starlark_set set_2;
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  set_1.add(&zero, error_callback);
  set_1.add(&one, error_callback);
  set_1.add(&three, error_callback);
  set_2.add(&three, error_callback);
  set_2.add(&zero, error_callback);
  set_2.add(&two, error_callback);

  auto* set_3 = set_1.binary_and(set_2,  ctx, error_callback);
  ASSERT_NE(set_3, nullptr);
  EXPECT_EQ(set_3->str(), "set([0, 3])");
  EXPECT_EQ(set_1.str(), "set([0, 1, 3])");
  EXPECT_EQ(set_2.str(), "set([3, 0, 2])");
}

TEST(StarlarkSet, AndEqualsAssignWhileIterating) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_set set;
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  set.add(&zero, error_callback);
  set.add(&one, error_callback);

  [[maybe_unused]] auto* it = set.get_iterator(true, ctx, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());
  auto* result = set.ampersand_equals_assign(set,  ctx, error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("cannot perform intersection, set value is temporarily immutable due to active for-loop iteration", error_callback.messages[0]);
}

TEST(StarlarkSet, BinaryAndWithNonSet) {
  starlark_set set;
  starlark_tuple tuple(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = set.binary_and(tuple, ctx, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "unsupported operand type(s) for &: 'set' and 'tuple'");
}

TEST(StarlarkSet, AndEqualsAssign) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_integer three(3);
  starlark_set set_1;
  starlark_set set_2;
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  set_1.add(&zero, error_callback);
  set_1.add(&one, error_callback);
  set_1.add(&three, error_callback);
  set_2.add(&three, error_callback);
  set_2.add(&zero, error_callback);
  set_2.add(&two, error_callback);

  auto* set_3 = set_1.ampersand_equals_assign(set_2,  ctx, error_callback);
  ASSERT_NE(set_3, nullptr);
  EXPECT_EQ(set_3->str(), "set([0, 3])");
  EXPECT_EQ(set_1.str(), "set([0, 3])");
  EXPECT_EQ(set_2.str(), "set([3, 0, 2])");
}

TEST(StarlarkSet, AndEqualsAssignSelf) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_set set;
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  set.add(&zero, error_callback);
  set.add(&one, error_callback);

  auto* result = set.ampersand_equals_assign(set,  ctx, error_callback);
  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->str(), "set([0, 1])");
  EXPECT_EQ(set.str(), "set([0, 1])");
}

TEST(StarlarkSet, AndEqualsAssignWithNonSet) {
  starlark_set set;
  starlark_tuple tuple(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = set.ampersand_equals_assign(tuple, ctx, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "unsupported operand type(s) for &=: 'set' and 'tuple'");
}

TEST(StarlarkSet, BinaryHat) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_integer three(3);
  starlark_set set_1;
  starlark_set set_2;
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  set_1.add(&zero, error_callback);
  set_1.add(&one, error_callback);
  set_1.add(&three, error_callback);
  set_2.add(&three, error_callback);
  set_2.add(&zero, error_callback);
  set_2.add(&two, error_callback);

  auto* set_3 = set_1.binary_hat(set_2,  ctx, error_callback);
  ASSERT_NE(set_3, nullptr);
  EXPECT_EQ(set_3->str(), "set([1, 2])");
  EXPECT_EQ(set_1.str(), "set([0, 1, 3])");
  EXPECT_EQ(set_2.str(), "set([3, 0, 2])");
}

TEST(StarlarkSet, BinaryHatWithNonSet) {
  starlark_set set;
  starlark_tuple tuple(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = set.binary_hat(tuple, ctx, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "unsupported operand type(s) for ^: 'set' and 'tuple'");
}

TEST(StarlarkSet, HatEqualsAssign) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_integer three(3);
  starlark_set set_1;
  starlark_set set_2;
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  set_1.add(&zero, error_callback);
  set_1.add(&one, error_callback);
  set_1.add(&three, error_callback);
  set_2.add(&three, error_callback);
  set_2.add(&zero, error_callback);
  set_2.add(&two, error_callback);

  auto* set_3 = set_1.hat_equals_assign(set_2,  ctx, error_callback);
  ASSERT_NE(set_3, nullptr);
  EXPECT_EQ(set_3->str(), "set([1, 2])");
  EXPECT_EQ(set_1.str(), "set([1, 2])");
  EXPECT_EQ(set_2.str(), "set([3, 0, 2])");
}

TEST(StarlarkSet, HatEqualsAssignSelf) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_set set;
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  set.add(&zero, error_callback);
  set.add(&one, error_callback);

  auto* result = set.hat_equals_assign(set,  ctx, error_callback);
  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->str(), "set()");
  EXPECT_EQ(set.str(), "set()");
}

TEST(StarlarkSet, HatEqualsAssignWhileIterating) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_set set;
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  set.add(&zero, error_callback);
  set.add(&one, error_callback);

  [[maybe_unused]] auto* it = set.get_iterator(true, ctx, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());
  auto* result = set.hat_equals_assign(set,  ctx, error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("cannot perform disjoin, set value is temporarily immutable due to active for-loop iteration", error_callback.messages[0]);
}

TEST(StarlarkSet, HatEqualsAssignWithNonSet) {
  starlark_set set;
  starlark_tuple tuple(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = set.hat_equals_assign(tuple, ctx, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "unsupported operand type(s) for ^=: 'set' and 'tuple'");
}

TEST(StarlarkSet, BinaryMinus) {
  starlark_integer two(2);
  starlark_integer three(3);
  starlark_set set_1;
  starlark_set set_2;
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  set_1.add(ctx.zero(), error_callback);
  set_1.add(ctx.one(), error_callback);
  set_1.add(&three, error_callback);
  set_2.add(&three, error_callback);
  set_2.add(ctx.zero(), error_callback);
  set_2.add(&two, error_callback);

  auto* set_3 = set_1.binary_minus(set_2,  ctx, error_callback);
  ASSERT_NE(set_3, nullptr);
  EXPECT_EQ(set_3->str(), "set([1])");
  EXPECT_EQ(set_1.str(), "set([0, 1, 3])");
  EXPECT_EQ(set_2.str(), "set([3, 0, 2])");
}

TEST(StarlarkSet, BinaryMinusWithNonSet) {
  starlark_set set;
  starlark_tuple tuple(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = set.binary_minus(tuple, ctx, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "unsupported operand type(s) for -: 'set' and 'tuple'");
}

TEST(StarlarkSet, MinusEqualsAssign) {
  starlark_integer two(2);
  starlark_integer three(3);
  starlark_set set_1;
  starlark_set set_2;
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  set_1.add(ctx.zero(), error_callback);
  set_1.add(ctx.one(), error_callback);
  set_1.add(&three, error_callback);
  set_2.add(&three, error_callback);
  set_2.add(ctx.zero(), error_callback);
  set_2.add(&two, error_callback);

  auto* set_3 = set_1.minus_equals_assign(set_2,  ctx, error_callback);
  ASSERT_NE(set_3, nullptr);
  EXPECT_EQ(set_3->str(), "set([1])");
  EXPECT_EQ(set_1.str(), "set([1])");
  EXPECT_EQ(set_2.str(), "set([3, 0, 2])");
}

TEST(StarlarkSet, MinusEqualsAssignSelf) {
  starlark_set set;
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);

  auto* result = set.minus_equals_assign(set,  ctx, error_callback);
  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->str(), "set()");
  EXPECT_EQ(set.str(), "set()");
}

TEST(StarlarkSet, MinusEqualsAssignWhileIterating) {
  starlark_set set;
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);

  [[maybe_unused]] auto* it = set.get_iterator(true, ctx, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());
  auto* result = set.minus_equals_assign(set,  ctx, error_callback);
  ASSERT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("cannot perform remove, set value is temporarily immutable due to active for-loop iteration", error_callback.messages[0]);
}

TEST(StarlarkSet, MinusEqualsAssignWithNonSet) {
  starlark_set set;
  starlark_tuple tuple(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = set.minus_equals_assign(tuple, ctx, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "unsupported operand type(s) for -=: 'set' and 'tuple'");
}

TEST(StarlarkSet, Len) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_integer three(3);
  starlark_set set_1;
  starlark_set set_2;
  error_handler error_callback;

  set_2.add(&zero, error_callback);
  set_2.add(&one, error_callback);
  set_2.add(&two, error_callback);
  set_2.add(&three, error_callback);

  EXPECT_EQ(0, set_1.len(true, error_callback));
  EXPECT_EQ(4, set_2.len(true, error_callback));
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkSet, GetIterator) {
  starlark_set set0;
  starlark_set set1;
  starlark_integer zero(0);
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  set1.add(&zero, error_callback);
  set1.add(&one, error_callback);

  auto* it0 = set0.get_iterator(true, ctx, error_callback);
  EXPECT_FALSE(it0->has_next());
  it0->end_iterator();

  auto* it1 = set1.get_iterator(true, ctx, error_callback);
  EXPECT_TRUE(it1->has_next());
  EXPECT_TRUE(it1->next()->equals(zero));
  EXPECT_TRUE(it1->has_next());
  EXPECT_TRUE(it1->next()->equals(one));
  EXPECT_FALSE(it1->has_next());
  it1->end_iterator();
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkSet, MutationWhileIterating) {
  starlark_set set;
  starlark_integer zero(0);
  starlark_integer one(1);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  set.add(&zero, error_callback);
  set.add(&one, error_callback);

  [[maybe_unused]] auto* it = set.get_iterator(true, ctx, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());
  set.add(&zero, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("cannot perform add, set value is temporarily immutable due to active for-loop iteration", error_callback.messages[0]);
}

TEST(StarlarkSet, Add) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  auto* method = set.dot("add", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_NE(nullptr, result);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set.str(), "set([0])");
  EXPECT_EQ(result->type(), starlark_types::none_t);
}

TEST(StarlarkSet, AddWhileIterating) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  auto* method = set.dot("add", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  [[maybe_unused]] auto* it = set.get_iterator(true, ctx, error_callback);
  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "cannot perform add, set value is temporarily immutable due to active for-loop iteration");
  EXPECT_EQ(set.str(), "set()");
}

TEST(StarlarkSet, AddNoPositionalArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = set.dot("add", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.add() takes exactly one argument (0 given)");
  EXPECT_EQ(set.str(), "set()");
}

TEST(StarlarkSet, AddTwoPositionalArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  auto* method = set.dot("add", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.add() takes exactly one argument (2 given)");
  EXPECT_EQ(set.str(), "set()");
}

TEST(StarlarkSet, AddWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("zero", ctx.zero());
  auto* method = set.dot("add", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.add() takes no keyword arguments");
  EXPECT_EQ(set.str(), "set()");
}

TEST(StarlarkSet, Clear) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = set.dot("clear", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set.str(), "set()");
}

TEST(StarlarkSet, ClearWhileIterating) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = set.dot("clear", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  [[maybe_unused]] auto* it = set.get_iterator(true, ctx, error_callback);
  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "cannot perform clear, set value is temporarily immutable due to active for-loop iteration");
  EXPECT_EQ(set.str(), "set([0])");
}

TEST(StarlarkSet, ClearOnePositionalArgument) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  auto* method = set.dot("clear", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.clear() takes no arguments (1 given)");
  EXPECT_EQ(set.str(), "set([0])");
}

TEST(StarlarkSet, ClearWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert("zero", ctx.zero());
  auto* method = set.dot("clear", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.clear() takes no keyword arguments");
  EXPECT_EQ(set.str(), "set([0])");
}

TEST(StarlarkSet, DifferenceNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = set1.dot("difference", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::set_t);
  EXPECT_EQ(result->str(), "set([0, 1])");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
}

TEST(StarlarkSet, DifferenceOneArgument) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set2.add(ctx.zero(), error_callback);
  set2.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  auto* method = set1.dot("difference", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::set_t);
  EXPECT_EQ(result->str(), "set([1])");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
  EXPECT_EQ(set2.str(), "set([0, -1])");
}

TEST(StarlarkSet, DifferenceTwoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_set set3;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  set2.add(ctx.minus_one(), error_callback);
  set3.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  pos_args.push_back(&set3);
  auto* method = set1.dot("difference", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::set_t);
  EXPECT_EQ(result->str(), "set([0])");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1, -1])");
  EXPECT_EQ(set2.str(), "set([-1])");
  EXPECT_EQ(set3.str(), "set([1])");
}

TEST(StarlarkSet, DifferenceThreeArgumentsIncludingSelf) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_set set3;
  set1.add(ctx.zero(), error_callback);
  set2.add(ctx.minus_one(), error_callback);
  set3.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set1);
  pos_args.push_back(&set2);
  pos_args.push_back(&set3);
  auto* method = set1.dot("difference", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::set_t);
  EXPECT_EQ(result->str(), "set()");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0])");
}

TEST(StarlarkSet, DifferenceOtherIterable) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_tuple tuple(2);
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  tuple.add(ctx.zero());
  tuple.add(ctx.one());

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = set1.dot("difference", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::set_t);
  EXPECT_EQ(result->str(), "set([-1])");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, DifferenceNonHashableElement) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_tuple tuple(1);
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  tuple.add(&set2);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = set1.dot("difference", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "cannot use 'set' as a set element (unhashable type: 'set')");
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, DifferenceNonIterable) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.one());
  auto* method = set1.dot("difference", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'int' object is not iterable");
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, DifferenceWhileIterating) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = set1.dot("difference", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  [[maybe_unused]] auto* it = set1.get_iterator(true, ctx, error_callback);
  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::set_t);
  EXPECT_EQ(result->str(), "set([0, -1])");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, DifferenceWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert("zero", ctx.zero());
  auto* method = set.dot("difference", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.difference() takes no keyword arguments");
  EXPECT_EQ(set.str(), "set([0, 1])");
}

TEST(StarlarkSet, DifferenceUpdateNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = set1.dot("difference_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
}

TEST(StarlarkSet, DifferenceUpdateOneArgument) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set2.add(ctx.zero(), error_callback);
  set2.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  auto* method = set1.dot("difference_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([1])");
  EXPECT_EQ(set2.str(), "set([0, -1])");
}

TEST(StarlarkSet, DifferenceUpdateTwoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_set set3;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  set2.add(ctx.minus_one(), error_callback);
  set3.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  pos_args.push_back(&set3);
  auto* method = set1.dot("difference_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0])");
  EXPECT_EQ(set2.str(), "set([-1])");
  EXPECT_EQ(set3.str(), "set([1])");
}

TEST(StarlarkSet, DifferenceUpdateThreeArgumentsIncludingSelf) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_set set3;
  set1.add(ctx.zero(), error_callback);
  set2.add(ctx.minus_one(), error_callback);
  set3.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set1);
  pos_args.push_back(&set2);
  pos_args.push_back(&set3);
  auto* method = set1.dot("difference_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set()");
}

TEST(StarlarkSet, DifferenceUpdateOtherIterable) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_tuple tuple(2);
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  tuple.add(ctx.zero());
  tuple.add(ctx.one());

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = set1.dot("difference_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([-1])");
}

TEST(StarlarkSet, DifferenceUpdateNonHashableElement) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_tuple tuple(1);
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  tuple.add(&set2);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = set1.dot("difference_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "cannot use 'set' as a set element (unhashable type: 'set')");
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, DifferenceUpdateNonIterable) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.one());
  auto* method = set1.dot("difference_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'int' object is not iterable");
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, DifferenceUpdateWhileIterating) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = set1.dot("difference_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  [[maybe_unused]] auto* it = set1.get_iterator(true, ctx, error_callback);
  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "cannot perform difference_update, set value is temporarily immutable due to active for-loop iteration");
}

TEST(StarlarkSet, DifferenceUpdateWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert("zero", ctx.zero());
  auto* method = set.dot("difference_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.difference_update() takes no keyword arguments");
  EXPECT_EQ(set.str(), "set([0, 1])");
}

TEST(StarlarkSet, Discard) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.one());
  auto* method = set.dot("discard", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set.str(), "set([0])");
}

TEST(StarlarkSet, DiscardElementNotInSet) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.minus_one());
  auto* method = set.dot("discard", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set.str(), "set([0, 1])");
}

TEST(StarlarkSet, DiscardWhileIterating) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.one());
  auto* method = set.dot("discard", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  [[maybe_unused]] auto* it = set.get_iterator(true, ctx, error_callback);
  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "cannot perform discard, set value is temporarily immutable due to active for-loop iteration");
  EXPECT_EQ(set.str(), "set([0, 1])");
}

TEST(StarlarkSet, DiscardUnhashable) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);
  starlark_set other_set;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&other_set);
  auto* method = set.dot("discard", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "cannot use 'set' as a set element (unhashable type: 'set')");
  EXPECT_EQ(set.str(), "set([0, 1])");
}

TEST(StarlarkSet, DiscardNoPositionalArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = set.dot("discard", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.discard() takes exactly one argument (0 given)");
  EXPECT_EQ(set.str(), "set([0, 1])");
}

TEST(StarlarkSet, DiscardWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert("zero", ctx.zero());
  auto* method = set.dot("discard", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.discard() takes no keyword arguments");
  EXPECT_EQ(set.str(), "set([0, 1])");
}

TEST(StarlarkSet, IntersectionNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = set1.dot("intersection", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::set_t);
  EXPECT_EQ(result->str(), "set([0, 1])");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
}

TEST(StarlarkSet, IntersectionOneArgument) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set2.add(ctx.zero(), error_callback);
  set2.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  auto* method = set1.dot("intersection", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::set_t);
  EXPECT_EQ(result->str(), "set([0])");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
  EXPECT_EQ(set2.str(), "set([0, -1])");
}

TEST(StarlarkSet, IntersectionTwoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_set set3;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  set2.add(ctx.minus_one(), error_callback);
  set3.add(ctx.one(), error_callback);
  set3.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  pos_args.push_back(&set3);
  auto* method = set1.dot("intersection", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::set_t);
  EXPECT_EQ(result->str(), "set([-1])");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1, -1])");
  EXPECT_EQ(set2.str(), "set([-1])");
  EXPECT_EQ(set3.str(), "set([1, -1])");
}

TEST(StarlarkSet, IntersectionThreeArgumentsIncludingSelf) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_set set3;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  set2.add(ctx.minus_one(), error_callback);
  set3.add(ctx.one(), error_callback);
  set3.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set1);
  pos_args.push_back(&set2);
  pos_args.push_back(&set3);
  auto* method = set1.dot("intersection", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::set_t);
  EXPECT_EQ(result->str(), "set([-1])");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, IntersectionOtherIterable) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_tuple tuple(2);
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  tuple.add(ctx.zero());
  tuple.add(ctx.one());

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = set1.dot("intersection", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::set_t);
  EXPECT_EQ(result->str(), "set([0])");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, IntersectionNonHashableElement) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_tuple tuple(1);
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  tuple.add(&set2);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = set1.dot("intersection", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "cannot use 'set' as a set element (unhashable type: 'set')");
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, IntersectionNonIterable) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.one());
  auto* method = set1.dot("intersection", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'int' object is not iterable");
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, IntersectionWhileIterating) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = set1.dot("intersection", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  [[maybe_unused]] auto* it = set1.get_iterator(true, ctx, error_callback);
  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::set_t);
  EXPECT_EQ(result->str(), "set([0, -1])");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, IntersectionWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert("zero", ctx.zero());
  auto* method = set.dot("intersection", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.intersection() takes no keyword arguments");
  EXPECT_EQ(set.str(), "set([0, 1])");
}

TEST(StarlarkSet, IntersectionUpdateNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = set1.dot("intersection_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
}

TEST(StarlarkSet, IntersectionUpdateOneArgument) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set2.add(ctx.zero(), error_callback);
  set2.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  auto* method = set1.dot("intersection_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0])");
  EXPECT_EQ(set2.str(), "set([0, -1])");
}

TEST(StarlarkSet, IntersectionUpdateTwoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_set set3;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  set2.add(ctx.minus_one(), error_callback);
  set3.add(ctx.one(), error_callback);
  set3.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  pos_args.push_back(&set3);
  auto* method = set1.dot("intersection_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([-1])");
  EXPECT_EQ(set2.str(), "set([-1])");
  EXPECT_EQ(set3.str(), "set([1, -1])");
}

TEST(StarlarkSet, IntersectionUpdateThreeArgumentsIncludingSelf) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_set set3;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  set2.add(ctx.minus_one(), error_callback);
  set3.add(ctx.one(), error_callback);
  set3.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set1);
  pos_args.push_back(&set2);
  pos_args.push_back(&set3);
  auto* method = set1.dot("intersection_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([-1])");
}

TEST(StarlarkSet, IntersectionUpdateOtherIterable) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_tuple tuple(2);
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  tuple.add(ctx.zero());
  tuple.add(ctx.one());

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = set1.dot("intersection_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0])");
}

TEST(StarlarkSet, IntersectionUpdateNonHashableElement) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_tuple tuple(1);
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  tuple.add(&set2);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = set1.dot("intersection_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "cannot use 'set' as a set element (unhashable type: 'set')");
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, IntersectionUpdateNonIterable) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.one());
  auto* method = set1.dot("intersection_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'int' object is not iterable");
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, IntersectionUpdateWhileIterating) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = set1.dot("intersection_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  [[maybe_unused]] auto* it = set1.get_iterator(true, ctx, error_callback);
  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "cannot perform intersection_update, set value is temporarily immutable due to active for-loop iteration");
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, IntersectionUpdateWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert("zero", ctx.zero());
  auto* method = set.dot("intersection_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.intersection_update() takes no keyword arguments");
  EXPECT_EQ(set.str(), "set([0, 1])");
}

TEST(StarlarkSet, IsdisjointNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = set1.dot("isdisjoint", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.isdisjoint() takes exactly one argument (0 given)");
  EXPECT_EQ(set1.str(), "set([0, 1])");
}

TEST(StarlarkSet, IsdisjointOverlap) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set2.add(ctx.zero(), error_callback);
  set2.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  auto* method = set1.dot("isdisjoint", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bool_t);
  EXPECT_EQ(result->str(), "False");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
  EXPECT_EQ(set2.str(), "set([0, -1])");
}

TEST(StarlarkSet, IsdisjointDisjoint) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set2.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  auto* method = set1.dot("isdisjoint", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bool_t);
  EXPECT_EQ(result->str(), "True");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
  EXPECT_EQ(set2.str(), "set([-1])");
}

TEST(StarlarkSet, IsdisjointSubset) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set2.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  auto* method = set1.dot("isdisjoint", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bool_t);
  EXPECT_EQ(result->str(), "False");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
  EXPECT_EQ(set2.str(), "set([1])");
}

TEST(StarlarkSet, IsdisjointSuperset) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set2.add(ctx.zero(), error_callback);
  set2.add(ctx.one(), error_callback);
  set2.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  auto* method = set1.dot("isdisjoint", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bool_t);
  EXPECT_EQ(result->str(), "False");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
  EXPECT_EQ(set2.str(), "set([0, 1, -1])");
}

TEST(StarlarkSet, IsdisjointEqual) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set2.add(ctx.zero(), error_callback);
  set2.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  auto* method = set1.dot("isdisjoint", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bool_t);
  EXPECT_EQ(result->str(), "False");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
  EXPECT_EQ(set2.str(), "set([0, 1])");
}

TEST(StarlarkSet, IsdisjointSelf) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set1);
  auto* method = set1.dot("isdisjoint", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bool_t);
  EXPECT_EQ(result->str(), "False");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
}

TEST(StarlarkSet, IsdisjointTwoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_set set3;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  set2.add(ctx.minus_one(), error_callback);
  set3.add(ctx.one(), error_callback);
  set3.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  pos_args.push_back(&set3);
  auto* method = set1.dot("isdisjoint", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.isdisjoint() takes exactly one argument (2 given)");
  EXPECT_EQ(set1.str(), "set([0, 1, -1])");
  EXPECT_EQ(set2.str(), "set([-1])");
  EXPECT_EQ(set3.str(), "set([1, -1])");
}

TEST(StarlarkSet, IsdisjointOtherIterable) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_tuple tuple(2);
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  tuple.add(ctx.zero());
  tuple.add(ctx.one());

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = set1.dot("isdisjoint", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bool_t);
  EXPECT_EQ(result->str(), "False");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, IsdisjointNonHashableElement) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_tuple tuple(1);
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  tuple.add(&set2);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = set1.dot("isdisjoint", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "cannot use 'set' as a set element (unhashable type: 'set')");
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, IsdisjointNonIterable) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.one());
  auto* method = set1.dot("isdisjoint", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'int' object is not iterable");
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, IsdisjointWhileIterating) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set1);
  auto* method = set1.dot("isdisjoint", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  [[maybe_unused]] auto* it = set1.get_iterator(true, ctx, error_callback);
  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bool_t);
  EXPECT_EQ(result->str(), "False");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, IsdisjointWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert("zero", ctx.zero());
  auto* method = set.dot("isdisjoint", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.isdisjoint() takes no keyword arguments");
  EXPECT_EQ(set.str(), "set([0, 1])");
}

TEST(StarlarkSet, IssubsetNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = set1.dot("issubset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.issubset() takes exactly one argument (0 given)");
  EXPECT_EQ(set1.str(), "set([0, 1])");
}

TEST(StarlarkSet, IssubsetOverlap) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set2.add(ctx.zero(), error_callback);
  set2.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  auto* method = set1.dot("issubset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bool_t);
  EXPECT_EQ(result->str(), "False");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
  EXPECT_EQ(set2.str(), "set([0, -1])");
}

TEST(StarlarkSet, IssubsetDisjoint) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set2.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  auto* method = set1.dot("issubset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bool_t);
  EXPECT_EQ(result->str(), "False");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
  EXPECT_EQ(set2.str(), "set([-1])");
}

TEST(StarlarkSet, IssubsetSubset) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set2.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  auto* method = set1.dot("issubset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bool_t);
  EXPECT_EQ(result->str(), "False");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
  EXPECT_EQ(set2.str(), "set([1])");
}

TEST(StarlarkSet, IssubsetSuperset) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set2.add(ctx.zero(), error_callback);
  set2.add(ctx.one(), error_callback);
  set2.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  auto* method = set1.dot("issubset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bool_t);
  EXPECT_EQ(result->str(), "True");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
  EXPECT_EQ(set2.str(), "set([0, 1, -1])");
}

TEST(StarlarkSet, IssubsetEqual) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set2.add(ctx.zero(), error_callback);
  set2.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  auto* method = set1.dot("issubset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bool_t);
  EXPECT_EQ(result->str(), "True");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
  EXPECT_EQ(set2.str(), "set([0, 1])");
}

TEST(StarlarkSet, IssubsetSelf) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set1);
  auto* method = set1.dot("issubset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bool_t);
  EXPECT_EQ(result->str(), "True");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
}

TEST(StarlarkSet, IssubsetTwoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_set set3;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  set2.add(ctx.minus_one(), error_callback);
  set3.add(ctx.one(), error_callback);
  set3.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  pos_args.push_back(&set3);
  auto* method = set1.dot("issubset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.issubset() takes exactly one argument (2 given)");
  EXPECT_EQ(set1.str(), "set([0, 1, -1])");
  EXPECT_EQ(set2.str(), "set([-1])");
  EXPECT_EQ(set3.str(), "set([1, -1])");
}

TEST(StarlarkSet, IssubsetOtherIterable) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_tuple tuple(2);
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  tuple.add(ctx.zero());
  tuple.add(ctx.one());

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = set1.dot("issubset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bool_t);
  EXPECT_EQ(result->str(), "False");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, IssubsetNonHashableElement) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_tuple tuple(1);
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  tuple.add(&set2);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = set1.dot("issubset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "cannot use 'set' as a set element (unhashable type: 'set')");
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, IssubsetNonIterable) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.one());
  auto* method = set1.dot("issubset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'int' object is not iterable");
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, IssubsetWhileIterating) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set1);
  auto* method = set1.dot("issubset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  [[maybe_unused]] auto* it = set1.get_iterator(true, ctx, error_callback);
  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bool_t);
  EXPECT_EQ(result->str(), "True");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, IssubsetWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert("zero", ctx.zero());
  auto* method = set.dot("issubset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.issubset() takes no keyword arguments");
  EXPECT_EQ(set.str(), "set([0, 1])");
}

TEST(StarlarkSet, IssupersetNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = set1.dot("issuperset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.issuperset() takes exactly one argument (0 given)");
  EXPECT_EQ(set1.str(), "set([0, 1])");
}

TEST(StarlarkSet, IssupersetOverlap) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set2.add(ctx.zero(), error_callback);
  set2.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  auto* method = set1.dot("issuperset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bool_t);
  EXPECT_EQ(result->str(), "False");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
  EXPECT_EQ(set2.str(), "set([0, -1])");
}

TEST(StarlarkSet, IssupersetDisjoint) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set2.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  auto* method = set1.dot("issuperset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bool_t);
  EXPECT_EQ(result->str(), "False");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
  EXPECT_EQ(set2.str(), "set([-1])");
}

TEST(StarlarkSet, IssupersetSubset) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set2.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  auto* method = set1.dot("issuperset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bool_t);
  EXPECT_EQ(result->str(), "True");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
  EXPECT_EQ(set2.str(), "set([1])");
}

TEST(StarlarkSet, IssupersetSuperset) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set2.add(ctx.zero(), error_callback);
  set2.add(ctx.one(), error_callback);
  set2.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  auto* method = set1.dot("issuperset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bool_t);
  EXPECT_EQ(result->str(), "False");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
  EXPECT_EQ(set2.str(), "set([0, 1, -1])");
}

TEST(StarlarkSet, IssupersetEqual) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set2.add(ctx.zero(), error_callback);
  set2.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  auto* method = set1.dot("issuperset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bool_t);
  EXPECT_EQ(result->str(), "True");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
  EXPECT_EQ(set2.str(), "set([0, 1])");
}

TEST(StarlarkSet, IssupersetSelf) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set1);
  auto* method = set1.dot("issuperset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bool_t);
  EXPECT_EQ(result->str(), "True");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
}

TEST(StarlarkSet, IssupersetTwoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_set set3;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  set2.add(ctx.minus_one(), error_callback);
  set3.add(ctx.one(), error_callback);
  set3.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  pos_args.push_back(&set3);
  auto* method = set1.dot("issuperset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.issuperset() takes exactly one argument (2 given)");
  EXPECT_EQ(set1.str(), "set([0, 1, -1])");
  EXPECT_EQ(set2.str(), "set([-1])");
  EXPECT_EQ(set3.str(), "set([1, -1])");
}

TEST(StarlarkSet, IssupersetOtherIterable) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_tuple tuple(2);
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  tuple.add(ctx.zero());
  tuple.add(ctx.one());

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = set1.dot("issuperset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bool_t);
  EXPECT_EQ(result->str(), "False");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, IssupersetNonHashableElement) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_tuple tuple(1);
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  tuple.add(&set2);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = set1.dot("issuperset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "cannot use 'set' as a set element (unhashable type: 'set')");
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, IssupersetNonIterable) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.one());
  auto* method = set1.dot("issuperset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'int' object is not iterable");
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, IssupersetWhileIterating) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set1);
  auto* method = set1.dot("issuperset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  [[maybe_unused]] auto* it = set1.get_iterator(true, ctx, error_callback);
  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bool_t);
  EXPECT_EQ(result->str(), "True");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, IssupersetWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert("zero", ctx.zero());
  auto* method = set.dot("issuperset", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.issuperset() takes no keyword arguments");
  EXPECT_EQ(set.str(), "set([0, 1])");
}

TEST(StarlarkSet, Pop) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = set.dot("pop", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_TRUE(result->equals(*ctx.zero()));

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set.str(), "set([1])");
}

TEST(StarlarkSet, PopWithEmpty) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = set.dot("pop", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "pop from an empty set");
  EXPECT_EQ(set.str(), "set()");
}

TEST(StarlarkSet, PopWhileIterating) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = set.dot("pop", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  [[maybe_unused]] auto* it = set.get_iterator(true, ctx, error_callback);
  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "cannot perform pop, set value is temporarily immutable due to active for-loop iteration");
  EXPECT_EQ(set.str(), "set([0])");
}

TEST(StarlarkSet, PopOnePositionalArgument) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  auto* method = set.dot("pop", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.pop() takes no arguments (1 given)");
  EXPECT_EQ(set.str(), "set([0])");
}

TEST(StarlarkSet, PopWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert("zero", ctx.zero());
  auto* method = set.dot("pop", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.pop() takes no keyword arguments");
  EXPECT_EQ(set.str(), "set([0])");
}

TEST(StarlarkSet, Remove) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.one());
  auto* method = set.dot("remove", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set.str(), "set([0])");
}

TEST(StarlarkSet, RemoveElementNotInSet) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.minus_one());
  auto* method = set.dot("remove", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "key not found '-1'");
  EXPECT_EQ(set.str(), "set([0, 1])");
}

TEST(StarlarkSet, RemoveWhileIterating) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.one());
  auto* method = set.dot("remove", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  [[maybe_unused]] auto* it = set.get_iterator(true, ctx, error_callback);
  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "cannot perform remove, set value is temporarily immutable due to active for-loop iteration");
  EXPECT_EQ(set.str(), "set([0, 1])");
}

TEST(StarlarkSet, RemoveUnhashable) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);
  starlark_set other_set;

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&other_set);
  auto* method = set.dot("remove", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "cannot use 'set' as a set element (unhashable type: 'set')");
  EXPECT_EQ(set.str(), "set([0, 1])");
}

TEST(StarlarkSet, RemoveNoPositionalArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = set.dot("remove", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.remove() takes exactly one argument (0 given)");
  EXPECT_EQ(set.str(), "set([0, 1])");
}

TEST(StarlarkSet, RemoveWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert("zero", ctx.zero());
  auto* method = set.dot("remove", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.remove() takes no keyword arguments");
  EXPECT_EQ(set.str(), "set([0, 1])");
}

TEST(StarlarkSet, SymmetricDifferenceNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = set1.dot("symmetric_difference", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(error_callback.messages[0], "set.symmetric_difference() takes exactly one argument (0 given)");
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(set1.str(), "set([0, 1])");
}

TEST(StarlarkSet, SymmetricDifferenceOneArgument) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set2.add(ctx.zero(), error_callback);
  set2.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  auto* method = set1.dot("symmetric_difference", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::set_t);
  EXPECT_EQ(result->str(), "set([1, -1])");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
  EXPECT_EQ(set2.str(), "set([0, -1])");
}

TEST(StarlarkSet, SymmetricDifferenceTwoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_set set3;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  set2.add(ctx.minus_one(), error_callback);
  set3.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  pos_args.push_back(&set3);
  auto* method = set1.dot("symmetric_difference", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.symmetric_difference() takes exactly one argument (2 given)");
  EXPECT_EQ(set1.str(), "set([0, 1, -1])");
  EXPECT_EQ(set2.str(), "set([-1])");
  EXPECT_EQ(set3.str(), "set([1])");
}

TEST(StarlarkSet, SymmetricDifferenceSelf) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  set1.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set1);
  auto* method = set1.dot("symmetric_difference", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::set_t);
  EXPECT_EQ(result->str(), "set()");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, -1, 1])");
}

TEST(StarlarkSet, SymmetricDifferenceOtherIterable) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_tuple tuple(2);
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  tuple.add(ctx.zero());
  tuple.add(ctx.one());

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = set1.dot("symmetric_difference", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::set_t);
  EXPECT_EQ(result->str(), "set([-1, 1])");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, SymmetricDifferenceNonHashableElement) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_tuple tuple(1);
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  tuple.add(&set2);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = set1.dot("symmetric_difference", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "cannot use 'set' as a set element (unhashable type: 'set')");
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, SymmetricDifferenceNonIterable) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.one());
  auto* method = set1.dot("symmetric_difference", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'int' object is not iterable");
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, SymmetricDifferenceWhileIterating) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set1);
  auto* method = set1.dot("symmetric_difference", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  [[maybe_unused]] auto* it = set1.get_iterator(true, ctx, error_callback);
  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::set_t);
  EXPECT_EQ(result->str(), "set()");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, SymmetricDifferenceWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert("zero", ctx.zero());
  auto* method = set.dot("symmetric_difference", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.symmetric_difference() takes no keyword arguments");
  EXPECT_EQ(set.str(), "set([0, 1])");
}

TEST(StarlarkSet, SymmetricDifferenceUpdateNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = set1.dot("symmetric_difference_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.symmetric_difference_update() takes exactly one argument (0 given)");
  EXPECT_EQ(set1.str(), "set([0, 1])");
}

TEST(StarlarkSet, SymmetricDifferenceUpdateOneArgument) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set2.add(ctx.zero(), error_callback);
  set2.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  auto* method = set1.dot("symmetric_difference_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([1, -1])");
  EXPECT_EQ(set2.str(), "set([0, -1])");
}

TEST(StarlarkSet, SymmetricDifferenceUpdateTwoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_set set3;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  set2.add(ctx.minus_one(), error_callback);
  set3.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  pos_args.push_back(&set3);
  auto* method = set1.dot("symmetric_difference_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.symmetric_difference_update() takes exactly one argument (2 given)");
  EXPECT_EQ(set1.str(), "set([0, 1, -1])");
  EXPECT_EQ(set2.str(), "set([-1])");
  EXPECT_EQ(set3.str(), "set([1])");
}

TEST(StarlarkSet, SymmetricDifferenceUpdateSelf) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  set1.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set1);
  auto* method = set1.dot("symmetric_difference_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set()");
}

TEST(StarlarkSet, SymmetricDifferenceUpdateOtherIterable) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_tuple tuple(2);
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  tuple.add(ctx.zero());
  tuple.add(ctx.one());

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = set1.dot("symmetric_difference_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([-1, 1])");
}

TEST(StarlarkSet, SymmetricDifferenceUpdateNonHashableElement) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_tuple tuple(1);
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  tuple.add(&set2);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = set1.dot("symmetric_difference_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "cannot use 'set' as a set element (unhashable type: 'set')");
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, SymmetricDifferenceUpdateNonIterable) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.one());
  auto* method = set1.dot("symmetric_difference_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'int' object is not iterable");
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, SymmetricDifferenceUpdateWhileIterating) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set1);
  auto* method = set1.dot("symmetric_difference_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  [[maybe_unused]] auto* it = set1.get_iterator(true, ctx, error_callback);
  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "cannot perform symmetric_difference_update, set value is temporarily immutable due to active for-loop iteration");
}

TEST(StarlarkSet, SymmetricDifferenceUpdateWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert("zero", ctx.zero());
  auto* method = set.dot("symmetric_difference_update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.symmetric_difference_update() takes no keyword arguments");
  EXPECT_EQ(set.str(), "set([0, 1])");
}

TEST(StarlarkSet, UnionNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = set1.dot("union", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::set_t);
  EXPECT_EQ(result->str(), "set([0, 1])");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
}

TEST(StarlarkSet, UnionOneArgument) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set2.add(ctx.zero(), error_callback);
  set2.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  auto* method = set1.dot("union", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::set_t);
  EXPECT_EQ(result->str(), "set([0, 1, -1])");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
  EXPECT_EQ(set2.str(), "set([0, -1])");
}

TEST(StarlarkSet, UnionTwoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_set set3;
  set1.add(ctx.zero(), error_callback);
  set2.add(ctx.minus_one(), error_callback);
  set3.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  pos_args.push_back(&set3);
  auto* method = set1.dot("union", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::set_t);
  EXPECT_EQ(result->str(), "set([0, -1, 1])");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0])");
}

TEST(StarlarkSet, UnionThreeArgumentsIncludingSelf) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_set set3;
  set1.add(ctx.zero(), error_callback);
  set2.add(ctx.minus_one(), error_callback);
  set3.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set1);
  pos_args.push_back(&set2);
  pos_args.push_back(&set3);
  auto* method = set1.dot("union", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::set_t);
  EXPECT_EQ(result->str(), "set([0, -1, 1])");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0])");
}

TEST(StarlarkSet, UnionOtherIterable) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_tuple tuple(1);
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  tuple.add(ctx.one());

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = set1.dot("union", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::set_t);
  EXPECT_EQ(result->str(), "set([0, -1, 1])");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, UnionNonHashableElement) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_tuple tuple(1);
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  tuple.add(&set2);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = set1.dot("union", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "cannot use 'set' as a set element (unhashable type: 'set')");
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, UnionNonIterable) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.one());
  auto* method = set1.dot("union", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'int' object is not iterable");
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, UnionWhileIterating) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = set1.dot("union", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  [[maybe_unused]] auto* it = set1.get_iterator(true, ctx, error_callback);
  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::set_t);
  EXPECT_EQ(result->str(), "set([0, -1])");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, UnionWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert("zero", ctx.zero());
  auto* method = set.dot("union", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.union() takes no keyword arguments");
  EXPECT_EQ(set.str(), "set([0, 1])");
}

TEST(StarlarkSet, UpdateNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = set1.dot("update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1])");
}

TEST(StarlarkSet, UpdateOneArgument) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.one(), error_callback);
  set2.add(ctx.zero(), error_callback);
  set2.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  auto* method = set1.dot("update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, 1, -1])");
  EXPECT_EQ(set2.str(), "set([0, -1])");
}

TEST(StarlarkSet, UpdateTwoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_set set3;
  set1.add(ctx.zero(), error_callback);
  set2.add(ctx.minus_one(), error_callback);
  set3.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set2);
  pos_args.push_back(&set3);
  auto* method = set1.dot("update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, -1, 1])");
}

TEST(StarlarkSet, UpdateThreeArgumentsIncludingSelf) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_set set3;
  set1.add(ctx.zero(), error_callback);
  set2.add(ctx.minus_one(), error_callback);
  set3.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&set1);
  pos_args.push_back(&set2);
  pos_args.push_back(&set3);
  auto* method = set1.dot("update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, -1, 1])");
}

TEST(StarlarkSet, UpdateOtherIterable) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_tuple tuple(1);
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  tuple.add(ctx.one());

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = set1.dot("update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::none_t);

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(set1.str(), "set([0, -1, 1])");
}

TEST(StarlarkSet, UpdateNonHashableElement) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  starlark_set set2;
  starlark_tuple tuple(1);
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);
  tuple.add(&set2);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = set1.dot("update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "cannot use 'set' as a set element (unhashable type: 'set')");
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, UpdateNonIterable) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.one());
  auto* method = set1.dot("update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "'int' object is not iterable");
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, UpdateWhileIterating) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set1;
  set1.add(ctx.zero(), error_callback);
  set1.add(ctx.minus_one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = set1.dot("update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  [[maybe_unused]] auto* it = set1.get_iterator(true, ctx, error_callback);
  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "cannot perform update, set value is temporarily immutable due to active for-loop iteration");
  EXPECT_EQ(set1.str(), "set([0, -1])");
}

TEST(StarlarkSet, UpdateWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_set set;
  set.add(ctx.zero(), error_callback);
  set.add(ctx.one(), error_callback);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert("zero", ctx.zero());
  auto* method = set.dot("update", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "set.update() takes no keyword arguments");
  EXPECT_EQ(set.str(), "set([0, 1])");
}

}  // namespace
