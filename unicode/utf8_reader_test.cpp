// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <limits>
#include <random>

#include "unicode/utf8_reader.hpp"

using unicode::utf8_reader;

namespace {

TEST(SourceTest, BOM) {
  EXPECT_EQ('a', utf8_reader("a", true).peek_code_point());
  EXPECT_EQ('a', utf8_reader("\xef\xbb\x{bf}a", true).peek_code_point());
}

TEST(SourceTest, Peek) {
  utf8_reader s("abc", true);
  EXPECT_EQ('a', s.peek());
  EXPECT_EQ('b', s.peek(1));
  EXPECT_EQ('c', s.peek(2));
  EXPECT_EQ('\0', s.peek(3));
  EXPECT_EQ('\0', s.peek(std::numeric_limits<std::size_t>::max() - 1));
  EXPECT_EQ('\0', s.peek(std::numeric_limits<std::size_t>::max()));
}

TEST(SourceTest, PeekCodepoint) {
  EXPECT_EQ('a', utf8_reader("a", true).peek_code_point());
  EXPECT_EQ(0x234, utf8_reader("\xc8\xb4", true).peek_code_point());
  EXPECT_EQ(0x1234, utf8_reader("\xe1\x88\xb4", true).peek_code_point());
  EXPECT_EQ(0x12345, utf8_reader("\xf0\x92\x8d\x85", true).peek_code_point());

  // Too short
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xc8", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xe1\x88", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xf0\x92\x8d", true).peek_code_point());

  // Invalid first byte
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xf8\xbf\xbf\xbf\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xfc\xbf\xbf\xbf\xbf\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xfe\xbf\xbf\xbf\xbf\xbf\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xff\xbf\xbf\xbf\xbf\xbf\xbf\xbf", true).peek_code_point());

  // Invalid follow-up byte
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xc8\x34", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xc8\xf4", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xe1\x08\xb4", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xe1\xc8\xb4", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xe1\x88\x34", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xe1\x88\xf4", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xf0\x12\x8d\x85", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xf0\xd2\x8d\x85", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xf0\x92\x0d\x85", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xf0\x92\xcd\x85", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xf0\x92\x8d\x05", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xf0\x92\x8d\xc5", true).peek_code_point());

  // The largest possible Unicode character, 0x10FFFF.
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xf4\x90\x80\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xf4\x8f\xbf\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::max_code_point, utf8_reader("\xf4\x8f\xbf\xbf", false).peek_code_point());
}

