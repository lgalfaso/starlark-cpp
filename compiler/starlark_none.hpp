// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_STARLARK_NONE_HPP_
#define COMPILER_STARLARK_NONE_HPP_

#include <string>

#include "compiler/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {

// TODO(lmirelmann): Split between the different starlark objects.

class starlark_none : public starlark_obj {
 public:
  const std::string& type() const override;
  std::string repr() const override;
  bool truthy() const override;
  bool equals(const starlark_obj& other) const override;
  int64_t hash() const override;

 private:
  static const std::string type_value;
};

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_STARLARK_NONE_HPP_

