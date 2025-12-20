// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_FLOAT_HPP_
#define RUNTIME_STARLARK_FLOAT_HPP_

#include <string>

#include "runtime/starlark_numeric.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

class starlark_float : public starlark_numeric {
 public:
  explicit starlark_float(double value);
  std::string_view type() const override;
  bool truthy() const override;
  starlark_obj* unary_plus(google::protobuf::Arena& arena, error_fn& error_callback) const override;
  starlark_obj* unary_minus(google::protobuf::Arena& arena, error_fn& error_callback) const override;
  starlark_obj* binary_plus(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;
  starlark_obj* binary_minus(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;
  starlark_obj* binary_star(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;
  starlark_obj* binary_slash(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;
  starlark_obj* binary_slash_slash(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;
  starlark_obj* binary_percent(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  std::variant<int64_t, pending_hash> inner_hash() const override;
  starlark_numeric_type numeric_type() const override;
  double as_float() const override;

 private:
  double value;
};

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_FLOAT_HPP_

