// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_tuple.hpp"

#include <algorithm>
#include <bit>
#include <string>
#include <vector>

namespace starlark {
namespace runtime {

starlark_tuple::starlark_tuple(int reserve_size) {
  values.reserve(reserve_size);
}

std::string_view starlark_tuple::type() const {
  return "tuple";
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
  const starlark_tuple* n_other = reinterpret_cast<const starlark_tuple*>(other);
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

void starlark_tuple::inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, error_fn* error_callback) const {
  if (other->type() != type()) {
    starlark_obj::inner_cmp(comp, other, op, error_callback);
    return;
  }
  const auto* t_other = static_cast<const starlark_tuple*>(other);
  if (values.size() != t_other->values.size()) {
    comp.add_task(order_comparator::pending_task{
      .type = values.size() > t_other->values.size() ? order_comparator::pending_task_type::kGreaterThan : order_comparator::pending_task_type::kLessThan,
    });
  }
  for (int i = std::min(values.size(), t_other->values.size()) - 1; i >= 0; --i) {
    comp.add_task(order_comparator::pending_task{
      .type = order_comparator::pending_task_type::kEvaluate,
      .lhs = values[i],
      .rhs = t_other->values[i],
    });
  }
}

bool starlark_tuple::truthy() const {
  return !values.empty();
}

void starlark_tuple::unpack(int32_t number_of_elements, std::vector<starlark_obj*>& consumer, error_fn* error_callback) {
  if (number_of_elements != values.size()) {
    if (error_callback != nullptr) {
      if (values.size() < number_of_elements) {
        error_callback->add_error(std::format("ValueError: not enough values to unpack (expected {}, got {})", number_of_elements, values.size()));
      } else {
        error_callback->add_error(std::format("ValueError: too manys values to unpack (expected {}, got {})", number_of_elements, values.size()));
      }
    }
    return;
  }

  for (auto it = values.rbegin(); it != values.rend(); ++it) {
    consumer.push_back(*it);
  }
}

bool starlark_tuple::binary_in(const starlark_obj& other, error_fn* error_callback) const {
  for (const auto& element : values) {
    if (other.equals(*element)) {
      return true;
    }
  }
  return false;
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

}  // namespace runtime
}  // namespace starlark


