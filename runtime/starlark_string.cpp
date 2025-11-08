// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_string.hpp"

#include <cassert>

#include <string>

#include "runtime/siphash.hpp"
#include "runtime/hex_encoder.hpp"
#include "unicode/utf8_reader.hpp"

using starlark::unicode::utf8_reader;

namespace starlark {
namespace runtime {

starlark_string::starlark_string(std::string_view value) : value(value) {}

std::string_view starlark_string::type() const {
  return "string";
}

std::string starlark_string::str() const {
  return value;
}

bool starlark_string::inner_repr(printer& print, printer_action action) const {
  assert(action == printer_action::kPrintTop);
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
    write_printable(reader.peek_code_point(), use_single_quote, /*allow_non_ascii_printable=*/ true, result);
    reader.skip_code_point();
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

bool starlark_string::inner_equals(comparator& comp, const starlark_obj* other) const {
  return type() == other->type() &&
         value == other->str();
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_string::inner_hash() const {
  if (value.length() == 0) {
    return 0;
  }
  return static_cast<int64_t>(siphash(value.data(), value.length(), 0x0001020304050607, 0x08090a0b0c0d0e0f));
}

}  // namespace runtime
}  // namespace starlark


