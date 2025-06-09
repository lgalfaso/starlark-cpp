// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_STARLARK_LIST_HPP_
#define COMPILER_STARLARK_LIST_HPP_

#include <string>
#include <vector>

#include "compiler/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {

class starlark_list : public starlark_obj {
 public:
  std::string_view type() const override;
  starlark_list& add(starlark_obj* element);
  bool truthy() const override;

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(comparator& comp, const starlark_obj* other) const override;
  std::variant<int64_t, pending_hash> inner_hash() const override;

 private:
  std::vector<starlark_obj*> values;
};

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_STARLARK_LIST_HPP_

