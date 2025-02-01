// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include "unicode/encode.hpp"
#include "unicode/ucd_code_points.hpp"
#include "unicode/utf8_reader.hpp"

using unicode::utf8_reader;
using unicode::utf8_encode_code_point;

namespace {

TEST(EncodeTest, AllCharacter) {
  for (int i = 0; i <= 0x10'ffff; ++i) {
    std::string encoded;
    utf8_encode_code_point(i, encoded, true);
    if (!ucd::is_assigned(i)) {
      EXPECT_EQ("\xef\xbf\xbd", encoded);
    } else if (0xd800 <= i && i <= 0xdfff) {  // Surrogates area
      EXPECT_EQ("", encoded);
    } else if (i == 0xfeff) {  // BOM
      EXPECT_EQ("\xef\xbb\xbf", encoded);
    } else {
      EXPECT_EQ(i, utf8_reader(encoded, true).peek_code_point());
    }
  }
}

TEST(EncodeTest, OutsideUnicodeRange) {
  std::string encoded;
  utf8_encode_code_point(0x11'0000, encoded, false);
  EXPECT_EQ("\xef\xbf\xbd", encoded);
}

}  // namespace
