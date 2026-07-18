// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <limits>
#include <random>
#include <string>
#include <utility>

#include "unicode/encode.hpp"
#include "unicode/utf8_reader.hpp"

using starlark::unicode::utf8_encode_code_point;
using starlark::unicode::utf8_reader;

namespace {

TEST(SourceTest, BOM) {
  EXPECT_EQ('a', utf8_reader("a", true, true).read_code_point());
  EXPECT_EQ('a', utf8_reader("\xef\xbb\x{bf}a", true, true).read_code_point());
  EXPECT_EQ(0xfeff, utf8_reader("\xef\xbb\x{bf}a", true, false).read_code_point());
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
  EXPECT_EQ(std::make_pair('a', 1), utf8_reader("a", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0x234, 2), utf8_reader("\xc8\xb4", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0x1234, 3), utf8_reader("\xe1\x88\xb4", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0x12345, 4), utf8_reader("\xf0\x92\x8d\x85", true, true).peek_code_point());

  // Too short
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 0), utf8_reader("", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 1), utf8_reader("\xc8", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 1), utf8_reader("\xe1\x88", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 1), utf8_reader("\xf0\x92\x8d", true, true).peek_code_point());

  // Invalid first byte
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 1), utf8_reader("\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 1), utf8_reader("\xf8\xbf\xbf\xbf\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 1), utf8_reader("\xfc\xbf\xbf\xbf\xbf\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 1), utf8_reader("\xfe\xbf\xbf\xbf\xbf\xbf\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 1), utf8_reader("\xff\xbf\xbf\xbf\xbf\xbf\xbf\xbf", true, true).peek_code_point());

  // Invalid follow-up byte
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 1), utf8_reader("\xc8\x34", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 1), utf8_reader("\xc8\xf4", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 1), utf8_reader("\xe1\x08\xb4", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 1), utf8_reader("\xe1\xc8\xb4", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 2), utf8_reader("\xe1\x88\x34", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 2), utf8_reader("\xe1\x88\xf4", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 1), utf8_reader("\xf0\x12\x8d\x85", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 1), utf8_reader("\xf0\xd2\x8d\x85", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 2), utf8_reader("\xf0\x92\x0d\x85", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 2), utf8_reader("\xf0\x92\xcd\x85", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xf0\x92\x8d\x05", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xf0\x92\x8d\xc5", true, true).peek_code_point());

  // The largest possible Unicode character, 0x10FFFF.
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 4), utf8_reader("\xf4\x90\x80\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 4), utf8_reader("\xf4\x8f\xbf\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kMaxCodePoint, 4), utf8_reader("\xf4\x8f\xbf\xbf", false, true).peek_code_point());
}

TEST(SourceTest, OverlongEncoding) {
  // Overlong encoding.
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 2), utf8_reader("\xc0\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 2), utf8_reader("\xc1\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0x7f, 1), utf8_reader("\x7f", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xe0\x9f\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0x7ff, 2), utf8_reader("\xdf\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 4), utf8_reader("\xf0\x8f\xbf\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xef\xbf\xbf", true, true).peek_code_point());
}

TEST(SourceTest, SurrogatesAreaStrict) {
  // The range 0xD800-0xDFFF is invalid in strict mode and valid otherwise.
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xa0\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xa0\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xa1\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xa1\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xa2\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xa2\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xa3\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xa3\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xa4\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xa4\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xa5\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xa5\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xa6\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xa6\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xa7\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xa7\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xa8\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xa8\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xa9\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xa9\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xaa\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xaa\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xab\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xab\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xac\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xac\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xad\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xad\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xae\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xae\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xaf\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xaf\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xb0\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xb0\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xb1\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xb1\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xb2\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xb2\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xb3\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xb3\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xb4\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xb4\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xb5\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xb5\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xb6\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xb6\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xb7\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xb7\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xb8\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xb8\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xb9\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xb9\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xba\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xba\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xbb\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xbb\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xbc\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xbc\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xbd\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xbd\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xbe\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xbe\xbf", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xbf\x80", true, true).peek_code_point());
  EXPECT_EQ(std::make_pair(utf8_reader::kReplacementCharacter, 3), utf8_reader("\xed\xbf\xbf", true, true).peek_code_point());
}

