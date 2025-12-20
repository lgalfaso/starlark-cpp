// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_range.hpp"

#include <string>

namespace starlark {
namespace runtime {

std::string_view starlark_range::type() const {
  return "range";
}

bool starlark_range::inner_repr(printer& print, printer_action action) const {
  // TODO(lmirelmann): Implement.
  return false;
}

bool starlark_range::truthy() const {
  // TODO(lmirelmann): Implement.
  return false;
}

bool starlark_range::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
  // TODO(lmirelmann): Implement.
  return false;
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_range::inner_hash() const {
  // TODO(lmirelmann): Implement.
  return -1;
}

void starlark_range::set_start(const starlark_obj* value) {
  // TODO(lmirelmann): Implement.
}

void starlark_range::set_end(const starlark_obj* value) {
  // TODO(lmirelmann): Implement.
}

void starlark_range::set_step(const starlark_obj* value) {
  // TODO(lmirelmann): Implement.
}

bool starlark_range::binary_in(const starlark_obj& other, error_fn& error_callback) const {
  // TODO(lmirelmann): Implement.
  return false;
}

}  // namespace runtime
}  // namespace starlark


