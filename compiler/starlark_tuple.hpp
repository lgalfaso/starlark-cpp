// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_STARLARK_TUPLE_HPP_
#define COMPILER_STARLARK_TUPLE_HPP_

#include <string>
#include <vector>

#include "compiler/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {

class starlark_tuple : public starlark_obj {
 public:
  const std::string& type() const override;
  starlark_tuple& add(starlark_obj* element);
  bool truthy() const override;
  bool equals(const starlark_obj& other) const override;
  int64_t hash() const override;

 protected:
  bool inner_repr(printer& print, uint64_t pos) const override;

 private:
  std::vector<starlark_obj*> value;
  static const std::string type_value;
};

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_STARLARK_TUPLE_HPP_

