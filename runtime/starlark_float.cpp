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

starlark_obj* starlark_float::unary_plus(Arena& arena, error_fn* error_callback) const {
  return const_cast<starlark_float*>(this);
}

starlark_obj* starlark_float::unary_minus(Arena& arena, error_fn* error_callback) const {
  // TODO(lmirelmann): Figure out whether it is possible to reuse `this`.
  return Arena::Create<starlark_float>(&arena, -value);
}

starlark_obj* starlark_float::binary_plus(const starlark_obj& other, Arena& arena, error_fn* error_callback) const {
  if (other.type() != "int" && other.type() != type()) {
    return starlark_obj::binary_plus(other, arena, error_callback);
  }
  auto* n_other = static_cast<const starlark_numeric*>(&other);
  if (n_other->numeric_type() == starlark_numeric_type::kFloat) {
    return Arena::Create<starlark_float>(&arena, value + n_other->as_float());
  } else if (n_other->numeric_type() == starlark_numeric_type::kInt64) {
    return Arena::Create<starlark_float>(&arena, value + n_other->as_int64());
  } else if (n_other->numeric_type() == starlark_numeric_type::kBigInt) {
    auto fother = to_double(n_other->as_bigint());
    if (std::isinf(fother)) {
      if (error_callback != nullptr) {
        error_callback->add_error("OverflowError: int too large to convert to float");
      }
      return nullptr;
    }
    return Arena::Create<starlark_float>(&arena, value + fother);
  } else {
    // Should not happen.
    assert(false);
    if (error_callback != nullptr) {
      error_callback->add_error("TypeError: unknown numeric type");
    }
    return nullptr;
  }
}

starlark_obj* starlark_float::binary_minus(const starlark_obj& other, Arena& arena, error_fn* error_callback) const {
  if (other.type() != "int" && other.type() != type()) {
    return starlark_obj::binary_minus(other, arena, error_callback);
  }
  auto* n_other = static_cast<const starlark_numeric*>(&other);
  if (n_other->numeric_type() == starlark_numeric_type::kFloat) {
    return Arena::Create<starlark_float>(&arena, value - n_other->as_float());
  } else if (n_other->numeric_type() == starlark_numeric_type::kInt64) {
    return Arena::Create<starlark_float>(&arena, value - n_other->as_int64());
  } else if (n_other->numeric_type() == starlark_numeric_type::kBigInt) {
    auto fother = to_double(n_other->as_bigint());
    if (std::isinf(fother)) {
      if (error_callback != nullptr) {
        error_callback->add_error("OverflowError: int too large to convert to float");
      }
      return nullptr;
    }
    return Arena::Create<starlark_float>(&arena, value - fother);
  } else {
    // Should not happen.
    assert(false);
    if (error_callback != nullptr) {
      error_callback->add_error("TypeError: unknown numeric type");
    }
    return nullptr;
  }
}

starlark_obj* starlark_float::binary_star(const starlark_obj& other, Arena& arena, error_fn* error_callback) const {
  if (other.type() != "int" && other.type() != type()) {
    return starlark_obj::binary_star(other, arena, error_callback);
  }
  auto* n_other = static_cast<const starlark_numeric*>(&other);
  if (n_other->numeric_type() == starlark_numeric_type::kFloat) {
    return Arena::Create<starlark_float>(&arena, value * n_other->as_float());
  } else if (n_other->numeric_type() == starlark_numeric_type::kInt64) {
    return Arena::Create<starlark_float>(&arena, value * n_other->as_int64());
  } else if (n_other->numeric_type() == starlark_numeric_type::kBigInt) {
    auto fother = to_double(n_other->as_bigint());
    if (std::isinf(fother)) {
      if (error_callback != nullptr) {
        error_callback->add_error("OverflowError: int too large to convert to float");
      }
      return nullptr;
    }
    return Arena::Create<starlark_float>(&arena, value * fother);
  } else {
    // Should not happen.
    assert(false);
    if (error_callback != nullptr) {
      error_callback->add_error("TypeError: unknown numeric type");
    }
    return nullptr;
  }
}

