// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_STARLARK_BOOL_HPP_
#define COMPILER_STARLARK_BOOL_HPP_

#include <string>

#include "compiler/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {

class starlark_bool : public starlark_obj {
 public:
  starlark_bool(bool value);
  const std::string& type() const override;
  std::string repr() const override;
  bool truthy() const override;
  bool equals(const starlark_obj& other) const override;
  int64_t hash() const override;

 private:
  bool value;
  static const std::string type_value;
};

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_STARLARK_BOOL_HPP_

