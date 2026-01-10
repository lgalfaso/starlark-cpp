// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_string.hpp"

#include <cassert>

#include <string>

#include "bigint/number.hpp"
#include "runtime/hex_encoder.hpp"
#include "runtime/options.hpp"
#include "runtime/siphash.hpp"
#include "runtime/starlark_types.hpp"
#include "unicode/utf8_reader.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;
using ::starlark::unicode::utf8_reader;

namespace starlark {
namespace runtime {

starlark_string::starlark_string(std::string_view value) : value(value) {}

std::string_view starlark_string::type() const {
  return starlark_types::string_t;
}

bool starlark_string::primitive() const {
  return true;
}

std::string starlark_string::str() const {
  return value;
}

int64_t starlark_string::len(bool produce_error, error_fn& error_callback) const {
  return value.size();
}

bool starlark_string::inner_repr(printer& print, printer_action action) const {
  assert(action == printer_action::kPrintTop);
  // TODO(lmirelmann): If this function were to be executed a lot and were to become
  // a performance issue, then there are a few things that can be optimized:
  // - The check for `use_single_quote` can be done in one pass
  // - It should be possible to check whether the original value can be used just adding quotes
  std::string result = "\"";
  utf8_reader reader(value, false, false);
  while (reader.pending()) {
    auto code_point = reader.peek_code_point();
    if (code_point != utf8_reader::kReplacementCharacter) {
      write_printable(reader.peek_code_point(), /*allow_non_ascii_printable=*/ true, result);
      reader.skip_code_point();
    } else {
      result += value[reader.pos()];
      reader.skip();
    }
  }
  result += "\"";
  print.append(result);
  return false;
}

bool starlark_string::truthy() const {
  return !value.empty();
}

bool starlark_string::binary_in(const starlark_obj& other, error_fn& error_callback) const {
  if (other.type() != type()) {
    error_callback.add_error(std::format("TypeError: 'in <string>' requires string as left operand, not {}", other.type()));
    return false;
  }

  const starlark_string& s_other = static_cast<const starlark_string&>(other);
  return value.contains(s_other.value);
}

starlark_obj* starlark_string::binary_plus(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  if (other.type() != type()) {
    error_callback.add_error(std::format("TypeError: can't concat {} to string", other.type()));
    return nullptr;
  }
  // TODO(lmirelmann): Check that the value length would not go over the limit.
  auto* result = Arena::Create<starlark_string>(&arena, value);
  const starlark_string& b_other = static_cast<const starlark_string&>(other);
  result->value += b_other.value;
  return result;
}

starlark_obj* starlark_string::binary_star(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64: {
      if (value.empty()) {
        return Arena::Create<starlark_string>(&arena, "");
      }
      auto multiplier = other.as_int64();
      if (multiplier <= 0) {
        return Arena::Create<starlark_string>(&arena, "");
      }
      // TODO(lmirelmann): Check whether the size will be over the maximum allowed.
      auto* result = Arena::Create<starlark_string>(&arena, value);
      for (int64_t i = 1; i < multiplier; ++i) {
        result->value += value;
      }
      return result;
    }
    case starlark_numeric_type::kBigInt: {
      if (value.empty()) {
        return Arena::Create<starlark_string>(&arena, "");
      }
      const auto& multiplier = other.as_bigint();
      if (multiplier <= number::zero) {
        return Arena::Create<starlark_string>(&arena, "");
      }
      if (multiplier.bit_size() >= 63) {
        error_callback.add_error(std::format("TypeError: sequences must be at most {} elements", max_string_length()));
        return nullptr;
      }
      int64_t int_value = multiplier.at(0);
      // TODO(lmirelmann): Check whether the size will be over the maximum allowed.
      auto* result = Arena::Create<starlark_string>(&arena, value);
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

starlark_obj* starlark_string::binary_percent(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string::index(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  auto idx = inner_index(other, value.size(), error_callback);
  if (idx < 0) {
    return nullptr;
  }
  return Arena::Create<starlark_string>(&arena, value.substr(idx, 1));
}

std::string_view starlark_string::as_string() const {
  return value;
}

bool starlark_string::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
  return type() == other->type() &&
         value == other->as_string();
}

void starlark_string::inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, error_fn& error_callback) const {
  if (other->type() != type()) {
    starlark_obj::inner_cmp(comp, other, op, error_callback);
    return;
  }
  auto result = value <=> static_cast<const starlark_string*>(other)->value;
  if (result != 0) {
    comp.add_task(result > 0 ? order_comparator::pending_task_type::kGreaterThan : order_comparator::pending_task_type::kLessThan);
  }
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_string::inner_hash() const {
  if (value.length() == 0) {
    return 0;
  }
  return static_cast<int64_t>(siphash(value.data(), value.length(), 0x243F6A8885A308D3, 0x13198A2E03707344));
}

/*
TODO: Implement the following methods:

capitalize
codepoint_ords
codepoints
count
elem_ords
elems
endswith
find
format
index
isalnum
isalpha
isdigit
islower
isspace
istitle
isupper
join
lower
lstrip
partition
replace
removeprefix
removesuffix
rfind
rindex
rpartition
rsplit
rstrip
split
splitlines
startswith
strip
title
upper
*/

}  // namespace runtime
}  // namespace starlark


