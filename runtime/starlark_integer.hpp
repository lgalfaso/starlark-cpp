// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_INTEGER_HPP_
#define RUNTIME_STARLARK_INTEGER_HPP_

#include <string>

#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

class starlark_integer : public starlark_obj {
 public:
  explicit starlark_integer(int64_t value);
  std::string_view type() const override;
  bool truthy() const override;
  bool primitive() const override;
  starlark_obj* unary_plus(google::protobuf::Arena& arena, error_fn& error_callback) const override;
  starlark_obj* unary_minus(google::protobuf::Arena& arena, error_fn& error_callback) const override;
  starlark_obj* unary_tilde(google::protobuf::Arena& arena, error_fn& error_callback) const override;
  starlark_obj* binary_lshift(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;
  starlark_obj* binary_rshift(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;
  starlark_obj* binary_and(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;
  starlark_obj* binary_pipe(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;
  starlark_obj* binary_hat(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;
  starlark_obj* binary_plus(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;
  starlark_obj* binary_minus(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;
  starlark_obj* binary_star(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;
  starlark_obj* binary_slash(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;
  starlark_obj* binary_slash_slash(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;
  starlark_obj* binary_percent(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;

  starlark_numeric_type numeric_type() const override;
  int64_t as_int64() const override;

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(equals_comparator& comp, const starlark_obj* other) const override;
  void inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, error_fn& error_callback) const override;
  std::variant<int64_t, pending_hash> inner_hash() const override;

 private:
  int64_t value;
};

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_INTEGER_HPP_

