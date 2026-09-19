// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_IR_OBJECT_OPS_HPP_
#define NATIVE_IR_OBJECT_OPS_HPP_

#include "native/ir/ir_exec_context.hpp"
#include "native/ir/lowering_context.hpp"
#include "vm/module_metadata.hpp"

#pragma GCC visibility push(default)

namespace llvm {
class BasicBlock;
class Function;
class Value;
class IRBuilderBase;
}  // namespace llvm

namespace starlark {
namespace native {

namespace ir_object_ops {

enum class int_cmp_kind {
  kEq,
  kNe,
  kLt,
  kLe,
  kGt,
  kGe,
};

enum class int_binop_kind {
  kAdd,
  kSub,
  kMul,
  kAnd,
  kOr,
  kXor,
  kLShift,
  kRShift,
};

llvm::Value* emit_ctx_none(ir_exec_context& ir_exec, llvm::IRBuilderBase& builder, llvm::Value* ctx);
llvm::Value* emit_ctx_true(ir_exec_context& ir_exec, llvm::IRBuilderBase& builder, llvm::Value* ctx);
llvm::Value* emit_ctx_false(ir_exec_context& ir_exec, llvm::IRBuilderBase& builder, llvm::Value* ctx);

llvm::Value* emit_obj_truthy(ir_exec_context& ir_exec,
    llvm::IRBuilderBase& builder,
    llvm::Function* fn,
    llvm::Value* value,
    llvm::Value* ctx);

void emit_push_none(lowering_context& lowering, llvm::IRBuilderBase& builder, llvm::Value* exec, llvm::Value* ctx);

void emit_binary_with_int_fastpath(lowering_context& lowering,
    llvm::IRBuilderBase& builder,
    llvm::Function* fn,
    llvm::Value* exec,
    llvm::Value* ctx,
    llvm::Value* err,
    int_binop_kind kind,
    const char* fallback_rt_name);

void emit_cmp_with_int_fastpath(lowering_context& lowering,
    llvm::IRBuilderBase& builder,
    llvm::Function* fn,
    llvm::Value* exec,
    llvm::Value* ctx,
    llvm::Value* err,
    int_cmp_kind kind,
    const char* fallback_rt_name);

void emit_create_frame_from_meta(lowering_context& lowering, llvm::IRBuilderBase& builder, llvm::Value* exec, int meta_index);

void emit_function_prologue_from_meta(lowering_context& lowering,
    llvm::IRBuilderBase& builder,
    llvm::Function* fn,
    llvm::Value* exec,
    int block_idx);

llvm::Value* emit_for_iterator_cond(lowering_context& lowering,
    llvm::IRBuilderBase& builder,
    llvm::Function* fn,
    llvm::Value* exec,
    llvm::Value* ctx,
    llvm::Value* err,
    bool extended);

void emit_make_tuple_inline(lowering_context& lowering,
    llvm::IRBuilderBase& builder,
    llvm::Value* exec,
    llvm::Value* ctx,
    int32_t count);

void emit_unpack_inline(lowering_context& lowering,
    llvm::IRBuilderBase& builder,
    llvm::Value* exec,
    llvm::Value* ctx,
    llvm::Value* err,
    int32_t count);

void emit_call_pos_inline(lowering_context& lowering,
    llvm::IRBuilderBase& builder,
    llvm::Function* fn,
    llvm::Value* exec,
    llvm::Value* ctx,
    llvm::Value* err,
    int32_t pos_count);

}  // namespace ir_object_ops

}  // namespace native
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_IR_OBJECT_OPS_HPP_
