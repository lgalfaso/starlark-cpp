// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_STARLARK_FUNCTION_HPP_
#define COMPILER_STARLARK_FUNCTION_HPP_

#include <string>

#include "compiler/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {

class starlark_function : public starlark_obj {
 public:
  std::string_view type() const override;
  bool truthy() const override;
  bool equals(const starlark_obj& other) const override;
  int64_t hash() const override;

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
};

class starlark_built_in_function : public starlark_obj {
 public:
  std::string_view type() const override;
  bool truthy() const override;
  bool equals(const starlark_obj& other) const override;
  int64_t hash() const override;

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
};

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_STARLARK_FUNCTION_HPP_

