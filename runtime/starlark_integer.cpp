// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_integer.hpp"

#include <cassert>

#include <bit>
#include <format>
#include <limits>
#include <string>

#include "runtime/hash.hpp"
#include "runtime/options.hpp"
#include "runtime/starlark_numeric.hpp"
#include "runtime/starlark_types.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;

namespace starlark {
namespace runtime {

starlark_integer::starlark_integer(int64_t value) : value(value) {}

std::string_view starlark_integer::type() const {
  return starlark_types::int_t;
}

bool starlark_integer::inner_repr(printer& print, printer_action action) const {
  assert(action == printer_action::kPrintTop);
  print.append(std::to_string(value));
  return false;
}

bool starlark_integer::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
  switch (other->numeric_type()) {
    case starlark_numeric_type::kFloat:
      return equals_fi(other->as_float(), as_int64());
    case starlark_numeric_type::kInt64:
      return as_int64() == other->as_int64();
    case starlark_numeric_type::kBigInt:
      return equals_ib(as_int64(), other->as_bigint());
    default:
      return false;
  }
}

void starlark_integer::inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, error_fn& error_callback) const {
  switch (other->numeric_type()) {
    case starlark_numeric_type::kFloat: {
      if (std::isnan(other->as_float())) {
        comp.add_task(order_comparator::pending_task_type::kLessThan);
        break;
      }
      auto r = cmp_fi(other->as_float(), as_int64());
      if (r != 0) {
        comp.add_task(r > 0 ? order_comparator::pending_task_type::kLessThan : order_comparator::pending_task_type::kGreaterThan);
      }
      break;
    }
    case starlark_numeric_type::kInt64: {
      auto r = as_int64() <=> other->as_int64();
      if (r != 0) {
        comp.add_task(r < 0 ? order_comparator::pending_task_type::kLessThan : order_comparator::pending_task_type::kGreaterThan);
      }
      break;
    }
    case starlark_numeric_type::kBigInt: {
      auto r = cmp_ib(as_int64(), other->as_bigint());
      if (r != 0) {
        comp.add_task(r < 0 ? order_comparator::pending_task_type::kLessThan : order_comparator::pending_task_type::kGreaterThan);
      }
      break;
    }
    default:
      starlark_obj::inner_cmp(comp, other, op, error_callback);
      break;
  }
}

bool starlark_integer::truthy() const {
  return value != 0;
}

starlark_obj* starlark_integer::unary_plus(Arena& arena, error_fn& error_callback) const {
  return const_cast<starlark_integer*>(this);
}

starlark_obj* starlark_integer::unary_minus(Arena& arena, error_fn& error_callback) const {
  if (value == std::numeric_limits<int64_t>::min()) {
    // Need to upgrade to bigint.
    return create_integer(number(static_cast<uint64_t>(value)), arena);
  }
  // TODO(lmirelmann): Figure out whether it is possible to reuse `this`.
  return create_integer(-value, arena);
}

starlark_obj* starlark_integer::unary_tilde(Arena& arena, error_fn& error_callback) const {
  // TODO(lmirelmann): Figure out whether it is possible to reuse `this`.
  return create_integer(~value, arena);
}

starlark_obj* starlark_integer::binary_lshift(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64: {
      if (value == 0) {
        return const_cast<starlark_integer*>(this);
      }
      auto shift = other.as_int64();
      if (shift < 0) {
        error_callback.add_error("ValueError: negative shift count");
        return nullptr;
      }
      // Check whether it will fit in an int64_t.
      int left_shift_space = (value > 0 ? std::countl_zero<uint64_t>(value) : std::countl_one<uint64_t>(value));
      if (shift < left_shift_space) {
        return create_integer(value << shift, arena);
      } else {
        if (log2_max_bigint() < 64 - std::countl_zero<uint64_t>(shift) + 64 - left_shift_space) {
          error_callback.add_error("OverflowError: too many digits in integer");
          return nullptr;
        }
        return create_integer(from_int64(value) << shift, arena);
      }
    }
    case starlark_numeric_type::kBigInt: {
      if (value == 0) {
        return const_cast<starlark_integer*>(this);
      }
      const auto& shift = other.as_bigint();
      if (shift.sign()) {
        error_callback.add_error("ValueError: negative shift count");
        return nullptr;
      }
      if (shift.length() > 1) {
        error_callback.add_error("OverflowError: too many digits in integer");
        return nullptr;
      }
      auto int_shift = shift.at(0);
      int left_shift_space = (value > 0 ? std::countl_zero<uint64_t>(value) : std::countl_one<uint64_t>(value));
      if (int_shift < left_shift_space) {
        return create_integer(value << int_shift, arena);
      } else {
        if (log2_max_bigint() < 64 - std::countl_zero<uint64_t>(int_shift) + 64 - left_shift_space) {
          error_callback.add_error("OverflowError: too many digits in integer");
          return nullptr;
        }
        return create_integer(from_int64(value) << int_shift, arena);
      }
    }
    default:
      return starlark_obj::binary_lshift(other, arena, error_callback);
  }
}

