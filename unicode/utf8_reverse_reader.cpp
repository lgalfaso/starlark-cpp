// Copyright 2026 Lucas Mirelmann

#include "unicode/utf8_reverse_reader.hpp"

#include <bit>

#include "unicode/ucd_code_points.hpp"
#include "unicode/utf8_reader.hpp"

using std::string_view;

namespace starlark {
namespace unicode {

utf8_reverse_reader::utf8_reverse_reader(string_view input, bool strict) : input(input), input_pos(input.size()), strict(strict) {}

std::size_t utf8_reverse_reader::pos() const {
  return input_pos;
}

std::size_t utf8_reverse_reader::pending() const {
  return input_pos;
}

char32_t utf8_reverse_reader::peek_code_point() {
  return read_code_point(false);
}

char32_t utf8_reverse_reader::read_code_point() {
  return read_code_point(true);
}

char32_t utf8_reverse_reader::read_code_point(bool move_forward) {
  if (input_pos == 0) {
    return utf8_reader::kReplacementCharacter;
  }
  const unsigned char current_char = input[input_pos - 1];
  if (current_char <= 127) {
    if (move_forward) {
      input_pos -= 1;
    }
    return current_char;
  }
  int length = 1;
  while (input_pos > length) {
    if (is_utf8_continue(input[input_pos - length])) {
      length++;
    } else {
      break;
    }
  }
  if (length != std::countl_one<unsigned char>(input[input_pos - length])) {
    if (move_forward) {
      input_pos -= 1;
    }
    return utf8_reader::kReplacementCharacter;
  }
  char32_t candidate;
  switch (length) {
    case 2:
      if (input[input_pos - 2] == '\xc0' || input[input_pos - 2] == '\xc1') {
        candidate = utf8_reader::kReplacementCharacter;
        break;
      }
      candidate = (static_cast<char32_t>(input[input_pos - 2]) & 0x1f) << 6 |
                  (static_cast<char32_t>(input[input_pos - 1]) & 0x3f);
      break;
    case 3:
      if (input[input_pos - 3] == '\xe0' && ((unsigned char)input[input_pos - 2]) < 0xa0) {
        candidate = utf8_reader::kReplacementCharacter;
        break;
      }
      candidate = (static_cast<char32_t>(input[input_pos - 3]) & 0x0f) << 12 |
                  (static_cast<char32_t>(input[input_pos - 2]) & 0x3f) << 6 |
                  (static_cast<char32_t>(input[input_pos - 1]) & 0x3f);
      break;
    case 4:
      if (input[input_pos - 4] == '\xf0' && ((unsigned char)input[input_pos - 3]) < 0x90) {
        candidate = utf8_reader::kReplacementCharacter;
        break;
      }
      if (input[input_pos - 4] == '\xf4' && ((unsigned char)input[input_pos - 3]) >= 0x90) {
        candidate = utf8_reader::kReplacementCharacter;
        break;
      }
      candidate = (static_cast<char32_t>(input[input_pos - 4]) & 0x07) << 18 |
                  (static_cast<char32_t>(input[input_pos - 3]) & 0x3f) << 12 |
                  (static_cast<char32_t>(input[input_pos - 2]) & 0x3f) << 6 |
                  (static_cast<char32_t>(input[input_pos - 1]) & 0x3f);
      break;
    default:
      candidate = utf8_reader::kReplacementCharacter;
      length = 1;
      break;
  }
  if ((strict && (!ucd::is_assigned(candidate) || is_surrogate(candidate))) || !is_in_range(candidate)) {
    candidate = utf8_reader::kReplacementCharacter;
  }
  if (move_forward) {
    input_pos -= length;
  }
  return candidate;
}

}  // namespace unicode
}  // namespace starlark

