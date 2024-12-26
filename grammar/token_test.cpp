// Copyright 2024 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include "grammar/token.hpp"

using bignum::number;
using grammar::token;
using grammar::token_type;

namespace {

TEST(Token, DefaultValues) {
  token t(token_type::illegal, {}, {});
  EXPECT_EQ("", t.string_value());
  EXPECT_EQ(number(0), t.int_value());
  t = token(token_type::illegal, {}, {}, "");
  EXPECT_EQ(0.0, t.double_value());
}

}  // namespace
