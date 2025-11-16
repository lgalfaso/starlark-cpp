// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_float.hpp"

#include <cassert>

#include <bit>
#include <format>
#include <limits>
#include <string>

#include "runtime/hash.hpp"
#include "runtime/starlark_numeric.hpp"

using ::google::protobuf::Arena;

namespace starlark {
namespace runtime {

starlark_float::starlark_float(double value) : value(value) {}

std::string_view starlark_float::type() const {
  return "float";
}

starlark_obj* starlark_float::unary_plus(google::protobuf::Arena& arena, error_fn* error_callback) {
  return this;
}

starlark_obj* starlark_float::unary_minus(google::protobuf::Arena& arena, error_fn* error_callback) {
  // TODO(lmirelmann): Figure out whether it is possible to reuse `this`.
  return Arena::Create<starlark_float>(&arena, -value);
}

bool starlark_float::inner_repr(printer& print, printer_action action) const {
  assert(action == printer_action::kPrintTop);
  // This tries to follow the same format as Python.
  auto result = std::format("{:.17g}", value);
  if (std::isfinite(value)) {
    // The formatting using %g is not the same as the one used by Python.
    // The following modifications need to be performed after the intial formatting:
    // - If the value is represented as an integer in non-scientific notation,
    //   then it has to have one a period and at least one decimal number after the period.
    // - If after the previous correction, there are more than 17 digit,
    //   then convert to scientific notation
    int start = 0;
    if (result[start] == '-' || result[start] == '+') {
        ++start;
    }
    int count = 0;
    while (start + count < result.size() && std::isdigit(result[start + count])) {
      count++;
    }
    if (start + count == result.size()) {
      // If the output is an integer.
      if (count == 17) {
        // If the number of significant digits is already the max, convert to scientific notation.
        result.erase(result.find_last_not_of('0') + 1);
        if (result.size() > start + 1) {
          result = result.substr(0, start + 1) + "." + result.substr(start + 1, result.size() - start - 1);
        }
        result += std::format("e{:+2d}", count - 1);
      } else {
        // If not, make sure that the representation is clear that this is a float.
        result += ".0";
      }
    }
  }
  print.append(result);
  return false;
}

bool starlark_float::truthy() const {
  return value != 0.0;
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_float::inner_hash() const {
  if (!std::isfinite(value)) {
    if (!std::isinf(value)) {
      return 0x10411c89;
    }
    return value < 0 ? -0x4cb2f : 0x4cb2f;
  }
  double inner_value = value;
  if (value < 0) {
    inner_value = -inner_value;
  }
  int e;
  double norm = std::frexp(inner_value, &e);
  double integral = std::ldexp(norm, std::numeric_limits<double>::digits);
  e -= std::numeric_limits<double>::digits;
  int64_t mantissa = static_cast<int64_t>(integral);
  {
     int countr = std::countr_zero<uint64_t>(mantissa);
     mantissa >>= countr;
     e += countr;
  }
  e = e >= 0 ? e % hash_size : hash_size-1-((-1-e) % hash_size);
  mantissa = ((mantissa << e) | (mantissa >> (hash_size - e))) & hash_mask;

  if (value < 0) {
    mantissa = -mantissa;
  }
  if (mantissa == -1) {
    mantissa = -2;
  }
  return mantissa;
}

starlark_numeric_type starlark_float::numeric_type() const {
  return starlark_numeric_type::kFloat;
}

double starlark_float::as_float() const {
  return value;
}

}  // namespace runtime
}  // namespace starlark


