// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <string>

#include "unicode/encode.hpp"
#include "unicode/ucd_code_points.hpp"
#include "unicode/utf8_reader.hpp"

using ::starlark::unicode::utf8_encode_code_point;
using ::starlark::unicode::utf8_reader;

namespace {

TEST(EncodeTest, AllCharacterStrictNoSurrogates) {
  for (int i = 0; i <= 0x10'ffff; ++i) {
    std::string encoded;
    utf8_encode_code_point(i, encoded, true, false);
    if (!starlark::ucd::is_assigned(i)) {
      EXPECT_EQ("\xef\xbf\xbd", encoded);
    } else if (0xd800 <= i && i <= 0xdfff) {  // Surrogates area
      EXPECT_EQ("", encoded);
    } else {
      EXPECT_EQ(i, utf8_reader(encoded, false, false).read_code_point());
    }
  }
}

TEST(EncodeTest, AllCharacterStrictWithSurrogates) {
  for (int i = 0; i <= 0x10'ffff; ++i) {
    std::string encoded;
    utf8_encode_code_point(i, encoded, true, true);
    if (!starlark::ucd::is_assigned(i)) {
      EXPECT_EQ("\xef\xbf\xbd", encoded);
    } else {
      EXPECT_EQ(i, utf8_reader(encoded, false, false).read_code_point());
    }
  }
}

TEST(EncodeTest, AllCharacterNotStrictNoSurrogates) {
  for (int i = 0; i <= 0x10'ffff; ++i) {
    std::string encoded;
    utf8_encode_code_point(i, encoded, false, false);
    if (0xd800 <= i && i <= 0xdfff) {  // Surrogates area
      EXPECT_EQ("", encoded);
    } else {
      EXPECT_EQ(i, utf8_reader(encoded, false, false).read_code_point());
    }
  }
}

TEST(EncodeTest, AllCharacterNotStrictWithSurrogates) {
  for (int i = 0; i <= 0x10'ffff; ++i) {
    std::string encoded;
    utf8_encode_code_point(i, encoded, false, true);
    EXPECT_EQ(i, utf8_reader(encoded, false, false).read_code_point());
  }
}

TEST(EncodeTest, OutsideUnicodeRange) {
  std::string encoded;
  utf8_encode_code_point(0x11'0000, encoded, false, true);
  EXPECT_EQ("\xef\xbf\xbd", encoded);
}

TEST(EncodeTest, Utf8DecodeValid) {
  std::vector<char32_t> codepoints;
  EXPECT_TRUE(starlark::unicode::utf8_decode("abc", codepoints));
  ASSERT_EQ(3u, codepoints.size());
  EXPECT_EQ('a', codepoints[0]);
  EXPECT_EQ('b', codepoints[1]);
  EXPECT_EQ('c', codepoints[2]);
}

TEST(EncodeTest, Utf8DecodeInvalid) {
  std::vector<char32_t> codepoints;
  EXPECT_FALSE(starlark::unicode::utf8_is_well_formed(std::string("🙂").substr(0, 1)));
  EXPECT_FALSE(starlark::unicode::utf8_decode(std::string("🙂").substr(0, 1), codepoints));
}

TEST(EncodeTest, Utf8EncodedSize) {
  EXPECT_EQ(1u, starlark::unicode::utf8_encoded_size('a'));
  EXPECT_EQ(2u, starlark::unicode::utf8_encoded_size(0x00e0));
}

}  // namespace
