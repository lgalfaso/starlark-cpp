// Copyright 2024 Lucas Mirelmann

#include "unicode/encode.hpp"

#include "unicode/ucd_code_points.hpp"
#include "unicode/utf8_reader.hpp"

namespace unicode {

void utf8_encode_code_point(std::uint32_t character, std::string& output) {
  // TODO(lmirelmann): This constraint might be too hard, and we should only check
  // that this is not a surrogate, and it is within the Unicode range.
  if (!ucd::is_assigned(character)) {
    character = utf8_reader::replacement_character;
  }
  if (character <= 0x7f) {
    output += (char)character;
  } else if (character <= 0x7ff) {
    output += ('\xc0' | (char)(character >> 6));
    output += ('\x80' | (char)(character & 0x3f));
  } else if (0xd800 <= character && character <= 0xdfff) {
    // No-op. Characters in this block will not be encoded.
  } else if (character <= 0xffff) {
    output += ('\xe0' | (char)(character >> 12));
    output += ('\x80' | (char)((character >> 6) & 0x3f));
    output += ('\x80' | (char)(character & 0x3f));
  } else if (character <= 0x10'ffff) {
    output += ('\xf0' | (char)(character >> 18));
    output += ('\x80' | (char)((character >> 12) & 0x3f));
    output += ('\x80' | (char)((character >> 6) & 0x3f));
    output += ('\x80' | (char)(character & 0x3f));
  }
  return;
}

}  // namespace unicode


