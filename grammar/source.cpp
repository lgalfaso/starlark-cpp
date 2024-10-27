// Copyright 2024 Lucas Mirelmann

#include "grammar/source.hpp"

#include <bit>

using std::string_view;

namespace grammar {

namespace {

inline bool is_utf8_continue(char input) {
  return (((unsigned char)input) & 0xc0) == 0x80;
}

}  // namespace

const std::uint64_t source::replacement_character;

source::source(string_view input) : input(input) {
  // If the source code starts with a BOM, then ignore it.
  if (peek_codepoint() == bom_character) {
    skip_codepoint();
  }
}

bool source::pending() const {
  return input_pos < input.length();
}

std::size_t source::pos() const {
  return input_pos;
}

char source::peek(std::size_t delta) const {
  if (delta >= input.length() - input_pos) {
    return 0;
  }
  return input[input_pos + delta];
}

std::uint64_t source::peek_codepoint() const {
  if (!pending()) {
    return replacement_character;
  }
  const unsigned char current_char = input[input_pos];
  int length = std::countl_one(current_char);
  if (length > input.length() - input_pos) {
    return replacement_character;
  }
  for (int i = 1; i < length; ++i) {
    if (!is_utf8_continue(input[input_pos + i])) {
      return replacement_character;
    }
  }
  switch (length) {
    case 0:
      return current_char;
    case 2:
      if (input[input_pos] == '\xc0' || input[input_pos] == '\xc1') {
        return replacement_character;
      }
      return (static_cast<std::uint64_t>(input[input_pos    ]) & 0x1f) << 6 |
             (static_cast<std::uint64_t>(input[input_pos + 1]) & 0x3f);
    case 3:
      if (input[input_pos] == '\xe0' && ((unsigned char)input[input_pos + 1]) < 0xa0) {
        return replacement_character;
      }
      if (input[input_pos] == '\xed' && ((unsigned char)input[input_pos + 1]) >= 0xa0) {
        return replacement_character;
      }
      return (static_cast<std::uint64_t>(input[input_pos    ]) & 0x0f) << 12 |
             (static_cast<std::uint64_t>(input[input_pos + 1]) & 0x3f) << 6 |
             (static_cast<std::uint64_t>(input[input_pos + 2]) & 0x3f);
    case 4:
      if (input[input_pos] == '\xf0' && ((unsigned char)input[input_pos + 1]) < 0x90) {
        return replacement_character;
      }
      if (input[input_pos] == '\xf4' && ((unsigned char)input[input_pos + 1]) >= 0x90) {
        return replacement_character;
      }
      return (static_cast<std::uint64_t>(input[input_pos    ]) & 0x07) << 18 |
             (static_cast<std::uint64_t>(input[input_pos + 1]) & 0x3f) << 12 |
             (static_cast<std::uint64_t>(input[input_pos + 2]) & 0x3f) << 6 |
             (static_cast<std::uint64_t>(input[input_pos + 3]) & 0x3f);
    default:
      return replacement_character;
  }
}

void source::skip(std::size_t delta) {
  if (delta >= input.length() - input_pos) {
    input_pos = input.length();
  } else {
    input_pos += delta;
  }
}

void source::skip_codepoint() {
  if (!pending()) {
    return;
  }
  const unsigned char current_char = input[input_pos];
  int length = std::countl_one(current_char);
  input_pos++;
  if (length == 0 ||  // Ascii char.
      length == 1 || length > 4) {  // Invalid first byte.
    return;
  }
  for (int i = 1; i < length && input_pos < input.length(); ++i) {
    if (is_utf8_continue(input[input_pos])) {
      ++input_pos;
    } else {
      break;
    }
  }
}

bool source::capture(string_view candidate) {
  if (input.length() - input_pos < candidate.length()) {
    return false;
  }
  if (input.substr(input_pos, candidate.length()) == candidate) {
    input_pos += candidate.length();
    return true;
  }
  return false;
}

}  // namespace grammar

