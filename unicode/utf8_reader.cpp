// Copyright 2024-2025 Lucas Mirelmann

#include "unicode/utf8_reader.hpp"

#include <bit>

#include "unicode/ucd_code_points.hpp"

using std::string_view;

namespace starlark {
namespace unicode {

bool is_utf8_continue(char input) {
  return (((unsigned char)input) & 0xc0) == 0x80;
}

bool is_in_range(std::uint32_t code_point) {
  return code_point <= utf8_reader::kMaxCodePoint;
}

bool is_surrogate(std::uint32_t code_point) {
  return (0xD800 <= code_point) && (code_point <= 0xDFFF);
}

std::string_view replacement_character_utf8() {
  return "\xEF\xBF\xBD";
}

const std::uint32_t utf8_reader::kReplacementCharacter;
const std::uint32_t utf8_reader::kBomCharacter;
const std::uint32_t utf8_reader::kMaxCodePoint;

utf8_reader::utf8_reader(string_view input, bool strict, bool remove_boom) : input(input), strict(strict) {
  // If the source code starts with a BOM, then ignore it.
  if (remove_boom) {
    capture("\xEF\xBB\xBF");
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

std::uint32_t utf8_reader::peek_code_point() {
  return read_code_point(false);
}

void utf8_reader::skip(std::size_t delta) {
  if (delta >= pending()) {
    input_pos = input.length();
  } else {
    input_pos += delta;
  }
}

std::uint32_t utf8_reader::read_code_point() {
  return read_code_point(true);
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

std::uint32_t utf8_reader::read_code_point(bool move_forward) {
  if (empty()) {
    return kReplacementCharacter;
  }
  const unsigned char current_char = input[input_pos];
  int length = std::countl_one(current_char);
  if (length > pending()) {
    if (move_forward) {
      input_pos += 1;
    }
    return kReplacementCharacter;
  }
  for (int i = 1; i < length; ++i) {
    if (!is_utf8_continue(input[input_pos + i])) {
      if (move_forward) {
        input_pos += i;
      }
      return kReplacementCharacter;
    }
  }
  std::uint32_t candidate;
  switch (length) {
    case 0:
      candidate = current_char;
      length = 1;
      break;
    case 2:
      if (input[input_pos] == '\xc0' || input[input_pos] == '\xc1') {
        candidate = kReplacementCharacter;
        break;
      }
      candidate = (static_cast<std::uint32_t>(input[input_pos    ]) & 0x1f) << 6 |
                  (static_cast<std::uint32_t>(input[input_pos + 1]) & 0x3f);
      break;
    case 3:
      if (input[input_pos] == '\xe0' && ((unsigned char)input[input_pos + 1]) < 0xa0) {
        candidate = kReplacementCharacter;
        break;
      }
      candidate = (static_cast<std::uint32_t>(input[input_pos    ]) & 0x0f) << 12 |
                  (static_cast<std::uint32_t>(input[input_pos + 1]) & 0x3f) << 6 |
                  (static_cast<std::uint32_t>(input[input_pos + 2]) & 0x3f);
      break;
    case 4:
      if (input[input_pos] == '\xf0' && ((unsigned char)input[input_pos + 1]) < 0x90) {
        candidate = kReplacementCharacter;
        break;
      }
      if (input[input_pos] == '\xf4' && ((unsigned char)input[input_pos + 1]) >= 0x90) {
        candidate = kReplacementCharacter;
        break;
      }
      candidate = (static_cast<std::uint32_t>(input[input_pos    ]) & 0x07) << 18 |
                  (static_cast<std::uint32_t>(input[input_pos + 1]) & 0x3f) << 12 |
                  (static_cast<std::uint32_t>(input[input_pos + 2]) & 0x3f) << 6 |
                  (static_cast<std::uint32_t>(input[input_pos + 3]) & 0x3f);
      break;
    default:
      candidate = kReplacementCharacter;
      length = 1;
      break;
  }
  if ((strict && (!ucd::is_assigned(candidate) || is_surrogate(candidate))) || !is_in_range(candidate)) {
    candidate = kReplacementCharacter;
  }
  if (move_forward) {
    input_pos += length;
  }
  return candidate;
}

}  // namespace unicode
}  // namespace starlark

