// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <string>
#include <vector>

#include "runtime/starlark_bool.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_none.hpp"
#include "runtime/starlark_set.hpp"
#include "runtime/starlark_testing.hpp"
#include "runtime/starlark_tuple.hpp"

using ::google::protobuf::Arena;
using ::starlark::runtime::starlark_bool;
using ::starlark::runtime::starlark_integer;
using ::starlark::runtime::starlark_none;
using ::starlark::runtime::starlark_set;
using ::starlark::runtime::starlark_tuple;
using ::starlark::testing::error_handler;
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

TEST(StarlarkSet, BinaryInWithUnhashable) {
  starlark_set set_1;
  starlark_set set_2;
  error_handler error_callback;
  EXPECT_FALSE(set_1.binary_in(set_2, error_callback));
  EXPECT_THAT(error_callback.messages, SizeIs(0));
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
  EXPECT_EQ(error_callback.messages[0], "TypeError: trying to mutate a frozen set value");
}

TEST(StarlarkSet, AddUnhashable) {
  starlark_set set_1;
  starlark_set set_2;
  error_handler error_callback;

  set_1.add(&set_2, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: cannot use 'set' as a set element (unhashable type: 'set')");
}

TEST(StarlarkSet, BinaryPipe) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_integer three(3);
  starlark_set set_1;
  starlark_set set_2;
  Arena arena;
  error_handler error_callback;

  set_1.add(&zero, error_callback);
  set_1.add(&one, error_callback);
  set_2.add(&zero, error_callback);
  set_2.add(&two, error_callback);
  set_2.add(&three, error_callback);

  auto* set_3 = set_1.binary_pipe(set_2,  arena, error_callback);
  ASSERT_NE(set_3, nullptr);
  EXPECT_EQ(set_3->str(), "set([0, 1, 2, 3])");
  EXPECT_EQ(set_1.str(), "set([0, 1])");
  EXPECT_EQ(set_2.str(), "set([0, 2, 3])");
}

TEST(StarlarkSet, BinaryPipeWithNonSet) {
  starlark_set set;
  starlark_tuple tuple;
  Arena arena;
  error_handler error_callback;

  auto* result = set.binary_pipe(tuple, arena, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for |: 'set' and 'tuple'");
}

TEST(StarlarkSet, BinaryAnd) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_integer three(3);
  starlark_set set_1;
  starlark_set set_2;
  Arena arena;
  error_handler error_callback;

  set_1.add(&zero, error_callback);
  set_1.add(&one, error_callback);
  set_1.add(&three, error_callback);
  set_2.add(&three, error_callback);
  set_2.add(&zero, error_callback);
  set_2.add(&two, error_callback);

  auto* set_3 = set_1.binary_and(set_2,  arena, error_callback);
  ASSERT_NE(set_3, nullptr);
  EXPECT_EQ(set_3->str(), "set([0, 3])");
  EXPECT_EQ(set_1.str(), "set([0, 1, 3])");
  EXPECT_EQ(set_2.str(), "set([3, 0, 2])");
}

TEST(StarlarkSet, BinaryAndWithNonSet) {
  starlark_set set;
  starlark_tuple tuple;
  Arena arena;
  error_handler error_callback;

  auto* result = set.binary_and(tuple, arena, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for &: 'set' and 'tuple'");
}

TEST(StarlarkSet, BinaryHat) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_integer three(3);
  starlark_set set_1;
  starlark_set set_2;
  Arena arena;
  error_handler error_callback;

  set_1.add(&zero, error_callback);
  set_1.add(&one, error_callback);
  set_1.add(&three, error_callback);
  set_2.add(&three, error_callback);
  set_2.add(&zero, error_callback);
  set_2.add(&two, error_callback);

  auto* set_3 = set_1.binary_hat(set_2,  arena, error_callback);
  ASSERT_NE(set_3, nullptr);
  EXPECT_EQ(set_3->str(), "set([1, 2])");
  EXPECT_EQ(set_1.str(), "set([0, 1, 3])");
  EXPECT_EQ(set_2.str(), "set([3, 0, 2])");
}

TEST(StarlarkSet, BinaryHatWithNonSet) {
  starlark_set set;
  starlark_tuple tuple;
  Arena arena;
  error_handler error_callback;

  auto* result = set.binary_hat(tuple, arena, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for ^: 'set' and 'tuple'");
}

TEST(StarlarkSet, BinaryMinus) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_integer three(3);
  starlark_set set_1;
  starlark_set set_2;
  Arena arena;
  error_handler error_callback;

  set_1.add(&zero, error_callback);
  set_1.add(&one, error_callback);
  set_1.add(&three, error_callback);
  set_2.add(&three, error_callback);
  set_2.add(&zero, error_callback);
  set_2.add(&two, error_callback);

  auto* set_3 = set_1.binary_minus(set_2,  arena, error_callback);
  ASSERT_NE(set_3, nullptr);
  EXPECT_EQ(set_3->str(), "set([1])");
  EXPECT_EQ(set_1.str(), "set([0, 1, 3])");
  EXPECT_EQ(set_2.str(), "set([3, 0, 2])");
}

TEST(StarlarkSet, BinaryMinusWithNonSet) {
  starlark_set set;
  starlark_tuple tuple;
  Arena arena;
  error_handler error_callback;

  auto* result = set.binary_minus(tuple, arena, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: unsupported operand type(s) for -: 'set' and 'tuple'");
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

  EXPECT_EQ(0, set_1.len(error_callback));
  EXPECT_EQ(4, set_2.len(error_callback));
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkSet, GetIterator) {
  starlark_set set0;
  starlark_set set1;
  starlark_integer zero(0);
  starlark_integer one(1);
  Arena arena;
  error_handler error_callback;
  set1.add(&zero, error_callback);
  set1.add(&one, error_callback);

  auto* it0 = set0.get_iterator(arena, error_callback);
  EXPECT_FALSE(it0->has_next());
  it0->end_iterator();

  auto* it1 = set1.get_iterator(arena, error_callback);
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
  error_handler error_callback;
  set.add(&zero, error_callback);
  set.add(&one, error_callback);

  [[maybe_unused]] auto* it = set.get_iterator(arena, error_callback);
  EXPECT_THAT(error_callback.messages, IsEmpty());
  set.add(&zero, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("Error in append: set value is temporarily immutable due to active for-loop iteration", error_callback.messages[0]);
}

}  // namespace
