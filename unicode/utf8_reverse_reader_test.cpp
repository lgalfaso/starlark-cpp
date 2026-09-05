// Copyright 2026 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <limits>
#include <random>
#include <string>

#include "unicode/encode.hpp"
#include "unicode/utf8_reader.hpp"
#include "unicode/utf8_reverse_reader.hpp"

using ::starlark::unicode::utf8_encode_code_point;
using ::starlark::unicode::utf8_reader;
using ::starlark::unicode::utf8_reverse_reader;

namespace {

TEST(Utf8ReverseReader, PeekCodepoint) {
  EXPECT_EQ('a', utf8_reverse_reader("a", true).peek_code_point());
  EXPECT_EQ(0x234, utf8_reverse_reader("\xc8\xb4", true).peek_code_point());
  EXPECT_EQ(0x1234, utf8_reverse_reader("\xe1\x88\xb4", true).peek_code_point());
  EXPECT_EQ(0x12345, utf8_reverse_reader("\xf0\x92\x8d\x85", true).peek_code_point());

  // Too short
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xc8", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xe1\x88", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xf0\x92\x8d", true).peek_code_point());

  // Invalid first byte
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xf8\xbf\xbf\xbf\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xfc\xbf\xbf\xbf\xbf\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xfe\xbf\xbf\xbf\xbf\xbf\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xff\xbf\xbf\xbf\xbf\xbf\xbf\xbf", true).peek_code_point());

  // Invalid follow-up byte
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xc8\xf4", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xe1\x08\xb4", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xe1\x88\xf4", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xf0\x12\x8d\x85", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xf0\xd2\x8d\x85", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xf0\x92\x0d\x85", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xf0\x92\x8d\xc5", true).peek_code_point());

  // The largest possible Unicode character, 0x10FFFF.
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xf4\x90\x80\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xf4\x8f\xbf\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kMaxCodePoint, utf8_reverse_reader("\xf4\x8f\xbf\xbf", false).peek_code_point());
}

