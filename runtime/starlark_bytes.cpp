// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_bytes.hpp"

#include <cassert>

#include <string>

#include "runtime/hex_encoder.hpp"
#include "runtime/siphash.hpp"

namespace starlark {
namespace runtime {

starlark_bytes::starlark_bytes(std::string_view value) : value(value) {}

std::string_view starlark_bytes::type() const {
  return "bytes";
}

bool starlark_bytes::inner_repr(printer& print, printer_action action) const {
  assert(action == printer_action::kPrintTop);
  std::string result = "b";
  bool use_single_quote = !value.contains('\'') || value.contains('"');
  if (use_single_quote) {
    result += "'";
  } else {
    result += "\"";
  }
  for (unsigned char c : value) {
    write_printable(c, use_single_quote, /*allow_non_ascii_printable=*/ false, result);
  }
  if (use_single_quote) {
    result += "'";
  } else {
    result += "\"";
  }
  print.append(result);
  return false;
}

bool starlark_bytes::truthy() const {
  return !value.empty();
}

bool starlark_bytes::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
  return type() == other->type() &&
      value == (static_cast<const starlark_bytes*>(other))->value;
}

void starlark_bytes::inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, error_fn* error_callback) const {
  if (other->type() != type()) {
    starlark_obj::inner_cmp(comp, other, op, error_callback);
    return;
  }
  auto result = value <=> static_cast<const starlark_bytes*>(other)->value;
  if (result != 0) {
    comp.add_task(order_comparator::pending_task{
      .type = result > 0 ? order_comparator::pending_task_type::kGreaterThan : order_comparator::pending_task_type::kLessThan,
    });
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


