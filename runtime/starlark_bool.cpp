// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_bool.hpp"

#include <cassert>

#include <string>

#include "runtime/starlark_types.hpp"

namespace starlark {
namespace runtime {

starlark_bool::starlark_bool(bool value) : starlark_obj(object_kind::kBool), value(value) {}

bool starlark_bool::primitive() const {
  return true;
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

void starlark_bool::inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, bool extended, error_fn& error_callback) const {
  if (!same_starlark_type(other->kind(), kind())) {
    starlark_obj::inner_cmp(comp, other, op, extended, error_callback);
    return;
  }
  bool other_truth = other->truthy();
  if (value != other_truth) {
    comp.add_task(value ? order_comparator::pending_task_type::kGreaterThan : order_comparator::pending_task_type::kLessThan);
  }
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_bool::inner_hash() const {
  return value ? 1 : 0;
}

}  // namespace runtime
}  // namespace starlark

