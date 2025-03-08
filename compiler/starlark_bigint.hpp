// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_STARLARK_BIGINT_HPP_
#define COMPILER_STARLARK_BIGINT_HPP_

#include <string_view>

#include "bigint/number.hpp"
#include "compiler/starlark_numeric.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {

class starlark_bigint : public starlark_numeric {
 public:
  explicit starlark_bigint(int64_t value);
  explicit starlark_bigint(const starlark::bigint::number& value);
  std::string_view type() const override;
  bool truthy() const override;
  int64_t hash() const override;

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  int numeric_type() const override;
  const starlark::bigint::number& as_bigint() const override;

 private:
  starlark::bigint::number value;
};

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_STARLARK_BIGINT_HPP_

