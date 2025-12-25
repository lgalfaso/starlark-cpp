// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_bigint.hpp"

#include <cassert>

#include <string>

#include "runtime/hash.hpp"
#include "runtime/options.hpp"
#include "runtime/starlark_numeric.hpp"
#include "runtime/starlark_types.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;

namespace starlark {
namespace runtime {

starlark_bigint::starlark_bigint(int64_t value) : value(from_int64(value)) {}

starlark_bigint::starlark_bigint(const starlark::bigint::number& value) : value(value) {}

std::string_view starlark_bigint::type() const {
  return starlark_types::int_t;
}

bool starlark_bigint::inner_repr(printer& print, printer_action action) const {
  assert(action == printer_action::kPrintTop);
  print.append(value.to_string(10));
  return false;
}

bool starlark_bigint::truthy() const {
  return value != bigint::number::zero;
}

starlark_obj* starlark_bigint::unary_plus(Arena& arena, error_fn& error_callback) const {
  return const_cast<starlark_bigint*>(this);
}

starlark_obj* starlark_bigint::unary_minus(Arena& arena, error_fn& error_callback) const {
  return create_integer(-value, arena);
}

starlark_obj* starlark_bigint::unary_tilde(Arena& arena, error_fn& error_callback) const {
  // TODO(lmirelmann): Figure out whether it is possible to reuse `this`.
  return create_integer(-(value + number::one), arena);
}

starlark_obj* starlark_bigint::binary_lshift(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  if (other.type() != type()) {
    return starlark_obj::binary_lshift(other, arena, error_callback);
  }
  if (value == number::zero) {
    return const_cast<starlark_bigint*>(this);
  }
  const starlark_numeric& n_other = static_cast<const starlark_numeric&>(other);
  if (n_other.numeric_type() == starlark_numeric_type::kInt64) {
    auto shift = n_other.as_int64();
    if (shift < 0) {
      error_callback.add_error("ValueError: negative shift count");
      return nullptr;
    }
    if (log2_max_bigint() < 64 - std::countl_zero<uint64_t>(shift) + value.bit_size()) {
      error_callback.add_error("OverflowError: too many digits in integer");
      return nullptr;
    }
    return create_integer(value << shift, arena);
  } else if (n_other.numeric_type() == starlark_numeric_type::kBigInt) {
    const auto& shift = n_other.as_bigint();
    if (shift.sign()) {
      error_callback.add_error("ValueError: negative shift count");
      return nullptr;
    }
    if (shift.length() > 1) {
      error_callback.add_error("OverflowError: too many digits in integer");
      return nullptr;
    }
    auto int_shift = shift.at(0);
    if (log2_max_bigint() < 64 - std::countl_zero<uint64_t>(int_shift) + value.bit_size()) {
      error_callback.add_error("OverflowError: too many digits in integer");
      return nullptr;
    }
    return create_integer(value << int_shift, arena);
  } else {
    // Should not happen.
    assert(false);
    error_callback.add_error("RuntimeError: unexpected number type");
    return nullptr;
  }
}

starlark_obj* starlark_bigint::binary_rshift(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  if (other.type() != type()) {
    return starlark_obj::binary_rshift(other, arena, error_callback);
  }
  if (value == number::zero) {
    return const_cast<starlark_bigint*>(this);
  }
  const starlark_numeric& n_other = static_cast<const starlark_numeric&>(other);
  if (n_other.numeric_type() == starlark_numeric_type::kInt64) {
    auto shift = n_other.as_int64();
    if (shift < 0) {
      error_callback.add_error("ValueError: negative shift count");
      return nullptr;
    }
    if (shift >= value.bit_size()) {
      return create_integer(value.sign() ? -1 : 0, arena);
    }
    return create_integer(value >> shift, arena);
  } else if (n_other.numeric_type() == starlark_numeric_type::kBigInt) {
    const auto& shift = n_other.as_bigint();
    if (shift.sign()) {
      error_callback.add_error("ValueError: negative shift count");
      return nullptr;
    }
    if (shift.length() > 1) {
      return create_integer(value.sign() ? -1 : 0, arena);
    }
    auto int_shift = shift.at(0);
    if (int_shift >= value.bit_size()) {
      return create_integer(value.sign() ? -1 : 0, arena);
    }
    return create_integer(value >> int_shift, arena);
  } else {
    // Should not happen.
    assert(false);
    error_callback.add_error("RuntimeError: unexpected number type");
    return nullptr;
  }
}

starlark_obj* starlark_bigint::binary_and(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  if (other.type() != type()) {
    return starlark_obj::binary_and(other, arena, error_callback);
  }
  const starlark_numeric& n_other = static_cast<const starlark_numeric&>(other);
  if (n_other.numeric_type() == starlark_numeric_type::kInt64) {
    return create_integer(value & from_int64(n_other.as_int64()), arena);
  } else if (n_other.numeric_type() == starlark_numeric_type::kBigInt) {
    return create_integer(value & n_other.as_bigint(), arena);
  } else {
    // Should not happen.
    assert(false);
    error_callback.add_error("RuntimeError: unexpected number type");
    return nullptr;
  }
}

starlark_obj* starlark_bigint::binary_pipe(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  if (other.type() != type()) {
    return starlark_obj::binary_pipe(other, arena, error_callback);
  }
  const starlark_numeric& n_other = static_cast<const starlark_numeric&>(other);
  if (n_other.numeric_type() == starlark_numeric_type::kInt64) {
    return create_integer(value | from_int64(n_other.as_int64()), arena);
  } else if (n_other.numeric_type() == starlark_numeric_type::kBigInt) {
    return create_integer(value | n_other.as_bigint(), arena);
  } else {
    // Should not happen.
    assert(false);
    error_callback.add_error("RuntimeError: unexpected number type");
    return nullptr;
  }
}

starlark_obj* starlark_bigint::binary_hat(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  if (other.type() != type()) {
    return starlark_obj::binary_hat(other, arena, error_callback);
  }
  const starlark_numeric& n_other = static_cast<const starlark_numeric&>(other);
  if (n_other.numeric_type() == starlark_numeric_type::kInt64) {
    return create_integer(value ^ from_int64(n_other.as_int64()), arena);
  } else if (n_other.numeric_type() == starlark_numeric_type::kBigInt) {
    return create_integer(value ^ n_other.as_bigint(), arena);
  } else {
    // Should not happen.
    assert(false);
    error_callback.add_error("RuntimeError: unexpected number type");
    return nullptr;
  }
}

starlark_obj* starlark_bigint::binary_plus(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  if (other.type() != starlark_types::float_t && other.type() != type()) {
    return starlark_obj::binary_plus(other, arena, error_callback);
  }
  auto* n_other = static_cast<const starlark_numeric*>(&other);
  switch (n_other->numeric_type()) {
    case starlark_numeric_type::kFloat: {
      auto fvalue = to_double(value);
      if (std::isinf(fvalue)) {
        error_callback.add_error("OverflowError: int too large to convert to float");
        return nullptr;
      }
      return create_float(fvalue + n_other->as_float(), arena);
    }
    case starlark_numeric_type::kInt64:
      return create_integer(value + from_int64(n_other->as_int64()), arena);
    case starlark_numeric_type::kBigInt:
      return create_integer(value + n_other->as_bigint(), arena);
  }
}

starlark_obj* starlark_bigint::binary_minus(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  if (other.type() != starlark_types::float_t && other.type() != type()) {
    return starlark_obj::binary_minus(other, arena, error_callback);
  }
  auto* n_other = static_cast<const starlark_numeric*>(&other);
  switch (n_other->numeric_type()) {
    case starlark_numeric_type::kFloat: {
      auto fvalue = to_double(value);
      if (std::isinf(fvalue)) {
        error_callback.add_error("OverflowError: int too large to convert to float");
        return nullptr;
      }
      return create_float(fvalue - n_other->as_float(), arena);
    }
    case starlark_numeric_type::kInt64:
      return create_integer(value - from_int64(n_other->as_int64()), arena);
    case starlark_numeric_type::kBigInt:
      return create_integer(value - n_other->as_bigint(), arena);
  }
}

starlark_obj* starlark_bigint::binary_star(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  if (other.type() == starlark_types::string_t || other.type() == starlark_types::bytes_t || other.type() == starlark_types::list_t || other.type() == starlark_types::tuple_t) {
    return other.binary_star(*this, arena, error_callback);
  }
  if (other.type() != starlark_types::float_t && other.type() != type()) {
    return starlark_obj::binary_star(other, arena, error_callback);
  }
  auto* n_other = static_cast<const starlark_numeric*>(&other);
  switch (n_other->numeric_type()) {
    case starlark_numeric_type::kFloat: {
      auto fvalue = to_double(value);
      if (std::isinf(fvalue)) {
        error_callback.add_error("OverflowError: int too large to convert to float");
        return nullptr;
      }
      return create_float(fvalue * n_other->as_float(), arena);
    }
    case starlark_numeric_type::kInt64:
      return create_integer(value * from_int64(n_other->as_int64()), arena);
    case starlark_numeric_type::kBigInt:
      return create_integer(value * n_other->as_bigint(), arena);
  }
}

starlark_obj* starlark_bigint::binary_slash(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  if (other.type() != starlark_types::float_t && other.type() != type()) {
    return starlark_obj::binary_slash(other, arena, error_callback);
  }
  auto* n_other = static_cast<const starlark_numeric*>(&other);
  auto fvalue = to_double(value);
  if (std::isinf(fvalue)) {
    error_callback.add_error("OverflowError: int too large to convert to float");
    return nullptr;
  }
  switch (n_other->numeric_type()) {
    case starlark_numeric_type::kFloat: {
      auto fother = n_other->as_float();
      if (fother == 0) {
        error_callback.add_error("ZeroDivisionError: division by zero");
        return nullptr;
      }
      return create_float(fvalue / fother, arena);
    }
    case starlark_numeric_type::kInt64: {
      auto iother = n_other->as_int64();
      if (iother == 0) {
        error_callback.add_error("ZeroDivisionError: division by zero");
        return nullptr;
      }
      return create_float(fvalue / iother, arena);
    }
    case starlark_numeric_type::kBigInt: {
      auto fother = to_double(n_other->as_bigint());
      if (fother == 0) {
        error_callback.add_error("ZeroDivisionError: division by zero");
        return nullptr;
      }
      if (std::isinf(fother)) {
        error_callback.add_error("OverflowError: int too large to convert to float");
        return nullptr;
      }
      return create_float(fvalue / fother, arena);
    }
  }
}

starlark_obj* starlark_bigint::binary_slash_slash(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  if (other.type() != starlark_types::float_t && other.type() != type()) {
    return starlark_obj::binary_slash_slash(other, arena, error_callback);
  }
  auto* n_other = static_cast<const starlark_numeric*>(&other);
  switch (n_other->numeric_type()) {
    case starlark_numeric_type::kFloat: {
      auto fvalue = to_double(value);
      if (std::isinf(fvalue)) {
        error_callback.add_error("OverflowError: int too large to convert to float");
        return nullptr;
      }
      auto fother = n_other->as_float();
      if (fother == 0) {
        error_callback.add_error("ZeroDivisionError: division by zero");
        return nullptr;
      }
      return create_float(std::floor(fvalue / fother), arena);
    }
    case starlark_numeric_type::kInt64: {
      auto iother = n_other->as_int64();
      if (iother == 0) {
        error_callback.add_error("ZeroDivisionError: division by zero");
        return nullptr;
      }
      return create_integer(starlark_div(value, from_int64(iother)), arena);
    }
    case starlark_numeric_type::kBigInt: {
      const auto& bother = n_other->as_bigint();
      if (bother == number::zero) {
        error_callback.add_error("ZeroDivisionError: division by zero");
        return nullptr;
      }
      return create_integer(starlark_div(value, bother), arena);
    }
  }
}

starlark_obj* starlark_bigint::binary_percent(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  if (other.type() != starlark_types::float_t && other.type() != type()) {
    return starlark_obj::binary_percent(other, arena, error_callback);
  }
  auto* n_other = static_cast<const starlark_numeric*>(&other);
  switch (n_other->numeric_type()) {
    case starlark_numeric_type::kFloat: {
      auto fvalue = to_double(value);
      if (std::isinf(fvalue)) {
        error_callback.add_error("OverflowError: int too large to convert to float");
        return nullptr;
      }
      auto fother = n_other->as_float();
      if (fother == 0) {
        error_callback.add_error("ZeroDivisionError: division by zero");
        return nullptr;
      }
      return create_float(starlark_fmod(fvalue, fother), arena);
    }
    case starlark_numeric_type::kInt64: {
      auto iother = n_other->as_int64();
      if (iother == 0) {
        error_callback.add_error("ZeroDivisionError: division by zero");
        return nullptr;
      }
      return create_integer(starlark_mod(value, from_int64(iother)), arena);
    }
    case starlark_numeric_type::kBigInt: {
      const auto& bother = n_other->as_bigint();
      if (bother == number::zero) {
        error_callback.add_error("ZeroDivisionError: division by zero");
        return nullptr;
      }
      return create_integer(starlark_mod(value, bother), arena);
    }
  }
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

const starlark::bigint::number& starlark_bigint::as_bigint() const {
  return value;
}

}  // namespace runtime
}  // namespace starlark


