// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_STARLARK_SET_HPP_
#define COMPILER_STARLARK_SET_HPP_

#include <string>
#include <vector>

#include "containers/linked_hash_set.hpp"
#include "compiler/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {

class starlark_set : public starlark_obj {
 public:
  std::string_view type() const override;
  bool truthy() const override;
  starlark_set& add(starlark_obj* element);
  bool contains(starlark_obj* obj) const;

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(comparator& comp, const starlark_obj* other) const override;
  std::variant<int64_t, pending_hash> inner_hash() const override;

 private:
  starlark::cnt::linked_hash_set<starlark_obj*, starlark_hash_op, starlark_equals_to> values;
};

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_STARLARK_SET_HPP_