starlark_obj* starlark_float::binary_slash(const starlark_obj& other, Arena& arena, error_fn* error_callback) const {
  if (other.type() != "int" && other.type() != type()) {
    return starlark_obj::binary_slash(other, arena, error_callback);
  }
  auto* n_other = static_cast<const starlark_numeric*>(&other);
  if (n_other->numeric_type() == starlark_numeric_type::kFloat) {
    auto v = n_other->as_float();
    if (v == 0) {
      if (error_callback != nullptr) {
        error_callback->add_error("ZeroDivisionError: division by zero");
      }
      return nullptr;
    }
    return Arena::Create<starlark_float>(&arena, value / v);
  } else if (n_other->numeric_type() == starlark_numeric_type::kInt64) {
    auto v = n_other->as_int64();
    if (v == 0) {
      if (error_callback != nullptr) {
        error_callback->add_error("ZeroDivisionError: division by zero");
      }
      return nullptr;
    }
    return Arena::Create<starlark_float>(&arena, value / v);
  } else if (n_other->numeric_type() == starlark_numeric_type::kBigInt) {
    auto fother = to_double(n_other->as_bigint());
    if (fother == 0) {
      if (error_callback != nullptr) {
        error_callback->add_error("ZeroDivisionError: division by zero");
      }
      return nullptr;
    }
    if (std::isinf(fother)) {
      if (error_callback != nullptr) {
        error_callback->add_error("OverflowError: int too large to convert to float");
      }
      return nullptr;
    }
    return Arena::Create<starlark_float>(&arena, value / fother);
  } else {
    // Should not happen.
    assert(false);
    if (error_callback != nullptr) {
      error_callback->add_error("TypeError: unknown numeric type");
    }
    return nullptr;
  }
}

starlark_obj* starlark_float::binary_slash_slash(const starlark_obj& other, Arena& arena, error_fn* error_callback) const {
  if (other.type() != "int" && other.type() != type()) {
    return starlark_obj::binary_slash_slash(other, arena, error_callback);
  }
  auto* n_other = static_cast<const starlark_numeric*>(&other);
  if (n_other->numeric_type() == starlark_numeric_type::kFloat) {
    auto v = n_other->as_float();
    if (v == 0) {
      if (error_callback != nullptr) {
        error_callback->add_error("ZeroDivisionError: division by zero");
      }
      return nullptr;
    }
    return Arena::Create<starlark_float>(&arena, std::floor(value / v));
  } else if (n_other->numeric_type() == starlark_numeric_type::kInt64) {
    auto v = n_other->as_int64();
    if (v == 0) {
      if (error_callback != nullptr) {
        error_callback->add_error("ZeroDivisionError: division by zero");
      }
      return nullptr;
    }
    return Arena::Create<starlark_float>(&arena, std::floor(value / v));
  } else if (n_other->numeric_type() == starlark_numeric_type::kBigInt) {
    auto fother = to_double(n_other->as_bigint());
    if (fother == 0) {
      if (error_callback != nullptr) {
        error_callback->add_error("ZeroDivisionError: division by zero");
      }
      return nullptr;
    }
    if (std::isinf(fother)) {
      if (error_callback != nullptr) {
        error_callback->add_error("OverflowError: int too large to convert to float");
      }
      return nullptr;
    }
    return Arena::Create<starlark_float>(&arena, std::floor(value / fother));
  } else {
    // Should not happen.
    assert(false);
    if (error_callback != nullptr) {
      error_callback->add_error("TypeError: unknown numeric type");
    }
    return nullptr;
  }
}

starlark_obj* starlark_float::binary_percent(const starlark_obj& other, Arena& arena, error_fn* error_callback) const {
  if (other.type() != "int" && other.type() != type()) {
    return starlark_obj::binary_percent(other, arena, error_callback);
  }
  auto* n_other = static_cast<const starlark_numeric*>(&other);
  if (n_other->numeric_type() == starlark_numeric_type::kFloat) {
    auto v = n_other->as_float();
    if (v == 0) {
      if (error_callback != nullptr) {
        error_callback->add_error("ZeroDivisionError: division by zero");
      }
      return nullptr;
    }
    return Arena::Create<starlark_float>(&arena, starlark_fmod(value, v));
  } else if (n_other->numeric_type() == starlark_numeric_type::kInt64) {
    auto v = n_other->as_int64();
    if (v == 0) {
      if (error_callback != nullptr) {
        error_callback->add_error("ZeroDivisionError: division by zero");
      }
      return nullptr;
    }
    return Arena::Create<starlark_float>(&arena, starlark_fmod(value, v));
  } else if (n_other->numeric_type() == starlark_numeric_type::kBigInt) {
    auto fother = to_double(n_other->as_bigint());
    if (fother == 0) {
      if (error_callback != nullptr) {
        error_callback->add_error("ZeroDivisionError: division by zero");
      }
      return nullptr;
    }
    if (std::isinf(fother)) {
      if (error_callback != nullptr) {
        error_callback->add_error("OverflowError: int too large to convert to float");
      }
      return nullptr;
    }
    return Arena::Create<starlark_float>(&arena, starlark_fmod(value, fother));
  } else {
    // Should not happen.
    assert(false);
    if (error_callback != nullptr) {
      error_callback->add_error("TypeError: unknown numeric type");
    }
    return nullptr;
  }
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


