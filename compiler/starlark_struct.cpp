// Copyright 2025 Lucas Mirelmann

#include "compiler/starlark_struct.hpp"

#include <string>

namespace starlark {
namespace compiler {

std::string_view starlark_struct::type() const {
  return "struct";
}

bool starlark_struct::truthy() const {
  return true;
}

bool starlark_struct::equals(const starlark_obj& other) const {
  // TODO(lmirelmann): Implement.
  return false;
}

int64_t starlark_struct::hash() const {
  // TODO(lmirelmann): Implement.
  return -1;
}

bool starlark_struct::inner_repr(printer& print, printer_action action) const {
  // TODO(lmirelmann): Implement.
  return false;
}

}  // namespace compiler
}  // namespace starlark