TEST(SourceTest, SurrogatesAreaNonStrict) {
  EXPECT_EQ(std::make_pair(0xD800, 3), utf8_reader("\xed\xa0\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xD83F, 3), utf8_reader("\xed\xa0\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xD840, 3), utf8_reader("\xed\xa1\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xD87F, 3), utf8_reader("\xed\xa1\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xD880, 3), utf8_reader("\xed\xa2\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xD8BF, 3), utf8_reader("\xed\xa2\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xD8C0, 3), utf8_reader("\xed\xa3\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xD8FF, 3), utf8_reader("\xed\xa3\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xD900, 3), utf8_reader("\xed\xa4\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xD93F, 3), utf8_reader("\xed\xa4\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xD940, 3), utf8_reader("\xed\xa5\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xD97F, 3), utf8_reader("\xed\xa5\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xD980, 3), utf8_reader("\xed\xa6\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xD9BF, 3), utf8_reader("\xed\xa6\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xD9C0, 3), utf8_reader("\xed\xa7\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xD9FF, 3), utf8_reader("\xed\xa7\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDA00, 3), utf8_reader("\xed\xa8\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDA3F, 3), utf8_reader("\xed\xa8\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDA40, 3), utf8_reader("\xed\xa9\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDA7F, 3), utf8_reader("\xed\xa9\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDA80, 3), utf8_reader("\xed\xaa\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDABF, 3), utf8_reader("\xed\xaa\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDAC0, 3), utf8_reader("\xed\xab\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDAFF, 3), utf8_reader("\xed\xab\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDB00, 3), utf8_reader("\xed\xac\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDB3F, 3), utf8_reader("\xed\xac\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDB40, 3), utf8_reader("\xed\xad\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDB7F, 3), utf8_reader("\xed\xad\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDB80, 3), utf8_reader("\xed\xae\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDBBF, 3), utf8_reader("\xed\xae\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDBC0, 3), utf8_reader("\xed\xaf\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDBFF, 3), utf8_reader("\xed\xaf\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDC00, 3), utf8_reader("\xed\xb0\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDC3F, 3), utf8_reader("\xed\xb0\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDC40, 3), utf8_reader("\xed\xb1\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDC7F, 3), utf8_reader("\xed\xb1\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDC80, 3), utf8_reader("\xed\xb2\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDCBF, 3), utf8_reader("\xed\xb2\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDCC0, 3), utf8_reader("\xed\xb3\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDCFF, 3), utf8_reader("\xed\xb3\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDD00, 3), utf8_reader("\xed\xb4\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDD3F, 3), utf8_reader("\xed\xb4\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDD40, 3), utf8_reader("\xed\xb5\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDD7F, 3), utf8_reader("\xed\xb5\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDD80, 3), utf8_reader("\xed\xb6\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDDBF, 3), utf8_reader("\xed\xb6\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDDC0, 3), utf8_reader("\xed\xb7\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDDFF, 3), utf8_reader("\xed\xb7\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDE00, 3), utf8_reader("\xed\xb8\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDE3F, 3), utf8_reader("\xed\xb8\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDE40, 3), utf8_reader("\xed\xb9\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDE7F, 3), utf8_reader("\xed\xb9\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDE80, 3), utf8_reader("\xed\xba\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDEBF, 3), utf8_reader("\xed\xba\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDEC0, 3), utf8_reader("\xed\xbb\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDEFF, 3), utf8_reader("\xed\xbb\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDF00, 3), utf8_reader("\xed\xbc\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDF3F, 3), utf8_reader("\xed\xbc\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDF40, 3), utf8_reader("\xed\xbd\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDF7F, 3), utf8_reader("\xed\xbd\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDF80, 3), utf8_reader("\xed\xbe\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDFBF, 3), utf8_reader("\xed\xbe\xbf", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDFC0, 3), utf8_reader("\xed\xbf\x80", false, true).peek_code_point());
  EXPECT_EQ(std::make_pair(0xDFFF, 3), utf8_reader("\xed\xbf\xbf", false, true).peek_code_point());
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
  s.read_code_point();
  EXPECT_EQ(expected_pos, s.pos()) << std::string_view(input, input_length);
}

TEST(SourceTest, ReadCodePoint) {
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
    reader.read_code_point();
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
    utf8_encode_code_point(code_point, encoded, true, false);
    if (reader.peek_code_point().first != code_point ||
        (code_point == utf8_reader::kReplacementCharacter && length != 3)) {
      EXPECT_EQ(reader.peek_code_point(), std::make_pair(0xFFFD, length));
      EXPECT_NE(encoded, std::string(&source[3], length));
    } else {
      EXPECT_EQ(encoded, std::string(&source[3], length));
    }
    reader.read_code_point();
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

TEST(UTF8Reader, AllCombinations) {
  char source[8];
  std::string output;
  std::set<int> total;
  for (int i = 0; i < 256; ++i) {
    for (int j = 0; j < 256; ++j) {
      for (int k = 0; k < 256; ++k) {
        for (int l = 0; l < 256; ++l) {
          source[0] = i;
          source[1] = j;
          source[2] = k;
          source[3] = l;
          utf8_reader reader(std::string_view(source, 4), false, false);
          auto cp = reader.peek_code_point();
          if (cp.first == utf8_reader::kReplacementCharacter) {
            continue;
          }
          std::string encoded;
          utf8_encode_code_point(cp.first, encoded, false, true);
          ASSERT_EQ(encoded.size(), cp.second) << (int)cp.first;
          if (encoded.size() >= 1) {
            EXPECT_TRUE(static_cast<unsigned char>(encoded[0]) == i);
          }
          if (encoded.size() >= 2) {
            EXPECT_TRUE(static_cast<unsigned char>(encoded[1]) == j);
          }
          if (encoded.size() >= 3) {
            EXPECT_TRUE(static_cast<unsigned char>(encoded[2]) == k);
          }
          if (encoded.size() >= 4) {
            EXPECT_TRUE(static_cast<unsigned char>(encoded[3]) == l);
          }
          total.insert(cp.first);
        }
      }
    }
  }
  EXPECT_EQ(total.size(), 0x10FFFF);
}

}  // namespace
