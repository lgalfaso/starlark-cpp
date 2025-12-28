// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_dictionary.hpp"

#include <format>
#include <string>

#include "runtime/starlark_types.hpp"

using ::google::protobuf::Arena;

namespace starlark {
namespace runtime {

starlark_dictionary::starlark_dictionary() : iterators_count(0) {}

std::string_view starlark_dictionary::type() const {
  return starlark_types::dict_t;
}

int64_t starlark_dictionary::len(error_fn& error_callback) const {
  return values.size();
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

bool starlark_dictionary::binary_in(const starlark_obj& other, error_fn& error_callback) const {
  // The const_cast is needed as there is no conversion from `const starlark_obj *const` to `starlark_obj *const`.
  return values.contains(&const_cast<starlark_obj&>(other));
}

starlark_obj* starlark_dictionary::binary_pipe(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  if (other.type() != type()) {
    return starlark_obj::binary_pipe(other, arena, error_callback);
  }
  auto* result = Arena::Create<starlark_dictionary>(&arena);
  for (const auto& [key, value] : values) {
    result->insert(key, value, error_callback);
  }
  const starlark_dictionary* d_other = static_cast<const starlark_dictionary*>(&other);
  for (auto& [key, value] : d_other->values) {
    result->insert(key, value, error_callback);
  }
  return result;
}

starlark_iterator* starlark_dictionary::get_iterator(Arena& arena, error_fn& error_callback) {
  return Arena::Create<starlark_dictionary_iterator>(&arena, this);
}

starlark_obj* starlark_dictionary::index(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  auto result = values.find(&const_cast<starlark_obj&>(other));
  if (result == values.end()) {
    error_callback.add_error(std::format("KeyError: {}", other.repr()));
    return nullptr;
  }
  return result->second;
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

void starlark_dictionary::inner_freeze(std::vector<starlark_obj*>& to_freeze) {
  for (auto& [k, v] : values) {
    to_freeze.push_back(k);
    to_freeze.push_back(v);
  }
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_dictionary::inner_hash() const {
  // My current understanding is that this is the right behavior.
  return -1;
}

bool starlark_dictionary::insert(starlark_obj* key, starlark_obj* value, error_fn& error_callback) {
  if (iterators_count) {
    error_callback.add_error("Error in append: dictionary value is temporarily immutable due to active for-loop iteration");
    return false;
  }
  if (freezed) {
    // This error does not exists in Python, so using a mix of the Python error type and Bazel message.
    error_callback.add_error(std::format("TypeError: trying to mutate a frozen {} value", type()));
    return false;
  }
  if (key->hash() == -1) {
    error_callback.add_error(std::format("TypeError: cannot use '{}' as a dict key (unhashable type: '{}')", key->type(), key->type()));
    return false;
  }
  auto [it, result] = values.insert(key, value);
  return result;
}

starlark_dictionary::starlark_dictionary_iterator::starlark_dictionary_iterator(starlark_dictionary* dictionary) : dictionary(dictionary), it(dictionary->values.begin()) {
  dictionary->iterators_count++;
}

bool starlark_dictionary::starlark_dictionary_iterator::has_next() const {
  return it != dictionary->values.end();
}

starlark_obj* starlark_dictionary::starlark_dictionary_iterator::next() {
  return (it++)->first;
}

void starlark_dictionary::starlark_dictionary_iterator::end_iterator() {
  dictionary->iterators_count--;
}

}  // namespace runtime
}  // namespace starlark


