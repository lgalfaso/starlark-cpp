// Copyright 2025 Lucas Mirelmann

#include "compiler/starlark_tuple.hpp"

#include <string>

namespace starlark {
namespace compiler {

std::string_view starlark_tuple::type() const {
  return "tuple";
}

bool starlark_tuple::inner_repr(printer& print, printer_action action) const {
  if (values.size() == 0) {
    print.append("()");
    return false;
  }
  switch (action) {
    case printer_action::print_top: {
      print.append("(");
      printer_action new_action = values.size() == 1 ? printer_action::print_single_element_final : printer_action::print_final;
      for (auto it = values.rbegin(); it != values.rend(); ++it) {
        print.add_task(printer::pending_task{
          .obj = this,
          .action = new_action,
        });
        print.add_task(printer::pending_task{
          .obj = *it,
          .action = printer_action::print_top,
        });
        new_action = printer_action::print_element_separator;
      }
      return true;
    }
    case printer_action::print_element_separator:
      print.append(", ");
      return true;
    case printer_action::print_in_element_separator:
      print.append(": ");
      return true;
    case printer_action::print_final:
      print.append(")");
      return false;
    case printer_action::print_single_element_final:
      print.append(",)");
      return false;
    case printer_action::print_recursion:
      print.append("(...)");
      return false;
  }
}

bool starlark_tuple::truthy() const {
  return !values.empty();
}

bool starlark_tuple::equals(const starlark_obj& other) const {
  // TODO(lmirelmann): Implement.
  return false;
}

int64_t starlark_tuple::hash() const {
  // TODO(lmirelmann): Implement.
  return 0;
}

starlark_tuple& starlark_tuple::add(starlark_obj* element) {
  values.push_back(element);
  return *this;
}

}  // namespace compiler
}  // namespace starlark