TEST(Utf8ReverseReader, OverlongEncoding) {
  // Overlong encoding.
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xc0\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xc1\xbf", true).peek_code_point());
  EXPECT_EQ(0x7f, utf8_reverse_reader("\x7f", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xe0\x9f\xbf", true).peek_code_point());
  EXPECT_EQ(0x7ff, utf8_reverse_reader("\xdf\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xf0\x8f\xbf\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xef\xbf\xbf", true).peek_code_point());
}

TEST(SourceTest, SurrogatesAreaStrict) {
  // The range 0xD800-0xDFFF is invalid in strict mode and valid otherwise.
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xa0\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xa0\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xa1\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xa1\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xa2\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xa2\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xa3\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xa3\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xa4\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xa4\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xa5\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xa5\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xa6\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xa6\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xa7\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xa7\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xa8\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xa8\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xa9\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xa9\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xaa\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xaa\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xab\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xab\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xac\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xac\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xad\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xad\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xae\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xae\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xaf\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xaf\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xb0\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xb0\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xb1\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xb1\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xb2\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xb2\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xb3\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xb3\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xb4\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xb4\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xb5\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xb5\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xb6\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xb6\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xb7\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xb7\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xb8\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xb8\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xb9\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xb9\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xba\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xba\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xbb\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xbb\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xbc\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xbc\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xbd\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xbd\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xbe\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xbe\xbf", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xbf\x80", true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reverse_reader("\xed\xbf\xbf", true).peek_code_point());
}

TEST(Utf8ReverseReader, SurrogatesAreaNonStrict) {
  EXPECT_EQ(0xD800, utf8_reverse_reader("\xed\xa0\x80", false).peek_code_point());
  EXPECT_EQ(0xD83F, utf8_reverse_reader("\xed\xa0\xbf", false).peek_code_point());
  EXPECT_EQ(0xD840, utf8_reverse_reader("\xed\xa1\x80", false).peek_code_point());
  EXPECT_EQ(0xD87F, utf8_reverse_reader("\xed\xa1\xbf", false).peek_code_point());
  EXPECT_EQ(0xD880, utf8_reverse_reader("\xed\xa2\x80", false).peek_code_point());
  EXPECT_EQ(0xD8BF, utf8_reverse_reader("\xed\xa2\xbf", false).peek_code_point());
  EXPECT_EQ(0xD8C0, utf8_reverse_reader("\xed\xa3\x80", false).peek_code_point());
  EXPECT_EQ(0xD8FF, utf8_reverse_reader("\xed\xa3\xbf", false).peek_code_point());
  EXPECT_EQ(0xD900, utf8_reverse_reader("\xed\xa4\x80", false).peek_code_point());
  EXPECT_EQ(0xD93F, utf8_reverse_reader("\xed\xa4\xbf", false).peek_code_point());
  EXPECT_EQ(0xD940, utf8_reverse_reader("\xed\xa5\x80", false).peek_code_point());
  EXPECT_EQ(0xD97F, utf8_reverse_reader("\xed\xa5\xbf", false).peek_code_point());
  EXPECT_EQ(0xD980, utf8_reverse_reader("\xed\xa6\x80", false).peek_code_point());
  EXPECT_EQ(0xD9BF, utf8_reverse_reader("\xed\xa6\xbf", false).peek_code_point());
  EXPECT_EQ(0xD9C0, utf8_reverse_reader("\xed\xa7\x80", false).peek_code_point());
  EXPECT_EQ(0xD9FF, utf8_reverse_reader("\xed\xa7\xbf", false).peek_code_point());
  EXPECT_EQ(0xDA00, utf8_reverse_reader("\xed\xa8\x80", false).peek_code_point());
  EXPECT_EQ(0xDA3F, utf8_reverse_reader("\xed\xa8\xbf", false).peek_code_point());
  EXPECT_EQ(0xDA40, utf8_reverse_reader("\xed\xa9\x80", false).peek_code_point());
  EXPECT_EQ(0xDA7F, utf8_reverse_reader("\xed\xa9\xbf", false).peek_code_point());
  EXPECT_EQ(0xDA80, utf8_reverse_reader("\xed\xaa\x80", false).peek_code_point());
  EXPECT_EQ(0xDABF, utf8_reverse_reader("\xed\xaa\xbf", false).peek_code_point());
  EXPECT_EQ(0xDAC0, utf8_reverse_reader("\xed\xab\x80", false).peek_code_point());
  EXPECT_EQ(0xDAFF, utf8_reverse_reader("\xed\xab\xbf", false).peek_code_point());
  EXPECT_EQ(0xDB00, utf8_reverse_reader("\xed\xac\x80", false).peek_code_point());
  EXPECT_EQ(0xDB3F, utf8_reverse_reader("\xed\xac\xbf", false).peek_code_point());
  EXPECT_EQ(0xDB40, utf8_reverse_reader("\xed\xad\x80", false).peek_code_point());
  EXPECT_EQ(0xDB7F, utf8_reverse_reader("\xed\xad\xbf", false).peek_code_point());
  EXPECT_EQ(0xDB80, utf8_reverse_reader("\xed\xae\x80", false).peek_code_point());
  EXPECT_EQ(0xDBBF, utf8_reverse_reader("\xed\xae\xbf", false).peek_code_point());
  EXPECT_EQ(0xDBC0, utf8_reverse_reader("\xed\xaf\x80", false).peek_code_point());
  EXPECT_EQ(0xDBFF, utf8_reverse_reader("\xed\xaf\xbf", false).peek_code_point());
  EXPECT_EQ(0xDC00, utf8_reverse_reader("\xed\xb0\x80", false).peek_code_point());
  EXPECT_EQ(0xDC3F, utf8_reverse_reader("\xed\xb0\xbf", false).peek_code_point());
  EXPECT_EQ(0xDC40, utf8_reverse_reader("\xed\xb1\x80", false).peek_code_point());
  EXPECT_EQ(0xDC7F, utf8_reverse_reader("\xed\xb1\xbf", false).peek_code_point());
  EXPECT_EQ(0xDC80, utf8_reverse_reader("\xed\xb2\x80", false).peek_code_point());
  EXPECT_EQ(0xDCBF, utf8_reverse_reader("\xed\xb2\xbf", false).peek_code_point());
  EXPECT_EQ(0xDCC0, utf8_reverse_reader("\xed\xb3\x80", false).peek_code_point());
  EXPECT_EQ(0xDCFF, utf8_reverse_reader("\xed\xb3\xbf", false).peek_code_point());
  EXPECT_EQ(0xDD00, utf8_reverse_reader("\xed\xb4\x80", false).peek_code_point());
  EXPECT_EQ(0xDD3F, utf8_reverse_reader("\xed\xb4\xbf", false).peek_code_point());
  EXPECT_EQ(0xDD40, utf8_reverse_reader("\xed\xb5\x80", false).peek_code_point());
  EXPECT_EQ(0xDD7F, utf8_reverse_reader("\xed\xb5\xbf", false).peek_code_point());
  EXPECT_EQ(0xDD80, utf8_reverse_reader("\xed\xb6\x80", false).peek_code_point());
  EXPECT_EQ(0xDDBF, utf8_reverse_reader("\xed\xb6\xbf", false).peek_code_point());
  EXPECT_EQ(0xDDC0, utf8_reverse_reader("\xed\xb7\x80", false).peek_code_point());
  EXPECT_EQ(0xDDFF, utf8_reverse_reader("\xed\xb7\xbf", false).peek_code_point());
  EXPECT_EQ(0xDE00, utf8_reverse_reader("\xed\xb8\x80", false).peek_code_point());
  EXPECT_EQ(0xDE3F, utf8_reverse_reader("\xed\xb8\xbf", false).peek_code_point());
  EXPECT_EQ(0xDE40, utf8_reverse_reader("\xed\xb9\x80", false).peek_code_point());
  EXPECT_EQ(0xDE7F, utf8_reverse_reader("\xed\xb9\xbf", false).peek_code_point());
  EXPECT_EQ(0xDE80, utf8_reverse_reader("\xed\xba\x80", false).peek_code_point());
  EXPECT_EQ(0xDEBF, utf8_reverse_reader("\xed\xba\xbf", false).peek_code_point());
  EXPECT_EQ(0xDEC0, utf8_reverse_reader("\xed\xbb\x80", false).peek_code_point());
  EXPECT_EQ(0xDEFF, utf8_reverse_reader("\xed\xbb\xbf", false).peek_code_point());
  EXPECT_EQ(0xDF00, utf8_reverse_reader("\xed\xbc\x80", false).peek_code_point());
  EXPECT_EQ(0xDF3F, utf8_reverse_reader("\xed\xbc\xbf", false).peek_code_point());
  EXPECT_EQ(0xDF40, utf8_reverse_reader("\xed\xbd\x80", false).peek_code_point());
  EXPECT_EQ(0xDF7F, utf8_reverse_reader("\xed\xbd\xbf", false).peek_code_point());
  EXPECT_EQ(0xDF80, utf8_reverse_reader("\xed\xbe\x80", false).peek_code_point());
  EXPECT_EQ(0xDFBF, utf8_reverse_reader("\xed\xbe\xbf", false).peek_code_point());
  EXPECT_EQ(0xDFC0, utf8_reverse_reader("\xed\xbf\x80", false).peek_code_point());
  EXPECT_EQ(0xDFFF, utf8_reverse_reader("\xed\xbf\xbf", false).peek_code_point());
}

void checkSkipCodepoint(const char* input, std::size_t input_length, std::size_t expected_pos) {
  std::string_view utf8_reverse_reader_code(input, input_length);
  utf8_reverse_reader s(utf8_reverse_reader_code, true);
  EXPECT_EQ(input_length, s.pending());
  s.read_code_point();
  EXPECT_EQ(expected_pos, s.pending()) << std::string_view(input, input_length);
}

TEST(Utf8ReverseReader, ReadCodePoint) {
  // Normal encoding.
  checkSkipCodepoint("", 0, 0);
  checkSkipCodepoint("abc", 3, 2);
  checkSkipCodepoint("abc\xc8\x{b4}", 5, 3);
  checkSkipCodepoint("abc\xe1\x88\x{b4}", 6, 3);
  checkSkipCodepoint("abc\xf0\x92\x8d\x{85}", 7, 3);

  // Invalid first byte, skip only that byte.
  checkSkipCodepoint("abc\x{80}", 4, 3);
  checkSkipCodepoint("\xf8\xbf\xbf\xbf\xbf", 5, 4);
  checkSkipCodepoint("\xfc\xbf\xbf\xbf\xbf\xbf", 6, 5);
  checkSkipCodepoint("\xfe\xbf\xbf\xbf\xbf\xbf\xbf", 7, 6);
  checkSkipCodepoint("\xff\xbf\xbf\xbf\xbf\xbf\xbf\xbf", 8, 7);

  // Too short, stop when finding a non UTF-8 continue character.
  checkSkipCodepoint("abc\x{c8}", 4, 3);
  checkSkipCodepoint("abc\xe1\x{88}", 5, 4);
  checkSkipCodepoint("abc\xf0\x92\x{8d}", 6, 5);

  // Overlong characters are skipped in full.
  checkSkipCodepoint("abc\xc0\x{bf}", 5, 3);
  checkSkipCodepoint("abc\xc1\x{bf}", 5, 3);
  checkSkipCodepoint("abc\xe0\x9f\x{bf}", 6, 3);
  checkSkipCodepoint("abc\xf0\x8f\xbf\x{bf}", 7, 3);
}

void utf8_peek_skip_code_point(const char* data, size_t size) {
  utf8_reverse_reader reader(std::string_view(data, size), true);
  while (reader.pending()) {
    reader.read_code_point();
  }
}

TEST(Utf8ReverseReader, ShortStrings) {
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

TEST(Utf8ReverseReader, Random4) {
  char source[4];
  std::mt19937 g(GTEST_FLAG_GET(random_seed));

  for (int i = 0; i < 100000; ++i) {
    for (int j = 0; j < sizeof(source); ++j) {
      source[j] = g();
    }
    utf8_peek_skip_code_point(source, sizeof(source));
  }
}

TEST(Utf8ReverseReader, Random5) {
  char source[5];
  std::mt19937 g(GTEST_FLAG_GET(random_seed));

  for (int i = 0; i < 100000; ++i) {
    for (int j = 0; j < sizeof(source); ++j) {
      source[j] = g();
    }
    utf8_peek_skip_code_point(source, sizeof(source));
  }
}

TEST(Utf8ReverseReader, Random6) {
  char source[6];
  std::mt19937 g(GTEST_FLAG_GET(random_seed));

  for (int i = 0; i < 100000; ++i) {
    for (int j = 0; j < sizeof(source); ++j) {
      source[j] = g();
    }
    utf8_peek_skip_code_point(source, sizeof(source));
  }
}

TEST(Utf8ReverseReader, AllCharacters) {
  char source[5];
  auto do_checks = [&source](int code_point, int length) {
    utf8_reverse_reader reader(std::string_view(source, length), true);
    std::string encoded;
    utf8_encode_code_point(code_point, encoded, true, false);
    if (reader.peek_code_point() != code_point ||
        (code_point == utf8_reader::kReplacementCharacter && length != 3)) {
      EXPECT_EQ(reader.peek_code_point(), 0xFFFD);
      EXPECT_NE(encoded, std::string(source, length));
    } else {
      EXPECT_EQ(encoded, std::string(source, length));
    }
    reader.read_code_point();
    EXPECT_FALSE(reader.pending());
  };

  // 1 char long.
  for (int i = 0; i < 0x7F; ++i) {
    source[0] = i;
    source[1] = 0;
    do_checks(i, 1);
  }

  // 2 chars long.
  for (int i = 0; i < 0x7FF; ++i) {
    source[0] = static_cast<char>((0xC0) | (i >> 6));
    source[1] = static_cast<char>((0x80) | (0x3F & i));
    source[2] = 0;
    do_checks(i, 2);
  }

  // 3 chars long.
  for (int i = 0; i < 0xFFFF; ++i) {
    source[0] = static_cast<char>((0xE0) | (i >> 12));
    source[1] = static_cast<char>((0x80) | (0x3F & (i >> 6)));
    source[2] = static_cast<char>((0x80) | (0x3F & i));
    source[3] = 0;
    do_checks(i, 3);
  }

  // 4 chars long.
  for (int i = 0; i < 0x10'FFFF; ++i) {
    source[0] = static_cast<char>((0xF0) | (i >> 18));
    source[1] = static_cast<char>((0x80) | (0x3F & (i >> 12)));
    source[2] = static_cast<char>((0x80) | (0x3F & (i >> 6)));
    source[3] = static_cast<char>((0x80) | (0x3F & i));
    source[4] = 0;
    do_checks(i, 4);
  }
}

}  // namespace
