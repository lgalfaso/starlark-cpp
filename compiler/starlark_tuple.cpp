// Copyright 2025 Lucas Mirelmann

#include "compiler/starlark_tuple.hpp"

#include <string>

namespace starlark {
namespace compiler {

const std::string starlark_tuple::type_value = "tuple";

const std::string& starlark_tuple::type() const {
  return type_value;
}

bool starlark_tuple::inner_repr(printer& print, uint64_t pos) const {
  if (value.size() == 0) {
    print.append("()");
    return false;
  }
  if (pos == 0) {
    print.append("(");
  } else if (pos == 1 && pos == value.size()) {
    print.append(",)");
    return false;
  } else if (pos == value.size()) {
    print.append(")");
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

bool starlark_tuple::truthy() const {
  return !value.empty();
}

bool starlark_tuple::equals(const starlark_obj& other) const {
  // TODO(lmirelmann): Implement.
  return false;
}

int64_t starlark_tuple::hash() const {
  // TODO(lmirelmann): Implement.
  return 0;
}

starlark_tuple& starlark_tuple::add(starlark_obj* element) {
  value.push_back(element);
  return *this;
}

}  // namespace compiler
}  // namespace starlark


