// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_none.hpp"

#include <cassert>

#include <string>

#include "runtime/starlark_types.hpp"

namespace starlark {
namespace runtime {

starlark_none::starlark_none() : starlark_obj(object_kind::kNone) {}

bool starlark_none::primitive() const {
  return true;
}

bool starlark_none::inner_repr(printer& print, printer_action action) const {
  assert(action == printer_action::kPrintTop);
  print.append("None");
  return false;
}

bool starlark_none::truthy() const {
  return false;
}

bool starlark_none::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
  return type() == other->type();
}

void starlark_none::inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, bool extended, error_fn& error_callback) const {
  if (extended && same_starlark_type(kind(), other->kind()) && equals(*other)) {
    return;
  }
  starlark_obj::inner_cmp(comp, other, op, extended, error_callback);
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_none::inner_hash() const {
  return 0xfca86420;
}

}  // namespace runtime
}  // namespace starlark

