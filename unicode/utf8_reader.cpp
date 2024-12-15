// Copyright 2024 Lucas Mirelmann

#include "unicode/utf8_reader.hpp"

#include <bit>

using std::string_view;

namespace unicode {

namespace {

inline bool is_utf8_continue(char input) {
  return (((unsigned char)input) & 0xc0) == 0x80;
}

}  // namespace

const std::uint64_t utf8_reader::replacement_character;

utf8_reader::utf8_reader(string_view input) : input(input) {
  // If the source code starts with a BOM, then ignore it.
  if (peek_code_point() == bom_character) {
    skip_code_point();
  }
}

bool utf8_reader::empty() const {
  return input_pos >= input.length();
}

std::size_t utf8_reader::pending() const {
  return input.length() - input_pos;
}

std::size_t utf8_reader::pos() const {
  return input_pos;
}

char utf8_reader::peek(std::size_t delta) const {
  if (delta >= pending()) {
    return 0;
  }
  return input[input_pos + delta];
}

std::uint32_t utf8_reader::peek_code_point() const {
  if (empty()) {
    return replacement_character;
  }
  const unsigned char current_char = input[input_pos];
  int length = std::countl_one(current_char);
  if (length > pending()) {
    return replacement_character;
  }
  for (int i = 1; i < length; ++i) {
    if (!is_utf8_continue(input[input_pos + i])) {
      return replacement_character;
    }
  }
  std::uint32_t candidate;
  switch (length) {
    case 0:
      candidate = current_char;
      break;
    case 2:
      if (input[input_pos] == '\xc0' || input[input_pos] == '\xc1') {
        return replacement_character;
      }
      candidate = (static_cast<std::uint32_t>(input[input_pos    ]) & 0x1f) << 6 |
                  (static_cast<std::uint32_t>(input[input_pos + 1]) & 0x3f);
      break;
    case 3:
      if (input[input_pos] == '\xe0' && ((unsigned char)input[input_pos + 1]) < 0xa0) {
        return replacement_character;
      }
      if (input[input_pos] == '\xed' && ((unsigned char)input[input_pos + 1]) >= 0xa0) {
        return replacement_character;
      }
      candidate = (static_cast<std::uint32_t>(input[input_pos    ]) & 0x0f) << 12 |
                  (static_cast<std::uint32_t>(input[input_pos + 1]) & 0x3f) << 6 |
                  (static_cast<std::uint32_t>(input[input_pos + 2]) & 0x3f);
      break;
    case 4:
      if (input[input_pos] == '\xf0' && ((unsigned char)input[input_pos + 1]) < 0x90) {
        return replacement_character;
      }
      if (input[input_pos] == '\xf4' && ((unsigned char)input[input_pos + 1]) >= 0x90) {
        return replacement_character;
      }
      candidate = (static_cast<std::uint32_t>(input[input_pos    ]) & 0x07) << 18 |
                  (static_cast<std::uint32_t>(input[input_pos + 1]) & 0x3f) << 12 |
                  (static_cast<std::uint32_t>(input[input_pos + 2]) & 0x3f) << 6 |
                  (static_cast<std::uint32_t>(input[input_pos + 3]) & 0x3f);
      break;
    default:
      return replacement_character;
  }
  // TODO(lmirelmann): Should we check that the candidate is assigned?
  return candidate;
}

void utf8_reader::skip(std::size_t delta) {
  if (delta >= pending()) {
    input_pos = input.length();
  } else {
    input_pos += delta;
  }
}

void utf8_reader::skip_code_point() {
  if (empty()) {
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

bool utf8_reader::next(string_view candidate) {
  if (pending() < candidate.length()) {
    return false;
  }
  if (input.substr(input_pos, candidate.length()) == candidate) {
    return true;
  }
  return false;
}

bool utf8_reader::capture(string_view candidate) {
  if (pending() < candidate.length()) {
    return false;
  }
  if (input.substr(input_pos, candidate.length()) == candidate) {
    input_pos += candidate.length();
    return true;
  }
  return false;
}

}  // namespace unicode

