// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_integer.hpp"

#include <cassert>

#include <bit>
#include <format>
#include <limits>
#include <string>

#include "grammar/options.hpp"
#include "runtime/hash.hpp"
#include "runtime/starlark_bigint.hpp"
#include "runtime/starlark_float.hpp"
#include "runtime/starlark_numeric.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;
using ::starlark::grammar::log2_max_bigint;

namespace starlark {
namespace runtime {

namespace {

starlark_integer* create_integer(std::int64_t value, Arena& arena) {
  // TODO(lmirelmann): Use a cache of small integers.
  return Arena::Create<starlark_integer>(&arena, value);
}

}  // namespace

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

starlark_obj* starlark_integer::unary_plus(Arena& arena, error_fn* error_callback) const {
  return const_cast<starlark_integer*>(this);
}

starlark_obj* starlark_integer::unary_minus(Arena& arena, error_fn* error_callback) const {
  if (value == std::numeric_limits<int64_t>::min()) {
    // Need to upgrade to bigint.
    return Arena::Create<starlark_bigint>(&arena, number(static_cast<uint64_t>(value)));
  }
  // TODO(lmirelmann): Figure out whether it is possible to reuse `this`.
  return create_integer(-value, arena);
}

starlark_obj* starlark_integer::unary_tilde(Arena& arena, error_fn* error_callback) const {
  // TODO(lmirelmann): Figure out whether it is possible to reuse `this`.
  return create_integer(~value, arena);
}

starlark_obj* starlark_integer::binary_lshift(const starlark_obj& other, Arena& arena, error_fn* error_callback) const {
  if (other.type() != type()) {
    return starlark_obj::binary_lshift(other, arena, error_callback);
  }
  if (value == 0) {
    return const_cast<starlark_integer*>(this);
  }
  const starlark_numeric& n_other = static_cast<const starlark_numeric&>(other);
  if (n_other.numeric_type() == starlark_numeric_type::kInt64) {
    auto shift = n_other.as_int64();
    if (shift < 0) {
      if (error_callback != nullptr) {
        error_callback->add_error("ValueError: negative shift count");
      }
      return nullptr;
    }
    // Check whether it will fit in an int64_t.
    int left_shift_space = (value > 0 ? std::countl_zero<uint64_t>(value) : std::countl_one<uint64_t>(value));
    if (shift < left_shift_space) {
      return create_integer(value << shift, arena);
    } else {
      if (log2_max_bigint() < 64 - std::countl_zero<uint64_t>(shift) + 64 - left_shift_space) {
        if (error_callback != nullptr) {
          error_callback->add_error("OverflowError: too many digits in integer");
        }
        return nullptr;
      }
      return Arena::Create<starlark_bigint>(&arena, from_int64(value) << shift);
    }
  } else if (n_other.numeric_type() == starlark_numeric_type::kBigInt) {
    const auto& shift = n_other.as_bigint();
    if (shift.sign()) {
      if (error_callback != nullptr) {
        error_callback->add_error("ValueError: negative shift count");
      }
      return nullptr;
    }
    if (shift.length() > 1) {
      if (error_callback != nullptr) {
        error_callback->add_error("OverflowError: too many digits in integer");
      }
      return nullptr;
    }
    auto int_shift = shift.at(0);
    int left_shift_space = (value > 0 ? std::countl_zero<uint64_t>(value) : std::countl_one<uint64_t>(value));
    if (int_shift < left_shift_space) {
      return create_integer(value << int_shift, arena);
    } else {
      if (log2_max_bigint() < 64 - std::countl_zero<uint64_t>(int_shift) + 64 - left_shift_space) {
        if (error_callback != nullptr) {
          error_callback->add_error("OverflowError: too many digits in integer");
        }
        return nullptr;
      }
      return Arena::Create<starlark_bigint>(&arena, from_int64(value) << int_shift);
    }
  } else {
    // Should not happen.
    assert(false);
    if (error_callback != nullptr) {
      error_callback->add_error("RuntimeError: unexpected number type");
    }
    return nullptr;
  }
}

starlark_obj* starlark_integer::binary_rshift(const starlark_obj& other, Arena& arena, error_fn* error_callback) const {
  if (other.type() != type()) {
    return starlark_obj::binary_rshift(other, arena, error_callback);
  }
  if (value == 0) {
    return const_cast<starlark_integer*>(this);
  }
  const starlark_numeric& n_other = static_cast<const starlark_numeric&>(other);
  if (n_other.numeric_type() == starlark_numeric_type::kInt64) {
    auto shift = n_other.as_int64();
    if (shift < 0) {
      if (error_callback != nullptr) {
        error_callback->add_error("ValueError: negative shift count");
      }
      return nullptr;
    }
    if (shift >= 64) {
      return create_integer(value >= 0 ? 0 : -1, arena);
    }
    return create_integer(value >> shift, arena);
  } else if (n_other.numeric_type() == starlark_numeric_type::kBigInt) {
    const auto& shift = n_other.as_bigint();
    if (shift.sign()) {
      if (error_callback != nullptr) {
        error_callback->add_error("ValueError: negative shift count");
      }
      return nullptr;
    }
    if (shift.length() > 1) {
      return create_integer(value >= 0 ? 0 : -1, arena);
    }
    auto int_shift = shift.at(0);
    if (int_shift >= 64) {
      return create_integer(value >= 0 ? 0 : -1, arena);
    }
    return create_integer(value >> int_shift, arena);
  } else {
    // Should not happen.
    assert(false);
    if (error_callback != nullptr) {
      error_callback->add_error("RuntimeError: unexpected number type");
    }
    return nullptr;
  }
}

starlark_obj* starlark_integer::binary_and(const starlark_obj& other, Arena& arena, error_fn* error_callback) const {
  if (other.type() != type()) {
    return starlark_obj::binary_and(other, arena, error_callback);
  }
  const starlark_numeric& n_other = static_cast<const starlark_numeric&>(other);
  if (n_other.numeric_type() == starlark_numeric_type::kInt64) {
    return create_integer(value & n_other.as_int64(), arena);
  } else if (n_other.numeric_type() == starlark_numeric_type::kBigInt) {
    return Arena::Create<starlark_bigint>(&arena, from_int64(value) & n_other.as_bigint());
  } else {
    // Should not happen.
    assert(false);
    if (error_callback != nullptr) {
      error_callback->add_error("RuntimeError: unexpected number type");
    }
    return nullptr;
  }
}

starlark_obj* starlark_integer::binary_pipe(const starlark_obj& other, Arena& arena, error_fn* error_callback) const {
  if (other.type() != type()) {
    return starlark_obj::binary_pipe(other, arena, error_callback);
  }
  const starlark_numeric& n_other = static_cast<const starlark_numeric&>(other);
  if (n_other.numeric_type() == starlark_numeric_type::kInt64) {
    return create_integer(value | n_other.as_int64(), arena);
  } else if (n_other.numeric_type() == starlark_numeric_type::kBigInt) {
    return Arena::Create<starlark_bigint>(&arena, from_int64(value) | n_other.as_bigint());
  } else {
    // Should not happen.
    assert(false);
    if (error_callback != nullptr) {
      error_callback->add_error("RuntimeError: unexpected number type");
    }
    return nullptr;
  }
}

starlark_obj* starlark_integer::binary_hat(const starlark_obj& other, Arena& arena, error_fn* error_callback) const {
  if (other.type() != type()) {
    return starlark_obj::binary_hat(other, arena, error_callback);
  }
  const starlark_numeric& n_other = static_cast<const starlark_numeric&>(other);
  if (n_other.numeric_type() == starlark_numeric_type::kInt64) {
    return create_integer(value ^ n_other.as_int64(), arena);
  } else if (n_other.numeric_type() == starlark_numeric_type::kBigInt) {
    return Arena::Create<starlark_bigint>(&arena, from_int64(value) ^ n_other.as_bigint());
  } else {
    // Should not happen.
    assert(false);
    if (error_callback != nullptr) {
      error_callback->add_error("RuntimeError: unexpected number type");
    }
    return nullptr;
  }
}

starlark_obj* starlark_integer::binary_plus(const starlark_obj& other, Arena& arena, error_fn* error_callback) const {
  if (other.type() != "float" && other.type() != type()) {
    return starlark_obj::binary_plus(other, arena, error_callback);
  }
  auto* n_other = static_cast<const starlark_numeric*>(&other);
  if (n_other->numeric_type() == starlark_numeric_type::kFloat) {
    return Arena::Create<starlark_float>(&arena, value + n_other->as_float());
  } else if (n_other->numeric_type() == starlark_numeric_type::kInt64) {
    auto iother = n_other->as_int64();
    auto result = value + iother;
    // Avoid upgrading to a bigint if possible.
    // It is possible to change the condition to `((value ^ result) & (iother ^ result)) >= 0`
    // as explained in Hacker's Delight -- 2–13 Overflow Detection
    // but the version below is a little less magical.
    //
    // Digression note: gcc 15.2 is able to optimize both variations to 5 instructions
    // clang 21.1.0 does this with 4 instructions for the version not used, and 10 instructions for the version below.
    // In all cases, all optimized versions are branchless. The difference in performance is negligible in all cases.
    if (value < 0 != iother < 0 || result < 0 == value < 0) {
      return Arena::Create<starlark_integer>(&arena, result);
    }
    return Arena::Create<starlark_bigint>(&arena, from_int64(value) + from_int64(iother));
  } else if (n_other->numeric_type() == starlark_numeric_type::kBigInt) {
    return Arena::Create<starlark_bigint>(&arena, from_int64(value) + n_other->as_bigint());
  } else {
    // Should not happen.
    assert(false);
    if (error_callback != nullptr) {
      error_callback->add_error("TypeError: unknown numeric type");
    }
    return nullptr;
  }
}

starlark_obj* starlark_integer::binary_minus(const starlark_obj& other, Arena& arena, error_fn* error_callback) const {
  if (other.type() != "float" && other.type() != type()) {
    return starlark_obj::binary_minus(other, arena, error_callback);
  }
  auto* n_other = static_cast<const starlark_numeric*>(&other);
  if (n_other->numeric_type() == starlark_numeric_type::kFloat) {
    return Arena::Create<starlark_float>(&arena, value - n_other->as_float());
  } else if (n_other->numeric_type() == starlark_numeric_type::kInt64) {
    auto iother = n_other->as_int64();
    auto result = value - iother;
    // Avoid upgrading if possible.
    if (value < 0 == iother < 0 || result < 0 == value < 0) {
      return Arena::Create<starlark_integer>(&arena, result);
    }
    return Arena::Create<starlark_bigint>(&arena, from_int64(value) - from_int64(iother));
  } else if (n_other->numeric_type() == starlark_numeric_type::kBigInt) {
    return Arena::Create<starlark_bigint>(&arena, from_int64(value) - n_other->as_bigint());
  } else {
    // Should not happen.
    assert(false);
    if (error_callback != nullptr) {
      error_callback->add_error("TypeError: unknown numeric type");
    }
    return nullptr;
  }
}

starlark_obj* starlark_integer::binary_star(const starlark_obj& other, Arena& arena, error_fn* error_callback) const {
  // TODO(lmirelmann): Handle the case of types `string`, `bytes`, `list` and `tuple`.
  if (other.type() != "float" && other.type() != type()) {
    return starlark_obj::binary_star(other, arena, error_callback);
  }
  auto* n_other = static_cast<const starlark_numeric*>(&other);
  if (n_other->numeric_type() == starlark_numeric_type::kFloat) {
    return Arena::Create<starlark_float>(&arena, value * n_other->as_float());
  } else if (n_other->numeric_type() == starlark_numeric_type::kInt64) {
    auto iother = n_other->as_int64();
    auto nlz = std::countl_zero<uint64_t>(value) + std::countl_one<uint64_t>(value) +
        std::countl_zero<uint64_t>(iother) + std::countl_one<uint64_t>(iother);
    // Avoid upgrading if possible.
    if (nlz >= 66) {
      return Arena::Create<starlark_integer>(&arena, value * iother);
    }
    if (nlz == 65) {
      auto iresult = value * iother;
      if (iresult != std::numeric_limits<int64_t>::min() || value >= 0 || iother >= 0) {
        return Arena::Create<starlark_integer>(&arena, value * iother);
      }
    }
    // Therea are cases that `nlz == 64` and there is no overflow, but these are harder to
    // detect without doing the full multiplication.
    return Arena::Create<starlark_bigint>(&arena, from_int64(value) * from_int64(iother));
  } else if (n_other->numeric_type() == starlark_numeric_type::kBigInt) {
    return Arena::Create<starlark_bigint>(&arena, from_int64(value) * n_other->as_bigint());
  } else {
    // Should not happen.
    assert(false);
    if (error_callback != nullptr) {
      error_callback->add_error("TypeError: unknown numeric type");
    }
    return nullptr;
  }
}

starlark_obj* starlark_integer::binary_slash(const starlark_obj& other, Arena& arena, error_fn* error_callback) const {
  if (other.type() != "float" && other.type() != type()) {
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
    auto iother = n_other->as_int64();
    if (iother == 0) {
      if (error_callback != nullptr) {
        error_callback->add_error("ZeroDivisionError: division by zero");
      }
      return nullptr;
    }
    return Arena::Create<starlark_float>(&arena, static_cast<double>(value) / iother);
  } else if (n_other->numeric_type() == starlark_numeric_type::kBigInt) {
    auto fother = to_double(n_other->as_bigint());
    if (fother == 0) {
      if (error_callback != nullptr) {
        error_callback->add_error("ZeroDivisionError: division by zero");
      }
      return nullptr;
    }
    // This is a difference between Python and the Starlark implementation in Bazel. Python is
    // happy to return `0` if the integer is too large. Bazel throws an error.
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

starlark_obj* starlark_integer::binary_slash_slash(const starlark_obj& other, Arena& arena, error_fn* error_callback) const {
  if (other.type() != "float" && other.type() != type()) {
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
    return Arena::Create<starlark_float>(&arena, std::floor(value / v));
  } else if (n_other->numeric_type() == starlark_numeric_type::kInt64) {
    auto iother = n_other->as_int64();
    if (iother == 0) {
      if (error_callback != nullptr) {
        error_callback->add_error("ZeroDivisionError: division by zero");
      }
      return nullptr;
    }
    // Handle the overflow.
    if (value == std::numeric_limits<int64_t>::min() && iother == -1) {
      return Arena::Create<starlark_bigint>(&arena, starlark_div(from_int64(value), from_int64(iother)));
    }
    return Arena::Create<starlark_integer>(&arena, starlark_div(value, iother));
  } else if (n_other->numeric_type() == starlark_numeric_type::kBigInt) {
    auto bother = n_other->as_bigint();
    if (bother == number::zero) {
      if (error_callback != nullptr) {
        error_callback->add_error("ZeroDivisionError: division by zero");
      }
      return nullptr;
    }
    return Arena::Create<starlark_bigint>(&arena, starlark_div(from_int64(value), bother));
  } else {
    // Should not happen.
    assert(false);
    if (error_callback != nullptr) {
      error_callback->add_error("TypeError: unknown numeric type");
    }
    return nullptr;
  }
}

starlark_obj* starlark_integer::binary_percent(const starlark_obj& other, Arena& arena, error_fn* error_callback) const {
  if (other.type() != "float" && other.type() != type()) {
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
    return Arena::Create<starlark_float>(&arena, starlark_fmod(value, v));
  } else if (n_other->numeric_type() == starlark_numeric_type::kInt64) {
    auto iother = n_other->as_int64();
    if (iother == 0) {
      if (error_callback != nullptr) {
        error_callback->add_error("ZeroDivisionError: division by zero");
      }
      return nullptr;
    }
    if (value == std::numeric_limits<int64_t>::min() && iother == -1) {
      return Arena::Create<starlark_integer>(&arena, 0);
    }
    return Arena::Create<starlark_integer>(&arena, starlark_mod(value, iother));
  } else if (n_other->numeric_type() == starlark_numeric_type::kBigInt) {
    auto bother = n_other->as_bigint();
    if (bother == number::zero) {
      if (error_callback != nullptr) {
        error_callback->add_error("ZeroDivisionError: division by zero");
      }
      return nullptr;
    }
    return Arena::Create<starlark_bigint>(&arena, starlark_mod(from_int64(value), bother));
  } else {
    // Should not happen.
    assert(false);
    if (error_callback != nullptr) {
      error_callback->add_error("TypeError: unknown numeric type");
    }
    return nullptr;
  }
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


