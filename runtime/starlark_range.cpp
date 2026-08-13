// Copyright 2025-2026 Lucas Mirelmann

#include "runtime/starlark_range.hpp"

#include <stdckdint.h>

#include <format>
#include <limits>
#include <string>
#include <vector>

#include "errors/runtime_error_messages.hpp"
#include "runtime/starlark_numeric.hpp"
#include "runtime/starlark_types.hpp"

using ::google::protobuf::Arena;
using ::starlark::error_messages::error_v2_unpack_too_few;
using ::starlark::error_messages::error_v2_unpack_too_many;

namespace starlark {
namespace runtime {

starlark_range::starlark_range(int64_t start, int64_t end, int64_t step) : state(calculate_state(start, end, step)) {}

std::string_view starlark_range::type() const {
  return starlark_types::range_t;
}

void starlark_range::unpack(int32_t number_of_elements, std::vector<starlark_obj*>& consumer, context& ctx, error_fn& error_callback) {
  if (number_of_elements != state.len) {
    if (state.len < number_of_elements) {
      error_callback.add_error(error_v2_unpack_too_few(state.len, number_of_elements));
    } else {
      error_callback.add_error(error_v2_unpack_too_many(state.len, number_of_elements));
    }
    return;
  }
  for (auto i = state.len; i > 0; --i) {
    consumer.push_back(create_integer(state.start + (i - 1) * state.step, ctx));
  }
}

int64_t starlark_range::unsafe_len() const {
  return state.len;
}

bool starlark_range::inner_repr(printer& print, printer_action action) const {
  assert(action == printer_action::kPrintTop);
  if (state.step != 1) {
    print.append(std::format("range({}, {}, {})", state.start, state.end, state.step));
  } else if (state.start != 0) {
    print.append(std::format("range({}, {})", state.start, state.end));
  } else {
    print.append(std::format("range({})", state.end));
  }
  return false;
}

bool starlark_range::truthy() const {
  return state.len > 0;
}

bool starlark_range::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
  if (other->type() != starlark_types::range_t) {
    return false;
  }
  auto* rother = static_cast<const starlark_range*>(other);
  if (state.len != rother->state.len) {
    return false;
  }
  if (state.len == 0) {
    return true;
  }
  if (state.start != rother->state.start) {
    return false;
  }
  if (state.len == 1) {
    return true;
  }
  return state.step == rother->state.step;
}

void starlark_range::inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, bool extended, error_fn& error_callback) const {
  if (extended && type() == other->type() && equals(*other)) {
    return;
  }
  starlark_obj::inner_cmp(comp, other, op, extended, error_callback);
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_range::inner_hash() const {
  // Range is not hashable in Starlark.
  return -1;
}

bool starlark_range::binary_in(const starlark_obj& other, error_fn& error_callback) const {
  auto check = [this](int64_t value) -> bool {
    if (state.step > 0) {
      return state.start <= value && value < state.end && (value - state.start) % state.step == 0;
    } else {
      return value <= state.start && state.end < value && (state.start - value) % state.step == 0;
    }
  };

  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64:
      return check(other.as_int64());
    case starlark_numeric_type::kBigInt: {
      const auto& bvalue = other.as_bigint();
      if (!bvalue.fits_in_int64()) {
        return false;
      }
      return check(bvalue.as_int64());
    }
    case starlark_numeric_type::kFloat: {
      // Note: There is a dicrepancy between the spec and Bazel. The spec states:
      //   The `x in y` operator, where `y` is a range, reports whether `x` is equal to
      //   some member of the sequence `y`; the operation fails unless `x` is a number.
      // A `float` is a number and given that `1 == 1.0` is `True`, then
      // `1.0 in range(100)` should also be `True`. This is the case in Python, but it
      // is `False` in Bazel. Given that there is this discrepancy, then I think that
      // the best way forward is to follow the spec as it matches Python.
      auto fvalue = other.as_float();
      if (!std::isfinite(fvalue)) {
        return false;
      }
      if (cmp_fi(fvalue, std::numeric_limits<int64_t>::min()) < 0 ||
          cmp_fi(fvalue, std::numeric_limits<int64_t>::max()) > 0) {
        return false;
      }
      double intpart;
      if (std::modf(fvalue, &intpart) != 0.0) {
        return false;
      }
      int64_t value = static_cast<int64_t>(intpart);
      return check(value);
    }
    default:
      // The spec says that this sould fail if `other` is not a number, but Bazel and Python do not fail
      // and both return `False`.
      return false;
  }
}

starlark_iterator* starlark_range::get_iterator(bool produce_error, context& ctx, error_fn& error_callback) {
  return Arena::Create<starlark_range_iterator>(&ctx.arena(), state.start, state.step, state.len, ctx);
}

starlark_obj* starlark_range::index(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  auto idx = inner_index(other, state.len, error_callback);
  if (!idx.ok()) {
    return nullptr;
  }
  return create_integer(state.start + (*idx) * state.step, ctx);
}

starlark_obj* starlark_range::slice_range(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, context& ctx, error_fn& error_callback) const {
  auto slice_result = inner_slice_range_range(start, stop, stride, state.start, state.end, state.step, state.len, error_callback);
  if (!slice_result.ok()) {
    return nullptr;
  }
  return Arena::Create<starlark_range>(&ctx.arena(), std::get<0>(*slice_result), std::get<1>(*slice_result), std::get<2>(*slice_result));
}

bool starlark_range::valid() const {
  return unsafe_len() >= 0;
}

starlark_range::starlark_range_iterator::starlark_range_iterator(int64_t current_pos, int64_t step, int64_t remaining, context& ctx)
  : current_pos(current_pos), step(step), remaining(remaining), ctx(ctx) {}

bool starlark_range::starlark_range_iterator::has_next() const {
  return remaining > 0;
}

starlark_obj* starlark_range::starlark_range_iterator::next() {
  auto* result = create_integer(current_pos, ctx);
  current_pos += step;
  remaining--;
  return result;
}

void starlark_range::starlark_range_iterator::end_iterator() {}

range_state calculate_state(int64_t start, int64_t end, int64_t step) {
  return range_state{
    .start = start,
    .end = end,
    .step = step,
    .len = calculate_len(start, end, step),
  };
}

}  // namespace runtime
}  // namespace starlark


