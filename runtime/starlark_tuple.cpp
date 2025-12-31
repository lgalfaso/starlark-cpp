// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_tuple.hpp"

#include <algorithm>
#include <bit>
#include <string>
#include <vector>

#include "runtime/options.hpp"
#include "runtime/starlark_bigint.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_types.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;

namespace starlark {
namespace runtime {

starlark_tuple::starlark_tuple(int reserve_size) {
  values.reserve(reserve_size);
}

std::string_view starlark_tuple::type() const {
  return starlark_types::tuple_t;
}

starlark_obj* starlark_tuple::binary_plus(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  if (other.type() != type()) {
    error_callback.add_error(std::format("TypeError: can only concatenate tuple (not \"{}\") to tuple", other.type()));
    return nullptr;
  }
  // TODO(lmirelmann): Check the result size.
  auto* result = Arena::Create<starlark_tuple>(&arena);
  for (auto& key : values) {
    result->add(key);
  }
  const starlark_tuple* t_other = static_cast<const starlark_tuple*>(&other);
  for (auto& key : t_other->values) {
    result->add(key);
  }
  return result;
}

starlark_obj* starlark_tuple::binary_star(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64: {
      if (values.empty()) {
        return Arena::Create<starlark_tuple>(&arena);
      }
      auto value = other.as_int64();
      if (value <= 0) {
        return Arena::Create<starlark_tuple>(&arena);
      }
      // TODO(lmirelmann): Check whether the size will be over the maximum allowed.
      auto* result = Arena::Create<starlark_tuple>(&arena);
      for (int64_t i = 0; i < value; ++i) {
        for (auto& key : values) {
          result->add(key);
        }
      }
      return result;
    }
    case starlark_numeric_type::kBigInt: {
      if (values.empty()) {
        return Arena::Create<starlark_tuple>(&arena);
      }
      const auto& value = other.as_bigint();
      if (value <= number::zero) {
        return Arena::Create<starlark_tuple>(&arena);
      }
      if (value.bit_size() >= 63) {
        error_callback.add_error(std::format("TypeError: sequences must be at most {} elements", max_sequence_size()));
        return nullptr;
      }
      int64_t int_value = value.at(0);
      // TODO(lmirelmann): Check whether the size will be over the maximum allowed.
      auto* result = Arena::Create<starlark_tuple>(&arena);
      for (int64_t i = 0; i < int_value; ++i) {
        for (auto& key : values) {
          result->add(key);
        }
      }
      return result;
    }
    default:
      error_callback.add_error(std::format("TypeError: can't multiply sequence by non-int of type '{}'", other.type()));
      return nullptr;
  }
}

int64_t starlark_tuple::len(bool produce_error, error_fn& error_callback) const {
  return values.size();
}

bool starlark_tuple::inner_repr(printer& print, printer_action action) const {
  switch (action) {
    case printer_action::kPrintTop: {
      if (values.size() == 0) {
        print.append("()");
        return false;
      }
      print.append("(");
      printer_action new_action = printer_action::kPrintFinal;
      for (auto it = values.rbegin(); it != values.rend(); ++it) {
        print.add_task(printer::pending_task{
          .obj = this,
          .action = new_action,
        });
        print.add_task(printer::pending_task{
          .obj = *it,
          .action = printer_action::kPrintTop,
        });
        new_action = printer_action::kPrintElementSeparator;
      }
      return true;
    }
    case printer_action::kPrintElementSeparator:
    case printer_action::kPrintInElementSeparator:
      print.append(", ");
      return true;
    case printer_action::kPrintFinal:
      if (values.size() == 1) {
        print.append(",)");
      } else {
        print.append(")");
      }
      return false;
    case printer_action::kPrintRecursion:
      print.append("(...)");
      return false;
  }
}

bool starlark_tuple::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
  if (type() != other->type()) {
    return false;
  }
  const starlark_tuple* t_other = reinterpret_cast<const starlark_tuple*>(other);
  if (values.size() != t_other->values.size()) {
    return false;
  }
  for (int i = 0; i < values.size(); ++i) {
    comp.add_task(equals_comparator::pending_task{
      .lhs = values[i],
      .rhs = t_other->values[i],
    });
  }
  return true;
}

void starlark_tuple::inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, error_fn& error_callback) const {
  if (other->type() != type()) {
    starlark_obj::inner_cmp(comp, other, op, error_callback);
    return;
  }
  const auto* t_other = static_cast<const starlark_tuple*>(other);
  if (values.size() != t_other->values.size()) {
    comp.add_task(values.size() > t_other->values.size() ? order_comparator::pending_task_type::kGreaterThan : order_comparator::pending_task_type::kLessThan);
  }
  for (int i = std::min(values.size(), t_other->values.size()) - 1; i >= 0; --i) {
    comp.add_task(values[i], t_other->values[i]);
  }
}

bool starlark_tuple::truthy() const {
  return !values.empty();
}

void starlark_tuple::unpack(int32_t number_of_elements, std::vector<starlark_obj*>& consumer, error_fn& error_callback) {
  if (number_of_elements != values.size()) {
    if (values.size() < number_of_elements) {
      error_callback.add_error(std::format("ValueError: not enough values to unpack (expected {}, got {})", number_of_elements, values.size()));
    } else {
      error_callback.add_error(std::format("ValueError: too many values to unpack (expected {}, got {})", number_of_elements, values.size()));
    }
    return;
  }

  for (auto it = values.rbegin(); it != values.rend(); ++it) {
    consumer.push_back(*it);
  }
}

bool starlark_tuple::binary_in(const starlark_obj& other, error_fn& error_callback) const {
  for (const auto& element : values) {
    if (other.equals(*element)) {
      return true;
    }
  }
  return false;
}

starlark_iterator* starlark_tuple::get_iterator(bool produce_error, Arena& arena, error_fn& error_callback) {
  return Arena::Create<starlark_tuple_iterator>(&arena, this);
}

starlark_obj* starlark_tuple::index(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  auto idx = inner_index(other, values.size(), error_callback);
  if (idx < 0) {
    return nullptr;
  }
  return values[idx];
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_tuple::inner_hash() const {
  return std::span(values.begin(), values.end());
}

void starlark_tuple::inner_freeze(std::vector<starlark_obj*>& to_freeze) {
  for (auto* element : values) {
    to_freeze.push_back(element);
  }
}

starlark_tuple& starlark_tuple::add(starlark_obj* element) {
  values.push_back(element);
  return *this;
}

starlark_tuple::starlark_tuple_iterator::starlark_tuple_iterator(starlark_tuple* tuple) : tuple(tuple), it(tuple->values.begin()) {}

bool starlark_tuple::starlark_tuple_iterator::has_next() const {
  return it != tuple->values.end();
}

starlark_obj* starlark_tuple::starlark_tuple_iterator::next() {
  return *it++;
}

void starlark_tuple::starlark_tuple_iterator::end_iterator() {}

}  // namespace runtime
}  // namespace starlark


