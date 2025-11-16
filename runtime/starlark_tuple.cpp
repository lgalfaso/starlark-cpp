// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_tuple.hpp"

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

bool starlark_tuple::inner_equals(comparator& comp, const starlark_obj* other) const {
  if (type() != other->type()) {
    return false;
  }
  const starlark_tuple* n_other = reinterpret_cast<const starlark_tuple*>(other);
  if (values.size() != n_other->values.size()) {
    return false;
  }
  for (int i = 0; i < values.size(); ++i) {
    comp.add_task(comparator::pending_task{
      .lhs = values[i],
      .rhs = n_other->values[i],
    });
  }
  return true;
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

std::variant<int64_t, starlark_obj::pending_hash> starlark_tuple::inner_hash() const {
  return std::span(values.begin(), values.end());
}

starlark_tuple& starlark_tuple::add(starlark_obj* element) {
  values.push_back(element);
  return *this;
}

}  // namespace runtime
}  // namespace starlark


