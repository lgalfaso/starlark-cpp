// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_dictionary.hpp"

#include <format>
#include <string>

namespace starlark {
namespace runtime {

std::string_view starlark_dictionary::type() const {
  return "dict";
}

bool starlark_dictionary::inner_repr(printer& print, printer_action action) const {
  switch (action) {
    case printer_action::kPrintTop: {
      if (values.size() == 0) {
        print.append("{}");
        return false;
      }
      print.append("{");
      printer_action new_action = printer_action::kPrintFinal;
      for (auto it = values.rbegin(); it != values.rend(); ++it) {
        print.add_task(printer::pending_task{
          .obj = this,
          .action = new_action,
        });
        print.add_task(printer::pending_task{
          .obj = it->second,
          .action = printer_action::kPrintTop,
        });
        print.add_task(printer::pending_task{
          .obj = this,
          .action = printer_action::kPrintInElementSeparator,
        });
        print.add_task(printer::pending_task{
          .obj = it->first,
          .action = printer_action::kPrintTop,
        });
        new_action = printer_action::kPrintElementSeparator;
      }
      return true;
    }
    case printer_action::kPrintElementSeparator:
      print.append(", ");
      return true;
    case printer_action::kPrintInElementSeparator:
      print.append(": ");
      return true;
    case printer_action::kPrintFinal:
      print.append("}");
      return false;
    case printer_action::kPrintRecursion:
    default:
      print.append("{...}");
      return false;
  }
}

bool starlark_dictionary::truthy() const {
  return !values.empty();
}

bool starlark_dictionary::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
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
    comp.add_task(equals_comparator::pending_task{
      .lhs = element.second,
      .rhs = other_element->second,
    });
  }
  return true;
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_dictionary::inner_hash() const {
  // My current understanding is that this is the right behavior.
  return -1;
}

bool starlark_dictionary::insert(starlark_obj* key, starlark_obj* value, error_fn* error_callback) {
  if (freezed) {
    if (error_callback != nullptr) {
      // This error does not exists in Python, so using a mix of the Python error type and Bazel message.
      error_callback->add_error(std::format("TypeError: trying to mutate a frozen {} value", type()));
    }
    return false;
  }
  if (key->hash() == -1) {
    if (error_callback != nullptr) {
      // TODO(lmirelmann): Would be nice to add the line number and position.
      error_callback->add_error(std::format("TypeError: cannot use '{}' as a dict key (unhashable type: '{}')", key->type(), key->type()));
    }
    return false;
  }
  auto [it, result] = values.insert(key, value);
  return result;
}

}  // namespace runtime
}  // namespace starlark


