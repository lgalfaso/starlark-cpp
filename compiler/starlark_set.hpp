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
  const std::string& type() const override;
  bool truthy() const override;
  bool equals(const starlark_obj& other) const override;
  int64_t hash() const override;
  starlark_set& add(starlark_obj* element);

 protected:
  bool inner_repr(printer& print, uint64_t pos) const override;

 private:
  starlark::cnt::linked_hash_set<starlark_obj*, starlark_hash, starlark_equals_to> value;
  static const std::string type_value;
};

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_STARLARK_SET_HPP_

