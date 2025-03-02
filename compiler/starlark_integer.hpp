// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_STARLARK_INTEGER_HPP_
#define COMPILER_STARLARK_INTEGER_HPP_

#include <string>

#include "compiler/starlark_numeric.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {

class starlark_integer : public starlark_numeric {
 public:
  starlark_integer(int64_t value);
  const std::string& type() const override;
  std::string repr() const override;
  bool truthy() const override;
  int64_t hash() const override;

 protected:
  int numeric_type() const override;
  int64_t as_int64() const override;

 private:
  int64_t value;
  static const std::string type_value;
};

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_STARLARK_INTEGER_HPP_

