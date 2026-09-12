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
  bool truthy() const override;
  bool primitive() const override;
  starlark_obj* unary_plus(context& ctx, error_fn& error_callback) const override;
  starlark_obj* unary_minus(context& ctx, error_fn& error_callback) const override;
  starlark_obj* unary_tilde(context& ctx, error_fn& error_callback) const override;
  starlark_obj* binary_lshift(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* binary_rshift(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* binary_and(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* binary_pipe(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* binary_hat(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* binary_plus(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* binary_minus(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* binary_star(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* binary_slash(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* binary_slash_slash(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* binary_percent(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* plus_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) override;
  starlark_obj* minus_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) override;
  starlark_obj* star_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) override;
  starlark_obj* slash_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) override;
  starlark_obj* slash_slash_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) override;
  starlark_obj* percent_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) override;
  starlark_obj* ampersand_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) override;
  starlark_obj* pipe_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) override;
  starlark_obj* hat_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) override;
  starlark_obj* less_less_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) override;
  starlark_obj* greater_greater_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) override;

  const starlark::bigint::number& as_bigint() const override;

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(equals_comparator& comp, const starlark_obj* other) const override;
  void inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, bool extended, error_fn& error_callback) const override;
  std::variant<int64_t, pending_hash> inner_hash() const override;

 private:
  starlark::bigint::number value;
};

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_BIGINT_HPP_

