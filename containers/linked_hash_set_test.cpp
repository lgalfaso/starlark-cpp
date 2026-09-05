// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include "containers/linked_hash_set.hpp"

using ::starlark::cnt::linked_hash_set;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::Pair;
using ::testing::Pointee;

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
  *result_listener << "where the iterator points to " << (*arg);
  return (*arg == n);
}

TEST(LinkedHashSet, Insert) {
  linked_hash_set<int, int_hash, int_equals_to> set;
  EXPECT_THAT(set.insert(1), Pair(IsIteratorPointingTo(1), Eq(true)));
  EXPECT_THAT(set.insert(2), Pair(IsIteratorPointingTo(2), Eq(true)));
  EXPECT_THAT(set.insert(3), Pair(IsIteratorPointingTo(3), Eq(true)));
  EXPECT_THAT(set.insert(2), Pair(IsIteratorPointingTo(2), Eq(false)));
}

TEST(LinkedHashSet, InsertWillNotChangeElement) {
  linked_hash_set<int, zero_hash, all_equals_to> set;
  EXPECT_THAT(set.insert(1), Pair(IsIteratorPointingTo(1), Eq(true)));
  EXPECT_THAT(set.insert(2), Pair(IsIteratorPointingTo(1), Eq(false)));
}

TEST(LinkedHashSet, IterateElementsInInserOrder) {
  linked_hash_set<int, int_hash, int_equals_to> set;
  set.insert(2);
  set.insert(1);
  set.insert(3);
  EXPECT_THAT(set, ElementsAre(2, 1, 3));
}

TEST(LinkedHashSet, Empty) {
  linked_hash_set<int, int_hash, int_equals_to> set;
  EXPECT_TRUE(set.empty());
  set.insert(1);
  EXPECT_FALSE(set.empty());
}

TEST(LinkedHashSet, Contains) {
  linked_hash_set<int, int_hash, int_equals_to> set;
  EXPECT_FALSE(set.contains(1));
  EXPECT_FALSE(set.contains(2));
  set.insert(1);
  EXPECT_TRUE(set.contains(1));
  EXPECT_FALSE(set.contains(2));
}

TEST(LinkedHashSet, Size) {
  linked_hash_set<int, int_hash, int_equals_to> set;
  EXPECT_EQ(0, set.size());
  set.insert(1);
  EXPECT_EQ(1, set.size());
  set.insert(2);
  EXPECT_EQ(2, set.size());
}

TEST(LinkedHashSet, Clear) {
  linked_hash_set<int, int_hash, int_equals_to> set;
  set.insert(1);
  set.clear();
  EXPECT_TRUE(set.empty());
}

TEST(LinkedHashSet, Erase) {
  linked_hash_set<int, int_hash, int_equals_to> set;
  EXPECT_EQ(0, set.erase(1));
  set.insert(1);
  set.insert(2);
  EXPECT_EQ(1, set.erase(1));
  EXPECT_THAT(set, ElementsAre(2));
  EXPECT_EQ(0, set.erase(1));
  EXPECT_EQ(1, set.erase(2));
  EXPECT_TRUE(set.empty());
}

}  // namespace

