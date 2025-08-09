// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_none.hpp"

#include <cassert>

#include <string>

namespace starlark {
namespace runtime {

std::string_view starlark_none::type() const {
  return "NoneType";
}

bool starlark_none::inner_repr(printer& print, printer_action action) const {
  assert(action == printer_action::kPrintTop);
  print.append("None");
  return false;
}

bool starlark_none::truthy() const {
  return false;
}

bool starlark_none::inner_equals(comparator& comp, const starlark_obj* other) const {
  return type() == other->type();
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_none::inner_hash() const {
  return 0xfca86420;
}

}  // namespace runtime
}  // namespace starlark


