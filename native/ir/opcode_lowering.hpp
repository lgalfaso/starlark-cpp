// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_IR_OPCODE_LOWERING_HPP_
#define NATIVE_IR_OPCODE_LOWERING_HPP_

#include "native/ir/lowering_context.hpp"
#include "proto/starlark_bytecode.pb.h"

#pragma GCC visibility push(default)

namespace llvm {
class BasicBlock;
class Function;
class IRBuilderBase;
class Module;
class Value;
}  // namespace llvm

namespace starlark {
namespace native {

void lower_opcode(lowering_context& lowering,
    llvm::IRBuilderBase& builder,
    llvm::Function* fn,
    llvm::Value* exec,
    llvm::Value* ctx,
    llvm::Value* err,
    llvm::BasicBlock* error_bb,
    llvm::BasicBlock* next_bb,
    int block_idx,
    int ip,
    const starlark::bytecode::OpCode& op,
    const starlark::bytecode::Program& program);

}  // namespace native
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_IR_OPCODE_LOWERING_HPP_
