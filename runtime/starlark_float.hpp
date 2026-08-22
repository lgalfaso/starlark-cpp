// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_FLOAT_HPP_
#define RUNTIME_STARLARK_FLOAT_HPP_

#include <string>

#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

class starlark_float : public starlark_obj {
 public:
  explicit starlark_float(double value);
  std::string_view type() const override;
  bool truthy() const override;
  bool primitive() const override;
  starlark::result::status_or<int> cmp(const starlark_obj& other, std::string_view op, error_fn& error_callback) const override;
  starlark_obj* unary_plus(context& ctx, error_fn& error_callback) const override;
  starlark_obj* unary_minus(context& ctx, error_fn& error_callback) const override;
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

  starlark_numeric_type numeric_type() const override;
  double as_float() const override;

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(equals_comparator& comp, const starlark_obj* other) const override;
  void inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, bool extended, error_fn& error_callback) const override;
  std::variant<int64_t, pending_hash> inner_hash() const override;

 private:
  double value;
};

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_FLOAT_HPP_

