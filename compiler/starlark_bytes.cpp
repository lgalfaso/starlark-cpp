// Copyright 2025 Lucas Mirelmann

#include "compiler/starlark_bytes.hpp"

#include <string>

#include "compiler/siphash.hpp"

namespace starlark {
namespace compiler {

namespace {

// TODO(lmirelmann): Move the hex encoding to a printer.
static const char hex[] = "0123456789abcdef";

}  // namespace

const std::string starlark_bytes::type_value = "bytes";

starlark_bytes::starlark_bytes(const std::string& value) : value(value) {}

const std::string& starlark_bytes::type() const {
  return type_value;
}

bool starlark_bytes::inner_repr(printer& print, uint64_t pos) const {
  // TODO(lmirelmann): Append directly to the printer.
  std::string result = "b";
  bool use_single_quote = !value.contains('\'') || value.contains('"');
  if (use_single_quote) {
    result += "'";
  } else {
    result += "\"";
  }
  for (unsigned char c : value) {
    switch (c) {
      case '\t':
        result += "\\t"; break;
      case '\n':
        result += "\\n"; break;
      case '\r':
        result += "\\r"; break;
      case '\'':
        if (use_single_quote) {
          result += "\\";
        }
        result += "'"; break;
      case '\\':
        result += "\\\\"; break;
      default:
        if (0x20 <= c && c <= 0x7e) {
          result += c;
        } else {
          result += "\\x";
          result += hex[c >> 4];
          result += hex[c & 0xf];
        }
        break;
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

bool starlark_bytes::truthy() const {
  return !value.empty();
}

bool starlark_bytes::equals(const starlark_obj& other) const {
  return &type() == &other.type() &&
      value == (static_cast<const starlark_bytes*>(&other))->value;
}

int64_t starlark_bytes::hash() const {
  if (value.length() == 0) {
    return 0;
  }
  return siphash(value.data(), value.length(), 0x0001020304050607, 0x08090a0b0c0d0e0f);
}

}  // namespace compiler
}  // namespace starlark


