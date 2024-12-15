// Copyright 2024 Lucas Mirelmann

#include "unicode/encode.hpp"

namespace unicode {

void utf8_encode_code_point(std::uint64_t character, std::string& output) {
  // TODO(lmirelmann): Should we check that the code point is assigned?
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


