// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_COMPILER_HPP_
#define COMPILER_COMPILER_HPP_

#include <set>
#include <string_view>

#include "proto/starlark_bytecode.pb.h"

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {

class compiler {
 public:
  explicit compiler(std::set<std::string, std::less<>> &binding);
  // TODO(lmirelmann): Define whether this should take an Arena and return a pointer to `Program`.
  starlark::bytecode::Program compile(std::string_view starlark_program);

 private:
  const std::set<std::string, std::less<>> binding;
};

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_COMPILER_HPP_

