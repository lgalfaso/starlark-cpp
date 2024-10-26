// Copyright 2024 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include "unicode/encode.hpp"
#include "grammar/source.hpp"

namespace {

TEST(EncodeTest, AllCharacter) {
  for (int i = 0; i <= 0x10'ffff; ++i) {
    std::string encoded = ucd::utf8_encode_code_point(i);
    if (0xd800 <= i && i <= 0xdfff) {  // Surrogates area
      EXPECT_EQ("", encoded);
    } else if (i == 0xfeff) {  // BOM
      EXPECT_EQ("\xef\xbb\xbf", encoded);
    } else {
      EXPECT_EQ(i, grammar::source(encoded).peek_codepoint());
    }
  }
  EXPECT_EQ("", ucd::utf8_encode_code_point(0x11'0000));
}

}  // namespace
