// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_set.hpp"

#include <iterator>
#include <string>

#include "runtime/starlark_types.hpp"

using ::google::protobuf::Arena;

namespace starlark {
namespace runtime {

starlark_set::starlark_set() : iterators_count(0) {}

std::string_view starlark_set::type() const {
  return starlark_types::set_t;
}

int64_t starlark_set::len(bool produce_error, error_fn& error_callback) const {
  return values.size();
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
      // This is only possible with some extension like `struct` that is hashable even when its elements are not.
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

bool starlark_set::binary_in(const starlark_obj& other, error_fn& error_callback) const {
  // The const_cast is needed as there is no conversion from `const starlark_obj *const` to `starlark_obj *const`
  return values.contains(&const_cast<starlark_obj&>(other));
}

starlark_obj* starlark_set::binary_and(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  if (other.type() != type()) {
    return starlark_obj::binary_and(other, arena, error_callback);
  }
  auto* result = Arena::Create<starlark_set>(&arena);
  const starlark_set& s_other = static_cast<const starlark_set&>(other);
  for (auto& key : values) {
    if (s_other.contains(key)) {
      result->add(key, error_callback);
    }
  }
  return result;
}

starlark_obj* starlark_set::binary_pipe(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  if (other.type() != type()) {
    return starlark_obj::binary_pipe(other, arena, error_callback);
  }
  auto* result = Arena::Create<starlark_set>(&arena);
  for (auto& key : values) {
    result->add(key, error_callback);
  }
  const starlark_set& s_other = static_cast<const starlark_set&>(other);
  for (auto& key : s_other.values) {
    result->add(key, error_callback);
  }
  return result;
}

starlark_obj* starlark_set::binary_hat(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  if (other.type() != type()) {
    return starlark_obj::binary_hat(other, arena, error_callback);
  }
  auto* result = Arena::Create<starlark_set>(&arena);
  const starlark_set& s_other = static_cast<const starlark_set&>(other);
  for (auto& key : values) {
    if (!s_other.values.contains(key)) {
      result->add(key, error_callback);
    }
  }
  for (auto& key : s_other.values) {
    if (!values.contains(key)) {
      result->add(key, error_callback);
    }
  }
  return result;
}

starlark_obj* starlark_set::binary_minus(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  if (other.type() != type()) {
    return starlark_obj::binary_minus(other, arena, error_callback);
  }
  auto* result = Arena::Create<starlark_set>(&arena);
  const starlark_set& s_other = static_cast<const starlark_set&>(other);
  for (auto& key : values) {
    if (!s_other.values.contains(key)) {
      result->add(key, error_callback);
    }
  }
  return result;
}

starlark_iterator* starlark_set::get_iterator(bool produce_error, Arena& arena, error_fn& error_callback) {
  return Arena::Create<starlark_set_iterator>(&arena, this);
}

bool starlark_set::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
  if (type() != other->type()) {
    return false;
  }
  const starlark_set* n_other = static_cast<const starlark_set*>(other);
  if (values.size() != n_other->values.size()) {
    return false;
  }
  for (const auto& element : values) {
    // This will not unboundly recurse as the element is hashable.
    if (!n_other->contains(element)) {
      return false;
    }
  }
  return true;
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_set::inner_hash() const {
  // My current understanding is that this is the right behavior.
  return -1;
}

void starlark_set::inner_freeze(std::vector<starlark_obj*>& to_freeze) {
  for (auto& element : values) {
    to_freeze.push_back(element);
  }
}

std::pair<bool, bool> starlark_set::add(starlark_obj* element, error_fn& error_callback) {
  if (iterators_count) {
    error_callback.add_error("Error in append: set value is temporarily immutable due to active for-loop iteration");
    return std::make_pair(false, true);
  }
  if (freezed) {
    // This error does not exists in Python, so using a mix of the Python error type and Bazel message.
    error_callback.add_error(std::format("TypeError: trying to mutate a frozen {} value", type()));
    return std::make_pair(false, true);
  }
  if (element->hash() == -1) {
    error_callback.add_error(std::format("TypeError: cannot use '{}' as a set element (unhashable type: '{}')", element->type(), element->type()));
    return std::make_pair(false, true);
  }
  return std::make_pair(values.insert(element).second, false);
}

starlark_set::starlark_set_iterator::starlark_set_iterator(starlark_set* set) : set(set), it(set->values.begin()) {
  set->iterators_count++;
}

bool starlark_set::starlark_set_iterator::has_next() const {
  return it != set->values.end();
}

starlark_obj* starlark_set::starlark_set_iterator::next() {
  return *it++;
}

void starlark_set::starlark_set_iterator::end_iterator() {
  set->iterators_count--;
}

/*
TODO(lmirelmann): Implement the following methods:

add
clear
difference
difference_update
discard
intersection
intersection_update
isdisjoint
issubset
issuperset
pop
remove
symmetric_difference
symmetric_difference_update
union
update
*/

}  // namespace runtime
}  // namespace starlark


