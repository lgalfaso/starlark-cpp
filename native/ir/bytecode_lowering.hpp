// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_BYTECODE_LOWERING_HPP_
#define NATIVE_BYTECODE_LOWERING_HPP_

#include <functional>
#include <map>

#include "native/ir/ir_exec_context.hpp"
#include "native/ir/irgen_options.hpp"
#include "proto/starlark_bytecode.pb.h"

#pragma GCC visibility push(default)

namespace llvm {
class BasicBlock;
class Function;
class IRBuilderBase;
class LLVMContext;
class Module;
class PointerType;
class Type;
class Value;
}  // namespace llvm

namespace starlark {
namespace native {

class bytecode_lowering {
 public:
  bytecode_lowering(llvm::LLVMContext& context, llvm::Module& module, const starlark::bytecode::Program& program, const irgen_options& options);

  llvm::Function* lower_module_init();
  void declare_function_block(int block_idx);
  llvm::Function* lower_function_block(int block_idx);

 private:
  void emit_opcode(llvm::Function* fn,
      llvm::BasicBlock* bb,
      llvm::BasicBlock* error_bb,
      llvm::BasicBlock* next_bb,
      int block_idx,
      int ip,
      llvm::Value* exec);
  void declare_runtime_functions();
  llvm::Value* emit_truthy_pop(llvm::IRBuilderBase& builder, llvm::Function* fn, llvm::Value* exec, llvm::Value* ctx);
  llvm::Value* emit_peek_truthy(llvm::IRBuilderBase& builder, llvm::Function* fn, llvm::Value* exec, llvm::Value* ctx);
  bool emit_control_flow_opcode(llvm::Function* fn,
      llvm::IRBuilderBase& builder,
      llvm::Value* exec,
      llvm::Value* ctx,
      llvm::Value* err,
      const starlark::bytecode::OpCode& op,
      int ip,
      int block_idx,
      std::map<int, llvm::BasicBlock*>& labels,
      llvm::BasicBlock* exit_bb,
      llvm::BasicBlock* error_bb,
      const std::function<llvm::BasicBlock*(int ip)>& next_bb_for);

  llvm::LLVMContext& context_;
  llvm::Module& module_;
  const starlark::bytecode::Program& program_;
  const irgen_options& options_;
  ir_exec_context ir_exec_;
  llvm::PointerType* exec_ty_ = nullptr;
  llvm::PointerType* obj_ptr_ty_ = nullptr;
  llvm::PointerType* ctx_ptr_ty_ = nullptr;
  llvm::PointerType* err_ptr_ty_ = nullptr;
  llvm::Type* i1_ty_ = nullptr;
  llvm::Type* i32_ty_ = nullptr;
  std::map<int, llvm::Function*> function_blocks_;
  std::map<int, std::map<int, llvm::BasicBlock*>> block_labels_;
};

}  // namespace native
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_BYTECODE_LOWERING_HPP_
