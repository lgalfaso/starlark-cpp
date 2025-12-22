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

TEST(StarlarkTuple, Type) {
  EXPECT_EQ("tuple", starlark_tuple().type());
}

TEST(StarlarkTuple, Str) {
  starlark_none none;
  starlark_bool true_obj(true);
  starlark_integer one(1);
  EXPECT_EQ("()", starlark_tuple().str());
  EXPECT_EQ("(None,)", starlark_tuple().add(&none).str());
  EXPECT_EQ("(None, True)", starlark_tuple().add(&none).add(&true_obj).str());
  EXPECT_EQ("(None, True, 1)", starlark_tuple().add(&none).add(&true_obj).add(&one).str());
}

TEST(StarlarkTuple, StrRecursion) {
  // This test is not fully real, as this specific case cannot happen. That said, it is
  // possible to have an equivalent case by doing:
  //   a = ([],)
  //   a[0].append(a)
  // In the example above, the result should be that `str(a) == '([(...)],)'`.
  // This test is equivalent.
  starlark_tuple tuple;
  tuple.add(&tuple);
  EXPECT_EQ("((...),)", tuple.str());
}

TEST(StarlarkTuple, Truthy) {
  starlark_none none;
  EXPECT_FALSE(starlark_tuple().truthy());
  EXPECT_TRUE(starlark_tuple().add(&none).truthy());
}

TEST(StarlarkTuple, Equals) {
  starlark_none none;
  starlark_integer one(1);
  EXPECT_FALSE(starlark_tuple().equals(none));
  EXPECT_TRUE(starlark_tuple().equals(starlark_tuple()));
  EXPECT_FALSE(starlark_tuple().add(&none).equals(starlark_tuple()));
  EXPECT_FALSE(starlark_tuple().add(&one).equals(starlark_tuple()));
  EXPECT_FALSE(starlark_tuple().add(&none).add(&one).equals(starlark_tuple()));
  EXPECT_FALSE(starlark_tuple().add(&one).add(&none).equals(starlark_tuple()));

  EXPECT_FALSE(starlark_tuple().equals(starlark_tuple().add(&none)));
  EXPECT_TRUE(starlark_tuple().add(&none).equals(starlark_tuple().add(&none)));
  EXPECT_FALSE(starlark_tuple().add(&one).equals(starlark_tuple().add(&none)));
  EXPECT_FALSE(starlark_tuple().add(&none).add(&one).equals(starlark_tuple().add(&none)));
  EXPECT_FALSE(starlark_tuple().add(&one).add(&none).equals(starlark_tuple().add(&none)));

  EXPECT_FALSE(starlark_tuple().equals(starlark_tuple().add(&one)));
  EXPECT_FALSE(starlark_tuple().add(&none).equals(starlark_tuple().add(&one)));
  EXPECT_TRUE(starlark_tuple().add(&one).equals(starlark_tuple().add(&one)));
  EXPECT_FALSE(starlark_tuple().add(&none).add(&one).equals(starlark_tuple().add(&one)));
  EXPECT_FALSE(starlark_tuple().add(&one).add(&none).equals(starlark_tuple().add(&one)));

  EXPECT_FALSE(starlark_tuple().equals(starlark_tuple().add(&none).add(&one)));
  EXPECT_FALSE(starlark_tuple().add(&none).equals(starlark_tuple().add(&none).add(&one)));
  EXPECT_FALSE(starlark_tuple().add(&one).equals(starlark_tuple().add(&none).add(&one)));
  EXPECT_TRUE(starlark_tuple().add(&none).add(&one).equals(starlark_tuple().add(&none).add(&one)));
  EXPECT_FALSE(starlark_tuple().add(&one).add(&none).equals(starlark_tuple().add(&none).add(&one)));
}

TEST(StarlarkTuple, EqualsRecursion) {
  starlark_tuple tuple_a;
  starlark_tuple tuple_b;
  tuple_a.add(&tuple_b);
  tuple_b.add(&tuple_a);
  EXPECT_TRUE(tuple_a.equals(tuple_b));
}

