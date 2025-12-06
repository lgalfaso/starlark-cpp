// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_list.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include "runtime/starlark_integer.hpp"
#include "grammar/options.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;
using ::starlark::grammar::max_sequence_size;

namespace starlark {
namespace runtime {

starlark_list::starlark_list(std::size_t reserve_size) {
  values.reserve(reserve_size);
}

std::string_view starlark_list::type() const {
  return "list";
}

bool starlark_list::inner_repr(printer& print, printer_action action) const {
  switch (action) {
    case printer_action::kPrintTop: {
      if (values.size() == 0) {
        print.append("[]");
        return false;
      }
      print.append("[");
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
      print.append("]");
      return false;
    case printer_action::kPrintRecursion:
      print.append("[...]");
      return false;
  }
}

bool starlark_list::truthy() const {
  return !values.empty();
}

void starlark_list::unpack(int32_t number_of_elements, std::vector<starlark_obj*>& consumer, error_fn* error_callback) {
  if (number_of_elements != values.size()) {
    if (error_callback != nullptr) {
      if (values.size() < number_of_elements) {
        error_callback->add_error(std::format("ValueError: not enough values to unpack (expected {}, got {})", number_of_elements, values.size()));
      } else {
        error_callback->add_error(std::format("ValueError: too many values to unpack (expected {}, got {})", number_of_elements, values.size()));
      }
    }
    return;
  }
  for (auto it = values.rbegin(); it != values.rend(); ++it) {
    consumer.push_back(*it);
  }
}

bool starlark_list::binary_in(const starlark_obj& other, error_fn* error_callback) const {
  for (const auto& element : values) {
    if (other.equals(*element)) {
      return true;
    }
  }
  return false;
}

starlark_obj* starlark_list::binary_plus(const starlark_obj& other, Arena& arena, error_fn* error_callback) const {
  if (other.type() != type()) {
    if (error_callback != nullptr) {
      error_callback->add_error(std::format("TypeError: can only concatenate list (not \"{}\") to list", other.type()));
    }
    return nullptr;
  }
  // TODO(lmirelmann): Check the result size.
  auto* result = Arena::Create<starlark_list>(&arena);
  // TODO(lmirelmann): It should be possible to insert the entire thing using one call to `std::vector::insert`, but
  // this would slightly break the fact that `add` is the only one adding elements.
  for (auto& key : values) {
    result->add(key, error_callback);
  }
  const starlark_list* l_other = static_cast<const starlark_list*>(&other);
  for (auto& key : l_other->values) {
    result->add(key, error_callback);
  }
  return result;
}

starlark_obj* starlark_list::binary_star(const starlark_obj& other, Arena& arena, error_fn* error_callback) const {
  if (other.type() != "int") {
    if (error_callback != nullptr) {
      error_callback->add_error(std::format("TypeError: can't multiply sequence by non-int of type '{}'", other.type()));
    }
    return nullptr;
  }
  if (values.empty()) {
    return Arena::Create<starlark_list>(&arena);
  }
  const starlark_numeric* n_other = static_cast<const starlark_numeric*>(&other);
  if (n_other->numeric_type() == starlark_numeric_type::kInt64) {
    auto value = n_other->as_int64();
    if (value <= 0) {
      return Arena::Create<starlark_list>(&arena);
    }
    // TODO(lmirelmann): Check whether the size will be over the maximum allowed.
    auto* result = Arena::Create<starlark_list>(&arena);
    for (int64_t i = 0; i < value; ++i) {
      for (auto& key : values) {
        result->add(key, error_callback);
      }
    }
    return result;
  } else if (n_other->numeric_type() == starlark_numeric_type::kBigInt) {
    const auto& value = n_other->as_bigint();
    if (value <= number::zero) {
      return Arena::Create<starlark_list>(&arena);
    }
    if (value.bit_size() >= 63) {
      if (error_callback != nullptr) {
        error_callback->add_error(std::format("TypeError: sequences must be at most {} elements", max_sequence_size()));
      }
      return nullptr;
    }
    int64_t int_value = value.at(0);
    // TODO(lmirelmann): Check whether the size will be over the maximum allowed.
    auto* result = Arena::Create<starlark_list>(&arena);
    for (int64_t i = 0; i < int_value; ++i) {
      for (auto& key : values) {
        result->add(key, error_callback);
      }
    }
    return result;
  } else {
    // Should not happen.
    assert(false);
    if (error_callback != nullptr) {
      error_callback->add_error("TypeError: unknown numeric type");
    }
    return nullptr;
  }
}

bool starlark_list::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
  if (type() != other->type()) {
    return false;
  }
  const starlark_list* n_other = reinterpret_cast<const starlark_list*>(other);
  if (values.size() != n_other->values.size()) {
    return false;
  }
  for (int i = 0; i < values.size(); ++i) {
    comp.add_task(equals_comparator::pending_task{
      .lhs = values[i],
      .rhs = n_other->values[i],
    });
  }
  return true;
}

void starlark_list::inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, error_fn* error_callback) const {
  if (other->type() != type()) {
    starlark_obj::inner_cmp(comp, other, op, error_callback);
    return;
  }
  const auto* l_other = static_cast<const starlark_list*>(other);
  if (values.size() != l_other->values.size()) {
    comp.add_task(order_comparator::pending_task{
      .type = values.size() > l_other->values.size() ? order_comparator::pending_task_type::kGreaterThan : order_comparator::pending_task_type::kLessThan,
    });
  }
  for (int i = std::min(values.size(), l_other->values.size()) - 1; i >= 0; --i) {
    comp.add_task(order_comparator::pending_task{
      .type = order_comparator::pending_task_type::kEvaluate,
      .lhs = values[i],
      .rhs = l_other->values[i],
    });
  }
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_list::inner_hash() const {
  // My current understanding is that this is the right behavior.
  return -1;
}

void starlark_list::inner_freeze(std::vector<starlark_obj*>& to_freeze) {
  for (auto* element : values) {
    to_freeze.push_back(element);
  }
}

void starlark_list::add(starlark_obj* element, error_fn* error_callback) {
  if (freezed) {
    if (error_callback != nullptr) {
      // This error does not exists in Python, so using a mix of the Python error type and Bazel message.
      error_callback->add_error(std::format("TypeError: trying to mutate a frozen {} value", type()));
    }
    return;
  }
  // TODO(lmirelmann): Check that this does not go over the maximum number of elements.
  values.push_back(element);
}

}  // namespace runtime
}  // namespace starlark


