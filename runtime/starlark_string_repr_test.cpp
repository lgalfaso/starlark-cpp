// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>

#include <string>

#include "runtime/starlark_string.hpp"
#include "unicode/encode.hpp"

using ::starlark::runtime::append_for_repr;
using ::starlark::runtime::starlark_string;
using ::starlark::unicode::utf8_encode_code_point;

namespace {

TEST(StarlarkString, ReprBlocks) {
  for (int start = 0; start < 0x1100; ++start) {
    std::string input;
    for (int i = 0; i < 256; ++i) {
      utf8_encode_code_point(start * 256 + i, input, false, true);
    }

    std::string expected = "\"";
    append_for_repr(expected, input);
    expected += '"';

    EXPECT_EQ(starlark_string(input).repr(), expected) << "start=" << start;
  }
}

}  // namespace
