// Copyright 2024 Lucas Mirelmann

#include "grammar/source.hpp"

#include <bit>

namespace grammar {

namespace {

bool is_utf8_continue(char input) {
  return (((unsigned char)input) & 0xc0) == 0x80;
}

}  // namespace

const std::uint64_t source::invalid_codepoint;

source::source(std::string_view source_code) : source_code(source_code) {}

std::uint64_t source::peek_codepoint() const {
  if (pos >= source_code.length()) {
    return invalid_codepoint;
  }
  const unsigned char current_char = source_code[pos];
  int length = std::countl_one(current_char);
  if (length > source_code.length() - pos) {
    return invalid_codepoint;
  }
  for (int i = 1; i < length; ++i) {
    if (!is_utf8_continue(source_code[pos + i])) {
      return invalid_codepoint;
    }
  }
  switch (length) {
    case 0:
      return current_char;
    case 2:
      return (static_cast<std::uint64_t>(source_code[pos    ]) & 0x1f) << 6 |
             (static_cast<std::uint64_t>(source_code[pos + 1]) & 0x3f);
    case 3:
      return (static_cast<std::uint64_t>(source_code[pos    ]) & 0x0f) << 12 |
             (static_cast<std::uint64_t>(source_code[pos + 1]) & 0x3f) << 6 |
             (static_cast<std::uint64_t>(source_code[pos + 2]) & 0x3f);
    case 4:
      return (static_cast<std::uint64_t>(source_code[pos    ]) & 0x07) << 18 |
             (static_cast<std::uint64_t>(source_code[pos + 1]) & 0x3f) << 12 |
             (static_cast<std::uint64_t>(source_code[pos + 2]) & 0x3f) << 6 |
             (static_cast<std::uint64_t>(source_code[pos + 3]) & 0x3f);
    default:
      return invalid_codepoint;
  }
}

}  // namespace grammar

