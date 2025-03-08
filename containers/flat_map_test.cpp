// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <string>

#include "containers/flat_map.hpp"

using starlark::cnt::flat_map;
using testing::Eq;
using testing::Pair;

// TODO(lmirelmann): Move matchers to a common place.
MATCHER_P(IsIteratorPointingTo, n, "") {
  return (*arg == n);
}

namespace {

TEST(FlatMap, Find) {
  flat_map<int, std::string> map({
    {1, "1"},
    {2, "2"},
    {4, "4"},
  });
  EXPECT_EQ(map.end(), map.find(0));
  EXPECT_THAT(map.find(1), IsIteratorPointingTo(std::make_pair(1, "1")));
  EXPECT_THAT(map.find(2), IsIteratorPointingTo(std::make_pair(2, "2")));
  EXPECT_EQ(map.end(), map.find(3));
  EXPECT_THAT(map.find(4), IsIteratorPointingTo(std::make_pair(4, "4")));
  EXPECT_EQ(map.end(), map.find(5));
}

}  // namespace
