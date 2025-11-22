// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_STRING_HPP_
#define RUNTIME_STARLARK_STRING_HPP_

#include <string>
#include <string_view>

#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

class starlark_string : public starlark_obj {
 public:
  explicit starlark_string(std::string_view value);
  std::string_view type() const override;
  std::string str() const override;
  bool truthy() const override;

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(equals_comparator& comp, const starlark_obj* other) const override;
  std::variant<int64_t, pending_hash> inner_hash() const override;

 private:
  std::string value;
};

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_STRING_HPP_

