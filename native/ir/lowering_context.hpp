// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_IR_LOWERING_CONTEXT_HPP_
#define NATIVE_IR_LOWERING_CONTEXT_HPP_

#include <map>

#include "native/ir/ir_exec_context.hpp"
#include "native/ir/irgen_options.hpp"

#pragma GCC visibility push(default)

namespace llvm {
class Function;
class Module;
}  // namespace llvm

namespace starlark {
namespace native {

struct lowering_context {
  ir_exec_context& ir_exec;
  llvm::Module& module;
  const irgen_options& options;
  const std::map<int, llvm::Function*>& function_blocks;
};

}  // namespace native
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_IR_LOWERING_CONTEXT_HPP_
