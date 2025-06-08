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
  explicit starlark_float(double value);
  std::string_view type() const override;
  bool truthy() const override;
  int64_t hash() const override;

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  starlark_numeric_type numeric_type() const override;
  double as_float() const override;

 private:
  double value;
};

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_STARLARK_FLOAT_HPP_

