// Copyright 2024-2025 Lucas Mirelmann

#include "grammar/numeric_parser.hpp"

#include <string>

using unicode::utf8_reader;

namespace grammar {

namespace {

std::optional<std::string> read_number_over(std::string_view chars, utf8_reader& input) {
  std::string result;
  bool accepted_digit = false;
  while (!input.empty() && input.peek() == '0') {
    accepted_digit = true;
    input.skip();
  }
  while (!input.empty()) {
    auto next = input.peek();
    if (chars.find(next) == std::string_view::npos) {
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
    return {};
  }
  if (!accepted_digit) {
    return {};
  }
  if (result.empty()) {
    result = "0";
  }
  return result;
}

}  // namespace


std::optional<std::string> read_number(utf8_reader& input) {
  std::string result;

  if (input.capture("0x") || input.capture("0X")) {
    auto number = read_number_over("0123456789abcdefABCDEF", input);
    if (!number) {
      return {};
    }
    if (number.value() == "0") {
      return "0";
    }
    result = "0x" + number.value();
  } else if (input.capture("0o") || input.capture("0O")) {
    auto number = read_number_over("01234567", input);
    if (!number) {
      return {};
    }
    if (number.value() == "0") {
      return "0";
    }
    result = "0" + number.value();
  } else {
    bool found_dot = false;
    bool found_e = false;
    bool accepted_digit = false;
    while (!input.empty() && input.peek() == '0') {
      accepted_digit = true;
      input.skip();
    }
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
          return {};
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
      return {};
    }
    if (result.empty()) {
      result = "0";
    }
  }
  return result;
}

}  // namespace grammar

