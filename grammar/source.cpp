// Copyright 2024 Lucas Mirelmann

#include "grammar/source.hpp"

#include <bit>

using std::string_view;

namespace grammar {

namespace {

bool is_utf8_continue(char input) {
  return (((unsigned char)input) & 0xc0) == 0x80;
}

}  // namespace

const std::uint64_t source::invalid_codepoint;

source::source(string_view source_code) : source_code(source_code) {}

bool source::is_end() const {
  return pos == source_code.length();
}

std::size_t source::get_pos() const {
  return pos;
}

char source::peek(std::size_t delta) const {
  if (delta >= source_code.length() - pos) {
    return 0;
  }
  return source_code[pos + delta];
}

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
      if (source_code[pos] == '\xc0' || source_code[pos] == '\xc1') {
        return invalid_codepoint;
      }
      return (static_cast<std::uint64_t>(source_code[pos    ]) & 0x1f) << 6 |
             (static_cast<std::uint64_t>(source_code[pos + 1]) & 0x3f);
    case 3:
      if (source_code[pos] == '\xe0' && ((unsigned char)source_code[pos + 1]) < 0xa0) {
        return invalid_codepoint;
      }
      return (static_cast<std::uint64_t>(source_code[pos    ]) & 0x0f) << 12 |
             (static_cast<std::uint64_t>(source_code[pos + 1]) & 0x3f) << 6 |
             (static_cast<std::uint64_t>(source_code[pos + 2]) & 0x3f);
    case 4:
      if (source_code[pos] == '\xf0' && ((unsigned char)source_code[pos + 1]) < 0x90) {
        return invalid_codepoint;
      }
      return (static_cast<std::uint64_t>(source_code[pos    ]) & 0x07) << 18 |
             (static_cast<std::uint64_t>(source_code[pos + 1]) & 0x3f) << 12 |
             (static_cast<std::uint64_t>(source_code[pos + 2]) & 0x3f) << 6 |
             (static_cast<std::uint64_t>(source_code[pos + 3]) & 0x3f);
    default:
      return invalid_codepoint;
  }
}

void source::skip(std::size_t delta) {
  if (delta >= source_code.length() - pos) {
    pos = source_code.length();
  } else {
    pos += delta;
  }
}

bool source::capture(string_view input) {
  if (source_code.length() - pos < input.length()) {
    return false;
  }
  if (source_code.substr(pos, input.length()) == input) {
    pos += input.length();
    return true;
  }
  return false;
}

}  // namespace grammar