starlark_obj* starlark_integer::binary_rshift(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64: {
      if (value == 0) {
        return const_cast<starlark_integer*>(this);
      }
      auto shift = other.as_int64();
      if (shift < 0) {
        error_callback.add_error("ValueError: negative shift count");
        return nullptr;
      }
      if (shift >= 64) {
        return create_integer(value >= 0 ? 0 : -1, arena);
      }
      return create_integer(value >> shift, arena);
    }
    case starlark_numeric_type::kBigInt: {
      if (value == 0) {
        return const_cast<starlark_integer*>(this);
      }
      const auto& shift = other.as_bigint();
      if (shift.sign()) {
        error_callback.add_error("ValueError: negative shift count");
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
    }
    default:
      return starlark_obj::binary_rshift(other, arena, error_callback);
  }
}

starlark_obj* starlark_integer::binary_and(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64:
      return create_integer(value & other.as_int64(), arena);
    case starlark_numeric_type::kBigInt:
      return create_integer(from_int64(value) & other.as_bigint(), arena);
    default:
      return starlark_obj::binary_and(other, arena, error_callback);
  }
}

starlark_obj* starlark_integer::binary_pipe(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64:
      return create_integer(value | other.as_int64(), arena);
    case starlark_numeric_type::kBigInt:
      return create_integer(from_int64(value) | other.as_bigint(), arena);
    default:
      return starlark_obj::binary_pipe(other, arena, error_callback);
  }
}

starlark_obj* starlark_integer::binary_hat(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64:
      return create_integer(value ^ other.as_int64(), arena);
    case starlark_numeric_type::kBigInt:
      return create_integer(from_int64(value) ^ other.as_bigint(), arena);
    default:
      return starlark_obj::binary_hat(other, arena, error_callback);
  }
}

starlark_obj* starlark_integer::binary_plus(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kFloat:
      return create_float(value + other.as_float(), arena);
    case starlark_numeric_type::kInt64: {
      auto iother = other.as_int64();
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
        return create_integer(result, arena);
      }
      return create_integer(from_int64(value) + from_int64(iother), arena);
    }
    case starlark_numeric_type::kBigInt:
      return create_integer(from_int64(value) + other.as_bigint(), arena);
    default:
      return starlark_obj::binary_plus(other, arena, error_callback);
  }
}

starlark_obj* starlark_integer::binary_minus(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kFloat:
      return create_float(value - other.as_float(), arena);
    case starlark_numeric_type::kInt64: {
      auto iother = other.as_int64();
      auto result = value - iother;
      // Avoid upgrading if possible.
      if (value < 0 == iother < 0 || result < 0 == value < 0) {
        return create_integer(result, arena);
      }
      return create_integer(from_int64(value) - from_int64(iother), arena);
    }
    case starlark_numeric_type::kBigInt:
      return create_integer(from_int64(value) - other.as_bigint(), arena);
    default:
      return starlark_obj::binary_minus(other, arena, error_callback);
  }
}

