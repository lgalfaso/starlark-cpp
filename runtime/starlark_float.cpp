// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_float.hpp"

#include <cassert>

#include <bit>
#include <limits>
#include <string>

#include "runtime/error_messages.hpp"
#include "runtime/hash.hpp"
#include "runtime/starlark_numeric.hpp"
#include "runtime/starlark_types.hpp"

namespace starlark {
namespace runtime {

starlark_float::starlark_float(double value) : value(value) {}

std::string_view starlark_float::type() const {
  return starlark_types::float_t;
}

bool starlark_float::primitive() const {
  return true;
}

starlark_obj* starlark_float::unary_plus(context& ctx, error_fn& error_callback) const {
  return const_cast<starlark_float*>(this);
}

starlark_obj* starlark_float::unary_minus(context& ctx, error_fn& error_callback) const {
  return create_float(-value, ctx);
}

void starlark_float::inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, error_fn& error_callback) const {
  switch (other->numeric_type()) {
    case starlark_numeric_type::kFloat: {
      if (std::isnan(as_float())) {
        if (!std::isnan(other->as_float())) {
          comp.add_task(order_comparator::pending_task_type::kGreaterThan);
        }
        break;
      }
      if (std::isnan(other->as_float())) {
        comp.add_task(order_comparator::pending_task_type::kLessThan);
        break;
      }
      auto r = as_float() <=> other->as_float();
      if (r != 0) {
        comp.add_task(r < 0 ? order_comparator::pending_task_type::kLessThan : order_comparator::pending_task_type::kGreaterThan);
      }
      break;
    }
    case starlark_numeric_type::kInt64: {
      if (std::isnan(as_float())) {
        comp.add_task(order_comparator::pending_task_type::kGreaterThan);
        break;
      }
      auto r = cmp_fi(as_float(), other->as_int64());
      if (r != 0) {
        comp.add_task(r < 0 ? order_comparator::pending_task_type::kLessThan : order_comparator::pending_task_type::kGreaterThan);
      }
      break;
    }
    case starlark_numeric_type::kBigInt: {
      if (std::isnan(as_float())) {
        comp.add_task(order_comparator::pending_task_type::kGreaterThan);
        break;
      }
      auto r = cmp_fb(as_float(), other->as_bigint());
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

namespace {

starlark_obj* plus_op(double value, const starlark_float& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kFloat:
      return create_float(value + other.as_float(), ctx);
    case starlark_numeric_type::kInt64:
      return create_float(value + other.as_int64(), ctx);
    case starlark_numeric_type::kBigInt: {
      auto fother = to_double(other.as_bigint());
      if (std::isinf(fother)) {
        error_callback.add_error(error_overflow(other.type(), this_obj.type()));
        return nullptr;
      }
      return create_float(value + fother, ctx);
    }
    default:
      error_callback.add_error(error_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

starlark_obj* minus_op(double value, const starlark_float& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kFloat:
      return create_float(value - other.as_float(), ctx);
    case starlark_numeric_type::kInt64:
      return create_float(value - other.as_int64(), ctx);
    case starlark_numeric_type::kBigInt: {
      auto fother = to_double(other.as_bigint());
      if (std::isinf(fother)) {
        error_callback.add_error(error_overflow(other.type(), this_obj.type()));
        return nullptr;
      }
      return create_float(value - fother, ctx);
    }
    default:
      error_callback.add_error(error_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

starlark_obj* star_op(double value, const starlark_float& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kFloat:
      return create_float(value * other.as_float(), ctx);
    case starlark_numeric_type::kInt64:
      return create_float(value * other.as_int64(), ctx);
    case starlark_numeric_type::kBigInt: {
      auto fother = to_double(other.as_bigint());
      if (std::isinf(fother)) {
        error_callback.add_error(error_overflow(other.type(), this_obj.type()));
        return nullptr;
      }
      return create_float(value * fother, ctx);
    }
    default:
      error_callback.add_error(error_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

starlark_obj* slash_op(double value, const starlark_float& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kFloat: {
      auto fother = other.as_float();
      if (fother == 0) {
        error_callback.add_error(error_division_by_zero());
        return nullptr;
      }
      return create_float(value / fother, ctx);
    }
    case starlark_numeric_type::kInt64: {
      auto iother = other.as_int64();
      if (iother == 0) {
        error_callback.add_error(error_division_by_zero());
        return nullptr;
      }
      return create_float(value / iother, ctx);
    }
    case starlark_numeric_type::kBigInt: {
      auto fother = to_double(other.as_bigint());
      if (fother == 0) {
        error_callback.add_error(error_division_by_zero());
        return nullptr;
      }
      if (std::isinf(fother)) {
        error_callback.add_error(error_overflow(other.type(), this_obj.type()));
        return nullptr;
      }
      return create_float(value / fother, ctx);
    }
    default:
      error_callback.add_error(error_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

starlark_obj* slash_slash_op(double value, const starlark_float& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kFloat: {
      auto fother = other.as_float();
      if (fother == 0) {
        error_callback.add_error(error_division_by_zero());
        return nullptr;
      }
      return create_float(std::floor(value / fother), ctx);
    }
    case starlark_numeric_type::kInt64: {
      auto iother = other.as_int64();
      if (iother == 0) {
        error_callback.add_error(error_division_by_zero());
        return nullptr;
      }
      return create_float(std::floor(value / iother), ctx);
    }
    case starlark_numeric_type::kBigInt: {
      auto fother = to_double(other.as_bigint());
      if (fother == 0) {
        error_callback.add_error(error_division_by_zero());
        return nullptr;
      }
      if (std::isinf(fother)) {
        error_callback.add_error(error_overflow(other.type(), this_obj.type()));
        return nullptr;
      }
      return create_float(std::floor(value / fother), ctx);
    }
    default:
      error_callback.add_error(error_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

starlark_obj* percent_op(double value, const starlark_float& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kFloat: {
      auto fother = other.as_float();
      if (fother == 0) {
        error_callback.add_error(error_division_by_zero());
        return nullptr;
      }
      return create_float(starlark_fmod(value, fother), ctx);
    }
    case starlark_numeric_type::kInt64: {
      auto iother = other.as_int64();
      if (iother == 0) {
        error_callback.add_error(error_division_by_zero());
        return nullptr;
      }
      return create_float(starlark_fmod(value, iother), ctx);
    }
    case starlark_numeric_type::kBigInt: {
      auto fother = to_double(other.as_bigint());
      if (fother == 0) {
        error_callback.add_error(error_division_by_zero());
        return nullptr;
      }
      if (std::isinf(fother)) {
        error_callback.add_error(error_overflow(other.type(), this_obj.type()));
        return nullptr;
      }
      return create_float(starlark_fmod(value, fother), ctx);
    }
    default:
      error_callback.add_error(error_bad_operand_binary(op, this_obj.type(), other.type()));
      return nullptr;
  }
}

}  // namespace

starlark_obj* starlark_float::binary_plus(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return plus_op(value, *this, other, "+", ctx, error_callback);
}

starlark_obj* starlark_float::plus_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return plus_op(value, *this, other, "+=", ctx, error_callback);
}

starlark_obj* starlark_float::binary_minus(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return minus_op(value, *this, other, "-", ctx, error_callback);
}

starlark_obj* starlark_float::minus_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return minus_op(value, *this, other, "-=", ctx, error_callback);
}

starlark_obj* starlark_float::binary_star(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return star_op(value, *this, other, "*", ctx, error_callback);
}

starlark_obj* starlark_float::star_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return star_op(value, *this, other, "*=", ctx, error_callback);
}

starlark_obj* starlark_float::binary_slash(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return slash_op(value, *this, other, "/", ctx, error_callback);
}

starlark_obj* starlark_float::slash_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return slash_op(value, *this, other, "/=", ctx, error_callback);
}

starlark_obj* starlark_float::binary_slash_slash(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return slash_slash_op(value, *this, other, "//", ctx, error_callback);
}

starlark_obj* starlark_float::slash_slash_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return slash_slash_op(value, *this, other, "//=", ctx, error_callback);
}

starlark_obj* starlark_float::binary_percent(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return percent_op(value, *this, other, "%", ctx, error_callback);
}

starlark_obj* starlark_float::percent_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return percent_op(value, *this, other, "%=", ctx, error_callback);
}

bool starlark_float::inner_repr(printer& print, printer_action action) const {
  assert(action == printer_action::kPrintTop);
  print.append(float_to_string(value, false));
  return false;
}

bool starlark_float::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
  switch (other->numeric_type()) {
    case starlark_numeric_type::kFloat:
      if (std::isnan(as_float()) && std::isnan(other->as_float())) {
        return true;
      }
      return as_float() == other->as_float();
    case starlark_numeric_type::kInt64:
      return equals_fi(as_float(), other->as_int64());
    case starlark_numeric_type::kBigInt:
      return equals_fb(as_float(), other->as_bigint());
    default:
      return false;
  }
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


