// Copyright 2025 Lucas Mirelmann

#include "compiler/starlark_none.hpp"

#include <string>

namespace starlark {
namespace compiler {

const std::string starlark_none::type_value = "NoneType";

const std::string& starlark_none::type() const {
  return type_value;
}

std::string starlark_none::repr() const {
  return "None";
}

bool starlark_none::truthy() const {
  return false;
}

bool starlark_none::equals(const starlark_obj& other) const {
  // Comparing pointers should be faster than comparing the string value.
  return &type() == &other.type();
}

int64_t starlark_none::hash() const {
  return 0xfca86420;
}

}  // namespace compiler
}  // namespace starlark


