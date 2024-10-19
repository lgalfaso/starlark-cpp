// Copyright 2024 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include "grammar/source.hpp"

using grammar::source;

namespace {

TEST(SourceTest, PeekCodepoint) {
  EXPECT_EQ('a', source("a").peek_codepoint());
  EXPECT_EQ(0x234, source("\xc8\xb4").peek_codepoint());
  EXPECT_EQ(0x1234, source("\xe1\x88\xb4").peek_codepoint());
  EXPECT_EQ(0x12345, source("\xf0\x92\x8d\x85").peek_codepoint());

  // Too short
  EXPECT_EQ(source::invalid_codepoint, source("").peek_codepoint());
  EXPECT_EQ(source::invalid_codepoint, source("\xc8").peek_codepoint());
  EXPECT_EQ(source::invalid_codepoint, source("\xe1\x88").peek_codepoint());
  EXPECT_EQ(source::invalid_codepoint, source("\xf0\x92\x8d").peek_codepoint());

  // Invalid first byte
  EXPECT_EQ(source::invalid_codepoint, source("\x80").peek_codepoint());
  EXPECT_EQ(source::invalid_codepoint, source("\xf8\xbf\xbf\xbf\xbf").peek_codepoint());
  EXPECT_EQ(source::invalid_codepoint, source("\xfc\xbf\xbf\xbf\xbf\xbf").peek_codepoint());
  EXPECT_EQ(source::invalid_codepoint, source("\xfe\xbf\xbf\xbf\xbf\xbf\xbf").peek_codepoint());
  EXPECT_EQ(source::invalid_codepoint, source("\xff\xbf\xbf\xbf\xbf\xbf\xbf\xbf").peek_codepoint());
  
  // Invalid follow-up byte
  EXPECT_EQ(source::invalid_codepoint, source("\xc8\x34").peek_codepoint());
  EXPECT_EQ(source::invalid_codepoint, source("\xc8\xf4").peek_codepoint());
  EXPECT_EQ(source::invalid_codepoint, source("\xe1\x08\xb4").peek_codepoint());
  EXPECT_EQ(source::invalid_codepoint, source("\xe1\xc8\xb4").peek_codepoint());
  EXPECT_EQ(source::invalid_codepoint, source("\xe1\x88\x34").peek_codepoint());
  EXPECT_EQ(source::invalid_codepoint, source("\xe1\x88\xf4").peek_codepoint());
  EXPECT_EQ(source::invalid_codepoint, source("\xf0\x12\x8d\x85").peek_codepoint());
  EXPECT_EQ(source::invalid_codepoint, source("\xf0\xd2\x8d\x85").peek_codepoint());
  EXPECT_EQ(source::invalid_codepoint, source("\xf0\x92\x0d\x85").peek_codepoint());
  EXPECT_EQ(source::invalid_codepoint, source("\xf0\x92\xcd\x85").peek_codepoint());
  EXPECT_EQ(source::invalid_codepoint, source("\xf0\x92\x8d\x05").peek_codepoint());
  EXPECT_EQ(source::invalid_codepoint, source("\xf0\x92\x8d\xc5").peek_codepoint());
}

}  // namespace
