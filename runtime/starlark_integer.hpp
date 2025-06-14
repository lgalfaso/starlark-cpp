// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_INTEGER_HPP_
#define RUNTIME_STARLARK_INTEGER_HPP_

#include <string>

#include "runtime/starlark_numeric.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

class starlark_integer : public starlark_numeric {
 public:
  explicit starlark_integer(int64_t value);
  std::string_view type() const override;
  bool truthy() const override;

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  std::variant<int64_t, pending_hash> inner_hash() const override;
  starlark_numeric_type numeric_type() const override;
  int64_t as_int64() const override;

 private:
  int64_t value;
};

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_INTEGER_HPP_

