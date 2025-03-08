// Copyright 2025 Lucas Mirelmann

#include "compiler/starlark_range.hpp"

#include <string>

namespace starlark {
namespace compiler {

const std::string starlark_range::type_value = "range";

const std::string& starlark_range::type() const {
  return type_value;
}

bool starlark_range::inner_repr(printer& print, uint64_t pos) const {
  // TODO(lmirelmann): Implement.
  return false;
}

bool starlark_range::truthy() const {
  // TODO(lmirelmann): Implement.
  return false;
}

bool starlark_range::equals(const starlark_obj& other) const {
  // TODO(lmirelmann): Implement.
  return false;
}

int64_t starlark_range::hash() const {
  // TODO(lmirelmann): Implement.
  return -1;
}

}  // namespace compiler
}  // namespace starlark


