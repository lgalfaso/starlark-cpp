// Copyright 2025 Lucas Mirelmann

#include "compiler/starlark_bigint.hpp"

#include <cassert>

#include <string>

#include "compiler/hash.hpp"
#include "compiler/starlark_numeric.hpp"

namespace starlark {
namespace compiler {

starlark_bigint::starlark_bigint(int64_t value) : value(from_int64(value)) {}

starlark_bigint::starlark_bigint(const starlark::bigint::number& value) : value(value) {}

std::string_view starlark_bigint::type() const {
  return "int";
}

bool starlark_bigint::inner_repr(printer& print, printer_action action) const {
  assert(action == printer_action::print_top);
  print.append(value.to_string(10));
  return false;
}

bool starlark_bigint::truthy() const {
  return value != bigint::number::zero;
}

int64_t starlark_bigint::hash() const {
  int64_t result = 0;
  for (int i = value.length() - 1; i >= 0; --i) {
    auto y = value.at(i);
    y = (y & hash_mask) + (y >> hash_size);
    // Make `result = (result * 2**64) % hash_mask`.
    // This is using the fact that `hash(1<<61) == 1`, and that `0 <= result < hash_mask`.
    // Say that `result == a * 2**58 + b` where `0 <= a <= 7` and `0 <= b < 2**58`.
    //   result * 2**64 (mod hash_mask) == result * 2**3 *2**61 (mod hash_mask) ==
    //   == result * 2**3 (mod hash_mask) == (a * 2**58 + b) * 2**3 (mod hash_mask) ==
    //   == a * 2**61 + b * 2**3 (mod hash_mask) == a + b * 2**3 (mod hash_mask)
    result = ((result << 3) | (result >> (hash_size - 3))) & hash_mask;
    result += y;
    while (result >= hash_mask) {
      result -= hash_mask;
    }
  }
  if (value.sign()) {
    result = -result;
  }
  if (result == -1) {
    result = -2;
  }
  return result;
}

int starlark_bigint::numeric_type() const {
  return type_bigint;
}

const starlark::bigint::number& starlark_bigint::as_bigint() const {
  return value;
}

}  // namespace compiler
}  // namespace starlark


