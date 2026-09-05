// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <string>

#include "unicode/encode.hpp"
#include "unicode/ucd_code_points.hpp"
#include "unicode/utf8_reader.hpp"

using ::starlark::unicode::utf8_reader;
using ::starlark::unicode::utf8_encode_code_point;

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

}  // namespace
