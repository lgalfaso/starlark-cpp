// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_BIGINT_HPP_
#define RUNTIME_STARLARK_BIGINT_HPP_

#include <string_view>

#include "bigint/number.hpp"
#include "runtime/starlark_numeric.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

class starlark_bigint : public starlark_numeric {
 public:
  explicit starlark_bigint(int64_t value);
  explicit starlark_bigint(const starlark::bigint::number& value);
  std::string_view type() const override;
  bool truthy() const override;
  starlark_obj* unary_plus(google::protobuf::Arena& arena, error_fn* error_callback) override;
  starlark_obj* unary_minus(google::protobuf::Arena& arena, error_fn* error_callback) override;

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  std::variant<int64_t, pending_hash> inner_hash() const override;
  starlark_numeric_type numeric_type() const override;
  const starlark::bigint::number& as_bigint() const override;

 private:
  starlark::bigint::number value;
};

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_BIGINT_HPP_

