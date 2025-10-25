// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include "containers/flat_map.hpp"

using starlark::cnt::flat_map;
using testing::Pair;

namespace {

TEST(FlatMap, Find) {
  flat_map<int, std::string> map({
    {1, "1"},
    {2, "2"},
    {4, "4"},
  });
  EXPECT_EQ(map.end(), map.find(0));

  auto it = map.find(1);
  ASSERT_NE(map.end(), it);
  EXPECT_THAT(*it, Pair(1, "1"));

  it = map.find(2);
  ASSERT_NE(map.end(), it);
  EXPECT_THAT(*it, Pair(2, "2"));

  EXPECT_EQ(map.end(), map.find(3));

  it = map.find(4);
  ASSERT_NE(map.end(), it);
  EXPECT_THAT(*it, Pair(4, "4"));

  EXPECT_EQ(map.end(), map.find(5));
}

}  // namespace
