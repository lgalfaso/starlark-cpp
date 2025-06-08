// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_STARLARK_NUMERIC_HPP_
#define COMPILER_STARLARK_NUMERIC_HPP_

#include "bigint/number.hpp"
#include "compiler/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {

enum class starlark_numeric_type {
  type_int64,
  type_bigint,
  type_float
};

class starlark_numeric : public starlark_obj {
 protected:
  bool inner_equals(comparator& comp, const starlark_obj* other) const override;
  virtual starlark_numeric_type numeric_type() const = 0;
  virtual int64_t as_int64() const;
  virtual const starlark::bigint::number& as_bigint() const;
  virtual double as_float() const;

};

starlark::bigint::number from_int64(int64_t value);

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_STARLARK_NUMERIC_HPP_

