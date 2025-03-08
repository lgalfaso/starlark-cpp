// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_STARLARK_STRING_HPP_
#define COMPILER_STARLARK_STRING_HPP_

#include <string>

#include "compiler/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {

class starlark_string : public starlark_obj {
 public:
  explicit starlark_string(const std::string& value);
  std::string_view type() const override;
  std::string str() const override;
  bool truthy() const override;
  bool equals(const starlark_obj& other) const override;
  int64_t hash() const override;

 protected:
  bool inner_repr(printer& print, printer_action action) const override;

 private:
  std::string value;
};

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_STARLARK_STRING_HPP_

