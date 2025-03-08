// Copyright 2025 Lucas Mirelmann

#include "compiler/starlark_string.hpp"

#include <cassert>

#include <string>

#include "compiler/siphash.hpp"
#include "unicode/encode.hpp"
#include "unicode/ucd_code_points.hpp"
#include "unicode/utf8_reader.hpp"

using starlark::unicode::utf8_encode_code_point;
using starlark::unicode::utf8_reader;

namespace starlark {
namespace compiler {

namespace {

// TODO(lmirelmann): Extract the hex encoding to a printer.
static const char hex[] = "0123456789abcdef";

}  // namespace

starlark_string::starlark_string(const std::string& value) : value(value) {}

std::string_view starlark_string::type() const {
  return "string";
}

std::string starlark_string::str() const {
  return value;
}

bool starlark_string::inner_repr(printer& print, printer_action action) const {
  assert(action == printer_action::print_top);
  // TODO(lmirelmann): Append directly to the printer.
  // TODO(lmirelmann): If this function were to be executed a lot and were to become
  // a performance issue, then there are a few things that can be optimized:
  // - The check for `use_single_quote` can be done in one pass
  // - It should be possible to check whether the original value can be used just adding quotes
  std::string result;
  bool use_single_quote = !value.contains('\'') || value.contains('"');
  if (use_single_quote) {
    result += "'";
  } else {
    result += "\"";
  }
  utf8_reader reader(value, false);
  while (reader.pending()) {
    uint32_t codepoint = reader.peek_code_point();
    reader.skip_code_point();
    if (codepoint == '\\' || (use_single_quote && codepoint == '\'')) {
      result += '\\';
      utf8_encode_code_point(codepoint, result, false);
    } else if (codepoint == '\t') {
      result += "\\t";
    } else if (codepoint == '\n') {
      result += "\\n";
    } else if (codepoint == '\r') {
      result += "\\r";
    } else if (codepoint < 0x20 || codepoint == 0x7f) {
      result += "\\x";
      result += hex[codepoint >> 4];
      result += hex[codepoint & 0xf];
    } else if (codepoint <= 0x7f) {
      utf8_encode_code_point(codepoint, result, false);
    } else if (starlark::ucd::is_printable(codepoint)) {
      utf8_encode_code_point(codepoint, result, false);
    } else if (codepoint <= 0xff) {
      result += "\\x";
      result += hex[codepoint >> 4];
      result += hex[codepoint & 0xf];
    } else if (codepoint <= 0xffff) {
      result += "\\u";
      result += hex[codepoint >> 12];
      result += hex[(codepoint >> 8) & 0xf];
      result += hex[(codepoint >> 4) & 0xf];
      result += hex[codepoint & 0xf];
    } else {
      result += "\\U";
      result += hex[codepoint >> 28];
      result += hex[(codepoint >> 24) & 0xf];
      result += hex[(codepoint >> 20) & 0xf];
      result += hex[(codepoint >> 16) & 0xf];
      result += hex[(codepoint >> 12) & 0xf];
      result += hex[(codepoint >> 8) & 0xf];
      result += hex[(codepoint >> 4) & 0xf];
      result += hex[codepoint & 0xf];
    }
  }
  if (use_single_quote) {
    result += "'";
  } else {
    result += "\"";
  }
  print.append(result);
  return false;
}

bool starlark_string::truthy() const {
  return !value.empty();
}

bool starlark_string::equals(const starlark_obj& other) const {
  return type() == other.type() &&
         value == other.str();
}

int64_t starlark_string::hash() const {
  if (value.length() == 0) {
    return 0;
  }
  return siphash(value.data(), value.length(), 0x0001020304050607, 0x08090a0b0c0d0e0f);
}

}  // namespace compiler
}  // namespace starlark


