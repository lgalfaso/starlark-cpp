// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_struct.hpp"

#include <string>

namespace starlark {
namespace runtime {

std::string_view starlark_struct::type() const {
  return "struct";
}

bool starlark_struct::truthy() const {
  return true;
}

bool starlark_struct::inner_equals(comparator& comp, const starlark_obj* other) const {
  // TODO(lmirelmann): Implement.
  return false;
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_struct::inner_hash() const {
  // TODO(lmirelmann): Implement.
  return -1;
}

bool starlark_struct::inner_repr(printer& print, printer_action action) const {
  // TODO(lmirelmann): Implement.
  return false;
}

}  // namespace runtime
}  // namespace starlark


