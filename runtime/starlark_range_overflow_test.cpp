// Copyright 2026 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <limits>

#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_range.hpp"
#include "runtime/starlark_testing.hpp"

using ::google::protobuf::Arena;
using ::starlark::runtime::context;
using ::starlark::runtime::starlark_integer;
using ::starlark::runtime::starlark_range;
using ::starlark::testing::error_handler;
using ::testing::IsEmpty;
using ::testing::SizeIs;

namespace {

// 2**63 - 1 == 7**2 * 73 * 127 * 337 * 92737 * 649657
// 2**63 + 1 == 3**3 * 19 * 43 * 5419 * 77158673929

constexpr int64_t under_a = 14197294936951L;  // 7 * 7 * 73 * 127 * 337 * 92737.
constexpr int64_t under_b = 649657L;
constexpr int64_t over_a = 77158673929L;
constexpr int64_t over_b = 119537721L;  // 3 * 3 * 3 * 19 * 43 * 5419.

TEST(StarlarkRange, SliceRangeOverflow) {
  auto test = [](int64_t rstart, int64_t rend, int64_t rstride, int64_t pstart, int64_t pend, int64_t pstride, bool error) {
    Arena arena;
    context ctx(arena);
    error_handler error_callback;
    starlark_range range(rstart, rend, rstride);
    starlark_integer start(pstart);
    starlark_integer end(pend);
    starlark_integer step(pstride);

    auto* result = range.slice_range(start, end, step, ctx, error_callback);

    if (error) {
      ASSERT_EQ(nullptr, result);
      ASSERT_THAT(error_callback.messages, SizeIs(1));
      EXPECT_EQ(error_callback.messages[0], "too many digits in integer");
    } else {
      ASSERT_NE(nullptr, result);
      EXPECT_THAT(error_callback.messages, IsEmpty());
    }
  };
  test(0, 10, under_a, 0, 1, under_b, false);
  test(0, 10, over_a, 0, 1, over_b, true);

  test(0, std::numeric_limits<int64_t>::max(), under_a, under_b, 1, 1, false);
  test(0, std::numeric_limits<int64_t>::max(), over_a, over_b, 1, 1, true);
  test(1, std::numeric_limits<int64_t>::max(), under_a, under_b, 1, 1, true);

  test(0, std::numeric_limits<int64_t>::max(), under_a, 0, under_b, 1, false);
  test(0, std::numeric_limits<int64_t>::max(), over_a, 0, over_b, 1, true);
  test(1, std::numeric_limits<int64_t>::max(), under_a, 0, under_b, 1, true);
}

}  // namespace

