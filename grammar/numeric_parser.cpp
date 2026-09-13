// Copyright 2024-2026 Lucas Mirelmann

#include "grammar/numeric_parser.hpp"

#include <string>
#include <utility>

using ::starlark::result::status_code;
using ::starlark::result::status_or;
using ::starlark::unicode::utf8_reader;

namespace starlark {
namespace grammar {

namespace {

bool is_binary_digit(char c) {
  return ('0' <= c && c <= '1');
}

bool is_octal_digit(char c) {
  return ('0' <= c && c <= '7');
}

bool is_hex_digit(char c) {
  return ('0' <= c && c <= '9') ||
         ('a' <= c && c <= 'f') ||
         ('A' <= c && c <= 'F');
}

status_or<std::string> read_number_over(bool(*match)(char), utf8_reader& input) {
  std::string result;
  bool accepted_digit = false;
  while (!input.empty()) {
    auto next = input.peek();
    if (!match(next)) {
      break;
    }
    accepted_digit = true;
    result += next;
    input.skip();
  }
  if (!input.empty() && isdigit(input.peek())) {
    while (!input.empty() && isdigit(input.peek())) {
      input.skip();
    }
    return status_or<std::string>(status_code::kStaticError);
  }
  if (!accepted_digit) {
    return status_or<std::string>(status_code::kStaticError);
  }
  return status_or<std::string>(std::move(result));
}

}  // namespace

status_or<std::string> read_number(utf8_reader& input, bool allow_binary_literals) {
  std::string result;

  if (input.capture("0x") || input.capture("0X")) {
    auto number = read_number_over(is_hex_digit, input);
    if (!number.ok()) {
      return status_or<std::string>(status_code::kStaticError);
    }
    if (*number == "0") {
      return status_or<std::string>("0");
    }
    result = "0x" + *number;
  } else if (input.capture("0o") || input.capture("0O")) {
    auto number = read_number_over(is_octal_digit, input);
    if (!number.ok()) {
      return status_or<std::string>(status_code::kStaticError);
    }
    if (*number == "0") {
      return status_or<std::string>("0");
    }
    result = "0o" + *number;
  } else if (input.capture("0b") || input.capture("0B")) {
    auto number = read_number_over(is_binary_digit, input);
    if (!allow_binary_literals || !number.ok()) {
      return status_or<std::string>(status_code::kStaticError);
    }
    if (*number == "0") {
      return status_or<std::string>("0");
    }
    result = "0b" + *number;
  } else {
    bool found_dot = false;
    bool found_e = false;
    bool accepted_digit = false;
    while (!input.empty()) {
      auto next = input.peek();
      if (next == '.') {
        if (found_dot || found_e) {
          break;
        }
        result += next;
        found_dot = true;
        input.skip();
        continue;
      }
      if (next == 'e' || next == 'E') {
        if (!accepted_digit) {
          return status_or<std::string>(status_code::kStaticError);
        }
        if (found_e) {
          break;
        }
        found_e = true;
        result += "e";
        input.skip();
        if (input.capture("-")) {
          result += "-";
        } else if (input.capture("+")) {
          // No-op.
        }
        accepted_digit = false;
        continue;
      }
      if (!isdigit(next)) {
        break;
      }
      accepted_digit = true;
      result += next;
      input.skip();
    }
    if (!accepted_digit) {
      return status_or<std::string>(status_code::kStaticError);
    }
  }
  return status_or<std::string>(std::move(result));
}

}  // namespace grammar
}  // namespace starlark

