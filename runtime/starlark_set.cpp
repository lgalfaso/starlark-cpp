// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_set.hpp"

#include <iterator>
#include <string>

namespace starlark {
namespace runtime {

std::string_view starlark_set::type() const {
  return "set";
}

bool starlark_set::inner_repr(printer& print, printer_action action) const {
  switch (action) {
    case printer_action::kPrintTop: {
      if (values.size() == 0) {
        print.append("set()");
        return false;
      }
      print.append("set([");
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
      print.append("])");
      return false;
    case printer_action::kPrintRecursion:
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

std::variant<int64_t, starlark_obj::pending_hash> starlark_set::inner_hash() const {
  // TODO(lmirelmann): At the moment, the understanding is that this is the right behavior.
  //   This may be revisited once we implement freeze.
  return -1;
}

starlark_set& starlark_set::add(starlark_obj* element) {
  if (element->hash() == -1) {
    // TODO(lmirelmann): Report the error.
    return *this;
  }
  values.insert(element);
  return *this;
}

}  // namespace runtime
}  // namespace starlark


