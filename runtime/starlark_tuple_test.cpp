// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <vector>

#include "runtime/starlark_bool.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_list.hpp"
#include "runtime/starlark_none.hpp"
#include "runtime/starlark_tuple.hpp"

using ::starlark::runtime::starlark_bool;
using ::starlark::runtime::starlark_integer;
using ::starlark::runtime::starlark_list;
using ::starlark::runtime::starlark_none;
using ::starlark::runtime::starlark_obj;
using ::starlark::runtime::starlark_tuple;
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
  // TODO(lmirelmann): This should change once hash recursions are implemented.
  EXPECT_EQ(-1, tuple1.hash());
}

TEST(StarlarkTuple, SequenceSize) {
  starlark_none none;
  starlark_integer one(1);
  starlark_tuple tuple;
  EXPECT_EQ(0, tuple.sequence_size());
  tuple.add(&one);
  EXPECT_EQ(1, tuple.sequence_size());
  tuple.add(&none);
  EXPECT_EQ(2, tuple.sequence_size());
}

TEST(StarlarkTuple, Unpack) {
  starlark_none none;
  starlark_integer one(1);
  starlark_tuple tuple;
  std::vector<starlark_obj*> stack;

  tuple.unpack(stack);
  EXPECT_THAT(stack, SizeIs(0));

  tuple.add(&one);
  tuple.unpack(stack);
  ASSERT_THAT(stack, SizeIs(1));
  EXPECT_THAT(stack[0], &one);

  stack.clear();
  tuple.add(&none);
  tuple.unpack(stack);
  ASSERT_THAT(stack, SizeIs(2));
  EXPECT_THAT(stack[0], &none);
  EXPECT_THAT(stack[1], &one);
}

}  // namespace
