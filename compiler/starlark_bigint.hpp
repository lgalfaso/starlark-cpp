// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_STARLARK_BIGINT_HPP_
#define COMPILER_STARLARK_BIGINT_HPP_

#include <string>

#include "bigint/number.hpp"
#include "compiler/starlark_numeric.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {

class starlark_bigint : public starlark_numeric {
 public:
  starlark_bigint(int64_t value);
  starlark_bigint(const starlark::bigint::number& value);
  const std::string& type() const override;
  bool truthy() const override;
  int64_t hash() const override;

 protected:
  bool inner_repr(printer& print, uint64_t pos) const override;
  int numeric_type() const override;
  const starlark::bigint::number& as_bigint() const override;

 private:
  starlark::bigint::number value;
  static const std::string type_value;
};

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_STARLARK_BIGINT_HPP_

