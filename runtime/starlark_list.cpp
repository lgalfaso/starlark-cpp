// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_list.hpp"

#include <string>

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

std::variant<int64_t, starlark_obj::pending_hash> starlark_list::inner_hash() const {
  // TODO(lmirelmann): Implement, the spec states that the object is hashable if:
  // - If is freezed
  // - All entries are freezed
  return -1;
}

void starlark_list::add(starlark_obj* element) {
  // TODO(lmirelmann): If this is freezed, then this is an error.
  values.push_back(element);
}

}  // namespace runtime
}  // namespace starlark


