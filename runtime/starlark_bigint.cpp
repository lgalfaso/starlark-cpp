// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_bigint.hpp"

#include <cassert>

#include <string>

#include "runtime/error_messages.hpp"
#include "runtime/hash.hpp"
#include "runtime/options.hpp"
#include "runtime/starlark_numeric.hpp"
#include "runtime/starlark_types.hpp"

using ::starlark::bigint::number;

namespace starlark {
namespace runtime {

starlark_bigint::starlark_bigint(int64_t value) : value(from_int64(value)) {}

starlark_bigint::starlark_bigint(const number& value) : value(value) {}

std::string_view starlark_bigint::type() const {
  return starlark_types::int_t;
}

bool starlark_bigint::primitive() const {
  return true;
}

bool starlark_bigint::inner_repr(printer& print, printer_action action) const {
  assert(action == printer_action::kPrintTop);
  print.append(value.to_string(10, false));
  return false;
}

bool starlark_bigint::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
  switch (other->numeric_type()) {
    case starlark_numeric_type::kFloat:
      return equals_fb(other->as_float(), as_bigint());
    case starlark_numeric_type::kInt64:
      return equals_ib(other->as_int64(), as_bigint());
    case starlark_numeric_type::kBigInt:
      return as_bigint() == other->as_bigint();
    default:
      return false;
  }
}

void starlark_bigint::inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, error_fn& error_callback) const {
  switch (other->numeric_type()) {
    case starlark_numeric_type::kFloat: {
      if (std::isnan(other->as_float())) {
        comp.add_task(order_comparator::pending_task_type::kLessThan);
        break;
      }
      auto r = cmp_fb(other->as_float(), as_bigint());
      if (r != 0) {
        comp.add_task(r > 0 ? order_comparator::pending_task_type::kLessThan : order_comparator::pending_task_type::kGreaterThan);
      }
      break;
    }
    case starlark_numeric_type::kInt64: {
      auto r = cmp_ib(other->as_int64(), as_bigint());
      if (r != 0) {
        comp.add_task(r > 0 ? order_comparator::pending_task_type::kLessThan : order_comparator::pending_task_type::kGreaterThan);
      }
      break;
    }
    case starlark_numeric_type::kBigInt: {
      auto r = as_bigint().cmp(other->as_bigint());
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

bool starlark_bigint::truthy() const {
  return value != number::zero();
}

starlark_obj* starlark_bigint::unary_plus(context& ctx, error_fn& error_callback) const {
  return const_cast<starlark_bigint*>(this);
}

starlark_obj* starlark_bigint::unary_minus(context& ctx, error_fn& error_callback) const {
  return create_integer(-value, ctx);
}

starlark_obj* starlark_bigint::unary_tilde(context& ctx, error_fn& error_callback) const {
  return create_integer(~value, ctx);
}

namespace {

starlark_obj* plus_op(const number& value, const starlark_bigint& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kFloat: {
      auto fvalue = to_double(value);
      if (std::isinf(fvalue)) {
        error_callback.add_error(error_overflow(this_obj.type(), starlark_types::float_t));
        return nullptr;
      }
      return create_float(fvalue + other.as_float(), ctx);
    }
    case starlark_numeric_type::kInt64:
      return create_integer(value + from_int64(other.as_int64()), ctx);
    case starlark_numeric_type::kBigInt:
      return create_integer(value + other.as_bigint(), ctx);
    default:
      error_callback.add_error(error_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

starlark_obj* minus_op(const number& value, const starlark_bigint& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kFloat: {
      auto fvalue = to_double(value);
      if (std::isinf(fvalue)) {
        error_callback.add_error(error_overflow(this_obj.type(), starlark_types::float_t));
        return nullptr;
      }
      return create_float(fvalue - other.as_float(), ctx);
    }
    case starlark_numeric_type::kInt64:
      return create_integer(value - from_int64(other.as_int64()), ctx);
    case starlark_numeric_type::kBigInt:
      return create_integer(value - other.as_bigint(), ctx);
    default:
      error_callback.add_error(error_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

starlark_obj* star_op(const number& value, const starlark_bigint& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  if (other.type() == starlark_types::string_t || other.type() == starlark_types::bytes_t || other.type() == starlark_types::list_t || other.type() == starlark_types::tuple_t) {
    return other.binary_star(this_obj, ctx, error_callback);
  }
  switch (other.numeric_type()) {
    case starlark_numeric_type::kFloat: {
      auto fvalue = to_double(value);
      if (std::isinf(fvalue)) {
        error_callback.add_error(error_overflow(this_obj.type(), starlark_types::float_t));
        return nullptr;
      }
      return create_float(fvalue * other.as_float(), ctx);
    }
    case starlark_numeric_type::kInt64:
      return create_integer(value * from_int64(other.as_int64()), ctx);
    case starlark_numeric_type::kBigInt:
      return create_integer(value * other.as_bigint(), ctx);
    default:
      error_callback.add_error(error_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

starlark_obj* slash_op(const number& value, const starlark_bigint& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kFloat: {
      auto fvalue = to_double(value);
      if (std::isinf(fvalue)) {
        error_callback.add_error(error_overflow(this_obj.type(), starlark_types::float_t));
        return nullptr;
      }
      auto fother = other.as_float();
      if (fother == 0) {
        error_callback.add_error(error_division_by_zero());
        return nullptr;
      }
      return create_float(fvalue / fother, ctx);
    }
    case starlark_numeric_type::kInt64: {
      auto fvalue = to_double(value);
      if (std::isinf(fvalue)) {
        error_callback.add_error(error_overflow(this_obj.type(), starlark_types::float_t));
        return nullptr;
      }
      auto iother = other.as_int64();
      if (iother == 0) {
        error_callback.add_error(error_division_by_zero());
        return nullptr;
      }
      return create_float(fvalue / iother, ctx);
    }
    case starlark_numeric_type::kBigInt: {
      auto fvalue = to_double(value);
      if (std::isinf(fvalue)) {
        error_callback.add_error(error_overflow(this_obj.type(), starlark_types::float_t));
        return nullptr;
      }
      auto fother = to_double(other.as_bigint());
      if (fother == 0) {
        error_callback.add_error(error_division_by_zero());
        return nullptr;
      }
      if (std::isinf(fother)) {
        error_callback.add_error(error_overflow(this_obj.type(), starlark_types::float_t));
        return nullptr;
      }
      return create_float(fvalue / fother, ctx);
    }
    default:
      error_callback.add_error(error_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

starlark_obj* slash_slash_op(const number& value, const starlark_bigint& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kFloat: {
      auto fvalue = to_double(value);
      if (std::isinf(fvalue)) {
        error_callback.add_error(error_overflow(this_obj.type(), starlark_types::float_t));
        return nullptr;
      }
      auto fother = other.as_float();
      if (fother == 0) {
        error_callback.add_error(error_division_by_zero());
        return nullptr;
      }
      return create_float(std::floor(fvalue / fother), ctx);
    }
    case starlark_numeric_type::kInt64: {
      auto iother = other.as_int64();
      if (iother == 0) {
        error_callback.add_error(error_division_by_zero());
        return nullptr;
      }
      return create_integer(starlark_div(value, from_int64(iother)), ctx);
    }
    case starlark_numeric_type::kBigInt: {
      const auto& bother = other.as_bigint();
      if (bother == number::zero()) {
        error_callback.add_error(error_division_by_zero());
        return nullptr;
      }
      return create_integer(starlark_div(value, bother), ctx);
    }
    default:
      error_callback.add_error(error_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

starlark_obj* percent_op(const number& value, const starlark_bigint& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kFloat: {
      auto fvalue = to_double(value);
      if (std::isinf(fvalue)) {
        error_callback.add_error(error_overflow(this_obj.type(), starlark_types::float_t));
        return nullptr;
      }
      auto fother = other.as_float();
      if (fother == 0) {
        error_callback.add_error(error_division_by_zero());
        return nullptr;
      }
      return create_float(starlark_fmod(fvalue, fother), ctx);
    }
    case starlark_numeric_type::kInt64: {
      auto iother = other.as_int64();
      if (iother == 0) {
        error_callback.add_error(error_division_by_zero());
        return nullptr;
      }
      return create_integer(starlark_mod(value, from_int64(iother)), ctx);
    }
    case starlark_numeric_type::kBigInt: {
      const auto& bother = other.as_bigint();
      if (bother == number::zero()) {
        error_callback.add_error(error_division_by_zero());
        return nullptr;
      }
      return create_integer(starlark_mod(value, bother), ctx);
    }
    default:
      error_callback.add_error(error_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

starlark_obj* and_op(const number& value, const starlark_bigint& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64:
      return create_integer(value & from_int64(other.as_int64()), ctx);
    case starlark_numeric_type::kBigInt:
      return create_integer(value & other.as_bigint(), ctx);
    default:
      error_callback.add_error(error_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

starlark_obj* pipe_op(const number& value, const starlark_bigint& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64:
      return create_integer(value | from_int64(other.as_int64()), ctx);
    case starlark_numeric_type::kBigInt:
      return create_integer(value | other.as_bigint(), ctx);
    default:
      error_callback.add_error(error_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

starlark_obj* hat_op(const number& value, const starlark_bigint& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64:
      return create_integer(value ^ from_int64(other.as_int64()), ctx);
    case starlark_numeric_type::kBigInt:
      return create_integer(value ^ other.as_bigint(), ctx);
    default:
      error_callback.add_error(error_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

starlark_obj* less_less_op(const number& value, const starlark_bigint& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64: {
      if (value == number::zero()) {
        return const_cast<starlark_bigint*>(&this_obj);
      }
      auto shift = other.as_int64();
      if (shift < 0) {
        error_callback.add_error(error_negative_shift());
        return nullptr;
      }
      if (ctx.options().log2_max_bigint < shift ||
          ctx.options().log2_max_bigint < shift + value.bit_size()) {
        error_callback.add_error(error_overflow_too_many_digits());
        return nullptr;
      }
      return create_integer(value << shift, ctx);
    }
    case starlark_numeric_type::kBigInt: {
      if (value == number::zero()) {
        return const_cast<starlark_bigint*>(&this_obj);
      }
      const auto& shift = other.as_bigint();
      if (shift.sign()) {
        error_callback.add_error(error_negative_shift());
        return nullptr;
      }
      if (shift.length() > 1) {
        error_callback.add_error(error_overflow_too_many_digits());
        return nullptr;
      }
      auto int_shift = shift.at(0);
      if (ctx.options().log2_max_bigint < int_shift ||
          ctx.options().log2_max_bigint < int_shift + value.bit_size()) {
        error_callback.add_error(error_overflow_too_many_digits());
        return nullptr;
      }
      return create_integer(value << int_shift, ctx);
    }
    default:
      error_callback.add_error(error_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

starlark_obj* greater_greater_op(const number& value, const starlark_bigint& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64: {
      if (value == number::zero()) {
        return const_cast<starlark_bigint*>(&this_obj);
      }
      auto shift = other.as_int64();
      if (shift < 0) {
        error_callback.add_error(error_negative_shift());
        return nullptr;
      }
      if (shift >= value.bit_size()) {
        return value.sign() ? ctx.minus_one() : ctx.zero();
      }
      return create_integer(value >> shift, ctx);
    }
    case starlark_numeric_type::kBigInt: {
      if (value == number::zero()) {
        return const_cast<starlark_bigint*>(&this_obj);
      }
      const auto& shift = other.as_bigint();
      if (shift.sign()) {
        error_callback.add_error(error_negative_shift());
        return nullptr;
      }
      if (shift.length() > 1) {
        return value.sign() ? ctx.minus_one() : ctx.zero();
      }
      auto int_shift = shift.at(0);
      if (int_shift >= value.bit_size()) {
        return value.sign() ? ctx.minus_one() : ctx.zero();
      }
      return create_integer(value >> int_shift, ctx);
    }
    default:
      error_callback.add_error(error_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

}  // namespace

starlark_obj* starlark_bigint::binary_plus(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return plus_op(value, *this, other, "+", ctx, error_callback);
}

starlark_obj* starlark_bigint::plus_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return plus_op(value, *this, other, "+=", ctx, error_callback);
}

starlark_obj* starlark_bigint::binary_minus(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return minus_op(value, *this, other, "-", ctx, error_callback);
}

starlark_obj* starlark_bigint::minus_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return minus_op(value, *this, other, "-=", ctx, error_callback);
}

starlark_obj* starlark_bigint::binary_star(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return star_op(value, *this, other, "*", ctx, error_callback);
}

starlark_obj* starlark_bigint::star_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return star_op(value, *this, other, "*=", ctx, error_callback);
}

starlark_obj* starlark_bigint::binary_slash(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return slash_op(value, *this, other, "/", ctx, error_callback);
}

starlark_obj* starlark_bigint::slash_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return slash_op(value, *this, other, "/=", ctx, error_callback);
}

starlark_obj* starlark_bigint::binary_slash_slash(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return slash_slash_op(value, *this, other, "//", ctx, error_callback);
}

starlark_obj* starlark_bigint::slash_slash_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return slash_slash_op(value, *this, other, "//=", ctx, error_callback);
}

starlark_obj* starlark_bigint::binary_percent(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return percent_op(value, *this, other, "%", ctx, error_callback);
}

starlark_obj* starlark_bigint::percent_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return percent_op(value, *this, other, "%=", ctx, error_callback);
}

starlark_obj* starlark_bigint::binary_and(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return and_op(value, *this, other, "&", ctx, error_callback);
}

starlark_obj* starlark_bigint::ampersand_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return and_op(value, *this, other, "&=", ctx, error_callback);
}

starlark_obj* starlark_bigint::binary_pipe(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return pipe_op(value, *this, other, "|", ctx, error_callback);
}

starlark_obj* starlark_bigint::pipe_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return pipe_op(value, *this, other, "|=", ctx, error_callback);
}

starlark_obj* starlark_bigint::binary_hat(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return hat_op(value, *this, other, "^", ctx, error_callback);
}

starlark_obj* starlark_bigint::hat_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return hat_op(value, *this, other, "^=", ctx, error_callback);
}

starlark_obj* starlark_bigint::binary_lshift(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return less_less_op(value, *this, other, "<<", ctx, error_callback);
}

starlark_obj* starlark_bigint::less_less_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return less_less_op(value, *this, other, "<<=", ctx, error_callback);
}

starlark_obj* starlark_bigint::binary_rshift(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return greater_greater_op(value, *this, other, ">>", ctx, error_callback);
}

starlark_obj* starlark_bigint::greater_greater_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return greater_greater_op(value, *this, other, ">>=", ctx, error_callback);
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_bigint::inner_hash() const {
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

starlark_numeric_type starlark_bigint::numeric_type() const {
  return starlark_numeric_type::kBigInt;
}

const number& starlark_bigint::as_bigint() const {
  return value;
}

}  // namespace runtime
}  // namespace starlark


