// Copyright 2025 Lucas Mirelmann

#include <limits>
#include <utility>

#include "google/protobuf/arena.h"
#include "errors/runtime_error_messages.hpp"
#include "runtime/starlark_bigint.hpp"
#include "runtime/starlark_float.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_numeric.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;
using ::starlark::error_messages::error_v2_convert_float_infinity_to_integer;
using ::starlark::error_messages::error_v2_convert_float_nan_to_integer;

namespace starlark {
namespace runtime {

starlark_obj* create_integer(std::int64_t value, context& ctx) {
  // TODO(lmirelmann): Use a cache of small integers.
  return Arena::Create<starlark_integer>(&ctx.arena(), value);
}

starlark_obj* create_integer(number&& value, context& ctx) {
  // Check whether we can downgrade.
  if (value.fits_in_int64()) {
    return create_integer(value.as_int64(), ctx);
  }
  return Arena::Create<starlark_bigint>(&ctx.arena(), std::move(value));
}

starlark_obj* create_integer_from_float(double value, context& ctx, error_fn& error_callback) {
  if (!std::isfinite(value)) {
    if (std::isinf(value)) {
      error_callback.add_error(error_v2_convert_float_infinity_to_integer());
    } else {
      error_callback.add_error(error_v2_convert_float_nan_to_integer());
    }
    return nullptr;
  }
  if (value == 0) {
    return ctx.zero();
  }
  bool neg = false;
  if (value < 0) {
    value = -value;
    neg = true;
  }
  int e;
  double norm = std::frexp(value, &e);
  double integral = std::ldexp(norm, std::numeric_limits<double>::digits);
  e -= std::numeric_limits<double>::digits;
  int64_t mantissa = static_cast<int64_t>(integral);
  {
     int countr = std::countr_zero<uint64_t>(mantissa);
     mantissa >>= countr;
     e += countr;
  }
  if (e < 0) {
    if (e < -63) {
      mantissa = 0;
      e = 0;
    } else {
      mantissa >>= (-e);
      e = std::countr_zero<uint64_t>(mantissa);
      mantissa >>= e;
    }
  }
  if (e < std::countl_zero<uint64_t>(mantissa)) {
    // Fits into an int64_t.
    mantissa <<= e;
    if (neg) {
      mantissa = -mantissa;
    }
    return create_integer(mantissa, ctx);
  } else {
    if (neg) {
      mantissa = -mantissa;
    }
    return create_integer(from_int64(mantissa) << e, ctx);
  }
}

starlark_obj* create_float(double value, context& ctx) {
  return Arena::Create<starlark_float>(&ctx.arena(), value);
}

}  // namespace runtime
}  // namespace starlark

