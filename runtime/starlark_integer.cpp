// Copyright 2025-2026 Lucas Mirelmann

#include "runtime/starlark_integer.hpp"

#include <cassert>

#include <bit>
#include <limits>
#include <string>

#include "errors/runtime_error_messages.hpp"
#include "runtime/hash.hpp"
#include "runtime/options.hpp"
#include "runtime/starlark_numeric.hpp"
#include "runtime/starlark_types.hpp"

using ::starlark::bigint::number;
using ::starlark::error_messages::error_v2_bad_operand_binary;
using ::starlark::error_messages::error_v2_division_by_zero;
using ::starlark::error_messages::error_v2_negative_shift;
using ::starlark::error_messages::error_v2_overflow;
using ::starlark::error_messages::error_v2_overflow_too_many_digits;

namespace starlark {
namespace runtime {

starlark_integer::starlark_integer(int64_t value) : value(value) {}

std::string_view starlark_integer::type() const {
  return starlark_types::int_t;
}

bool starlark_integer::primitive() const {
  return true;
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

void starlark_integer::inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, bool extended, error_fn& error_callback) const {
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
      starlark_obj::inner_cmp(comp, other, op, extended, error_callback);
      break;
  }
}

bool starlark_integer::truthy() const {
  return value != 0;
}

starlark_obj* starlark_integer::unary_plus(context& ctx, error_fn& error_callback) const {
  return const_cast<starlark_integer*>(this);
}

starlark_obj* starlark_integer::unary_minus(context& ctx, error_fn& error_callback) const {
  if (value == std::numeric_limits<int64_t>::min()) {
    // Need to upgrade to bigint.
    return create_integer(number(static_cast<uint64_t>(value)), ctx);
  }
  return create_integer(-value, ctx);
}

starlark_obj* starlark_integer::unary_tilde(context& ctx, error_fn& error_callback) const {
  return create_integer(~value, ctx);
}

namespace {

starlark_obj* plus_op(int64_t value, const starlark_integer& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kFloat:
      return create_float(value + other.as_float(), ctx);
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
        return create_integer(result, ctx);
      }
      return create_integer(from_int64(value) + from_int64(iother), ctx);
    }
    case starlark_numeric_type::kBigInt:
      return create_integer(from_int64(value) + other.as_bigint(), ctx);
    default:
      error_callback.add_error(error_v2_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

starlark_obj* minus_op(int64_t value, const starlark_integer& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kFloat:
      return create_float(value - other.as_float(), ctx);
    case starlark_numeric_type::kInt64: {
      auto iother = other.as_int64();
      auto result = value - iother;
      // Avoid upgrading if possible.
      if (value < 0 == iother < 0 || result < 0 == value < 0) {
        return create_integer(result, ctx);
      }
      return create_integer(from_int64(value) - from_int64(iother), ctx);
    }
    case starlark_numeric_type::kBigInt:
      return create_integer(from_int64(value) - other.as_bigint(), ctx);
    default:
      error_callback.add_error(error_v2_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

starlark_obj* star_op(int64_t value, const starlark_integer& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  if (other.type() == starlark_types::string_t || other.type() == starlark_types::bytes_t || other.type() == starlark_types::list_t || other.type() == starlark_types::tuple_t) {
    return other.binary_star(this_obj, ctx, error_callback);
  }
  switch (other.numeric_type()) {
    case starlark_numeric_type::kFloat:
      return create_float(value * other.as_float(), ctx);
    case starlark_numeric_type::kInt64: {
      auto iother = other.as_int64();
      auto nlz = std::countl_zero<uint64_t>(value) + std::countl_one<uint64_t>(value) +
          std::countl_zero<uint64_t>(iother) + std::countl_one<uint64_t>(iother);
      // Avoid upgrading if possible.
      if (nlz >= 66) {
        return create_integer(value * iother, ctx);
      }
      if (nlz == 65) {
        auto iresult = value * iother;
        if (iresult != std::numeric_limits<int64_t>::min() || value >= 0 || iother >= 0) {
          return create_integer(value * iother, ctx);
        }
      }
      // Therea are cases that `nlz == 64` and there is no overflow, but these are harder to
      // detect without doing the full multiplication.
      return create_integer(from_int64(value) * from_int64(iother), ctx);
    }
    case starlark_numeric_type::kBigInt:
      return create_integer(from_int64(value) * other.as_bigint(), ctx);
    default:
      error_callback.add_error(error_v2_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

starlark_obj* slash_op(int64_t value, const starlark_integer& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kFloat: {
      auto fother = other.as_float();
      if (fother == 0) {
        error_callback.add_error(error_v2_division_by_zero());
        return nullptr;
      }
      return create_float(value / fother, ctx);
    }
    case starlark_numeric_type::kInt64: {
      auto iother = other.as_int64();
      if (iother == 0) {
        error_callback.add_error(error_v2_division_by_zero());
        return nullptr;
      }
      return create_float(static_cast<double>(value) / iother, ctx);
    }
    case starlark_numeric_type::kBigInt: {
      auto fother = to_double(other.as_bigint());
      if (fother == 0) {
        error_callback.add_error(error_v2_division_by_zero());
        return nullptr;
      }
      // This is a difference between Python and the Starlark implementation in Bazel. Python is
      // happy to return `0` if the integer is too large. Bazel throws an error.
      if (std::isinf(fother)) {
        error_callback.add_error(error_v2_overflow(this_obj.type(), starlark_types::float_t));
        return nullptr;
      }
      return create_float(value / fother, ctx);
    }
    default:
      error_callback.add_error(error_v2_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

starlark_obj* slash_slash_op(int64_t value, const starlark_integer& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kFloat: {
      auto fother = other.as_float();
      if (fother == 0) {
        error_callback.add_error(error_v2_division_by_zero());
        return nullptr;
      }
      return create_float(std::floor(value / fother), ctx);
    }
    case starlark_numeric_type::kInt64: {
      auto iother = other.as_int64();
      if (iother == 0) {
        error_callback.add_error(error_v2_division_by_zero());
        return nullptr;
      }
      // Handle the overflow.
      if (value == std::numeric_limits<int64_t>::min() && iother == -1) {
        return create_integer(starlark_div(from_int64(value), from_int64(iother)), ctx);
      }
      return create_integer(starlark_div(value, iother), ctx);
    }
    case starlark_numeric_type::kBigInt: {
      auto bother = other.as_bigint();
      if (bother == number::zero()) {
        error_callback.add_error(error_v2_division_by_zero());
        return nullptr;
      }
      return create_integer(starlark_div(from_int64(value), bother), ctx);
    }
    default:
      error_callback.add_error(error_v2_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

starlark_obj* percent_op(int64_t value, const starlark_integer& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kFloat: {
      auto fother = other.as_float();
      if (fother == 0) {
        error_callback.add_error(error_v2_division_by_zero());
        return nullptr;
      }
      return create_float(starlark_fmod(value, fother), ctx);
    }
    case starlark_numeric_type::kInt64: {
      auto iother = other.as_int64();
      if (iother == 0) {
        error_callback.add_error(error_v2_division_by_zero());
        return nullptr;
      }
      if (value == std::numeric_limits<int64_t>::min() && iother == -1) {
        return ctx.zero();
      }
      return create_integer(starlark_mod(value, iother), ctx);
    }
    case starlark_numeric_type::kBigInt: {
      auto bother = other.as_bigint();
      if (bother == number::zero()) {
        error_callback.add_error(error_v2_division_by_zero());
        return nullptr;
      }
      return create_integer(starlark_mod(from_int64(value), bother), ctx);
    }
    default:
      error_callback.add_error(error_v2_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

starlark_obj* and_op(int64_t value, const starlark_integer& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64:
      return create_integer(value & other.as_int64(), ctx);
    case starlark_numeric_type::kBigInt:
      return create_integer(from_int64(value) & other.as_bigint(), ctx);
    default:
      error_callback.add_error(error_v2_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

starlark_obj* pipe_op(int64_t value, const starlark_integer& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64:
      return create_integer(value | other.as_int64(), ctx);
    case starlark_numeric_type::kBigInt:
      return create_integer(from_int64(value) | other.as_bigint(), ctx);
    default:
      error_callback.add_error(error_v2_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

starlark_obj* hat_op(int64_t value, const starlark_integer& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64:
      return create_integer(value ^ other.as_int64(), ctx);
    case starlark_numeric_type::kBigInt:
      return create_integer(from_int64(value) ^ other.as_bigint(), ctx);
    default:
      error_callback.add_error(error_v2_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

starlark_obj* less_less_op(int64_t value, const starlark_integer& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64: {
      if (value == 0) {
        return const_cast<starlark_integer*>(&this_obj);
      }
      auto shift = other.as_int64();
      if (shift < 0) {
        error_callback.add_error(error_v2_negative_shift());
        return nullptr;
      }
      // Check whether it will fit in an int64_t.
      int left_shift_space = (value > 0 ? std::countl_zero<uint64_t>(value) : std::countl_one<uint64_t>(value));
      if (shift < left_shift_space) {
        return create_integer(value << shift, ctx);
      } else {
        if (ctx.options().log2_max_bigint < shift ||
            ctx.options().log2_max_bigint < shift + 64 - left_shift_space) {
          error_callback.add_error(error_v2_overflow_too_many_digits());
          return nullptr;
        }
        return create_integer(from_int64(value) << shift, ctx);
      }
    }
    case starlark_numeric_type::kBigInt: {
      if (value == 0) {
        return const_cast<starlark_integer*>(&this_obj);
      }
      const auto& shift = other.as_bigint();
      if (shift.sign()) {
        error_callback.add_error(error_v2_negative_shift());
        return nullptr;
      }
      if (shift.length() > 1) {
        error_callback.add_error(error_v2_overflow_too_many_digits());
        return nullptr;
      }
      auto int_shift = shift.at(0);
      int left_shift_space = (value > 0 ? std::countl_zero<uint64_t>(value) : std::countl_one<uint64_t>(value));
      if (int_shift < left_shift_space) {
        return create_integer(value << int_shift, ctx);
      } else {
        if (ctx.options().log2_max_bigint < int_shift ||
            ctx.options().log2_max_bigint < int_shift + 64 - left_shift_space) {
          error_callback.add_error(error_v2_overflow_too_many_digits());
          return nullptr;
        }
        return create_integer(from_int64(value) << int_shift, ctx);
      }
    }
    default:
      error_callback.add_error(error_v2_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

starlark_obj* greater_greater_op(int64_t value, const starlark_integer& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64: {
      if (value == 0) {
        return const_cast<starlark_integer*>(&this_obj);
      }
      auto shift = other.as_int64();
      if (shift < 0) {
        error_callback.add_error(error_v2_negative_shift());
        return nullptr;
      }
      if (shift >= 64) {
        return value >= 0 ? ctx.zero() : ctx.minus_one();
      }
      return create_integer(value >> shift, ctx);
    }
    case starlark_numeric_type::kBigInt: {
      if (value == 0) {
        return const_cast<starlark_integer*>(&this_obj);
      }
      const auto& shift = other.as_bigint();
      if (shift.sign()) {
        error_callback.add_error(error_v2_negative_shift());
        return nullptr;
      }
      if (shift.length() > 1) {
        return value >= 0 ? ctx.zero() : ctx.minus_one();
      }
      auto int_shift = shift.at(0);
      if (int_shift >= 64) {
        return value >= 0 ? ctx.zero() : ctx.minus_one();
      }
      return create_integer(value >> int_shift, ctx);
    }
    default:
      error_callback.add_error(error_v2_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

}  // namespace

starlark_obj* starlark_integer::binary_plus(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return plus_op(value, *this, other, "+", ctx, error_callback);
}

starlark_obj* starlark_integer::plus_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return plus_op(value, *this, other, "+=", ctx, error_callback);
}

starlark_obj* starlark_integer::binary_minus(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return minus_op(value, *this, other, "-", ctx, error_callback);
}

starlark_obj* starlark_integer::minus_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return minus_op(value, *this, other, "-=", ctx, error_callback);
}

starlark_obj* starlark_integer::binary_star(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return star_op(value, *this, other, "*", ctx, error_callback);
}

starlark_obj* starlark_integer::star_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return star_op(value, *this, other, "*=", ctx, error_callback);
}

starlark_obj* starlark_integer::binary_slash(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return slash_op(value, *this, other, "/", ctx, error_callback);
}

starlark_obj* starlark_integer::slash_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return slash_op(value, *this, other, "/=", ctx, error_callback);
}

starlark_obj* starlark_integer::binary_slash_slash(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return slash_slash_op(value, *this, other, "//", ctx, error_callback);
}

starlark_obj* starlark_integer::slash_slash_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return slash_slash_op(value, *this, other, "//=", ctx, error_callback);
}

starlark_obj* starlark_integer::binary_percent(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return percent_op(value, *this, other, "%", ctx, error_callback);
}

starlark_obj* starlark_integer::percent_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return percent_op(value, *this, other, "%=", ctx, error_callback);
}

starlark_obj* starlark_integer::binary_and(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return and_op(value, *this, other, "&", ctx, error_callback);
}

starlark_obj* starlark_integer::ampersand_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return and_op(value, *this, other, "&=", ctx, error_callback);
}

starlark_obj* starlark_integer::binary_pipe(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return pipe_op(value, *this, other, "|", ctx, error_callback);
}

starlark_obj* starlark_integer::pipe_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return pipe_op(value, *this, other, "|=", ctx, error_callback);
}

starlark_obj* starlark_integer::binary_hat(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return hat_op(value, *this, other, "^", ctx, error_callback);
}

starlark_obj* starlark_integer::hat_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return hat_op(value, *this, other, "^=", ctx, error_callback);
}

starlark_obj* starlark_integer::binary_lshift(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return less_less_op(value, *this, other, "<<", ctx, error_callback);
}

starlark_obj* starlark_integer::less_less_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return less_less_op(value, *this, other, "<<=", ctx, error_callback);
}

starlark_obj* starlark_integer::binary_rshift(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return greater_greater_op(value, *this, other, ">>", ctx, error_callback);
}

starlark_obj* starlark_integer::greater_greater_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return greater_greater_op(value, *this, other, ">>=", ctx, error_callback);
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


