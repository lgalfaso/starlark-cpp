// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_STARLARK_FLOAT_HPP_
#define COMPILER_STARLARK_FLOAT_HPP_

#include <string>

#include "compiler/starlark_numeric.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {

class starlark_float : public starlark_numeric {
 public:
  starlark_float(double value);
  const std::string& type() const override;
  bool truthy() const override;
  int64_t hash() const override;

 protected:
  bool inner_repr(printer& print, uint64_t pos) const override;
  int numeric_type() const override;
  double as_float() const override;

 private:
  double value;
  static const std::string type_value;
};

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_STARLARK_FLOAT_HPP_

