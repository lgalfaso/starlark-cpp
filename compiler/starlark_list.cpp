// Copyright 2025 Lucas Mirelmann

#include "compiler/starlark_list.hpp"

#include <string>

namespace starlark {
namespace compiler {

std::string_view starlark_list::type() const {
  return "list";
}

bool starlark_list::inner_repr(printer& print, printer_action action) const {
  switch (action) {
    case printer_action::print_top: {
      if (values.size() == 0) {
        print.append("[]");
        return false;
      }
      print.append("[");
      printer_action new_action = printer_action::print_final;
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
      return true;
    case printer_action::print_final:
      print.append("]");
      return false;
    case printer_action::print_recursion:
      print.append("[...]");
      return false;
  }
}

bool starlark_list::truthy() const {
  return !values.empty();
}

bool starlark_list::equals(const starlark_obj& other) const {
  // TODO(lmirelmann): Implement.
  return false;
}

int64_t starlark_list::hash() const {
  // TODO(lmirelmann): Implement.
  return -1;
}

starlark_list& starlark_list::add(starlark_obj* element) {
  values.push_back(element);
  return *this;
}

}  // namespace compiler
}  // namespace starlark


