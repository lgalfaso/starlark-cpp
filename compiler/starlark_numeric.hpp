// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_STARLARK_NUMERIC_HPP_
#define COMPILER_STARLARK_NUMERIC_HPP_

#include "bignum/number.hpp"
#include "compiler/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {

class starlark_numeric : public starlark_obj {
 public:
  bool equals(const starlark_obj& other) const override;

 protected:
  virtual int numeric_type() const = 0;
  virtual int64_t as_int64() const;
  virtual const starlark::bignum::number& as_bigint() const;
  virtual double as_float() const;

  static const int type_int64 = 1;
  static const int type_bigint = 2;
  static const int type_float = 3;
};

starlark::bignum::number from_int64(int64_t value);

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_OBJECTS_HPP_

