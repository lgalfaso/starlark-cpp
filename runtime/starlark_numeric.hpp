// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_NUMERIC_HPP_
#define RUNTIME_STARLARK_NUMERIC_HPP_

#include "bigint/number.hpp"
#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

enum class starlark_numeric_type {
  kInt64,
  kBigInt,
  kFloat
};

class starlark_numeric : public starlark_obj {
 public:
  virtual starlark_numeric_type numeric_type() const = 0;
  virtual int64_t as_int64() const;
  virtual const starlark::bigint::number& as_bigint() const;
  virtual double as_float() const;

 protected:
  bool inner_equals(equals_comparator& comp, const starlark_obj* other) const override;
  void inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, error_fn* error_callback) const override;
};

starlark::bigint::number from_int64(int64_t value);

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_NUMERIC_HPP_