TEST(StarlarkTuple, Hash) {
  starlark_none none;
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_tuple tuple;
  starlark_list list;
  EXPECT_EQ(5740354900026072187, starlark_tuple().hash());
  EXPECT_EQ(-8753497827991233192, starlark_tuple().add(&zero).hash());
  EXPECT_EQ(-8458139203682520985, starlark_tuple().add(&zero).add(&zero).hash());
  EXPECT_EQ(-6644214454873602895, starlark_tuple().add(&one).hash());
  EXPECT_EQ(9181102132670838864, starlark_tuple().add(&none).hash());
  EXPECT_EQ(-5486347211504344842, starlark_tuple().add(&tuple).hash());
  EXPECT_EQ(-1, starlark_tuple().add(&list).hash());
}

TEST(StarlarkTuple, DeepHash) {
  std::vector<starlark_tuple> all_tuples;
  all_tuples.reserve(100);
  all_tuples.emplace_back();
  for (int i = 0; i < 20; ++i) {
    all_tuples.emplace_back();
    for (int j = 0; j < 3; ++j) {
      all_tuples.back().add(&*++all_tuples.rbegin());
    }
  }
  EXPECT_EQ(5945621570837202953, all_tuples.back().hash());
  for (int i = 0; i < 20; ++i) {
    all_tuples.emplace_back();
    for (int j = 0; j < 3; ++j) {
      all_tuples.back().add(&*++all_tuples.rbegin());
    }
  }
  EXPECT_EQ(-1112958194652280302, all_tuples.back().hash());
}

TEST(StarlarkTuple, HashRecursion) {
  // In theory, this construction is not possible. This test is designed to
  // check whether we are able to detect and handle this pathological case.
  starlark_tuple tuple1;
  starlark_tuple tuple2;
  tuple1.add(&tuple2);
  tuple2.add(&tuple1);
  EXPECT_EQ(-5827241394322601009, tuple1.hash());
}

TEST(StarlarkTuple, Unpack) {
  starlark_none none;
  starlark_integer one(1);
  starlark_tuple tuple;
  std::vector<starlark_obj*> stack;
  error_handler error_callback;

  tuple.unpack(0, stack, error_callback);
  EXPECT_THAT(stack, SizeIs(0));

  tuple.add(&one);
  tuple.unpack(1, stack, error_callback);
  ASSERT_THAT(stack, SizeIs(1));
  EXPECT_THAT(stack[0], &one);

  stack.clear();
  tuple.add(&none);
  tuple.unpack(2, stack, error_callback);
  ASSERT_THAT(stack, SizeIs(2));
  EXPECT_THAT(stack[0], &none);
  EXPECT_THAT(stack[1], &one);
}

TEST(StarlarkTuple, UnpackError) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_tuple tuple;
  tuple.add(&zero);
  tuple.add(&one);
  {
    std::vector<starlark_obj*> consumer;
    error_handler error_callback;

    tuple.unpack(3, consumer, error_callback);
    ASSERT_THAT(consumer, IsEmpty());
    EXPECT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "ValueError: not enough values to unpack (expected 3, got 2)");
  }
  {
    std::vector<starlark_obj*> consumer;
    error_handler error_callback;

    tuple.unpack(1, consumer, error_callback);
    ASSERT_THAT(consumer, IsEmpty());
    EXPECT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "ValueError: too many values to unpack (expected 1, got 2)");
  }
}

