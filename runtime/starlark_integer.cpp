// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_integer.hpp"

#include <cassert>

#include <bit>
#include <format>
#include <string>

#include "runtime/hash.hpp"
#include "runtime/starlark_bigint.hpp"
#include "runtime/starlark_numeric.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;

namespace starlark {
namespace runtime {

starlark_integer::starlark_integer(int64_t value) : value(value) {}

std::string_view starlark_integer::type() const {
  return "int";
}

bool starlark_integer::inner_repr(printer& print, printer_action action) const {
  assert(action == printer_action::kPrintTop);
  print.append(std::to_string(value));
  return false;
}

bool starlark_integer::truthy() const {
  return value != 0;
}

starlark_obj* starlark_integer::unary_plus(google::protobuf::Arena& arena, error_fn* error_callback) {
  return this;
}

starlark_obj* starlark_integer::unary_minus(google::protobuf::Arena& arena, error_fn* error_callback) {
  if (value == std::numeric_limits<int64_t>::min()) {
    // Need to upgrade to bigint.
    return Arena::Create<starlark_bigint>(&arena, number(static_cast<uint64_t>(value)));
  }
  // TODO(lmirelmann): Figure out whether it is possible to reuse `this`.
  return Arena::Create<starlark_integer>(&arena, -value);
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_integer::inner_hash() const {
  // This implementation makes the assumption that the right shifting of
  // negative numbers is not sign extended, and that the representation of
  // integers is two's coplement. The point in the code that makes
  // this assumption is not trivial, and it is based on the fact that
  // `std::numeric_limits<int64_t>::min() == -std::numeric_limits<int64_t>::min()`.
  // This is, that the minimum representable integer is its own negative as
  // the corresponding positive number does not have a representation.
  //
  // If this were to be compiled in an architecture where right shift works
  // differently, then it is possible to have an alternative (slower) implementation
  // that just makes the assumption that integers are represented in two's complement
  // as the following:
  /*
   * int64_t result = value;
   * bool sign = result < 0;
   * if (sign) {
   *   result = ~result;
   * }
   * result = (result & hash_mask) + (result >> hash_size);
   * if (sign) {
   *   result++;
   * }
   * if (result >= hash_mask) {
   *   result -= hash_mask;
   * }
   * if (sign) {
   *   result = -result;
   * }
   * if (result == -1) {
   *   result = -2;
   * }
   * return result;
  */
  uint64_t result = value;
  bool sign = value < 0;
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
  return static_cast<int64_t>(result);
}

starlark_numeric_type starlark_integer::numeric_type() const {
  return starlark_numeric_type::kInt64;
}

int64_t starlark_integer::as_int64() const {
  return value;
}

}  // namespace runtime
}  // namespace starlark


