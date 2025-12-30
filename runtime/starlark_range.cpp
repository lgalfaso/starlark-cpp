// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_range.hpp"

#include <string>

using ::google::protobuf::Arena;

namespace starlark {
namespace runtime {

starlark_range::starlark_range() : start(0), step(1) {}

std::string_view starlark_range::type() const {
  return "range";
}

int64_t starlark_range::len(error_fn& error_callback) const {
  // TODO(lmirelmann): Implement.
  return 0;
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
  error_callback.add_error("Unimplemented");
  return false;
}

starlark_iterator* starlark_range::get_iterator(bool produce_error, Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_range::index(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

}  // namespace runtime
}  // namespace starlark


