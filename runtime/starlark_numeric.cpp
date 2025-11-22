// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_numeric.hpp"

#include <cmath>

#include <bit>
#include <limits>
#include <string>

namespace starlark {
namespace runtime {

namespace {

bool equals_fb(double lhs, const starlark::bigint::number& rhs) {
  if (!std::isfinite(lhs)) {
    return false;
  }
  if (rhs == starlark::bigint::number::zero) {
    return lhs == 0;
  }
  if (lhs == 0) {
    return false;
  }
  if ((lhs < 0) ^ (rhs.sign())) {
    return false;
  }
  if (lhs < 0) {
    lhs = -lhs;
  }
  int e;
  double norm = std::frexp(lhs, &e);
  double integral = std::ldexp(norm, std::numeric_limits<double>::digits);
  e -= std::numeric_limits<double>::digits;
  int64_t mantissa = static_cast<int64_t>(integral);
  {
     int countr = std::countr_zero<uint64_t>(mantissa);
     mantissa >>= countr;
     e += countr;
  }
  if (e < 0) {
    return false;
  }
  if (rhs.countr_zero() != e ||
      rhs.bit_size() != e + 64 - std::countl_zero<uint64_t>(mantissa)) {
    return false;
  }
  return mantissa == rhs.bits(e, 64);
}

bool equals_ib(int64_t lhs, const starlark::bigint::number& rhs) {
  if (rhs == starlark::bigint::number::zero) {
    return lhs == 0;
  }
  if (lhs == 0) {
    return false;
  }
  if ((lhs < 0) ^ (rhs.sign())) {
    return false;
  }
  if (rhs.length() != 1) {
    return false;
  }
  if (lhs < 0) {
    // This only works if the representation is two's complement, as other
    // representations might do the wrong thing for `std::numeric_limits<int64_t>::min()`.
    lhs = -lhs;
  }
  return rhs.at(0) == lhs;
}

}  // namespace

starlark::bigint::number from_int64(int64_t value) {
  starlark::bigint::number result(value);
  if (value < 0) {
    result -= starlark::bigint::number::one << 64;
  }
  return result;
}

bool starlark_numeric::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
  if (other->type() != "float" && other->type() != "int") {
    return false;
  }
  const starlark_numeric& n_other = *static_cast<const starlark_numeric*>(other);
  if (numeric_type() == starlark_numeric_type::kFloat) {
    if (n_other.numeric_type() == starlark_numeric_type::kFloat) {
      if (std::isnan(as_float()) && std::isnan(n_other.as_float())) {
        return true;
      }
      return as_float() == n_other.as_float();
    } else if (n_other.numeric_type() == starlark_numeric_type::kInt64) {
      return as_float() == n_other.as_int64();
    } else {
      return equals_fb(as_float(), n_other.as_bigint());
    }
  } else if (numeric_type() == starlark_numeric_type::kInt64) {
    if (n_other.numeric_type() == starlark_numeric_type::kFloat) {
      return as_int64() == n_other.as_float();
    } else if (n_other.numeric_type() == starlark_numeric_type::kInt64) {
      return as_int64() == n_other.as_int64();
    } else {
      return equals_ib(as_int64(), n_other.as_bigint());
    }
  } else {
    if (n_other.numeric_type() == starlark_numeric_type::kFloat) {
      return equals_fb(n_other.as_float(), as_bigint());
    } else if (n_other.numeric_type() == starlark_numeric_type::kInt64) {
      return equals_ib(n_other.as_int64(), as_bigint());
    } else {
      return as_bigint() == n_other.as_bigint();
    }
  }
}

int64_t starlark_numeric::as_int64() const {
  return 0;
}

const starlark::bigint::number& starlark_numeric::as_bigint() const {
  return starlark::bigint::number::zero;
}

double starlark_numeric::as_float() const {
  return 0;
}

}  // namespace runtime
}  // namespace starlark


