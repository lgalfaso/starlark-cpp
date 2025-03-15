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
  std::string_view type() const override;
  bool truthy() const override;
  int64_t hash() const override;
  void set_start(const starlark_obj* value);
  void set_end(const starlark_obj* value);
  void set_step(const starlark_obj* value);

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(comparator& comp, const starlark_obj* other) const override;

  uint64_t start = 0;
  uint64_t end;
  uint64_t step = 1;
  uint64_t last;
};

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_STARLARK_RANGE_HPP_

