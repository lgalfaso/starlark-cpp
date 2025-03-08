// Copyright 2025 Lucas Mirelmann

#include "compiler/starlark_set.hpp"

#include <iterator>
#include <string>

namespace starlark {
namespace compiler {

const std::string starlark_set::type_value = "set";

const std::string& starlark_set::type() const {
  return type_value;
}

bool starlark_set::inner_repr(printer& print, uint64_t pos) const {
  if (value.size() == 0) {
    print.append("set()");
    return false;
  }
  if (pos == 0) {
    // There is no cheap way to go to a specific element, so we add
    // all the entries in one go.
    print.append("set([");
    uint64_t element_pos = value.size();
    for (auto it = value.rbegin(); it != value.rend(); ++it) {
      print.add_task(printer::pending_task{
        .obj = this, 
        .pos = element_pos,
      });
      print.add_task(printer::pending_task{
        .obj = *it,
        .pos = 0,
      });
      element_pos--;
    }
    return true;
  } else if (pos == value.size()) {
    print.append("])");
    return false;
  } else {
    print.append(", ");
    return true;
  }
}

bool starlark_set::truthy() const {
  return !value.empty();
}

bool starlark_set::equals(const starlark_obj& other) const {
  // TODO(lmirelmann): Implement.
  return false;
}

int64_t starlark_set::hash() const {
  // TODO(lmirelmann): Implement.
  return -1;
}

starlark_set& starlark_set::add(starlark_obj* element) {
  if (element->hash() == -1) {
    // TODO(lmirelmann): Handle this case.
    return *this;
  }
  value.insert(element);
  return *this;
}

}  // namespace compiler
}  // namespace starlark


