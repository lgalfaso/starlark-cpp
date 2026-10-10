// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_IR_IR_EXEC_CONTEXT_HPP_
#define NATIVE_IR_IR_EXEC_CONTEXT_HPP_

#include <cstddef>
#include <cstdint>

#pragma GCC visibility push(default)

namespace llvm {
class StringRef;
class BasicBlock;
class Function;
class FunctionCallee;
class IRBuilderBase;
class Module;
class PointerType;
class Type;
class Value;
class LLVMContext;
}  // namespace llvm

namespace starlark {
namespace native {

class ir_exec_context {
 public:
  ir_exec_context(llvm::LLVMContext& context, llvm::Module& module);

  void declare_object_runtime_functions();

  void emit_push(llvm::IRBuilderBase& builder, llvm::Value* exec, llvm::Value* value) const;
  llvm::Value* emit_pop(llvm::IRBuilderBase& builder, llvm::Value* exec) const;
  llvm::Value* emit_peek(llvm::IRBuilderBase& builder, llvm::Value* exec) const;
  llvm::Value* emit_stack_size(llvm::IRBuilderBase& builder, llvm::Value* exec) const;
  llvm::Value* emit_stack_at(llvm::IRBuilderBase& builder, llvm::Value* exec, llvm::Value* index) const;
  llvm::Value* emit_load_slot_inline(llvm::IRBuilderBase& builder,
      llvm::Function* fn,
      llvm::Value* exec,
      llvm::Value* frame_off,
      llvm::Value* slot,
      llvm::Value* err) const;
  void emit_store_slot_inline(llvm::IRBuilderBase& builder, llvm::Value* exec, llvm::Value* frame_off, llvm::Value* slot, llvm::Value* value) const;
  llvm::Value* emit_box_int_value(llvm::IRBuilderBase& builder, llvm::Value* ctx, llvm::Value* value_i64) const;
  void emit_pop_frame(llvm::IRBuilderBase& builder, llvm::Value* exec) const;
  llvm::Value* emit_get_failed(llvm::IRBuilderBase& builder, llvm::Value* exec) const;
  void emit_set_failed(llvm::IRBuilderBase& builder, llvm::Value* exec, llvm::Value* failed) const;
  llvm::Value* emit_load_ctx(llvm::IRBuilderBase& builder, llvm::Value* exec) const;
  llvm::Value* emit_load_err(llvm::IRBuilderBase& builder, llvm::Value* exec) const;
  void emit_set_location(llvm::IRBuilderBase& builder, llvm::Value* exec, int32_t block_idx, int32_t ip) const;
  void emit_branch_if_failed(llvm::IRBuilderBase& builder, llvm::Value* exec, llvm::BasicBlock* error_bb, llvm::BasicBlock* next_bb) const;

  llvm::FunctionCallee object_fn(llvm::StringRef name) const;

  llvm::PointerType* exec_ty() const { return exec_ty_; }
  llvm::PointerType* obj_ptr_ty() const { return obj_ptr_ty_; }
  llvm::PointerType* ctx_ptr_ty() const { return ctx_ptr_ty_; }
  llvm::PointerType* err_ptr_ty() const { return err_ptr_ty_; }
  llvm::Type* i1_ty() const { return i1_ty_; }
  llvm::Type* i32_ty() const { return i32_ty_; }
  llvm::Type* i64_ty() const { return i64_ty_; }

 private:
  llvm::Value* gep_exec_byte(llvm::IRBuilderBase& builder, llvm::Value* exec, std::size_t offset) const;
  llvm::Value* load_exec_field(llvm::IRBuilderBase& builder, llvm::Value* exec, std::size_t offset, llvm::Type* field_ty) const;
  void store_exec_field(llvm::IRBuilderBase& builder, llvm::Value* exec, std::size_t offset, llvm::Value* value) const;
  llvm::Value* emit_frame_at(llvm::IRBuilderBase& builder, llvm::Value* exec, llvm::Value* frame_off) const;
  llvm::Value* frame_slot_ptr(llvm::IRBuilderBase& builder, llvm::Value* frame, llvm::Value* slot) const;

  llvm::LLVMContext& context_;
  llvm::Module& module_;
  llvm::PointerType* exec_ty_ = nullptr;
  llvm::PointerType* obj_ptr_ty_ = nullptr;
  llvm::PointerType* ctx_ptr_ty_ = nullptr;
  llvm::PointerType* err_ptr_ty_ = nullptr;
  llvm::Type* i1_ty_ = nullptr;
  llvm::Type* i32_ty_ = nullptr;
  llvm::Type* i64_ty_ = nullptr;
};

}  // namespace native
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_IR_IR_EXEC_CONTEXT_HPP_