TEST(StarlarkTuple, Order) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_tuple tuple1;
  starlark_tuple tuple2;
  tuple2.add(&zero);
  starlark_tuple tuple3;
  tuple3.add(&zero);
  tuple3.add(&one);
  starlark_tuple tuple4;
  tuple4.add(&one);
  starlark_tuple tuple5;
  tuple5.add(&one);
  tuple5.add(&zero);
  error_handler error_callback;

  EXPECT_THAT(tuple1.cmp(tuple1, "cmp", error_callback), Eq(0));
  EXPECT_THAT(tuple1.cmp(tuple2, "cmp", error_callback), Lt(0));
  EXPECT_THAT(tuple1.cmp(tuple3, "cmp", error_callback), Lt(0));
  EXPECT_THAT(tuple1.cmp(tuple4, "cmp", error_callback), Lt(0));
  EXPECT_THAT(tuple1.cmp(tuple5, "cmp", error_callback), Lt(0));

  EXPECT_THAT(tuple2.cmp(tuple1, "cmp", error_callback), Gt(0));
  EXPECT_THAT(tuple2.cmp(tuple2, "cmp", error_callback), Eq(0));
  EXPECT_THAT(tuple2.cmp(tuple3, "cmp", error_callback), Lt(0));
  EXPECT_THAT(tuple2.cmp(tuple4, "cmp", error_callback), Lt(0));
  EXPECT_THAT(tuple2.cmp(tuple5, "cmp", error_callback), Lt(0));

  EXPECT_THAT(tuple3.cmp(tuple1, "cmp", error_callback), Gt(0));
  EXPECT_THAT(tuple3.cmp(tuple2, "cmp", error_callback), Gt(0));
  EXPECT_THAT(tuple3.cmp(tuple3, "cmp", error_callback), Eq(0));
  EXPECT_THAT(tuple3.cmp(tuple4, "cmp", error_callback), Lt(0));
  EXPECT_THAT(tuple3.cmp(tuple5, "cmp", error_callback), Lt(0));

  EXPECT_THAT(tuple4.cmp(tuple1, "cmp", error_callback), Gt(0));
  EXPECT_THAT(tuple4.cmp(tuple2, "cmp", error_callback), Gt(0));
  EXPECT_THAT(tuple4.cmp(tuple3, "cmp", error_callback), Gt(0));
  EXPECT_THAT(tuple4.cmp(tuple4, "cmp", error_callback), Eq(0));
  EXPECT_THAT(tuple4.cmp(tuple5, "cmp", error_callback), Lt(0));

  EXPECT_THAT(tuple5.cmp(tuple1, "cmp", error_callback), Gt(0));
  EXPECT_THAT(tuple5.cmp(tuple2, "cmp", error_callback), Gt(0));
  EXPECT_THAT(tuple5.cmp(tuple3, "cmp", error_callback), Gt(0));
  EXPECT_THAT(tuple5.cmp(tuple4, "cmp", error_callback), Gt(0));
  EXPECT_THAT(tuple5.cmp(tuple5, "cmp", error_callback), Eq(0));
}

TEST(StarlarkTuple, OrderError) {
  error_handler error_callback;
  starlark_integer one(1);
  starlark_tuple tuple;

  EXPECT_FALSE(tuple.cmp(one, "<", error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: '<' not supported between instances of 'tuple' and 'int'");
}

TEST(StarlarkTuple, Membership) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_tuple tuple;
  tuple.add(&zero);
  error_handler error_callback;

  EXPECT_TRUE(tuple.binary_in(zero, error_callback));
  EXPECT_FALSE(tuple.binary_in(one, error_callback));
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkTuple, Freeze) {
  error_handler error_callback;
  starlark_integer one(1);
  starlark_list list;
  starlark_tuple tuple;

  tuple.add(&list);
  tuple.freeze();
  list.add(&one, error_callback);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: trying to mutate a frozen list value");
}

TEST(StarlarkTuple, BinaryPlus) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_tuple tuple_1;
  starlark_tuple tuple_2;
  tuple_1.add(&zero);
  tuple_2.add(&one);
  Arena arena;
  error_handler error_callback;

  auto* result = tuple_1.binary_plus(tuple_2, arena, error_callback);

  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->str(), "(0, 1)");
}

