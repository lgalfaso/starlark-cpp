// Copyright 2025 Lucas Mirelmann

#include "compiler/starlark_list.hpp"

#include <string>

namespace starlark {
namespace compiler {

const std::string starlark_list::type_value = "list";

const std::string& starlark_list::type() const {
  return type_value;
}

bool starlark_list::inner_repr(printer& print, uint64_t pos) const {
  if (value.size() == 0) {
    print.append("[]");
    return false;
  }
  if (pos == 0) {
    print.append("[");
  } else if (pos == value.size()) {
    print.append("]");
    return false;
  } else {
    print.append(", ");
  }
  print.add_task(printer::pending_task{
    .obj = this,
    .pos = pos + 1,
  });
  print.add_task(printer::pending_task{
    .obj = value[pos],
    .pos = 0,
  });
  return true;
}

bool starlark_list::truthy() const {
  return !value.empty();
}

bool starlark_list::equals(const starlark_obj& other) const {
  // TODO(lmirelmann): Implement.
  return false;
}

int64_t starlark_list::hash() const {
  // TODO(lmirelmann): Implement.
  return -1;
}

starlark_list& starlark_list::add(starlark_obj* element) {
  value.push_back(element);
  return *this;
}

}  // namespace compiler
}  // namespace starlark


