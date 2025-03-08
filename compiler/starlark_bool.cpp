// Copyright 2025 Lucas Mirelmann

#include "compiler/starlark_bool.hpp"

#include <string>

namespace starlark {
namespace compiler {

const std::string starlark_bool::type_value = "bool";

starlark_bool::starlark_bool(bool value) : value(value) {}

const std::string& starlark_bool::type() const {
  return type_value;
}

bool starlark_bool::inner_repr(printer& print, uint64_t pos) const {
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

bool starlark_bool::equals(const starlark_obj& other) const {
  return &type() == &other.type() &&
    value == other.truthy();
}

int64_t starlark_bool::hash() const {
  return value ? 1 : 0;
}

}  // namespace compiler
}  // namespace starlark


