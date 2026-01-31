// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <limits>

#include "runtime/starlark_bigint.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_float.hpp"
#include "runtime/starlark_none.hpp"
#include "runtime/starlark_range.hpp"
#include "runtime/starlark_testing.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;
using ::starlark::runtime::context;
using ::starlark::runtime::starlark_bigint;
using ::starlark::runtime::starlark_integer;
using ::starlark::runtime::starlark_float;
using ::starlark::runtime::starlark_none;
using ::starlark::runtime::starlark_range;
using ::starlark::testing::error_handler;
using ::testing::Ge;
using ::testing::IsEmpty;
using ::testing::Lt;
using ::testing::SizeIs;

namespace {

TEST(StarlarkRange, Type) {
  EXPECT_EQ("range", starlark_range(0, 100, 1).type());
}

TEST(StarlarkRange, Primitve) {
  EXPECT_FALSE(starlark_range(0, 100, 1).primitive());
}

TEST(StarlarkRange, Truthy) {
  auto test = [](int64_t start, int64_t end, int64_t step, bool expected) {
    starlark_range range(start, end, step);
    EXPECT_EQ(range.truthy(), expected) << range.str();
  };

  test(0, 0, 1, false);
  test(0, 0, -1, false);
  test(0, 100, 1, true);
  test(0, 100, -1, false);
  test(0, 100, 1000, true);
  test(0, 100, -1000, false);
  test(100, 0, 1, false);
  test(100, 0, -1, true);
  test(100, 0, 1000, false);
  test(100, 0, -1000, true);
}

TEST(StarlarkRange, Repr) {
  auto test = [](int64_t start, int64_t end, int64_t step, std::string_view expected) {
    starlark_range range(start, end, step);
    EXPECT_EQ(range.repr(), expected);
  };

  test(0, 0, 1, "range(0)");
  test(0, 1, 1, "range(1)");
  test(0, 0, -1, "range(0, 0, -1)");
  test(0, 100, 1, "range(100)");
  test(1, 100, 1, "range(1, 100)");
  test(0, 100, -1, "range(0, 100, -1)");
  test(0, 100, 1000, "range(0, 100, 1000)");
  test(0, 100, -1000, "range(0, 100, -1000)");
  test(100, 0, 1, "range(100, 0)");
  test(100, 0, -1, "range(100, 0, -1)");
  test(100, 0, 1000, "range(100, 0, 1000)");
  test(100, 0, -1000, "range(100, 0, -1000)");
}

TEST(StarlarkRange, Hash) {
  EXPECT_EQ(starlark_range(0, 100, 1).hash(), -1);
}

TEST(StarlarkRange, Len) {
  auto test = [](int64_t start, int64_t end, int64_t step, int64_t expected) {
    error_handler error_callback;

    starlark_range range(start, end, step);
    EXPECT_EQ(range.len(true, error_callback), expected);
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test(0, 0, 1, 0);
  test(0, 0, -1, 0);
  test(0, 99, 1, 99);
  test(0, 100, 1, 100);
  test(0, 101, 1, 101);
  test(0, 99, 10, 10);
  test(0, 100, 10, 10);
  test(0, 101, 10, 11);
  test(0, 100, -1, 0);
  test(0, 100, 1000, 1);
  test(0, 100, -1000, 0);
  test(100, 0, 1, 0);
  test(99, 0, -1, 99);
  test(100, 0, -1, 100);
  test(101, 0, -1, 101);
  test(99, 0, -10, 10);
  test(100, 0, -10, 10);
  test(101, 0, -10, 11);
  test(100, 0, 1000, 0);
  test(100, 0, -1000, 1);
}

TEST(StarlarkRange, LenHardCases) {
  auto test = [](int64_t start, int64_t end, int64_t step, bool valid) {
    error_handler error_callback;
    starlark_range range(start, end, step);

    if (valid) {
      EXPECT_THAT(range.len(true, error_callback), Ge(0));
      EXPECT_THAT(error_callback.messages, IsEmpty());
    } else {
      EXPECT_THAT(range.len(true, error_callback), Lt(0));
      EXPECT_THAT(error_callback.messages, IsEmpty());
    }
  };

  test(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max(), -2, true);
  test(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max(), -1, true);
  test(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max(), 1, false);
  test(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max(), 2, true);
  test(std::numeric_limits<int64_t>::max(), std::numeric_limits<int64_t>::min(), -2, true);
  test(std::numeric_limits<int64_t>::max(), std::numeric_limits<int64_t>::min(), -1, false);
  test(std::numeric_limits<int64_t>::max(), std::numeric_limits<int64_t>::min(), 1, true);
  test(std::numeric_limits<int64_t>::max(), std::numeric_limits<int64_t>::min(), 2, true);
}

TEST(StarlarkRange, Equals) {
  auto test = [](int64_t start1, int64_t end1, int64_t step1, int64_t start2, int64_t end2, int64_t step2, bool expected) {
    starlark_range range1(start1, end1, step1);
    starlark_range range2(start2, end2, step2);
    EXPECT_EQ(range1.equals(range2), expected);
    EXPECT_EQ(range2.equals(range1), expected);
  };
  test(0, 100, 1, 0, 100, 1, true);
  test(1, 100, 1, 0, 100, 1, false);
  test(0, 100, 2, 0, 100, 2, true);
  test(0, 100, 1, 0, 100, 2, false);
  test(0, 99, 2, 0, 100, 2, true);
  test(0, 100, 2, 0, 101, 2, false);
  test(0, 0, 2, 100, 100, 2, true);
  EXPECT_FALSE(starlark_range(0, 100, 1).equals(starlark_none()));
}

TEST(StarlarkRange, Index) {
  auto test = [](int64_t start, int64_t end, int64_t step, int64_t idx, int64_t expected) {
    Arena arena;
    context ctx(arena);
    error_handler error_callback;
    starlark_integer index(idx);

    starlark_range range(start, end, step);
    auto* result = range.index(index, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_TRUE(result->equals(starlark_integer(expected)));
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };
  test(0, 100, 1, 0, 0);
  test(0, 100, 1, 1, 1);
  test(0, 100, 2, 1, 2);
  test(1, 100, 1, 0, 1);
  test(1, 100, 1, 1, 2);
  test(1, 100, 2, 0, 1);
  test(1, 100, 2, 1, 3);
  test(0, 100, 1, 99, 99);
  test(1, 100, 1, 98, 99);
  test(1, 100, 1, -1, 99);
}

TEST(StarlarkRange, IndexOutOfRange) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_integer index(100);

  starlark_range range(0, 100, 1);
  auto* result = range.index(index, ctx, error_callback);
  EXPECT_EQ(nullptr, result);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "IndexError: range index out of range");
}

TEST(StarlarkRange, BinaryIn) {
  auto test = [](int64_t start, int64_t end, int64_t step, int64_t element, bool expected) {
    error_handler error_callback;

    starlark_range range(start, end, step);
    EXPECT_EQ(range.binary_in(starlark_integer(element), error_callback), expected) << "Range: " << range.repr() << ", E: " << element;
    EXPECT_EQ(range.binary_in(starlark_bigint(element), error_callback), expected) << "Range: " << range.repr() << ", E: " << element;
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test(0, 100, 1, -1, false);
  test(0, 100, 1, 0, true);
  test(0, 100, 1, 1, true);
  test(0, 100, 1, 99, true);
  test(0, 100, 1, 100, false);
  test(100, 0, -1, 0, false);
  test(100, 0, -1, 1, true);
  test(100, 0, -1, 99, true);
  test(100, 0, -1, 100, true);
  test(100, 0, -1, 101, false);

  test(-1, 100, 1, std::numeric_limits<int64_t>::min(), false);
  test(-1, 100, 1, std::numeric_limits<int64_t>::max(), false);
  test(0, 100, 1, std::numeric_limits<int64_t>::min(), false);
  test(0, 100, 1, std::numeric_limits<int64_t>::max(), false);
  test(1, 100, 1, std::numeric_limits<int64_t>::min(), false);
  test(1, 100, 1, std::numeric_limits<int64_t>::max(), false);

  test(100, -1, -1, std::numeric_limits<int64_t>::min(), false);
  test(100, -1, -1, std::numeric_limits<int64_t>::max(), false);
  test(100, 0, -1, std::numeric_limits<int64_t>::min(), false);
  test(100, 0, -1, std::numeric_limits<int64_t>::max(), false);
  test(100, 1, -1, std::numeric_limits<int64_t>::min(), false);
  test(100, 1, -1, std::numeric_limits<int64_t>::max(), false);

  test(std::numeric_limits<int64_t>::min() + 1, std::numeric_limits<int64_t>::max(), 1, std::numeric_limits<int64_t>::min(), false);
  test(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max() - 1, 1, std::numeric_limits<int64_t>::max(), false);
  test(std::numeric_limits<int64_t>::max() - 1, std::numeric_limits<int64_t>::min(), -1, std::numeric_limits<int64_t>::max(), false);
  test(std::numeric_limits<int64_t>::max(), std::numeric_limits<int64_t>::min() + 1, -1, std::numeric_limits<int64_t>::min(), false);

  test(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max(), 3, std::numeric_limits<int64_t>::min() + 1, false);
  test(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max(), 3, std::numeric_limits<int64_t>::min() + 2, false);
  test(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max(), 3, std::numeric_limits<int64_t>::min() + 3, true);
  test(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max(), 3, std::numeric_limits<int64_t>::max() - 1, false);
  test(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max(), 3, std::numeric_limits<int64_t>::max() - 2, true);
  test(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max(), 3, std::numeric_limits<int64_t>::max() - 3, false);
}

TEST(StarlarkRange, BinaryInBigInt) {
  auto test = [](int64_t start, int64_t end, int64_t step, const number element, bool expected) {
    error_handler error_callback;

    starlark_range range(start, end, step);
    EXPECT_EQ(range.binary_in(starlark_bigint(element), error_callback), expected) << "Range: " << range.repr() << ", E: " << element.to_string(10);
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max(), 1, (number::minus_one() << 63) - number::one(), false);
  test(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max(), 1, number::one() << 63, false);
  test(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max(), 1, number::one() << 64, false);
}

TEST(StarlarkRange, BinaryInFloat) {
  auto test = [](int64_t start, int64_t end, int64_t step, double element, bool expected) {
    error_handler error_callback;

    starlark_range range(start, end, step);
    EXPECT_EQ(range.binary_in(starlark_float(element), error_callback), expected) << "Range: " << range.repr() << ", E: " << element;
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test(0, 100, 1, -1, false);
  test(0, 100, 1, -0.5, false);
  test(0, 100, 1, 0, true);
  test(0, 100, 1, 0.5, false);
  test(0, 100, 1, 1, true);
  test(0, 100, 1, 1.5, false);
  test(0, 100, 1, 98.5, false);
  test(0, 100, 1, 99, true);
  test(0, 100, 1, 99.5, false);
  test(0, 100, 1, 100, false);
  test(0, 100, 1, 100.5, false);
  test(0, 100, 1, -0x1.0000000000001p+63, false);
  test(0, 100, 1, 0x1.0000000000000p+63, false);
  test(0, 100, 1, -std::numeric_limits<double>::infinity(), false);
  test(0, 100, 1, std::numeric_limits<double>::infinity(), false);
  test(0, 100, 1, std::numeric_limits<double>::quiet_NaN(), false);
  test(100, 0, -1, -0.5, false);
  test(100, 0, -1, 0, false);
  test(100, 0, -1, 0.5, false);
  test(100, 0, -1, 1, true);
  test(100, 0, -1, 1.5, false);
  test(100, 0, -1, 98.5, false);
  test(100, 0, -1, 99, true);
  test(100, 0, -1, 99.5, false);
  test(100, 0, -1, 100, true);
  test(100, 0, -1, 100.5, false);
  test(100, 0, -1, 101, false);
  test(100, 0, -1, 101.5, false);
}

TEST(StarlarkRange, BinaryInNone) {
  error_handler error_callback;
  starlark_none none;

  starlark_range range(0, 100, 1);
  EXPECT_FALSE(range.binary_in(none, error_callback));
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkRange, GetIterator) {
  starlark_range range0(10, 20, -3);
  starlark_range range1(10, 20, 3);
  starlark_range range2(20, 10, -3);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* it0 = range0.get_iterator(true, ctx, error_callback);
  EXPECT_FALSE(it0->has_next());
  it0->end_iterator();

  auto* it1 = range1.get_iterator(true, ctx, error_callback);
  EXPECT_TRUE(it1->has_next());
  EXPECT_EQ(it1->next()->str(), "10");
  EXPECT_TRUE(it1->has_next());
  EXPECT_EQ(it1->next()->str(), "13");
  EXPECT_TRUE(it1->has_next());
  EXPECT_EQ(it1->next()->str(), "16");
  EXPECT_TRUE(it1->has_next());
  EXPECT_EQ(it1->next()->str(), "19");
  EXPECT_FALSE(it1->has_next());
  it1->end_iterator();

  auto* it2 = range2.get_iterator(true, ctx, error_callback);
  EXPECT_TRUE(it2->has_next());
  EXPECT_EQ(it2->next()->str(), "20");
  EXPECT_TRUE(it2->has_next());
  EXPECT_EQ(it2->next()->str(), "17");
  EXPECT_TRUE(it2->has_next());
  EXPECT_EQ(it2->next()->str(), "14");
  EXPECT_TRUE(it2->has_next());
  EXPECT_EQ(it2->next()->str(), "11");
  EXPECT_FALSE(it2->has_next());
  it2->end_iterator();
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

}  // namespace
