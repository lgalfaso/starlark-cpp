// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_bytes.hpp"

#include <cassert>

#include <string>

#include "runtime/hex_encoder.hpp"
#include "runtime/options.hpp"
#include "runtime/siphash.hpp"
#include "runtime/starlark_types.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;

namespace starlark {
namespace runtime {

starlark_bytes::starlark_bytes(std::string_view value) : value(value) {}

std::string_view starlark_bytes::type() const {
  return starlark_types::bytes_t;
}

bool starlark_bytes::primitive() const {
  return true;
}

int64_t starlark_bytes::len(error_fn& error_callback) const {
  return value.size();
}

bool starlark_bytes::inner_repr(printer& print, printer_action action) const {
  assert(action == printer_action::kPrintTop);
  std::string result = "b\"";
  for (unsigned char c : value) {
    write_printable(c, /*allow_non_ascii_printable=*/ false, result);
  }
  result += "\"";
  print.append(result);
  return false;
}

bool starlark_bytes::truthy() const {
  return !value.empty();
}

bool starlark_bytes::binary_in(const starlark_obj& other, error_fn& error_callback) const {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64: {
      auto other_value = other.as_int64();
      if (other_value < 0 || 255 < other_value) {
        error_callback.add_error("ValueError: byte must be in range(0, 256)");
        return false;
      }
      return value.contains(static_cast<char>(other.as_int64()));
    }
    case starlark_numeric_type::kBigInt: {
      auto& other_value = other.as_bigint();
      if (other_value.sign() || other_value.bit_size() >= 8) {
        error_callback.add_error("ValueError: byte must be in range(0, 256)");
        return false;
      }
      return value.contains(static_cast<char>(other_value.at(0)));
    }
    case starlark_numeric_type::kNotNumeric: {
      if (other.type() != type()) {
        error_callback.add_error(std::format("TypeError: a bytes-like object is required, not '{}'", other.type()));
        return false;
      }
      const starlark_bytes& s_other = static_cast<const starlark_bytes&>(other);
      return value.contains(s_other.value);
    }
    default:
     error_callback.add_error(std::format("TypeError: a bytes-like object is required, not '{}'", other.type()));
     return false;
  }
}

starlark_obj* starlark_bytes::binary_plus(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  if (other.type() != type()) {
    error_callback.add_error(std::format("TypeError: can't concat {} to bytes", other.type()));
    return nullptr;
  }
  // TODO(lmirelmann): Check that the value length would not go over the limit.
  auto* result = Arena::Create<starlark_bytes>(&arena, value);
  const starlark_bytes* b_other = static_cast<const starlark_bytes*>(&other);
  result->value += b_other->value;
  return result;
}

starlark_obj* starlark_bytes::binary_star(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64: {
      if (value.empty()) {
        return Arena::Create<starlark_bytes>(&arena, "");
      }
      auto multiplier = other.as_int64();
      if (multiplier <= 0) {
        return Arena::Create<starlark_bytes>(&arena, "");
      }
      // TODO(lmirelmann): Check whether the size will be over the maximum allowed.
      auto* result = Arena::Create<starlark_bytes>(&arena, value);
      for (int64_t i = 1; i < multiplier; ++i) {
        result->value += value;
      }
      return result;
    }
    case starlark_numeric_type::kBigInt: {
      if (value.empty()) {
        return Arena::Create<starlark_bytes>(&arena, "");
      }
      const auto& multiplier = other.as_bigint();
      if (multiplier <= number::zero) {
        return Arena::Create<starlark_bytes>(&arena, "");
      }
      if (multiplier.bit_size() >= 63) {
        error_callback.add_error(std::format("TypeError: sequences must be at most {} elements", max_string_length()));
        return nullptr;
      }
      int64_t int_value = multiplier.at(0);
      // TODO(lmirelmann): Check whether the size will be over the maximum allowed.
      auto* result = Arena::Create<starlark_bytes>(&arena, value);
      for (int64_t i = 1; i < int_value; ++i) {
        result->value += value;
      }
      return result;
    }
    default:
      error_callback.add_error(std::format("TypeError: can't multiply sequence by non-int of type '{}'", other.type()));
      return nullptr;
  }
}

starlark_obj* starlark_bytes::index(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  auto idx = inner_index(other, value.size(), error_callback);
  if (idx < 0) {
    return nullptr;
  }
  return Arena::Create<starlark_bytes>(&arena, value.substr(idx, 1));
}

bool starlark_bytes::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
  return type() == other->type() &&
      value == (static_cast<const starlark_bytes*>(other))->value;
}

void starlark_bytes::inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, error_fn& error_callback) const {
  if (other->type() != type()) {
    starlark_obj::inner_cmp(comp, other, op, error_callback);
    return;
  }
  auto result = value <=> static_cast<const starlark_bytes*>(other)->value;
  if (result != 0) {
    comp.add_task(result > 0 ? order_comparator::pending_task_type::kGreaterThan : order_comparator::pending_task_type::kLessThan);
  }
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_bytes::inner_hash() const {
  if (value.length() == 0) {
    return 0;
  }
  return static_cast<int64_t>(siphash(value.data(), value.length(), 0x0001020304050607, 0x08090a0b0c0d0e0f));
}

}  // namespace runtime
}  // namespace starlark