TEST(StarlarkTuple, BinaryPlusNotList) {
  starlark_list list;
  starlark_tuple tuple;
  Arena arena;
  error_handler error_callback;

  auto* result = tuple.binary_plus(list, arena, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: can only concatenate tuple (not \"list\") to tuple");
}

TEST(StarlarkTuple, BinaryStar) {
  starlark_bigint minus_two(-2);
  starlark_integer minus_one(-1);
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_bigint three(3);
  starlark_bigint big(number::one << 64);
  starlark_tuple tuple;
  starlark_tuple tuple0;
  tuple.add(&zero);
  tuple.add(&one);
  Arena arena;
  error_handler error_callback;

  auto* result_0 = tuple0.binary_star(big, arena, error_callback);
  auto* result_1 = tuple.binary_star(two, arena, error_callback);
  auto* result_2 = tuple.binary_star(three, arena, error_callback);
  auto* result_3 = tuple.binary_star(minus_two, arena, error_callback);
  auto* result_4 = tuple.binary_star(minus_one, arena, error_callback);

  ASSERT_NE(result_0, nullptr);
  EXPECT_EQ(result_0->str(), "()");
  ASSERT_NE(result_1, nullptr);
  EXPECT_EQ(result_1->str(), "(0, 1, 0, 1)");
  ASSERT_NE(result_2, nullptr);
  EXPECT_EQ(result_2->str(), "(0, 1, 0, 1, 0, 1)");
  ASSERT_NE(result_3, nullptr);
  EXPECT_EQ(result_3->str(), "()");
  ASSERT_NE(result_4, nullptr);
  EXPECT_EQ(result_4->str(), "()");
}

TEST(StarlarkTuple, BinaryStarReverse) {
  starlark_bigint minus_two(-2);
  starlark_integer minus_one(-1);
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_bigint three(3);
  starlark_bigint big(number::one << 64);
  starlark_tuple tuple;
  starlark_tuple tuple0;
  tuple.add(&zero);
  tuple.add(&one);
  Arena arena;
  error_handler error_callback;

  auto* result_0 = big.binary_star(tuple0, arena, error_callback);
  auto* result_1 = two.binary_star(tuple, arena, error_callback);
  auto* result_2 = three.binary_star(tuple, arena, error_callback);
  auto* result_3 = minus_two.binary_star(tuple, arena, error_callback);
  auto* result_4 = minus_one.binary_star(tuple, arena, error_callback);

  ASSERT_NE(result_0, nullptr);
  EXPECT_EQ(result_0->str(), "()");
  ASSERT_NE(result_1, nullptr);
  EXPECT_EQ(result_1->str(), "(0, 1, 0, 1)");
  ASSERT_NE(result_2, nullptr);
  EXPECT_EQ(result_2->str(), "(0, 1, 0, 1, 0, 1)");
  ASSERT_NE(result_3, nullptr);
  EXPECT_EQ(result_3->str(), "()");
  ASSERT_NE(result_4, nullptr);
  EXPECT_EQ(result_4->str(), "()");
}

TEST(StarlarkTuple, BinaryStarNotInt) {
  starlark_list list;
  starlark_tuple tuple;
  Arena arena;
  error_handler error_callback;

  auto* result = tuple.binary_star(list, arena, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: can't multiply sequence by non-int of type 'list'");
}

TEST(StarlarkTuple, BinaryStarTooBig) {
  starlark_tuple tuple;
  starlark_bigint big(number::one << 64);
  Arena arena;
  error_handler error_callback;
  tuple.add(&big);

  auto* result = tuple.binary_star(big, arena, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: sequences must be at most 2147483647 elements");
}

TEST(StarlarkTuple, Len) {
  starlark_integer zero(0);
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_integer three(3);
  starlark_tuple tuple_1;
  starlark_tuple tuple_2;
  error_handler error_callback;

  tuple_2.add(&zero);
  tuple_2.add(&one);
  tuple_2.add(&two);
  tuple_2.add(&three);

  EXPECT_EQ(0, tuple_1.len(error_callback));
  EXPECT_EQ(4, tuple_2.len(error_callback));
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

}  // namespace
