// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_STARLARK_RANGE_HPP_
#define COMPILER_STARLARK_RANGE_HPP_

#include <string>

#include "compiler/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {

class starlark_range : public starlark_obj {
 public:
  const std::string& type() const override;
  bool truthy() const override;
  bool equals(const starlark_obj& other) const override;
  int64_t hash() const override;

 protected:
  bool inner_repr(printer& print, uint64_t pos) const override;

 private:
  static const std::string type_value;
};

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_STARLARK_RANGE_HPP_

