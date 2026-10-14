// Copyright 2024-2025 Lucas Mirelmann

#include "unicode/encode.hpp"

#include <string>
#include <string_view>
#include <vector>

#include "unicode/ucd_code_points.hpp"
#include "unicode/utf8_reader.hpp"

namespace starlark {
namespace unicode {

void utf8_encode(const std::vector<char32_t>& code_points, std::string& output) {
  for (char32_t code_point : code_points) {
    utf8_encode_code_point(code_point, output, false, true);
  }
}

bool utf8_decode(std::string_view input, std::vector<char32_t>& output) {
  output.clear();
  utf8_reader reader(input, false, false);
  while (reader.pending()) {
    auto start = reader.pos();
    auto code_point = reader.read_code_point();
    if (code_point == utf8_reader::kReplacementCharacter &&
        input.substr(start, reader.pos() - start) != replacement_character_utf8()) {
      return false;
    }
    output.push_back(code_point);
  }
  return true;
}

bool utf8_is_well_formed(std::string_view input) {
  std::vector<char32_t> ignored;
  return utf8_decode(input, ignored);
}

std::size_t utf8_encoded_size(char32_t code_point) {
  if (code_point <= 0x7f) {
    return 1;
  }
  if (code_point <= 0x7ff) {
    return 2;
  }
  if (code_point <= 0xffff) {
    return 3;
  }
  return 4;
}

std::size_t utf8_encoded_size(const std::vector<char32_t>& code_points) {
  std::size_t size = 0;
  for (char32_t code_point : code_points) {
    size += utf8_encoded_size(code_point);
  }
  return size;
}

void utf8_encode_code_point(char32_t code_point, std::string& output, bool strict, bool encode_surrogate) {
  if ((strict && !ucd::is_assigned(code_point)) || !is_in_range(code_point)) {
    code_point = utf8_reader::kReplacementCharacter;
  }
  if (code_point <= 0x7f) {
    output += static_cast<char>(code_point);
  } else if (code_point <= 0x7ff) {
    output += ('\xc0' | static_cast<char>(code_point >> 6));
    output += ('\x80' | static_cast<char>(code_point & 0x3f));
  } else if (!encode_surrogate && is_surrogate(code_point)) {
    // No-op. Code points in this block will not be encoded.
  } else if (code_point <= 0xffff) {
    output += ('\xe0' | static_cast<char>(code_point >> 12));
    output += ('\x80' | static_cast<char>((code_point >> 6) & 0x3f));
    output += ('\x80' | static_cast<char>(code_point & 0x3f));
  } else if (code_point <= 0x10'ffff) {
    output += ('\xf0' | static_cast<char>(code_point >> 18));
    output += ('\x80' | static_cast<char>((code_point >> 12) & 0x3f));
    output += ('\x80' | static_cast<char>((code_point >> 6) & 0x3f));
    output += ('\x80' | static_cast<char>(code_point & 0x3f));
  }
  return;
}

}  // namespace unicode
}  // namespace starlark

