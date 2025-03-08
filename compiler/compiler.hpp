// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_COMPILER_HPP_
#define COMPILER_COMPILER_HPP_

#include <string_view>

#include "compiler/bytecode.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {

class compiler {
 public:
  explicit compiler(std::string_view starlark_program);
  program compile();

 private:
  std::string_view starlark_program;
};

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_COMPILER_HPP_

