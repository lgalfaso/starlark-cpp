// Copyright 2026 Lucas Mirelmann

#include "native/ir/ir_exec_context.hpp"

#include <cstddef>

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "native/exec/jit_ctx_layout.hpp"
#include "native/exec/jit_exec_layout.hpp"
#include "native/exec/native_exec_context.hpp"
#include "native/exec/object_runtime.hpp"
#include "native/exec/module_runtime_state.hpp"

namespace starlark {
namespace native {

namespace {

llvm::FunctionCallee declare_fn(llvm::Module& module, llvm::StringRef name, llvm::FunctionType* ty) {
  return module.getOrInsertFunction(name, ty);
}

}  // namespace

ir_exec_context::ir_exec_context(llvm::LLVMContext& context, llvm::Module& module) : context_(context), module_(module) {
  exec_ty_ = llvm::PointerType::getUnqual(context_);
  obj_ptr_ty_ = llvm::PointerType::getUnqual(context_);
  ctx_ptr_ty_ = llvm::PointerType::getUnqual(context_);
  err_ptr_ty_ = llvm::PointerType::getUnqual(context_);
  i1_ty_ = llvm::Type::getInt1Ty(context_);
  i32_ty_ = llvm::Type::getInt32Ty(context_);
  i64_ty_ = llvm::Type::getInt64Ty(context_);
  declare_object_runtime_functions();
}

void ir_exec_context::declare_object_runtime_functions() {
  auto* void_ty = llvm::Type::getVoidTy(context_);
  auto* exec_ptr = exec_ty_;
  auto* obj_ptr = obj_ptr_ty_;
  auto* ctx_ptr = ctx_ptr_ty_;
  auto* err_ptr = err_ptr_ty_;

  declare_fn(module_, "starlark_obj_rt_create_int", llvm::FunctionType::get(obj_ptr, {ctx_ptr, i64_ty_}, false));
  declare_fn(module_, "starlark_obj_rt_create_float", llvm::FunctionType::get(obj_ptr, {ctx_ptr, llvm::Type::getDoubleTy(context_)}, false));
  declare_fn(module_, "starlark_obj_rt_get_const_string", llvm::FunctionType::get(obj_ptr, {exec_ptr, i64_ty_}, false));

  auto* bin_ty = llvm::FunctionType::get(void_ty, {exec_ptr, ctx_ptr, err_ptr}, false);
  const char* bin_ops[] = {"starlark_obj_rt_binary_plus",
      "starlark_obj_rt_binary_minus",
      "starlark_obj_rt_binary_star",
      "starlark_obj_rt_binary_slash",
      "starlark_obj_rt_binary_slash_slash",
      "starlark_obj_rt_binary_percent",
      "starlark_obj_rt_binary_ampersand",
      "starlark_obj_rt_binary_pipe",
      "starlark_obj_rt_binary_hat",
      "starlark_obj_rt_binary_lshift",
      "starlark_obj_rt_binary_rshift"};
  for (const char* name : bin_ops) {
    declare_fn(module_, name, bin_ty);
  }

  const char* unary_ops[] = {"starlark_obj_rt_unary_not", "starlark_obj_rt_unary_plus", "starlark_obj_rt_unary_minus", "starlark_obj_rt_unary_tilde"};
  for (const char* name : unary_ops) {
    declare_fn(module_, name, bin_ty);
  }

  const char* cmp_ops[] = {"starlark_obj_rt_cmp_eq",
      "starlark_obj_rt_cmp_ne",
      "starlark_obj_rt_cmp_lt",
      "starlark_obj_rt_cmp_le",
      "starlark_obj_rt_cmp_gt",
      "starlark_obj_rt_cmp_ge",
      "starlark_obj_rt_cmp_in",
      "starlark_obj_rt_cmp_not_in"};
  for (const char* name : cmp_ops) {
    declare_fn(module_, name, bin_ty);
  }

  declare_fn(module_, "starlark_obj_rt_truthy", llvm::FunctionType::get(i1_ty_, {obj_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_ctx_none_value", llvm::FunctionType::get(obj_ptr, {ctx_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_ctx_true_value", llvm::FunctionType::get(obj_ptr, {ctx_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_ctx_false_value", llvm::FunctionType::get(obj_ptr, {ctx_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_obj_numeric_type", llvm::FunctionType::get(i32_ty_, {obj_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_obj_as_int64", llvm::FunctionType::get(i64_ty_, {obj_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_dot", llvm::FunctionType::get(void_ty, {exec_ptr, llvm::PointerType::getUnqual(context_), i64_ty_, ctx_ptr, err_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_index", bin_ty);
  declare_fn(module_, "starlark_obj_rt_slice_range", bin_ty);

  const char* call_ops[] = {"starlark_obj_rt_call_pos0", "starlark_obj_rt_call_pos1", "starlark_obj_rt_call_pos2", "starlark_obj_rt_call_pos3"};
  for (const char* name : call_ops) {
    declare_fn(module_, name, bin_ty);
  }
  declare_fn(module_, "starlark_obj_rt_call_pos", llvm::FunctionType::get(void_ty, {exec_ptr, i32_ty_, ctx_ptr, err_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_try_predeclared_call_pos",
      llvm::FunctionType::get(i1_ty_, {exec_ptr, i32_ty_, ctx_ptr, err_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_is_native_function", llvm::FunctionType::get(i1_ty_, {obj_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_native_fn_direct_eligible", llvm::FunctionType::get(i1_ty_, {obj_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_call_native_direct", llvm::FunctionType::get(void_ty, {exec_ptr, i32_ty_, ctx_ptr, err_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_call_named", llvm::FunctionType::get(void_ty, {exec_ptr, i32_ty_, i32_ty_, ctx_ptr, err_ptr}, false));

  declare_fn(module_, "starlark_obj_rt_make_list", llvm::FunctionType::get(void_ty, {exec_ptr, i32_ty_, ctx_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_make_dict", llvm::FunctionType::get(void_ty, {exec_ptr, ctx_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_add_to_list", llvm::FunctionType::get(void_ty, {exec_ptr, i32_ty_, ctx_ptr, err_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_add_to_dict", llvm::FunctionType::get(void_ty, {exec_ptr, i32_ty_, ctx_ptr, err_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_make_bigint", llvm::FunctionType::get(void_ty, {exec_ptr, llvm::PointerType::getUnqual(context_), ctx_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_make_bytes", llvm::FunctionType::get(void_ty, {exec_ptr, llvm::PointerType::getUnqual(context_), i64_ty_, ctx_ptr}, false));

  declare_fn(module_, "starlark_obj_rt_unpack", llvm::FunctionType::get(void_ty, {exec_ptr, i32_ty_, ctx_ptr, err_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_get_iterator", bin_ty);
  auto* iter_ptr = llvm::PointerType::getUnqual(context_);
  declare_fn(module_, "starlark_obj_rt_exec_frame_back_iterator", llvm::FunctionType::get(iter_ptr, {exec_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_iterator_has_next", llvm::FunctionType::get(i1_ty_, {iter_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_iterator_next_push", llvm::FunctionType::get(i1_ty_, {exec_ptr, iter_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_iterator_next_ext", llvm::FunctionType::get(i1_ty_, {iter_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_end_iterator", llvm::FunctionType::get(void_ty, {exec_ptr}, false));

  declare_fn(module_, "starlark_obj_rt_make_tuple", llvm::FunctionType::get(void_ty, {exec_ptr, i32_ty_, ctx_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_make_native_function_meta",
      llvm::FunctionType::get(void_ty,
          {exec_ptr, llvm::PointerType::getUnqual(context_), llvm::PointerType::getUnqual(context_), i64_ty_, i32_ty_, i32_ty_},
          false));
  declare_fn(module_, "starlark_obj_rt_load_frame_slot", llvm::FunctionType::get(obj_ptr, {exec_ptr, i32_ty_, i32_ty_, err_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_ensure_stack_capacity", llvm::FunctionType::get(void_ty, {exec_ptr, i32_ty_}, false));

  declare_fn(module_, "starlark_obj_rt_call_method_pos0", llvm::FunctionType::get(void_ty, {exec_ptr, llvm::PointerType::getUnqual(context_), i64_ty_, ctx_ptr, err_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_call_method_pos1", llvm::FunctionType::get(void_ty, {exec_ptr, llvm::PointerType::getUnqual(context_), i64_ty_, ctx_ptr, err_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_call_method_pos2", llvm::FunctionType::get(void_ty, {exec_ptr, llvm::PointerType::getUnqual(context_), i64_ty_, ctx_ptr, err_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_call_method_pos3", llvm::FunctionType::get(void_ty, {exec_ptr, llvm::PointerType::getUnqual(context_), i64_ty_, ctx_ptr, err_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_call_method_pos", llvm::FunctionType::get(void_ty, {exec_ptr, llvm::PointerType::getUnqual(context_), i64_ty_, i32_ty_, ctx_ptr, err_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_call_pos_star", llvm::FunctionType::get(void_ty, {exec_ptr, i32_ty_, ctx_ptr, err_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_call_full", llvm::FunctionType::get(void_ty, {exec_ptr, i32_ty_, i32_ty_, i1_ty_, i1_ty_, ctx_ptr, err_ptr}, false));

  declare_fn(module_, "starlark_obj_rt_assign_index_member", bin_ty);
  declare_fn(module_, "starlark_obj_rt_assign_dot_member", llvm::FunctionType::get(void_ty, {exec_ptr, llvm::PointerType::getUnqual(context_), i64_ty_, ctx_ptr, err_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_assign_slice_range", bin_ty);
  declare_fn(module_, "starlark_obj_rt_assign_slice_range_op", llvm::FunctionType::get(void_ty, {exec_ptr, i32_ty_, ctx_ptr, err_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_frame_compound_assign", llvm::FunctionType::get(void_ty, {exec_ptr, i32_ty_, i32_ty_, i32_ty_, ctx_ptr, err_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_index_compound_assign", llvm::FunctionType::get(void_ty, {exec_ptr, i32_ty_, ctx_ptr, err_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_dot_compound_assign", llvm::FunctionType::get(void_ty, {exec_ptr, llvm::PointerType::getUnqual(context_), i64_ty_, i32_ty_, ctx_ptr, err_ptr}, false));
  declare_fn(module_, "starlark_obj_rt_load_module_symbol", llvm::FunctionType::get(void_ty, {exec_ptr, llvm::PointerType::getUnqual(context_), i64_ty_, llvm::PointerType::getUnqual(context_), i64_ty_, i32_ty_, i32_ty_, err_ptr}, false));
}

llvm::Value* ir_exec_context::gep_exec_byte(llvm::IRBuilderBase& builder, llvm::Value* exec, std::size_t offset) const {
  auto* i8 = llvm::Type::getInt8Ty(context_);
  return builder.CreateGEP(i8, exec, llvm::ConstantInt::get(i64_ty_, offset));
}

llvm::Value* ir_exec_context::load_exec_field(llvm::IRBuilderBase& builder, llvm::Value* exec, std::size_t offset, llvm::Type* field_ty) const {
  auto* ptr = builder.CreateBitCast(gep_exec_byte(builder, exec, offset), llvm::PointerType::getUnqual(context_));
  return builder.CreateLoad(field_ty, ptr);
}

void ir_exec_context::store_exec_field(llvm::IRBuilderBase& builder, llvm::Value* exec, std::size_t offset, llvm::Value* value) const {
  auto* ptr = builder.CreateBitCast(gep_exec_byte(builder, exec, offset), llvm::PointerType::getUnqual(context_));
  builder.CreateStore(value, ptr);
}

void ir_exec_context::emit_push(llvm::IRBuilderBase& builder, llvm::Value* exec, llvm::Value* value) const {
  auto* size = load_exec_field(builder, exec, jit_exec_layout::offsetof_stack_size(), i32_ty_);
  auto* required = builder.CreateAdd(size, llvm::ConstantInt::get(i32_ty_, 1));
  builder.CreateCall(object_fn("starlark_obj_rt_ensure_stack_capacity"), {exec, required});
  auto* size_after = load_exec_field(builder, exec, jit_exec_layout::offsetof_stack_size(), i32_ty_);
  auto* data_ptr = load_exec_field(builder, exec, jit_exec_layout::offsetof_stack_data(), obj_ptr_ty_);
  auto* slot_ptr = builder.CreateGEP(obj_ptr_ty_, data_ptr, size_after);
  builder.CreateStore(value, slot_ptr);
  builder.CreateStore(builder.CreateAdd(size_after, llvm::ConstantInt::get(i32_ty_, 1)), gep_exec_byte(builder, exec, jit_exec_layout::offsetof_stack_size()));
}

llvm::Value* ir_exec_context::emit_pop(llvm::IRBuilderBase& builder, llvm::Value* exec) const {
  auto* size = load_exec_field(builder, exec, jit_exec_layout::offsetof_stack_size(), i32_ty_);
  auto* new_size = builder.CreateSub(size, llvm::ConstantInt::get(i32_ty_, 1));
  store_exec_field(builder, exec, jit_exec_layout::offsetof_stack_size(), new_size);
  auto* data_ptr = load_exec_field(builder, exec, jit_exec_layout::offsetof_stack_data(), obj_ptr_ty_);
  auto* slot_ptr = builder.CreateGEP(obj_ptr_ty_, data_ptr, new_size);
  return builder.CreateLoad(obj_ptr_ty_, slot_ptr);
}

llvm::Value* ir_exec_context::emit_peek(llvm::IRBuilderBase& builder, llvm::Value* exec) const {
  auto* size = load_exec_field(builder, exec, jit_exec_layout::offsetof_stack_size(), i32_ty_);
  auto* index = builder.CreateSub(size, llvm::ConstantInt::get(i32_ty_, 1));
  return emit_stack_at(builder, exec, index);
}

llvm::Value* ir_exec_context::emit_stack_size(llvm::IRBuilderBase& builder, llvm::Value* exec) const {
  return load_exec_field(builder, exec, jit_exec_layout::offsetof_stack_size(), i32_ty_);
}

llvm::Value* ir_exec_context::emit_stack_at(llvm::IRBuilderBase& builder, llvm::Value* exec, llvm::Value* index) const {
  auto* data_ptr = load_exec_field(builder, exec, jit_exec_layout::offsetof_stack_data(), obj_ptr_ty_);
  auto* slot_ptr = builder.CreateGEP(obj_ptr_ty_, data_ptr, index);
  return builder.CreateLoad(obj_ptr_ty_, slot_ptr);
}

llvm::Value* ir_exec_context::emit_frame_at(llvm::IRBuilderBase& builder, llvm::Value* exec, llvm::Value* frame_off) const {
  auto* depth = load_exec_field(builder, exec, jit_exec_layout::offsetof_frame_depth(), i32_ty_);
  auto* frames_data = load_exec_field(builder, exec, jit_exec_layout::offsetof_frame_chain_frames(), obj_ptr_ty_);
  auto* index = builder.CreateSub(builder.CreateSub(depth, llvm::ConstantInt::get(i32_ty_, 1)), frame_off);
  auto* frame_ptr = builder.CreateGEP(obj_ptr_ty_, frames_data, index);
  return builder.CreateLoad(obj_ptr_ty_, frame_ptr);
}

llvm::Value* ir_exec_context::frame_slot_ptr(llvm::IRBuilderBase& builder, llvm::Value* frame, llvm::Value* slot) const {
  auto* elements_gep = builder.CreateGEP(llvm::Type::getInt8Ty(context_), frame, llvm::ConstantInt::get(i64_ty_, jit_exec_layout::offsetof_frame_elements(nullptr)));
  auto* elements_vec_data = builder.CreateLoad(obj_ptr_ty_, builder.CreateBitCast(elements_gep, llvm::PointerType::getUnqual(context_)));
  return builder.CreateGEP(obj_ptr_ty_, elements_vec_data, slot);
}

llvm::Value* ir_exec_context::emit_load_slot_inline(llvm::IRBuilderBase& builder,
    llvm::Function* fn,
    llvm::Value* exec,
    llvm::Value* frame_off,
    llvm::Value* slot,
    llvm::Value* err) const {
  auto* entry = builder.GetInsertBlock();
  auto* done = llvm::BasicBlock::Create(entry->getContext(), "load_done", fn);
  auto* slow = llvm::BasicBlock::Create(entry->getContext(), "load_slow", fn);
  auto* fast = llvm::BasicBlock::Create(entry->getContext(), "load_fast", fn);
  auto* phi = llvm::PHINode::Create(obj_ptr_ty_, 2, "load_val", done);

  auto* frame = emit_frame_at(builder, exec, frame_off);
  auto* slot_ptr = frame_slot_ptr(builder, frame, slot);
  auto* value = builder.CreateLoad(obj_ptr_ty_, slot_ptr);
  auto* is_null = builder.CreateIsNull(value);
  builder.CreateCondBr(is_null, slow, fast);

  builder.SetInsertPoint(fast);
  builder.CreateBr(done);
  phi->addIncoming(value, fast);

  builder.SetInsertPoint(slow);
  auto* slow_val = builder.CreateCall(object_fn("starlark_obj_rt_load_frame_slot"), {exec, frame_off, slot, err});
  builder.CreateBr(done);
  phi->addIncoming(slow_val, slow);

  builder.SetInsertPoint(done);
  return phi;
}

void ir_exec_context::emit_store_slot_inline(llvm::IRBuilderBase& builder, llvm::Value* exec, llvm::Value* frame_off, llvm::Value* slot, llvm::Value* value) const {
  auto* frame = emit_frame_at(builder, exec, frame_off);
  auto* slot_ptr = frame_slot_ptr(builder, frame, slot);
  builder.CreateStore(value, slot_ptr);
}

llvm::Value* ir_exec_context::emit_box_int_value(llvm::IRBuilderBase& builder, llvm::Value* ctx, llvm::Value* value_i64) const {
  auto* min_small = llvm::ConstantInt::get(i64_ty_, starlark::runtime::context::MIN_SMALL_INT);
  auto* max_small = llvm::ConstantInt::get(i64_ty_, starlark::runtime::context::MAX_SMALL_INT);
  auto* entry = builder.GetInsertBlock();
  auto* fn = entry->getParent();
  auto* done = llvm::BasicBlock::Create(entry->getContext(), "box_int_done", fn);
  auto* heap = llvm::BasicBlock::Create(entry->getContext(), "box_int_heap", fn);
  auto* small = llvm::BasicBlock::Create(entry->getContext(), "box_int_small", fn);
  auto* phi = llvm::PHINode::Create(obj_ptr_ty_, 2, "boxed_int", done);

  auto* in_range = builder.CreateAnd(builder.CreateICmpSGE(value_i64, min_small), builder.CreateICmpSLE(value_i64, max_small));
  builder.CreateCondBr(in_range, small, heap);

  builder.SetInsertPoint(small);
  auto* index = builder.CreateSub(value_i64, min_small);
  auto* table_gep = builder.CreateGEP(llvm::Type::getInt8Ty(context_),
      ctx,
      llvm::ConstantInt::get(i64_ty_, jit_ctx_layout::offsetof_small_integers()));
  auto* table_ptr = builder.CreateBitCast(table_gep, llvm::PointerType::getUnqual(context_));
  auto* elem_ptr = builder.CreateGEP(obj_ptr_ty_, table_ptr, index);
  auto* interned = builder.CreateLoad(obj_ptr_ty_, elem_ptr);
  builder.CreateBr(done);
  phi->addIncoming(interned, small);

  builder.SetInsertPoint(heap);
  auto* heap_val = builder.CreateCall(object_fn("starlark_obj_rt_create_int"), {ctx, value_i64});
  builder.CreateBr(done);
  phi->addIncoming(heap_val, heap);

  builder.SetInsertPoint(done);
  return phi;
}

void ir_exec_context::emit_pop_frame(llvm::IRBuilderBase& builder, llvm::Value* exec) const {
  auto* depth = load_exec_field(builder, exec, jit_exec_layout::offsetof_frame_depth(), i32_ty_);
  store_exec_field(builder, exec, jit_exec_layout::offsetof_frame_depth(), builder.CreateSub(depth, llvm::ConstantInt::get(i32_ty_, 1)));
}

llvm::Value* ir_exec_context::emit_get_failed(llvm::IRBuilderBase& builder, llvm::Value* exec) const {
  return load_exec_field(builder, exec, jit_exec_layout::offsetof_failed(), i1_ty_);
}

void ir_exec_context::emit_set_failed(llvm::IRBuilderBase& builder, llvm::Value* exec, llvm::Value* failed) const {
  store_exec_field(builder, exec, jit_exec_layout::offsetof_failed(), failed);
}

llvm::Value* ir_exec_context::emit_load_ctx(llvm::IRBuilderBase& builder, llvm::Value* exec) const {
  return load_exec_field(builder, exec, offsetof(native_exec_context, ctx), ctx_ptr_ty_);
}

llvm::Value* ir_exec_context::emit_load_err(llvm::IRBuilderBase& builder, llvm::Value* exec) const {
  return load_exec_field(builder, exec, offsetof(native_exec_context, err), err_ptr_ty_);
}

void ir_exec_context::emit_set_location(llvm::IRBuilderBase& builder, llvm::Value* exec, int32_t block_idx, int32_t ip) const {
  auto* mod = load_exec_field(builder, exec, offsetof(native_exec_context, mod), obj_ptr_ty_);
  auto* block_ptr = builder.CreateGEP(llvm::Type::getInt8Ty(context_),
      mod,
      llvm::ConstantInt::get(i64_ty_, jit_exec_layout::offsetof_mod_error_block()));
  builder.CreateStore(llvm::ConstantInt::get(i32_ty_, block_idx), builder.CreateBitCast(block_ptr, llvm::PointerType::getUnqual(context_)));
  auto* ip_ptr = builder.CreateGEP(llvm::Type::getInt8Ty(context_),
      mod,
      llvm::ConstantInt::get(i64_ty_, jit_exec_layout::offsetof_mod_error_ip()));
  builder.CreateStore(llvm::ConstantInt::get(i32_ty_, ip), builder.CreateBitCast(ip_ptr, llvm::PointerType::getUnqual(context_)));
}

void ir_exec_context::emit_branch_if_failed(llvm::IRBuilderBase& builder, llvm::Value* exec, llvm::BasicBlock* error_bb, llvm::BasicBlock* next_bb) const {
  auto* failed = emit_get_failed(builder, exec);
  builder.CreateCondBr(failed, error_bb, next_bb);
}

llvm::FunctionCallee ir_exec_context::object_fn(llvm::StringRef name) const {
  return module_.getFunction(name);
}

}  // namespace native
}  // namespace starlark
