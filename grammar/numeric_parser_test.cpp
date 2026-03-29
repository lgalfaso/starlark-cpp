// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <string>

#include "grammar/numeric_parser.hpp"

using starlark::grammar::read_number;
using starlark::unicode::utf8_reader;

namespace {

void check(std::string_view input, std::string_view expected, int expected_pos) {
  utf8_reader s(input, true, false);
  auto result = read_number(s, false);
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(*result, expected);
  EXPECT_EQ(s.pos(), expected_pos);
}

void check_fail(std::string_view input, int expected_pos) {
  utf8_reader s(input, true, false);
  auto result = read_number(s, false);
  ASSERT_FALSE(result.ok());
  EXPECT_EQ(s.pos(), expected_pos);
}

void check_with_binary(std::string_view input, std::string_view expected, int expected_pos) {
  utf8_reader s(input, true, false);
  auto result = read_number(s, true);
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(*result, expected);
  EXPECT_EQ(s.pos(), expected_pos);
}

void check_fail_with_binary(std::string_view input, int expected_pos) {
  utf8_reader s(input, true, false);
  auto result = read_number(s, true);
  ASSERT_FALSE(result.ok());
  EXPECT_EQ(s.pos(), expected_pos);
}

TEST(NumericParser, Hex) {
  check("0x0123456789abcdef", "0x0123456789abcdef", 18);
  check("0X0123456789abcdef", "0x0123456789abcdef", 18);
  check("0x0123456789abcdefABCDEF", "0x0123456789abcdefABCDEF", 24);
  check("0x0123456789abcdef or 1", "0x0123456789abcdef", 18);
  check("0x0123456789abcdefor1", "0x0123456789abcdef", 18);
  check("0x0", "0", 3);
  check("0x0 - 4", "0", 3);
  check_fail("0x", 2);
  check_fail("0xg", 2);
}

TEST(NumericParser, Octal) {
  check("0o01234567", "0o01234567", 10);
  check("0O01234567", "0o01234567", 10);
  check("0o0", "0", 3);
  check("0o0 - 4", "0", 3);
  check_fail("0o", 2);
  check_fail("0o8", 3);
  check_fail("0o08", 4);
}

TEST(NumericParser, Binary) {
  check_fail("0b10101001", 10);
  check_fail("0B10101001", 10);
  check_with_binary("0b10101001", "0b10101001", 10);
  check_with_binary("0B10101001", "0b10101001", 10);
  check_with_binary("0b0", "0", 3);
  check_with_binary("0b0 - 4", "0", 3);
  check_fail_with_binary("0b", 2);
  check_fail_with_binary("0b2", 3);
  check_fail_with_binary("0b02", 4);
}

TEST(NumericParser, Integer) {
  check("1234 - 4321", "1234", 4);
  check("0 - 4321", "0", 1);
  check("0", "0", 1);
  check("0123456789", "0123456789", 10);
}

TEST(NumericParser, Float) {
  check(".1234-4321", ".1234", 5);
  check("1234.56789-4321", "1234.56789", 10);
  check("1234.-4321", "1234.", 5);
  check("1234.56789.to_string() - 4321", "1234.56789", 10);

  check("1234e1-4321", "1234e1", 6);
  check(".1234e1-4321", ".1234e1", 7);
  check("12.34e1-4321", "12.34e1", 7);
  check("1234.e1-4321", "1234.e1", 7);
  check_fail("1234e.1-4321", 5);
  check("1234e1.to_string() - 4321", "1234e1", 6);
  check("1234E1-4321", "1234e1", 6);

  check("1234e-1-4321", "1234e-1", 7);
  check("1234e+1-4321", "1234e1", 7);
  check_fail("1234e+-1-4321", 6);
  check_fail("1234e-+1-4321", 6);
  check_fail("1234e- 1", 6);

  check("1234e1echo-4321", "1234e1", 6);
  check_fail("1234e - 4321", 5);
  check_fail(".e1-4321", 1);
}

TEST(NumericParser, Invalid) {
  check_fail("", 0);
  check_fail("foo", 0);
}

}  // namespace