TEST(SourceTest, OverlongEncoding) {
  // Overlong encoding.
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xc0\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xc1\xbf", true).peek_code_point());
  EXPECT_EQ(0x7f, utf8_reader("\x7f", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xe0\x9f\xbf", true).peek_code_point());
  EXPECT_EQ(0x7ff, utf8_reader("\xdf\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xf0\x8f\xbf\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xef\xbf\xbf", true).peek_code_point());
}

TEST(SourceTest, SurrogatesArea) {
  // The range 0xD800-0xDFFF is invalid.
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa0\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa0\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa1\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa1\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa2\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa2\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa3\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa3\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa4\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa4\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa5\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa5\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa6\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa6\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa7\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa7\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa8\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa8\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa9\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xa9\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xaa\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xaa\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xab\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xab\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xac\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xac\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xad\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xad\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xae\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xae\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xaf\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xaf\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb0\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb0\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb1\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb1\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb2\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb2\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb3\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb3\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb4\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb4\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb5\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb5\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb6\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb6\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb7\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb7\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb8\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb8\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb9\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xb9\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xba\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xba\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xbb\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xbb\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xbc\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xbc\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xbd\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xbd\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xbe\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xbe\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xbf\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::replacement_character, utf8_reader("\xed\xbf\xbf", true).peek_code_point());
}

TEST(SourceTest, Skip) {
  utf8_reader s("abc", true);
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
  utf8_reader s(utf8_reader_code, true);
  EXPECT_EQ(0, s.pos());
  s.skip_code_point();
  EXPECT_EQ(expected_pos, s.pos());
}

TEST(SourceTest, SkipCodepoint) {
  // Normal encoding.
  checkSkipCodepoint("", 0, 0);
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

TEST(SourceTest, Next) {
  {
    utf8_reader s("", true);
    EXPECT_EQ(true, s.next(""));
    EXPECT_EQ(0, s.pos());
    EXPECT_EQ(false, s.next("a"));
    EXPECT_EQ(0, s.pos());
  }
  {
    utf8_reader s("abc", true);
    EXPECT_EQ(true, s.next(""));
    EXPECT_EQ(0, s.pos());
    EXPECT_EQ(true, s.next("a"));
    EXPECT_EQ(0, s.pos());
    EXPECT_EQ(true, s.next("a"));
    EXPECT_EQ(0, s.pos());
    EXPECT_EQ(false, s.next("b"));
    EXPECT_EQ(0, s.pos());
  }
}

TEST(SourceTest, Capture) {
  {
    utf8_reader s("", true);
    EXPECT_EQ(0, s.pos());
    EXPECT_EQ(true, s.empty());
    EXPECT_EQ(0, s.pending());
    EXPECT_EQ(true, s.capture(""));
    EXPECT_EQ(0, s.pos());
    EXPECT_EQ(true, s.empty());
    EXPECT_EQ(0, s.pending());
    EXPECT_EQ(false, s.capture("a"));
    EXPECT_EQ(0, s.pos());
    EXPECT_EQ(true, s.empty());
    EXPECT_EQ(0, s.pending());
  }
  {
    utf8_reader s("abc", true);
    EXPECT_EQ(false, s.empty());
    EXPECT_EQ(3, s.pending());

    EXPECT_EQ(true, s.capture(""));
    EXPECT_EQ(0, s.pos());
    EXPECT_EQ(false, s.empty());
    EXPECT_EQ(3, s.pending());

    EXPECT_EQ(true, s.capture("a"));
    EXPECT_EQ(1, s.pos());
    EXPECT_EQ(false, s.empty());
    EXPECT_EQ(2, s.pending());

    EXPECT_EQ(false, s.capture("a"));
    EXPECT_EQ(1, s.pos());
    EXPECT_EQ(false, s.empty());
    EXPECT_EQ(2, s.pending());

    EXPECT_EQ(true, s.capture("b"));
    EXPECT_EQ(2, s.pos());
    EXPECT_EQ(false, s.empty());
    EXPECT_EQ(1, s.pending());

    EXPECT_EQ(true, s.capture("c"));
    EXPECT_EQ(3, s.pos());
    EXPECT_EQ(true, s.empty());
    EXPECT_EQ(0, s.pending());
  }
  {
    utf8_reader s("abc", true);
    EXPECT_EQ(false, s.capture("abcdef"));
    EXPECT_EQ(0, s.pos());
  }
}

void utf8_peek_skip_code_point(const char* data, size_t size) {
  utf8_reader reader(std::string_view(data, size), true);
  while (reader.pending()) {
    reader.peek_code_point();
    reader.skip_code_point();
  }
}

TEST(SourceTest, ShortStrings) {
  utf8_peek_skip_code_point("", 0);
  char source[3];
  for (int i = 0; i < 256; ++i) {
    source[0] = i;
    utf8_peek_skip_code_point(source, 1);
    for (int j = 0; j < 256; ++j) {
      source[1] = j;
      utf8_peek_skip_code_point(source, 2);
      for (int k = 0; k < 256; ++k) {
        source[2] = k;
        utf8_peek_skip_code_point(source, 3);
      }
    }
  }
}

TEST(SourceTest, Random4) {
  char source[4];
  std::mt19937 g(GTEST_FLAG_GET(random_seed));

  for (int i = 0; i < 100000; ++i) {
    for (int j = 0; j < sizeof(source); ++j) {
      source[j] = g();
    }
    utf8_peek_skip_code_point(source, sizeof(source));
  }
}

TEST(SourceTest, Random5) {
  char source[5];
  std::mt19937 g(GTEST_FLAG_GET(random_seed));

  for (int i = 0; i < 100000; ++i) {
    for (int j = 0; j < sizeof(source); ++j) {
      source[j] = g();
    }
    utf8_peek_skip_code_point(source, sizeof(source));
  }
}

TEST(SourceTest, Random6) {
  char source[6];
  std::mt19937 g(GTEST_FLAG_GET(random_seed));

  for (int i = 0; i < 100000; ++i) {
    for (int j = 0; j < sizeof(source); ++j) {
      source[j] = g();
    }
    utf8_peek_skip_code_point(source, sizeof(source));
  }
}

}  // namespace
