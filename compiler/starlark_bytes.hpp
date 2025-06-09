// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_STARLARK_BYTES_HPP_
#define COMPILER_STARLARK_BYTES_HPP_

#include <string>

#include "compiler/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {

class starlark_bytes : public starlark_obj {
 public:
  explicit starlark_bytes(const std::string& value);
  std::string_view type() const override;
  bool truthy() const override;

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(comparator& comp, const starlark_obj* other) const override;
  std::variant<int64_t, pending_hash> inner_hash() const override;

 private:
  std::string value;
};

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_STARLARK_BYTES_HPP_

