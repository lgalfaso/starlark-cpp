// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_STARLARK_NUMERIC_HPP_
#define COMPILER_STARLARK_NUMERIC_HPP_

#include "bigint/number.hpp"
#include "compiler/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {

class starlark_numeric : public starlark_obj {
 protected:
  bool inner_equals(comparator& comp, const starlark_obj* other) const override;
  virtual int numeric_type() const = 0;
  virtual int64_t as_int64() const;
  virtual const starlark::bigint::number& as_bigint() const;
  virtual double as_float() const;

  // TODO(lmirelmann): Make this an enum class.
  static const int type_int64 = 1;
  static const int type_bigint = 2;
  static const int type_float = 3;
};

starlark::bigint::number from_int64(int64_t value);

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_STARLARK_NUMERIC_HPP_

