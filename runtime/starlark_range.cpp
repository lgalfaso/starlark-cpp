// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_range.hpp"

#include <format>
#include <string>

#include "runtime/starlark_numeric.hpp"
#include "runtime/starlark_types.hpp"

using ::google::protobuf::Arena;

namespace starlark {
namespace runtime {

namespace {

int64_t calculate_len(int64_t start, int64_t end, int64_t step) {
  assert(step != 0);
  if (step > 0 && start < end) {
    return (end - 1 - start) / step + 1;
  } else if (step < 0 && start > end)
    return (start - 1 - end) / -step + 1;
  else {
    return 0;
  }
}

}  // namespace

starlark_range::starlark_range(int64_t start, int64_t end, int64_t step) : start(start), end(end), step(step), len_(calculate_len(start, end, step)) {}

std::string_view starlark_range::type() const {
  return starlark_types::range_t;
}

int64_t starlark_range::len(bool produce_error, error_fn& error_callback) const {
  return len_;
}

bool starlark_range::inner_repr(printer& print, printer_action action) const {
  assert(action == printer_action::kPrintTop);
  if (step != 1) {
    print.append(std::format("range({}, {}, {})", start, end, step));
  } else if (start != 0) {
    print.append(std::format("range({}, {})", start, end));
  } else {
    print.append(std::format("range({})", end));
  }
  return false;
}

bool starlark_range::truthy() const {
  if (start == end) {
    return false;
  }
  if ((step > 0) ^ (start < end)) {
    return false;
  }
  return true;
}

bool starlark_range::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
  if (other->type() != starlark_types::range_t) {
    return false;
  }
  auto* rother = static_cast<const starlark_range*>(other);
  if (len_ == 0) {
    return rother->len_ == 0;
  }
  return start == rother->start && step == rother->step && len_ == rother->len_;
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_range::inner_hash() const {
  // Range is not hashable in Starlark.
  return -1;
}

bool starlark_range::binary_in(const starlark_obj& other, error_fn& error_callback) const {
  auto check = [this](int64_t value) -> bool {
    if (step > 0) {
      return start <= value && value < end && (value - start) % step == 0;
    } else {
      return value <= start && end < value && (start - value) % step == 0;
    }
  };

  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64:
      return check(other.as_int64());
    case starlark_numeric_type::kBigInt: {
      const auto& bvalue = other.as_bigint();
      if (bvalue.length() > 1) {
        return false;
      }
      auto uvalue = bvalue.at(0);
      if (bvalue.sign()) {
        if (uvalue <= static_cast<uint64_t>(std::numeric_limits<int64_t>::min())) {
          return check(-uvalue);
        } else {
          return false;
        }
      } else {
        if (uvalue <= std::numeric_limits<int64_t>::max()) {
          return check(uvalue);
        } else {
          return false;
        }
      }
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

starlark_iterator* starlark_range::get_iterator(bool produce_error, Arena& arena, error_fn& error_callback) {
  return Arena::Create<starlark_range_iterator>(&arena, start, step, len_, arena);
}

starlark_obj* starlark_range::index(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  auto idx = inner_index(other, len_, error_callback);
  if (idx < 0) {
    return nullptr;
  }
  return create_integer(start + idx * step, arena);
}

starlark_range::starlark_range_iterator::starlark_range_iterator(int64_t current_pos, int64_t step, int64_t remaining, Arena& arena)
  : current_pos(current_pos), step(step), remaining(remaining), arena(arena) {}

bool starlark_range::starlark_range_iterator::has_next() const {
  return remaining > 0;
}

starlark_obj* starlark_range::starlark_range_iterator::next() {
  auto* result = create_integer(current_pos, arena);
  current_pos += step;
  remaining--;
  return result;
}

void starlark_range::starlark_range_iterator::end_iterator() {}
 
}  // namespace runtime
}  // namespace starlark


