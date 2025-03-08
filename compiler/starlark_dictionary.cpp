// Copyright 2025 Lucas Mirelmann

#include "compiler/starlark_dictionary.hpp"

#include <string>

namespace starlark {
namespace compiler {

const std::string starlark_dictionary::type_value = "dict";

const std::string& starlark_dictionary::type() const {
  return type_value;
}

bool starlark_dictionary::inner_repr(printer& print, uint64_t pos) const {
  if (values.size() == 0) {
    print.append("{}");
    return false;
  }
  if (pos == 0) {
    // There is no cheap way to go to a specific element, so we add
    // all the entries in one go.
    print.append("{");
    uint64_t element_pos = values.size();
    for (auto it = values.rbegin(); it != values.rend(); ++it) {
      print.add_task(printer::pending_task{
        .obj = this, 
        .pos = element_pos,
      });
      print.add_task(printer::pending_task{
        .obj = it->second,
        .pos = 0,
      });
      print.add_task(printer::pending_task{
        .obj = this, 
        .pos = values.size() + 1,
      });
      print.add_task(printer::pending_task{
        .obj = it->first,
        .pos = 0,
      });
      element_pos--;
    }
    return true;
  } else if (pos == values.size()) {
    print.append("}");
    return false;
  } else if (pos == values.size() + 1) {  // TODO(lmirelmann): In theory, this can be an overflow.
    print.append(": ");
    return false;
  } else {
    print.append(", ");
    return true;
  }
}

bool starlark_dictionary::truthy() const {
  return !values.empty();
}

bool starlark_dictionary::equals(const starlark_obj& other) const {
  // TODO(lmirelmann): Implement.
  return false;
}

int64_t starlark_dictionary::hash() const {
  // TODO(lmirelmann): Implement.
  return -1;
}

starlark_dictionary& starlark_dictionary::insert(starlark_obj* key, starlark_obj* value) {
  if (key->hash() == -1) {
    // TODO(lmirelmann): Handle this case.
    return *this;
  }
  values.insert(key, value);
  return *this;
}

}  // namespace compiler
}  // namespace starlark


