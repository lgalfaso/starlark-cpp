// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_BYTECODE_HPP_
#define COMPILER_BYTECODE_HPP_

#include <string>

#include "bigint/number.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {

class bytecode {
 public:
  virtual void run() = 0;
};

// TODO(lmirelmann): Frame.

// TODO(lmirelmann): The program should allocate the frame using an arena.
class program {
};

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_BYTECODE_HPP_

