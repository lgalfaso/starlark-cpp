// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <limits>
#include <random>
#include <string>

#include "unicode/encode.hpp"
#include "unicode/utf8_reader.hpp"

using starlark::unicode::utf8_encode_code_point;
using starlark::unicode::utf8_reader;

namespace {

TEST(SourceTest, BOM) {
  EXPECT_EQ('a', utf8_reader("a", true, true).peek_code_point());
  EXPECT_EQ('a', utf8_reader("\xef\xbb\x{bf}a", true, true).peek_code_point());
  EXPECT_EQ(0xfeff, utf8_reader("\xef\xbb\x{bf}a", true, false).peek_code_point());
}

TEST(SourceTest, Peek) {
  utf8_reader s("abc", true, true);
  EXPECT_EQ('a', s.peek());
  EXPECT_EQ('b', s.peek(1));
  EXPECT_EQ('c', s.peek(2));
  EXPECT_EQ('\0', s.peek(3));
  EXPECT_EQ('\0', s.peek(std::numeric_limits<std::size_t>::max() - 1));
  EXPECT_EQ('\0', s.peek(std::numeric_limits<std::size_t>::max()));
}

TEST(SourceTest, PeekCodepoint) {
  EXPECT_EQ('a', utf8_reader("a", true, true).peek_code_point());
  EXPECT_EQ(0x234, utf8_reader("\xc8\xb4", true, true).peek_code_point());
  EXPECT_EQ(0x1234, utf8_reader("\xe1\x88\xb4", true, true).peek_code_point());
  EXPECT_EQ(0x12345, utf8_reader("\xf0\x92\x8d\x85", true, true).peek_code_point());

  // Too short
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xc8", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xe1\x88", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xf0\x92\x8d", true, true).peek_code_point());

  // Invalid first byte
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xf8\xbf\xbf\xbf\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xfc\xbf\xbf\xbf\xbf\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xfe\xbf\xbf\xbf\xbf\xbf\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xff\xbf\xbf\xbf\xbf\xbf\xbf\xbf", true, true).peek_code_point());

  // Invalid follow-up byte
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xc8\x34", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xc8\xf4", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xe1\x08\xb4", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xe1\xc8\xb4", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xe1\x88\x34", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xe1\x88\xf4", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xf0\x12\x8d\x85", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xf0\xd2\x8d\x85", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xf0\x92\x0d\x85", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xf0\x92\xcd\x85", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xf0\x92\x8d\x05", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xf0\x92\x8d\xc5", true, true).peek_code_point());

  // The largest possible Unicode character, 0x10FFFF.
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xf4\x90\x80\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xf4\x8f\xbf\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kMaxCodePoint, utf8_reader("\xf4\x8f\xbf\xbf", false, true).peek_code_point());
}

TEST(SourceTest, OverlongEncoding) {
  // Overlong encoding.
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xc0\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xc1\xbf", true, true).peek_code_point());
  EXPECT_EQ(0x7f, utf8_reader("\x7f", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xe0\x9f\xbf", true, true).peek_code_point());
  EXPECT_EQ(0x7ff, utf8_reader("\xdf\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xf0\x8f\xbf\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xef\xbf\xbf", true, true).peek_code_point());
}

TEST(SourceTest, SurrogatesAreaStrict) {
  // The range 0xD800-0xDFFF is invalid in strict mode and valid otherwise.
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xa0\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xa0\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xa1\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xa1\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xa2\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xa2\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xa3\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xa3\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xa4\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xa4\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xa5\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xa5\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xa6\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xa6\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xa7\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xa7\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xa8\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xa8\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xa9\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xa9\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xaa\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xaa\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xab\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xab\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xac\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xac\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xad\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xad\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xae\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xae\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xaf\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xaf\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xb0\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xb0\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xb1\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xb1\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xb2\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xb2\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xb3\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xb3\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xb4\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xb4\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xb5\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xb5\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xb6\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xb6\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xb7\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xb7\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xb8\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xb8\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xb9\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xb9\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xba\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xba\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xbb\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xbb\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xbc\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xbc\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xbd\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xbd\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xbe\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xbe\xbf", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xbf\x80", true, true).peek_code_point());
  EXPECT_EQ(utf8_reader::kReplacementCharacter, utf8_reader("\xed\xbf\xbf", true, true).peek_code_point());
}

