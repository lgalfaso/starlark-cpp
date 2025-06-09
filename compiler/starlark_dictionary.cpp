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

bool starlark_dictionary::inner_equals(comparator& comp, const starlark_obj* other) const {
  if (type() != other->type()) {
    return false;
  }
  const starlark_dictionary* n_other = reinterpret_cast<const starlark_dictionary*>(other);
  if (values.size() != n_other->values.size()) {
    return false;
  }
  for (const auto& element : values) {
    auto other_element = n_other->values.find(element.first);
    if (other_element == n_other->values.end()) {
      return false;
    }
    comp.add_task(comparator::pending_task{
      .lhs = element.second,
      .rhs = other_element->second,
    });
  }
  return true;
}

int64_t starlark_dictionary::hash() const {
  // TODO(lmirelmann): At the moment, the understanding is that this is the right behavior.
  //   This may be revisited once we implement freeze.
  return -1;
}

starlark_dictionary& starlark_dictionary::insert(starlark_obj* key, starlark_obj* value) {
  if (key->hash() == -1) {
    // TODO(lmirelmann): Report the error.
    return *this;
  }
  values.insert(key, value);
  return *this;
}

}  // namespace compiler
}  // namespace starlark


