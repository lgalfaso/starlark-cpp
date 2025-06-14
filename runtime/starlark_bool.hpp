// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_BOOL_HPP_
#define RUNTIME_STARLARK_BOOL_HPP_

#include <string>

#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

class starlark_bool : public starlark_obj {
 public:
  explicit starlark_bool(bool value);
  std::string_view type() const override;
  bool truthy() const override;

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(comparator& comp, const starlark_obj* other) const override;
  std::variant<int64_t, pending_hash> inner_hash() const override;

 private:
  bool value;
};

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_BOOL_HPP_

