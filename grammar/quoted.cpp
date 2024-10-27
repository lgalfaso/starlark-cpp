// Copyright 2024 Lucas Mirelmann

#include "grammar/quoted.hpp"

#include <format>

namespace grammar {

std::string quoted(const std::string& input) {
  std::string result;
  result += "\"";
  for (unsigned char c : input) {
    if (c == '\\' || c == '"') {
      result += '\\';
      result += c;
    } else if (32 <= c && c < 127) {
      result += c;
    } else {
      result += std::format("\\{:03o}", c);
    }
  }
  result += "\"";
  return result;
}

}  // namespace grammar