TEST(SourceTest, SurrogatesAreaNonStrict) {
  EXPECT_EQ(0xD800, utf8_reader("\xed\xa0\x80", false, true).peek_code_point());
  EXPECT_EQ(0xD83F, utf8_reader("\xed\xa0\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xD840, utf8_reader("\xed\xa1\x80", false, true).peek_code_point());
  EXPECT_EQ(0xD87F, utf8_reader("\xed\xa1\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xD880, utf8_reader("\xed\xa2\x80", false, true).peek_code_point());
  EXPECT_EQ(0xD8BF, utf8_reader("\xed\xa2\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xD8C0, utf8_reader("\xed\xa3\x80", false, true).peek_code_point());
  EXPECT_EQ(0xD8FF, utf8_reader("\xed\xa3\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xD900, utf8_reader("\xed\xa4\x80", false, true).peek_code_point());
  EXPECT_EQ(0xD93F, utf8_reader("\xed\xa4\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xD940, utf8_reader("\xed\xa5\x80", false, true).peek_code_point());
  EXPECT_EQ(0xD97F, utf8_reader("\xed\xa5\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xD980, utf8_reader("\xed\xa6\x80", false, true).peek_code_point());
  EXPECT_EQ(0xD9BF, utf8_reader("\xed\xa6\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xD9C0, utf8_reader("\xed\xa7\x80", false, true).peek_code_point());
  EXPECT_EQ(0xD9FF, utf8_reader("\xed\xa7\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xDA00, utf8_reader("\xed\xa8\x80", false, true).peek_code_point());
  EXPECT_EQ(0xDA3F, utf8_reader("\xed\xa8\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xDA40, utf8_reader("\xed\xa9\x80", false, true).peek_code_point());
  EXPECT_EQ(0xDA7F, utf8_reader("\xed\xa9\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xDA80, utf8_reader("\xed\xaa\x80", false, true).peek_code_point());
  EXPECT_EQ(0xDABF, utf8_reader("\xed\xaa\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xDAC0, utf8_reader("\xed\xab\x80", false, true).peek_code_point());
  EXPECT_EQ(0xDAFF, utf8_reader("\xed\xab\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xDB00, utf8_reader("\xed\xac\x80", false, true).peek_code_point());
  EXPECT_EQ(0xDB3F, utf8_reader("\xed\xac\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xDB40, utf8_reader("\xed\xad\x80", false, true).peek_code_point());
  EXPECT_EQ(0xDB7F, utf8_reader("\xed\xad\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xDB80, utf8_reader("\xed\xae\x80", false, true).peek_code_point());
  EXPECT_EQ(0xDBBF, utf8_reader("\xed\xae\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xDBC0, utf8_reader("\xed\xaf\x80", false, true).peek_code_point());
  EXPECT_EQ(0xDBFF, utf8_reader("\xed\xaf\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xDC00, utf8_reader("\xed\xb0\x80", false, true).peek_code_point());
  EXPECT_EQ(0xDC3F, utf8_reader("\xed\xb0\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xDC40, utf8_reader("\xed\xb1\x80", false, true).peek_code_point());
  EXPECT_EQ(0xDC7F, utf8_reader("\xed\xb1\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xDC80, utf8_reader("\xed\xb2\x80", false, true).peek_code_point());
  EXPECT_EQ(0xDCBF, utf8_reader("\xed\xb2\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xDCC0, utf8_reader("\xed\xb3\x80", false, true).peek_code_point());
  EXPECT_EQ(0xDCFF, utf8_reader("\xed\xb3\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xDD00, utf8_reader("\xed\xb4\x80", false, true).peek_code_point());
  EXPECT_EQ(0xDD3F, utf8_reader("\xed\xb4\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xDD40, utf8_reader("\xed\xb5\x80", false, true).peek_code_point());
  EXPECT_EQ(0xDD7F, utf8_reader("\xed\xb5\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xDD80, utf8_reader("\xed\xb6\x80", false, true).peek_code_point());
  EXPECT_EQ(0xDDBF, utf8_reader("\xed\xb6\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xDDC0, utf8_reader("\xed\xb7\x80", false, true).peek_code_point());
  EXPECT_EQ(0xDDFF, utf8_reader("\xed\xb7\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xDE00, utf8_reader("\xed\xb8\x80", false, true).peek_code_point());
  EXPECT_EQ(0xDE3F, utf8_reader("\xed\xb8\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xDE40, utf8_reader("\xed\xb9\x80", false, true).peek_code_point());
  EXPECT_EQ(0xDE7F, utf8_reader("\xed\xb9\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xDE80, utf8_reader("\xed\xba\x80", false, true).peek_code_point());
  EXPECT_EQ(0xDEBF, utf8_reader("\xed\xba\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xDEC0, utf8_reader("\xed\xbb\x80", false, true).peek_code_point());
  EXPECT_EQ(0xDEFF, utf8_reader("\xed\xbb\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xDF00, utf8_reader("\xed\xbc\x80", false, true).peek_code_point());
  EXPECT_EQ(0xDF3F, utf8_reader("\xed\xbc\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xDF40, utf8_reader("\xed\xbd\x80", false, true).peek_code_point());
  EXPECT_EQ(0xDF7F, utf8_reader("\xed\xbd\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xDF80, utf8_reader("\xed\xbe\x80", false, true).peek_code_point());
  EXPECT_EQ(0xDFBF, utf8_reader("\xed\xbe\xbf", false, true).peek_code_point());
  EXPECT_EQ(0xDFC0, utf8_reader("\xed\xbf\x80", false, true).peek_code_point());
  EXPECT_EQ(0xDFFF, utf8_reader("\xed\xbf\xbf", false, true).peek_code_point());
}

TEST(SourceTest, Skip) {
  utf8_reader s("abc", true, true);
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
  utf8_reader s(utf8_reader_code, true, true);
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
    utf8_reader s("", true, true);
    EXPECT_EQ(true, s.next(""));
    EXPECT_EQ(0, s.pos());
    EXPECT_EQ(false, s.next("a"));
    EXPECT_EQ(0, s.pos());
  }
  {
    utf8_reader s("abc", true, true);
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
    utf8_reader s("", true, true);
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
    utf8_reader s("abc", true, true);
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
    utf8_reader s("abc", true, true);
    EXPECT_EQ(false, s.capture("abcdef"));
    EXPECT_EQ(0, s.pos());
  }
}

void utf8_peek_skip_code_point(const char* data, size_t size) {
  utf8_reader reader(std::string_view(data, size), true, true);
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

TEST(UTF8Reader, AllCharacters) {
  char source[8];
  source[0] = static_cast<char>((0xE0) | (utf8_reader::kBomCharacter >> 12));
  source[1] = static_cast<char>((0x80) | (0x3F & (utf8_reader::kBomCharacter >> 6)));
  source[2] = static_cast<char>((0x80) | (0x3F & utf8_reader::kBomCharacter));

  auto do_checks = [&source](int code_point, int length) {
    utf8_reader reader(std::string_view(source, 3 + length), true, true);
    std::string encoded;
    utf8_encode_code_point(code_point, encoded, true);
    if (reader.peek_code_point() != code_point ||
        (code_point == utf8_reader::kReplacementCharacter && length != 3)) {
      EXPECT_EQ(reader.peek_code_point(), 0xFFFD);
      EXPECT_NE(encoded, std::string(&source[3], length));
    } else {
      EXPECT_EQ(encoded, std::string(&source[3], length));
    }
    reader.skip_code_point();
    EXPECT_FALSE(reader.pending());
  };

  // 1 char long.
  for (int i = 0; i < 0x7F; ++i) {
    source[3] = i;
    source[4] = 0;
    do_checks(i, 1);
  }

  // 2 chars long.
  for (int i = 0; i < 0x7FF; ++i) {
    source[3] = static_cast<char>((0xC0) | (i >> 6));
    source[4] = static_cast<char>((0x80) | (0x3F & i));
    source[5] = 0;
    do_checks(i, 2);
  }

  // 3 chars long.
  for (int i = 0; i < 0xFFFF; ++i) {
    source[3] = static_cast<char>((0xE0) | (i >> 12));
    source[4] = static_cast<char>((0x80) | (0x3F & (i >> 6)));
    source[5] = static_cast<char>((0x80) | (0x3F & i));
    source[6] = 0;
    do_checks(i, 3);
  }

  // 4 chars long.
  for (int i = 0; i < 0x10'FFFF; ++i) {
    source[3] = static_cast<char>((0xF0) | (i >> 18));
    source[4] = static_cast<char>((0x80) | (0x3F & (i >> 12)));
    source[5] = static_cast<char>((0x80) | (0x3F & (i >> 6)));
    source[6] = static_cast<char>((0x80) | (0x3F & i));
    source[7] = 0;
    do_checks(i, 4);
  }
}

}  // namespace
