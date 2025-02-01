// Copyright 2024-2025 Lucas Mirelmann

#include "grammar/quoted.hpp"

#include <format>
#include <string>

namespace grammar {

std::string quoted(std::string_view input) {
  std::string result;
  result += "\"";
  for (unsigned char c : input) {
    if (c == '\\' || c == '"') {
      result += '\\';
      result += c;
    } else if (32 <= c && c < 127) {
      result += c;
    } else if (c == '\a') {
      result += "\\a";
    } else if (c == '\b') {
      result += "\\b";
    } else if (c == '\f') {
      result += "\\f";
    } else if (c == '\n') {
      result += "\\n";
    } else if (c == '\r') {
      result += "\\r";
    } else if (c == '\t') {
      result += "\\t";
    } else if (c == '\v') {
      result += "\\v";
    } else {
      result += std::format("\\{:03o}", c);
    }
  }
  result += "\"";
  return result;
}

}  // namespace grammar

