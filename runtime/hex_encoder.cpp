// Copyright 2025 Lucas Mirelmann

#include "runtime/hex_encoder.hpp"

#include <string>

#include "unicode/encode.hpp"
#include "unicode/ucd_code_points.hpp"

using ::starlark::ucd::is_printable;
using ::starlark::unicode::utf8_encode_code_point;

namespace starlark {
namespace runtime {

namespace {

static const char hex[] = "0123456789abcdef";

}  // namespace

void write_printable(uint64_t codepoint, bool allow_non_ascii_printable, std::string& output) {
  if (codepoint == '\\' || codepoint == '"') {
    output += '\\';
    utf8_encode_code_point(codepoint, output, false, false);
  } else if (codepoint == '\t') {
    output += "\\t";
  } else if (codepoint == '\n') {
    output += "\\n";
  } else if (codepoint == '\r') {
    output += "\\r";
  } else if (codepoint < 0x20 || codepoint == 0x7f) {
    output += "\\x";
    output += hex[(codepoint >> 4) & 0xf];
    output += hex[codepoint & 0xf];
  } else if (codepoint <= 0x7f) {
    utf8_encode_code_point(codepoint, output, false, false);
  } else if (allow_non_ascii_printable && is_printable(codepoint)) {
    utf8_encode_code_point(codepoint, output, false, false);
  } else if (codepoint <= 0xff) {
    output += "\\x";
    output += hex[(codepoint >> 4) & 0xf];
    output += hex[codepoint & 0xf];
  } else if (codepoint <= 0xffff) {
    output += "\\u";
    output += hex[(codepoint >> 12) & 0xf];
    output += hex[(codepoint >> 8) & 0xf];
    output += hex[(codepoint >> 4) & 0xf];
    output += hex[codepoint & 0xf];
  } else {
    output += "\\U";
    output += hex[(codepoint >> 28) & 0xf];
    output += hex[(codepoint >> 24) & 0xf];
    output += hex[(codepoint >> 20) & 0xf];
    output += hex[(codepoint >> 16) & 0xf];
    output += hex[(codepoint >> 12) & 0xf];
    output += hex[(codepoint >> 8) & 0xf];
    output += hex[(codepoint >> 4) & 0xf];
    output += hex[codepoint & 0xf];
  }
}

}  // namespace runtime
}  // namespace starlark

