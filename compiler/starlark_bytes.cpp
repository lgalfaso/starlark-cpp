// Copyright 2025 Lucas Mirelmann

#include "compiler/starlark_bytes.hpp"

#include <cassert>

#include <string>

#include "compiler/hex_encoder.hpp"
#include "compiler/siphash.hpp"

namespace starlark {
namespace compiler {

starlark_bytes::starlark_bytes(const std::string& value) : value(value) {}

std::string_view starlark_bytes::type() const {
  return "bytes";
}

bool starlark_bytes::inner_repr(printer& print, printer_action action) const {
  assert(action == printer_action::print_top);
  std::string result = "b";
  bool use_single_quote = !value.contains('\'') || value.contains('"');
  if (use_single_quote) {
    result += "'";
  } else {
    result += "\"";
  }
  for (unsigned char c : value) {
    write_printable(c, use_single_quote, /*allow_non_ascii_printable=*/ false, result);
  }
  if (use_single_quote) {
    result += "'";
  } else {
    result += "\"";
  }
  print.append(result);
  return false;
}

bool starlark_bytes::truthy() const {
  return !value.empty();
}

bool starlark_bytes::inner_equals(comparator& comp, const starlark_obj* other) const {
  return type() == other->type() &&
      value == (static_cast<const starlark_bytes*>(other))->value;
}

int64_t starlark_bytes::hash() const {
  if (value.length() == 0) {
    return 0;
  }
  return siphash(value.data(), value.length(), 0x0001020304050607, 0x08090a0b0c0d0e0f);
}

}  // namespace compiler
}  // namespace starlark


