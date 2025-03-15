// Copyright 2025 Lucas Mirelmann

#include "compiler/starlark_set.hpp"

#include <iterator>
#include <string>

namespace starlark {
namespace compiler {

std::string_view starlark_set::type() const {
  return "set";
}

bool starlark_set::inner_repr(printer& print, printer_action action) const {
  switch (action) {
    case printer_action::print_top: {
      if (values.size() == 0) {
        print.append("set()");
        return false;
      }
      print.append("set([");
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
    case printer_action::print_in_element_separator:
      print.append(", ");
      return true;
    case printer_action::print_final:
      print.append("])");
      return false;
    case printer_action::print_recursion:
      // Not entirelly sure whether it is possible to trigger this path within Starlark.
      print.append("set([...])");
      return false;
  }
}

bool starlark_set::truthy() const {
  return !values.empty();
}

bool starlark_set::contains(starlark_obj* obj) const {
  return values.contains(obj);
}

bool starlark_set::inner_equals(comparator& comp, const starlark_obj* other) const {
  if (type() != other->type()) {
    return false;
  }
  const starlark_set* n_other = reinterpret_cast<const starlark_set*>(other);
  if (values.size() != n_other->values.size()) {
    return false;
  }
  for (const auto& element : values) {
    // TODO(lmirelmann): Implement without recursion.
    if (!n_other->contains(element)) {
      return false;
    }
  }
  return true;
}

int64_t starlark_set::hash() const {
  // TODO(lmirelmann): Implement once we implement freeze.
  return -1;
}

starlark_set& starlark_set::add(starlark_obj* element) {
  if (element->hash() == -1) {
    // TODO(lmirelmann): Handle this case.
    return *this;
  }
  values.insert(element);
  return *this;
}

}  // namespace compiler
}  // namespace starlark


