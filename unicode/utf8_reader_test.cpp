// Copyright 2024 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <limits>

#include "unicode/utf8_reader.hpp"

using unicode::utf8_reader;

namespace {

TEST(SourceTest, BOM) {
  EXPECT_EQ('a', utf8_reader("a").peek_code_point());
  EXPECT_EQ('a', utf8_reader("\xef\xbb\x{bf}a").peek_code_point());
}

TEST(SourceTest, Peek) {
  utf8_reader s("abc");
  EXPECT_EQ('a', s.peek());
  EXPECT_EQ('b', s.peek(1));
  EXPECT_EQ('c', s.peek(2));
  EXPECT_EQ('\0', s.peek(3));
  EXPECT_EQ('\0', s.peek(std::numeric_limits<std::size_t>::max() - 1));
  EXPECT_EQ('\0', s.peek(std::numeric_limits<std::size_t>::max()));
}

TEST(SourceTest, PeekCodepoint) {
  EXPECT_EQ('a', utf8_reader("a").peek_code_point());
  EXPECT_EQ(0x234, utf8_reader("\xc8\xb4").peek_code_point());
  EXPECT_EQ(0x1234, utf8_reader("\xe1\x88\xb4").peek_code_point());
  EXPECT_EQ(0x12345, utf8_reader("\xf0\x92\x8d\x85").peek_code_point());

  // Too short
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xc8").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xe1\x88").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xf0\x92\x8d").peek_code_point());

  // Invalid first byte
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xf8\xbf\xbf\xbf\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xfc\xbf\xbf\xbf\xbf\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xfe\xbf\xbf\xbf\xbf\xbf\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xff\xbf\xbf\xbf\xbf\xbf\xbf\xbf").peek_code_point());
  
  // Invalid follow-up byte
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xc8\x34").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xc8\xf4").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xe1\x08\xb4").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xe1\xc8\xb4").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xe1\x88\x34").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xe1\x88\xf4").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xf0\x12\x8d\x85").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xf0\xd2\x8d\x85").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xf0\x92\x0d\x85").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xf0\x92\xcd\x85").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xf0\x92\x8d\x05").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xf0\x92\x8d\xc5").peek_code_point());

  // The largest possible Unicode character is 0x10FFFF.
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xf4\x90\x80\x80").peek_code_point());
  EXPECT_EQ(0x10ffff, utf8_reader("\xf4\x8f\xbf\xbf").peek_code_point());
}

TEST(SourceTest, OverlongEncoding) {
  // Overlong encoding.
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xc0\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xc1\xbf").peek_code_point());
  EXPECT_EQ(0x7f, utf8_reader("\x7f").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xe0\x9f\xbf").peek_code_point());
  EXPECT_EQ(0x7ff, utf8_reader("\xdf\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xf0\x8f\xbf\xbf").peek_code_point());
  EXPECT_EQ(0xffff, utf8_reader("\xef\xbf\xbf").peek_code_point());
}

TEST(SourceTest, SurrogatesArea) {
  // The range 0xD800-0xDFFF is invalid.
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa0\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa0\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa1\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa1\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa2\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa2\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa3\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa3\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa4\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa4\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa5\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa5\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa6\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa6\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa7\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa7\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa8\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa8\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa9\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa9\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xaa\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xaa\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xab\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xab\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xac\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xac\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xad\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xad\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xae\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xae\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xaf\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xaf\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb0\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb0\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb1\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb1\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb2\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb2\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb3\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb3\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb4\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb4\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb5\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb5\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb6\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb6\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb7\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb7\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb8\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb8\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb9\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb9\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xba\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xba\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xbb\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xbb\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xbc\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xbc\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xbd\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xbd\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xbe\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xbe\xbf").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xbf\x80").peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xbf\xbf").peek_code_point());
}

TEST(SourceTest, Skip) {
  utf8_reader s("abc");
  s.skip(0);
  EXPECT_EQ(0, s.pos());
  s.skip();
  EXPECT_EQ(1, s.pos());
  s.skip(2);
  EXPECT_EQ(3, s.pos());
  s.skip();
  EXPECT_EQ(3, s.pos());
  s.skip();
  s.skip(std::numeric_limits<std::size_t>::max());
  EXPECT_EQ(3, s.pos());
}

void checkSkipCodepoint(const char* input, std::size_t input_length, std::size_t expected_pos) {
  std::string_view utf8_reader_code(input, input_length);
  utf8_reader s(utf8_reader_code);
  EXPECT_EQ(0, s.pos());
  s.skip_code_point();
  EXPECT_EQ(expected_pos, s.pos());
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
    utf8_reader s("");
    EXPECT_EQ(0, s.pos());
    EXPECT_EQ(false, s.pending());
    EXPECT_EQ(true, s.capture(""));
    EXPECT_EQ(0, s.pos());
    EXPECT_EQ(false, s.pending());
    EXPECT_EQ(false, s.capture("a"));
    EXPECT_EQ(0, s.pos());
    EXPECT_EQ(false, s.pending());
  }
  {
    utf8_reader s("abc");
    EXPECT_EQ(true, s.pending());
    EXPECT_EQ(true, s.capture(""));
    EXPECT_EQ(0, s.pos());
    EXPECT_EQ(true, s.pending());
    EXPECT_EQ(true, s.capture("a"));
    EXPECT_EQ(1, s.pos());
    EXPECT_EQ(true, s.pending());
    EXPECT_EQ(false, s.capture("a"));
    EXPECT_EQ(1, s.pos());
    EXPECT_EQ(true, s.pending());
    EXPECT_EQ(true, s.capture("b"));
    EXPECT_EQ(2, s.pos());
    EXPECT_EQ(true, s.pending());
    EXPECT_EQ(true, s.capture("c"));
    EXPECT_EQ(3, s.pos());
    EXPECT_EQ(false, s.pending());
  }
  {
    utf8_reader s("abc");
    EXPECT_EQ(false, s.capture("abcdef"));
    EXPECT_EQ(0, s.pos());
  }
}

}  // namespace
