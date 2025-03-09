// Copyright 2025 Lucas Mirelmann

#include "compiler/starlark_range.hpp"

#include <string>

namespace starlark {
namespace compiler {

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

bool starlark_range::inner_equals(comparator& comp, const starlark_obj* other) const {
  // TODO(lmirelmann): Implement.
  return false;
}

int64_t starlark_range::hash() const {
  // TODO(lmirelmann): Implement.
  return -1;
}

}  // namespace compiler
}  // namespace starlark


