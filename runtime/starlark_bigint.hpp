// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_BIGINT_HPP_
#define RUNTIME_STARLARK_BIGINT_HPP_

#include <string_view>

#include "bigint/number.hpp"
#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

class starlark_bigint : public starlark_obj {
 public:
  explicit starlark_bigint(int64_t value);
  explicit starlark_bigint(const starlark::bigint::number& value);
  std::string_view type() const override;
  bool truthy() const override;
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
  const starlark::bigint::number& as_bigint() const override;

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(equals_comparator& comp, const starlark_obj* other) const override;
  void inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, error_fn& error_callback) const override;
  std::variant<int64_t, pending_hash> inner_hash() const override;

 private:
  starlark::bigint::number value;
};

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_BIGINT_HPP_

