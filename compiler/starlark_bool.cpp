// Copyright 2025 Lucas Mirelmann

#include "compiler/starlark_bool.hpp"

#include <cassert>

#include <string>

namespace starlark {
namespace compiler {

starlark_bool::starlark_bool(bool value) : value(value) {}

std::string_view starlark_bool::type() const {
  return "bool";
}

bool starlark_bool::inner_repr(printer& print, printer_action action) const {
  assert(action == printer_action::print_top);
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

bool starlark_bool::inner_equals(comparator& comp, const starlark_obj* other) const {
  return type() == other->type() &&
    value == other->truthy();
}

int64_t starlark_bool::hash() const {
  return value ? 1 : 0;
}

}  // namespace compiler
}  // namespace starlark


