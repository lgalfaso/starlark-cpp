// Copyright 2026 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include "string/levenshtein.hpp"

using ::starlark::string::levenshtein;

namespace {

TEST(Levenshtein, OneEntry) {
  EXPECT_EQ(0, levenshtein("kitten", {"sitting"}));
  EXPECT_EQ(-1, levenshtein("abcdef", {"ghijklm"}));
}

TEST(Levenshtein, MultipleEntries) {
  EXPECT_EQ(0, levenshtein("count", {"cnt", "something_else"}));
}

}  // namespace
