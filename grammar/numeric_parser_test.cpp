// Copyright 2024 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include "grammar/numeric_parser.hpp"

using grammar::read_number;
using unicode::utf8_reader;

namespace {

void check(std::string_view input, std::optional<std::string> expected) {
  utf8_reader s(input);
  EXPECT_EQ(read_number(s), expected);
}

TEST(NumericParser, Hex) {
  check("0x0123456789abcdef", "0x123456789abcdef");
  check("0X0123456789abcdef", "0x123456789abcdef");
  check("0x0123456789abcdefABCDEF", "0x123456789abcdefABCDEF");
  check("0x0123456789abcdef or 1", "0x123456789abcdef");
  check("0x0123456789abcdefor1", "0x123456789abcdef");
  check("0x0", "0");
  check("0x0 - 4", "0");
  check("0x", {});
  check("0xg", {});
}

TEST(NumericParser, Octal) {
  check("0o01234567", "01234567");
  check("0O01234567", "01234567");
  check("0o0", "0");
  check("0o0 - 4", "0");
  check("0o", {});
  check("0o8", {});
  check("0o08", {});
}

TEST(NumericParser, Integer) {
  check("1234 - 4321", "1234");
  check("0 - 4321", "0");
  check("0", "0");
  check("0123456789", "123456789");
}

TEST(NumericParser, Float) {
  check(".1234-4321", ".1234");
  check("1234.56789-4321", "1234.56789");
  check("1234.-4321", "1234.");
  check("1234.56789.to_string() - 4321", "1234.56789");

  check("1234e1-4321", "1234e1");
  check(".1234e1-4321", ".1234e1");
  check("12.34e1-4321", "12.34e1");
  check("1234.e1-4321", "1234.e1");
  check("1234e.1-4321", {});
  check("1234e1.to_string() - 4321", "1234e1");
  check("1234E1-4321", "1234e1");

  check("1234e-1-4321", "1234e-1");
  check("1234e+1-4321", "1234e1");
  check("1234e+-1-4321", {});
  check("1234e-+1-4321", {});
  check("1234e- 1", {});

  check("1234e1echo-4321", "1234e1");
  check("1234e - 4321", {});
  check(".e1-4321", {});
}

TEST(NumericParser, Invalid) {
  check("", {});
  check("foo", {});
}

}  // namespace
