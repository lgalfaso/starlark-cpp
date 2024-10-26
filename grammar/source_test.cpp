// Copyright 2024 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <limits>

#include "grammar/source.hpp"

using grammar::source;

namespace {

TEST(SourceTest, BOM) {
  EXPECT_EQ('a', source("a").peek_codepoint());
  EXPECT_EQ('a', source("\xef\xbb\x{bf}a").peek_codepoint());
}

TEST(SourceTest, Peek) {
  source s("abc");
  EXPECT_EQ('a', s.peek());
  EXPECT_EQ('b', s.peek(1));
  EXPECT_EQ('c', s.peek(2));
  EXPECT_EQ('\0', s.peek(3));
  EXPECT_EQ('\0', s.peek(std::numeric_limits<std::size_t>::max() - 1));
  EXPECT_EQ('\0', s.peek(std::numeric_limits<std::size_t>::max()));
}

TEST(SourceTest, PeekCodepoint) {
  EXPECT_EQ('a', source("a").peek_codepoint());
  EXPECT_EQ(0x234, source("\xc8\xb4").peek_codepoint());
  EXPECT_EQ(0x1234, source("\xe1\x88\xb4").peek_codepoint());
  EXPECT_EQ(0x12345, source("\xf0\x92\x8d\x85").peek_codepoint());

  // Too short
  EXPECT_EQ(source::replacement_character, source("").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xc8").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xe1\x88").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xf0\x92\x8d").peek_codepoint());

  // Invalid first byte
  EXPECT_EQ(source::replacement_character, source("\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xf8\xbf\xbf\xbf\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xfc\xbf\xbf\xbf\xbf\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xfe\xbf\xbf\xbf\xbf\xbf\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xff\xbf\xbf\xbf\xbf\xbf\xbf\xbf").peek_codepoint());
  
  // Invalid follow-up byte
  EXPECT_EQ(source::replacement_character, source("\xc8\x34").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xc8\xf4").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xe1\x08\xb4").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xe1\xc8\xb4").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xe1\x88\x34").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xe1\x88\xf4").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xf0\x12\x8d\x85").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xf0\xd2\x8d\x85").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xf0\x92\x0d\x85").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xf0\x92\xcd\x85").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xf0\x92\x8d\x05").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xf0\x92\x8d\xc5").peek_codepoint());

  // The largest possible Unicode character is 0x10FFFF.
  EXPECT_EQ(source::replacement_character, source("\xf4\x90\x80\x80").peek_codepoint());
  EXPECT_EQ(0x10ffff, source("\xf4\x8f\xbf\xbf").peek_codepoint());
}

