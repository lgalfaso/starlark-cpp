// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include "containers/linked_hash_map.hpp"

using starlark::cnt::linked_hash_map;
using testing::ElementsAre;
using testing::Eq;
using testing::Pair;
using testing::Pointee;

namespace {

struct int_hash {
  size_t operator()(const int& value) const {
    return value;
  }
};

struct int_equals_to {
  bool operator()(const int& lhs, const int& rhs) const {
    return lhs == rhs;
  }
};

struct zero_hash {
  size_t operator()(const int& value) const {
    return 0;
  }
};

struct all_equals_to {
  bool operator()(const int& lhs, const int& rhs) const {
    return true;
  }
};

MATCHER_P(IsIteratorPointingTo, n, "") {
  return (*arg == n);
}

TEST(LinkedHashMap, Insert) {
  linked_hash_map<int, int, int_hash, int_equals_to> map;
  EXPECT_THAT(map.insert(1, 101), Pair(IsIteratorPointingTo(std::make_pair(1, 101)), Eq(true)));
  EXPECT_THAT(map.insert(2, 102), Pair(IsIteratorPointingTo(std::make_pair(2, 102)), Eq(true)));
  EXPECT_THAT(map.insert(3, 103), Pair(IsIteratorPointingTo(std::make_pair(3, 103)), Eq(true)));
  EXPECT_THAT(map.insert(2, 102), Pair(IsIteratorPointingTo(std::make_pair(2, 102)), Eq(false)));
}

TEST(LinkedHashMap, InsertWillNotChangeKey) {
  linked_hash_map<int, int, zero_hash, all_equals_to> map;
  EXPECT_THAT(map.insert(1, 101), Pair(IsIteratorPointingTo(std::make_pair(1, 101)), Eq(true)));
  EXPECT_THAT(map.insert(2, 102), Pair(IsIteratorPointingTo(std::make_pair(1, 102)), Eq(false)));
}

TEST(LinkedHashMap, IterateElementsInInserOrder) {
  linked_hash_map<int, int, int_hash, int_equals_to> map;
  map.insert(2, 102);
  map.insert(1, 101);
  map.insert(3, 103);
  EXPECT_THAT(map, ElementsAre(std::make_pair(2, 102), std::make_pair(1, 101), std::make_pair(3, 103)));
}

TEST(LinkedHashMap, Empty) {
  linked_hash_map<int, int, int_hash, int_equals_to> map;
  EXPECT_TRUE(map.empty());
  map.insert(1, 101);
  EXPECT_FALSE(map.empty());
}

TEST(LinkedHashMap, Contains) {
  linked_hash_map<int, int, int_hash, int_equals_to> map;
  EXPECT_FALSE(map.contains(1));
  EXPECT_FALSE(map.contains(2));
  map.insert(1, 101);
  EXPECT_TRUE(map.contains(1));
  EXPECT_FALSE(map.contains(2));
}

TEST(LinkedHashMap, Size) {
  linked_hash_map<int, int, int_hash, int_equals_to> map;
  EXPECT_EQ(0, map.size());
  map.insert(1, 101);
  EXPECT_EQ(1, map.size());
  map.insert(2, 102);
  EXPECT_EQ(2, map.size());
}

TEST(LinkedHashMap, Clear) {
  linked_hash_map<int, int, int_hash, int_equals_to> map;
  map.insert(1, 101);
  map.clear();
  EXPECT_TRUE(map.empty());
}

TEST(LinkedHashMap, Erase) {
  linked_hash_map<int, int, int_hash, int_equals_to> map;
  EXPECT_EQ(0, map.erase(1));
  map.insert(1, 101);
  map.insert(2, 102);
  EXPECT_EQ(1, map.erase(1));
  EXPECT_THAT(map, ElementsAre(std::pair(2, 102)));
  EXPECT_EQ(0, map.erase(1));
  EXPECT_EQ(1, map.erase(2));
  EXPECT_TRUE(map.empty());
}

}  // namespace

