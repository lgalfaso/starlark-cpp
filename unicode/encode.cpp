// Copyright 2024-2025 Lucas Mirelmann

#include "unicode/encode.hpp"

#include <string>

#include "unicode/ucd_code_points.hpp"
#include "unicode/utf8_reader.hpp"

namespace starlark {
namespace unicode {

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

