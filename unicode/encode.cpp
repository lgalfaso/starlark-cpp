// Copyright 2024 Lucas Mirelmann

#include "unicode/encode.hpp"

namespace ucd {

std::string utf8_encode_code_point(std::uint64_t character) {
  std::string result;
  if (character <= 0x7f) {
    result += (char)character;
  } else if (character <= 0x7ff) {
    result += ('\xc0' | (char)(character >> 6));
    result += ('\x80' | (char)(character & 0x3f));
  } else if (0xd800 <= character && character <= 0xdfff) {
    // No-op. Characters in this block will not be encoded.
  } else if (character <= 0xffff) {
    result += ('\xe0' | (char)(character >> 12));
    result += ('\x80' | (char)((character >> 6) & 0x3f));
    result += ('\x80' | (char)(character & 0x3f));
  } else if (character <= 0x10'ffff) {
    result += ('\xf0' | (char)(character >> 18));
    result += ('\x80' | (char)((character >> 12) & 0x3f));
    result += ('\x80' | (char)((character >> 6) & 0x3f));
    result += ('\x80' | (char)(character & 0x3f));
  }
  return result;
}

}  // namespace ucd


