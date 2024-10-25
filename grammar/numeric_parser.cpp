// Copyright 2024 Lucas Mirelmann

#include <iostream>

#include "grammar/numeric_parser.hpp"

namespace grammar {

namespace {

std::optional<std::string> read_number_over(std::string_view chars, source& input) {
  std::string result;
  bool accepted_digit = false;
  while (!input.is_end()) {
    auto next = input.peek();
    if (chars.find(next) == std::string_view::npos) {
      break;
    }
    accepted_digit = true;
    result += next;
    input.skip();
  }
  if (!accepted_digit) {
    return {};
  }
  if (!input.is_end() && isdigit(input.peek())) {
    return {};
  }
  return result;
}

}  // namespace


std::optional<std::string> read_number(source& input) {
  std::string result;

  if (input.capture("0x") || input.capture("0X")) {
    result += "0x";
    auto number = read_number_over("0123456789abcdefABCDEF", input);
    if (!number) {
      return {};
    }
    result += number.value();
  } else if (input.capture("0o") || input.capture("0O")) {
    result += "0";
    auto number = read_number_over("01234567", input);
    if (!number) {
      return {};
    }
    result += number.value();
  } else {
    bool found_dot = false;
    bool found_e = false;
    bool accepted_digit = false;
    while (!input.is_end()) {
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
  } 
  return result;
}

}  // namespace grammar