starlark_obj* starlark_integer::binary_star(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  if (other.type() == starlark_types::string_t || other.type() == starlark_types::bytes_t || other.type() == starlark_types::list_t || other.type() == starlark_types::tuple_t) {
    return other.binary_star(*this, arena, error_callback);
  }
  switch (other.numeric_type()) {
    case starlark_numeric_type::kFloat:
      return create_float(value * other.as_float(), arena);
    case starlark_numeric_type::kInt64: {
      auto iother = other.as_int64();
      auto nlz = std::countl_zero<uint64_t>(value) + std::countl_one<uint64_t>(value) +
          std::countl_zero<uint64_t>(iother) + std::countl_one<uint64_t>(iother);
      // Avoid upgrading if possible.
      if (nlz >= 66) {
        return create_integer(value * iother, arena);
      }
      if (nlz == 65) {
        auto iresult = value * iother;
        if (iresult != std::numeric_limits<int64_t>::min() || value >= 0 || iother >= 0) {
          return create_integer(value * iother, arena);
        }
      }
      // Therea are cases that `nlz == 64` and there is no overflow, but these are harder to
      // detect without doing the full multiplication.
      return create_integer(from_int64(value) * from_int64(iother), arena);
    }
    case starlark_numeric_type::kBigInt:
      return create_integer(from_int64(value) * other.as_bigint(), arena);
    default:
      return starlark_obj::binary_star(other, arena, error_callback);
  }
}

starlark_obj* starlark_integer::binary_slash(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kFloat: {
      auto fother = other.as_float();
      if (fother == 0) {
        error_callback.add_error("ZeroDivisionError: division by zero");
        return nullptr;
      }
      return create_float(value / fother, arena);
    }
    case starlark_numeric_type::kInt64: {
      auto iother = other.as_int64();
      if (iother == 0) {
        error_callback.add_error("ZeroDivisionError: division by zero");
        return nullptr;
      }
      return create_float(static_cast<double>(value) / iother, arena);
    }
    case starlark_numeric_type::kBigInt: {
      auto fother = to_double(other.as_bigint());
      if (fother == 0) {
        error_callback.add_error("ZeroDivisionError: division by zero");
        return nullptr;
      }
      // This is a difference between Python and the Starlark implementation in Bazel. Python is
      // happy to return `0` if the integer is too large. Bazel throws an error.
      if (std::isinf(fother)) {
        error_callback.add_error("OverflowError: int too large to convert to float");
        return nullptr;
      }
      return create_float(value / fother, arena);
    }
    default:
      return starlark_obj::binary_slash(other, arena, error_callback);
  }
}

starlark_obj* starlark_integer::binary_slash_slash(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kFloat: {
      auto fother = other.as_float();
      if (fother == 0) {
        error_callback.add_error("ZeroDivisionError: division by zero");
        return nullptr;
      }
      return create_float(std::floor(value / fother), arena);
    }
    case starlark_numeric_type::kInt64: {
      auto iother = other.as_int64();
      if (iother == 0) {
        error_callback.add_error("ZeroDivisionError: division by zero");
        return nullptr;
      }
      // Handle the overflow.
      if (value == std::numeric_limits<int64_t>::min() && iother == -1) {
        return create_integer(starlark_div(from_int64(value), from_int64(iother)), arena);
      }
      return create_integer(starlark_div(value, iother), arena);
    }
    case starlark_numeric_type::kBigInt: {
      auto bother = other.as_bigint();
      if (bother == number::zero) {
        error_callback.add_error("ZeroDivisionError: division by zero");
        return nullptr;
      }
      return create_integer(starlark_div(from_int64(value), bother), arena);
    }
    default:
      return starlark_obj::binary_slash_slash(other, arena, error_callback);
  }
}

starlark_obj* starlark_integer::binary_percent(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kFloat: {
      auto fother = other.as_float();
      if (fother == 0) {
        error_callback.add_error("ZeroDivisionError: division by zero");
        return nullptr;
      }
      return create_float(starlark_fmod(value, fother), arena);
    }
    case starlark_numeric_type::kInt64: {
      auto iother = other.as_int64();
      if (iother == 0) {
        error_callback.add_error("ZeroDivisionError: division by zero");
        return nullptr;
      }
      if (value == std::numeric_limits<int64_t>::min() && iother == -1) {
        return create_integer(0, arena);
      }
      return create_integer(starlark_mod(value, iother), arena);
    }
    case starlark_numeric_type::kBigInt: {
      auto bother = other.as_bigint();
      if (bother == number::zero) {
        error_callback.add_error("ZeroDivisionError: division by zero");
        return nullptr;
      }
      return create_integer(starlark_mod(from_int64(value), bother), arena);
    }
    default:
      return starlark_obj::binary_percent(other, arena, error_callback);
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