TEST(SourceTest, OverlongEncoding) {
  // Overlong encoding.
  EXPECT_EQ(source::replacement_character, source("\xc0\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xc1\xbf").peek_codepoint());
  EXPECT_EQ(0x7f, source("\x7f").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xe0\x9f\xbf").peek_codepoint());
  EXPECT_EQ(0x7ff, source("\xdf\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xf0\x8f\xbf\xbf").peek_codepoint());
  EXPECT_EQ(0xffff, source("\xef\xbf\xbf").peek_codepoint());
}

TEST(SourceTest, SurrogatesArea) {
  // The range 0xD800-0xDFFF is invalid.
  EXPECT_EQ(source::replacement_character, source("\xed\xa0\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xa0\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xa1\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xa1\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xa2\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xa2\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xa3\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xa3\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xa4\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xa4\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xa5\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xa5\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xa6\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xa6\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xa7\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xa7\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xa8\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xa8\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xa9\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xa9\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xaa\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xaa\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xab\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xab\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xac\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xac\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xad\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xad\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xae\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xae\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xaf\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xaf\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xb0\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xb0\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xb1\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xb1\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xb2\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xb2\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xb3\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xb3\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xb4\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xb4\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xb5\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xb5\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xb6\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xb6\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xb7\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xb7\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xb8\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xb8\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xb9\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xb9\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xba\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xba\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xbb\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xbb\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xbc\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xbc\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xbd\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xbd\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xbe\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xbe\xbf").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xbf\x80").peek_codepoint());
  EXPECT_EQ(source::replacement_character, source("\xed\xbf\xbf").peek_codepoint());
}

TEST(SourceTest, Skip) {
  source s("abc");
  s.skip(0);
  EXPECT_EQ(0, s.get_pos());
  s.skip();
  EXPECT_EQ(1, s.get_pos());
  s.skip(2);
  EXPECT_EQ(3, s.get_pos());
  s.skip();
  EXPECT_EQ(3, s.get_pos());
  s.skip();
  s.skip(std::numeric_limits<std::size_t>::max());
  EXPECT_EQ(3, s.get_pos());
}

void checkSkipCodepoint(const char* input, std::size_t input_length, std::size_t expected_pos) {
  std::string_view source_code(input, input_length);
  source s(source_code);
  EXPECT_EQ(0, s.get_pos());
  s.skip_codepoint();
  EXPECT_EQ(expected_pos, s.get_pos());
}

TEST(SourceTest, SkipCodepoint) {
  // Normal encoding.
  checkSkipCodepoint("abc", 3, 1);
  checkSkipCodepoint("\xc8\x{b4}abc", 5, 2);
  checkSkipCodepoint("\xe1\x88\x{b4}abc", 6, 3);
  checkSkipCodepoint("\xf0\x92\x8d\x{85}abc", 7, 4);

  // Invalid first byte, skip only that byte.
  checkSkipCodepoint("\x{80}abc", 4, 1);
  checkSkipCodepoint("\xf8\xbf\xbf\xbf\xbf", 5, 1);
  checkSkipCodepoint("\xfc\xbf\xbf\xbf\xbf\xbf", 6, 1);
  checkSkipCodepoint("\xfe\xbf\xbf\xbf\xbf\xbf\xbf", 7, 1);
  checkSkipCodepoint("\xff\xbf\xbf\xbf\xbf\xbf\xbf\xbf", 8, 1);

  // Too short, stop when finding a non UTF-8 continue character.
  checkSkipCodepoint("\x{c8}abc", 4, 1);
  checkSkipCodepoint("\xe1\x{88}abc", 5, 2);
  checkSkipCodepoint("\xf0\x92\x{8d}abc", 6, 3);

  // Overlong characters are skipped in full.
  checkSkipCodepoint("\xc0\x{bf}abc", 5, 2);
  checkSkipCodepoint("\xc1\x{bf}abc", 5, 2);
  checkSkipCodepoint("\xe0\x9f\x{bf}abc", 6, 3);
  checkSkipCodepoint("\xf0\x8f\xbf\x{bf}abc", 7, 4);
}

TEST(SourceTest, Capture) {
  {
    source s("");
    EXPECT_EQ(0, s.get_pos());
    EXPECT_EQ(true, s.is_end());
    EXPECT_EQ(true, s.capture(""));
    EXPECT_EQ(0, s.get_pos());
    EXPECT_EQ(true, s.is_end());
    EXPECT_EQ(false, s.capture("a"));
    EXPECT_EQ(0, s.get_pos());
    EXPECT_EQ(true, s.is_end());
  }
  {
    source s("abc");
    EXPECT_EQ(false, s.is_end());
    EXPECT_EQ(true, s.capture(""));
    EXPECT_EQ(0, s.get_pos());
    EXPECT_EQ(false, s.is_end());
    EXPECT_EQ(true, s.capture("a"));
    EXPECT_EQ(1, s.get_pos());
    EXPECT_EQ(false, s.is_end());
    EXPECT_EQ(false, s.capture("a"));
    EXPECT_EQ(1, s.get_pos());
    EXPECT_EQ(false, s.is_end());
    EXPECT_EQ(true, s.capture("b"));
    EXPECT_EQ(2, s.get_pos());
    EXPECT_EQ(false, s.is_end());
    EXPECT_EQ(true, s.capture("c"));
    EXPECT_EQ(3, s.get_pos());
    EXPECT_EQ(true, s.is_end());
  }
  {
    source s("abc");
    EXPECT_EQ(false, s.capture("abcdef"));
    EXPECT_EQ(0, s.get_pos());
  }
}

}  // namespace
