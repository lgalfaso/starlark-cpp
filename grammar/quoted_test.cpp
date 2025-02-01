// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <string>

#include "grammar/quoted.hpp"

using grammar::quoted;

namespace {

TEST(QuotedTest, CommonStrings) {
  EXPECT_EQ("\"\"", quoted(""));
  EXPECT_EQ("\"abc\"", quoted("abc"));
  EXPECT_EQ("\"\\000\"", quoted(std::string("\0", 1)));
  EXPECT_EQ("\"\\177\"", quoted("\x7f"));
  EXPECT_EQ("\"\\\\\\\"\"", quoted("\\\""));
}

TEST(QuotedTest, SpecialCharacters) {
  EXPECT_EQ("\"\\a\\b\\f\\n\\r\\t\\v\"", quoted("\a\b\f\n\r\t\v"));
}

}  // namespace
