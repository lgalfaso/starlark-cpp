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
  explicit starlark_bool(bool value);
  std::string_view type() const override;
  bool truthy() const override;
  int64_t hash() const override;

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(comparator& comp, const starlark_obj* other) const override;

 private:
  bool value;
};

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_STARLARK_BOOL_HPP_

