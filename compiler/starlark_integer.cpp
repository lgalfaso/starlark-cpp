// Copyright 2025 Lucas Mirelmann

#include "compiler/starlark_integer.hpp"

#include <bit>
#include <format>
#include <string>

#include "compiler/hash.hpp"
#include "compiler/starlark_numeric.hpp"

namespace starlark {
namespace compiler {

const std::string starlark_integer::type_value = "int";

starlark_integer::starlark_integer(int64_t value) : value(value) {}

const std::string& starlark_integer::type() const {
  return type_value;
}

bool starlark_integer::inner_repr(printer& print, uint64_t pos) const {
  print.append(std::to_string(value));
  return false;
}

bool starlark_integer::truthy() const {
  return value != 0;
}

int64_t starlark_integer::hash() const {
  int64_t result = value;
  bool sign = result < 0;
  if (sign) {
    result = -result;
  }
  result = (result & hash_mask) + (result >> hash_size);
  if (result >= hash_mask) {
    result -= hash_mask;
  }
  if (sign) {
    result = -result;
  }
  if (result == -1) {
    result = -2;
  }
  return result;
}

int starlark_integer::numeric_type() const {
  return type_int64;
}

int64_t starlark_integer::as_int64() const {
  return value;
}

}  // namespace compiler
}  // namespace starlark


