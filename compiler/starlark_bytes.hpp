// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_STARLARK_BYTES_HPP_
#define COMPILER_STARLARK_BYTES_HPP_

#include <string>

#include "compiler/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {

class starlark_bytes : public starlark_obj {
 public:
  starlark_bytes(const std::string& value);
  const std::string& type() const override;
  bool truthy() const override;
  bool equals(const starlark_obj& other) const override;
  int64_t hash() const override;

 protected:
  bool inner_repr(printer& print, uint64_t pos) const override;

 private:
  std::string value;
  static const std::string type_value;
};

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_STARLARK_BYTES_HPP_

