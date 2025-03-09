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

bool starlark_list::inner_equals(comparator& comp, const starlark_obj* other) const {
  if (type() != other->type()) {
    return false;
  }
  const starlark_list* n_other = reinterpret_cast<const starlark_list*>(other);
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

int64_t starlark_list::hash() const {
  // TODO(lmirelmann): Implement, the spec states that the object is hashable if:
  // - If is freezed
  // - All entries are freezed
  return -1;
}

starlark_list& starlark_list::add(starlark_obj* element) {
  values.push_back(element);
  return *this;
}

}  // namespace compiler
}  // namespace starlark


