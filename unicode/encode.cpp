// Copyright 2024-2025 Lucas Mirelmann

#include "unicode/encode.hpp"

#include <string>

#include "unicode/ucd_code_points.hpp"
#include "unicode/utf8_reader.hpp"

namespace starlark {
namespace unicode {

void utf8_encode_code_point(std::uint32_t character, std::string& output, bool strict) {
  if ((strict && !ucd::is_assigned(character)) || !is_in_range(character)) {
    character = utf8_reader::replacement_character;
  }
  if (character <= 0x7f) {
    output += static_cast<char>(character);
  } else if (character <= 0x7ff) {
    output += ('\xc0' | static_cast<char>(character >> 6));
    output += ('\x80' | static_cast<char>(character & 0x3f));
  } else if (is_surrogate(character)) {
    // No-op. Characters in this block will not be encoded.
  } else if (character <= 0xffff) {
    output += ('\xe0' | static_cast<char>(character >> 12));
    output += ('\x80' | static_cast<char>((character >> 6) & 0x3f));
    output += ('\x80' | static_cast<char>(character & 0x3f));
  } else if (character <= 0x10'ffff) {
    output += ('\xf0' | static_cast<char>(character >> 18));
    output += ('\x80' | static_cast<char>((character >> 12) & 0x3f));
    output += ('\x80' | static_cast<char>((character >> 6) & 0x3f));
    output += ('\x80' | static_cast<char>(character & 0x3f));
  }
  return;
}

}  // namespace unicode
}  // namespace starlark


