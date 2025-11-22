// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_bool.hpp"

#include <cassert>

#include <string>

namespace starlark {
namespace runtime {

starlark_bool::starlark_bool(bool value) : value(value) {}

std::string_view starlark_bool::type() const {
  return "bool";
}

bool starlark_bool::inner_repr(printer& print, printer_action action) const {
  assert(action == printer_action::kPrintTop);
  if (value) {
    print.append("True");
  } else {
    print.append("False");
  }
  return false;
}

bool starlark_bool::truthy() const {
  return value;
}

bool starlark_bool::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
  return type() == other->type() &&
    value == other->truthy();
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_bool::inner_hash() const {
  return value ? 1 : 0;
}

}  // namespace runtime
}  // namespace starlark


