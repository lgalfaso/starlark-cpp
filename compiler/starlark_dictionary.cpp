// Copyright 2025 Lucas Mirelmann

#include "compiler/starlark_dictionary.hpp"

#include <string>

namespace starlark {
namespace compiler {

std::string_view starlark_dictionary::type() const {
  return "dict";
}

bool starlark_dictionary::inner_repr(printer& print, printer_action action) const {
  switch (action) {
    case printer_action::print_top: {
      if (values.size() == 0) {
        print.append("{}");
        return false;
      }
      print.append("{");
      printer_action new_action = printer_action::print_final;
      for (auto it = values.rbegin(); it != values.rend(); ++it) {
        print.add_task(printer::pending_task{
          .obj = this,
          .action = new_action,
        });
        print.add_task(printer::pending_task{
          .obj = it->second,
          .action = printer_action::print_top,
        });
        print.add_task(printer::pending_task{
          .obj = this,
          .action = printer_action::print_in_element_separator,
        });
        print.add_task(printer::pending_task{
          .obj = it->first,
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
      print.append("}");
      return false;
    case printer_action::print_recursion:
      print.append("{...}");
      return false;
  }
}

bool starlark_dictionary::truthy() const {
  return !values.empty();
}

bool starlark_dictionary::equals(const starlark_obj& other) const {
  // TODO(lmirelmann): Implement.
  return false;
}

int64_t starlark_dictionary::hash() const {
  // TODO(lmirelmann): Implement.
  return -1;
}

starlark_dictionary& starlark_dictionary::insert(starlark_obj* key, starlark_obj* value) {
  if (key->hash() == -1) {
    // TODO(lmirelmann): Handle this case.
    return *this;
  }
  values.insert(key, value);
  return *this;
}

}  // namespace compiler
}  // namespace starlark


