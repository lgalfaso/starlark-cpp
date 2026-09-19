// Copyright 2026 Lucas Mirelmann

#include "native/ir/ir_object_ops.hpp"

#include <cassert>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "native/ir/lowering_context.hpp"
#include "runtime/starlark_object.hpp"

namespace starlark {
namespace native {
namespace ir_object_ops {

namespace {

using ::starlark::bytecode::BlockType;

llvm::Value* emit_obj_is_int(ir_exec_context& ir_exec, llvm::IRBuilderBase& builder, llvm::Value* obj) {
  return builder.CreateCall(ir_exec.object_fn("starlark_obj_rt_obj_is_int"), {obj});
}

llvm::Value* emit_obj_as_int64(ir_exec_context& ir_exec, llvm::IRBuilderBase& builder, llvm::Value* obj) {
  return builder.CreateCall(ir_exec.object_fn("starlark_obj_rt_obj_as_int64"), {obj});
}

llvm::Value* compare_int64(llvm::IRBuilderBase& builder, llvm::Value* lhs, llvm::Value* rhs, int_cmp_kind kind) {
  switch (kind) {
    case int_cmp_kind::kEq:
      return builder.CreateICmpEQ(lhs, rhs);
    case int_cmp_kind::kNe:
      return builder.CreateICmpNE(lhs, rhs);
    case int_cmp_kind::kLt:
      return builder.CreateICmpSLT(lhs, rhs);
    case int_cmp_kind::kLe:
      return builder.CreateICmpSLE(lhs, rhs);
    case int_cmp_kind::kGt:
      return builder.CreateICmpSGT(lhs, rhs);
    case int_cmp_kind::kGe:
      return builder.CreateICmpSGE(lhs, rhs);
  }
  return builder.getFalse();
}

llvm::Value* emit_arithmetic_result(llvm::IRBuilderBase& builder, ir_exec_context& ir_exec, llvm::Value* lhs, llvm::Value* rhs, int_binop_kind kind) {
  switch (kind) {
    case int_binop_kind::kAdd:
      return builder.CreateAdd(lhs, rhs);
    case int_binop_kind::kSub:
      return builder.CreateSub(lhs, rhs);
    case int_binop_kind::kMul:
      return builder.CreateMul(lhs, rhs);
    case int_binop_kind::kAnd:
      return builder.CreateAnd(lhs, rhs);
    case int_binop_kind::kOr:
      return builder.CreateOr(lhs, rhs);
    case int_binop_kind::kXor:
      return builder.CreateXor(lhs, rhs);
    case int_binop_kind::kLShift:
      return builder.CreateShl(lhs, rhs);
    case int_binop_kind::kRShift:
      return builder.CreateAShr(lhs, rhs);
  }
  return lhs;
}

bool arithmetic_has_overflow_check(int_binop_kind kind) {
  return kind == int_binop_kind::kAdd || kind == int_binop_kind::kSub || kind == int_binop_kind::kMul;
}

}  // namespace

llvm::Value* emit_ctx_none(ir_exec_context& ir_exec, llvm::IRBuilderBase& builder, llvm::Value* ctx) {
  return builder.CreateCall(ir_exec.object_fn("starlark_obj_rt_ctx_none_value"), {ctx});
}

llvm::Value* emit_ctx_true(ir_exec_context& ir_exec, llvm::IRBuilderBase& builder, llvm::Value* ctx) {
  return builder.CreateCall(ir_exec.object_fn("starlark_obj_rt_ctx_true_value"), {ctx});
}

llvm::Value* emit_ctx_false(ir_exec_context& ir_exec, llvm::IRBuilderBase& builder, llvm::Value* ctx) {
  return builder.CreateCall(ir_exec.object_fn("starlark_obj_rt_ctx_false_value"), {ctx});
}

// Equivalent C++:
//   if (value == nullptr || value == ctx->none_value()) return false;
//   if (value == ctx->false_value()) return false;
//   if (value == ctx->true_value()) return true;
//   if (value->kind() == object_kind::kInt) return value->as_int64() != 0;
//   return value->truthy();
llvm::Value* emit_obj_truthy(ir_exec_context& ir_exec,
    llvm::IRBuilderBase& builder,
    llvm::Function* fn,
    llvm::Value* value,
    llvm::Value* ctx) {
  auto* i1 = ir_exec.i1_ty();
  auto* entry = builder.GetInsertBlock();
  auto* done = llvm::BasicBlock::Create(entry->getContext(), "truthy_done", fn);
  auto* slow = llvm::BasicBlock::Create(entry->getContext(), "truthy_slow", fn);
  auto* phi = llvm::PHINode::Create(i1, 6, "truthy", done);

  auto* is_null = builder.CreateIsNull(value);
  builder.CreateCondBr(is_null, done, slow);
  phi->addIncoming(llvm::ConstantInt::getFalse(entry->getContext()), entry);

  builder.SetInsertPoint(slow);
  auto* none_val = emit_ctx_none(ir_exec, builder, ctx);
  auto* false_val = emit_ctx_false(ir_exec, builder, ctx);
  auto* true_val = emit_ctx_true(ir_exec, builder, ctx);
  auto* is_none = builder.CreateICmpEQ(value, none_val);
  auto* not_none = llvm::BasicBlock::Create(entry->getContext(), "truthy_not_none", fn);
  builder.CreateCondBr(is_none, done, not_none);
  phi->addIncoming(llvm::ConstantInt::getFalse(entry->getContext()), slow);

  builder.SetInsertPoint(not_none);
  auto* is_false = builder.CreateICmpEQ(value, false_val);
  auto* not_false = llvm::BasicBlock::Create(entry->getContext(), "truthy_not_false", fn);
  builder.CreateCondBr(is_false, done, not_false);
  phi->addIncoming(llvm::ConstantInt::getFalse(entry->getContext()), not_none);

  builder.SetInsertPoint(not_false);
  auto* is_true = builder.CreateICmpEQ(value, true_val);
  auto* check_int = llvm::BasicBlock::Create(entry->getContext(), "truthy_check_int", fn);
  auto* true_bb = llvm::BasicBlock::Create(entry->getContext(), "truthy_true", fn);
  builder.CreateCondBr(is_true, true_bb, check_int);
  phi->addIncoming(llvm::ConstantInt::getTrue(entry->getContext()), true_bb);
  builder.SetInsertPoint(true_bb);
  builder.CreateBr(done);

  builder.SetInsertPoint(check_int);
  auto* is_int = emit_obj_is_int(ir_exec, builder, value);
  auto* int_fast = llvm::BasicBlock::Create(entry->getContext(), "truthy_int_fast", fn);
  auto* fallback = llvm::BasicBlock::Create(entry->getContext(), "truthy_fallback", fn);
  builder.CreateCondBr(is_int, int_fast, fallback);

  builder.SetInsertPoint(int_fast);
  auto* int_val = emit_obj_as_int64(ir_exec, builder, value);
  auto* int_truthy = builder.CreateICmpNE(int_val, llvm::ConstantInt::get(ir_exec.i64_ty(), 0));
  builder.CreateBr(done);
  phi->addIncoming(int_truthy, int_fast);

  builder.SetInsertPoint(fallback);
  auto* slow_truthy = builder.CreateCall(ir_exec.object_fn("starlark_obj_rt_truthy"), {value});
  builder.CreateBr(done);
  phi->addIncoming(slow_truthy, fallback);

  builder.SetInsertPoint(done);
  return phi;
}

void emit_push_none(lowering_context& lowering, llvm::IRBuilderBase& builder, llvm::Value* exec, llvm::Value* ctx) {
  auto* none = emit_ctx_none(lowering.ir_exec, builder, ctx);
  lowering.ir_exec.emit_push(builder, exec, none);
}

// Equivalent C++:
//   auto* rhs = exec->eval_stack.pop();
//   auto* lhs = exec->eval_stack.peek();
//   if (lhs->kind() == object_kind::kInt && rhs->kind() == object_kind::kInt) {
//     int64_t a = lhs->as_int64(), b = rhs->as_int64();
//     // result_i64 = a op b (+, -, *, &, |, ^, <<, >>); for +/-/*, fall back on signed overflow
//     exec->eval_stack.pop();
//     exec->eval_stack.push(box_int(ctx, result_i64));
//     return;
//   }
//   exec->eval_stack.push(rhs);
//   fallback_rt(exec, ctx, err);  // lhs->binary_*(*rhs, *ctx, *err) replaces top-of-stack
void emit_binary_with_int_fastpath(lowering_context& lowering,
    llvm::IRBuilderBase& builder,
    llvm::Function* fn,
    llvm::Value* exec,
    llvm::Value* ctx,
    llvm::Value* err,
    int_binop_kind kind,
    const char* fallback_rt_name) {
  auto& ir_exec = lowering.ir_exec;
  auto* rhs = ir_exec.emit_pop(builder, exec);
  auto* lhs = ir_exec.emit_peek(builder, exec);

  auto* entry = builder.GetInsertBlock();
  auto* slow = llvm::BasicBlock::Create(entry->getContext(), "bin_slow", fn);
  auto* done = llvm::BasicBlock::Create(entry->getContext(), "bin_done", fn);

  auto* lhs_int = emit_obj_is_int(ir_exec, builder, lhs);
  auto* rhs_int = emit_obj_is_int(ir_exec, builder, rhs);
  auto* both_int = builder.CreateAnd(lhs_int, rhs_int);
  auto* fast = llvm::BasicBlock::Create(entry->getContext(), "bin_fast", fn);
  builder.CreateCondBr(both_int, fast, slow);

  builder.SetInsertPoint(fast);
  auto* lhs_i64 = emit_obj_as_int64(ir_exec, builder, lhs);
  auto* rhs_i64 = emit_obj_as_int64(ir_exec, builder, rhs);
  auto* fast_ok = llvm::BasicBlock::Create(entry->getContext(), "bin_fast_ok", fn);
  if (arithmetic_has_overflow_check(kind)) {
    llvm::Intrinsic::ID intrinsic_id = llvm::Intrinsic::sadd_with_overflow;
    if (kind == int_binop_kind::kSub) {
      intrinsic_id = llvm::Intrinsic::ssub_with_overflow;
    } else if (kind == int_binop_kind::kMul) {
      intrinsic_id = llvm::Intrinsic::smul_with_overflow;
    }
    auto* i64 = ir_exec.i64_ty();
    auto* intrinsic = llvm::Intrinsic::getDeclaration(&lowering.module, intrinsic_id, {i64});
    auto* pair = builder.CreateCall(intrinsic, {lhs_i64, rhs_i64});
    auto* result_i64 = builder.CreateExtractValue(pair, 0);
    auto* overflow = builder.CreateExtractValue(pair, 1);
    builder.CreateCondBr(overflow, slow, fast_ok);
    builder.SetInsertPoint(fast_ok);
    auto* boxed = ir_exec.emit_box_int_value(builder, ctx, result_i64);
    ir_exec.emit_pop(builder, exec);
    ir_exec.emit_push(builder, exec, boxed);
  } else {
    auto* result_i64 = emit_arithmetic_result(builder, ir_exec, lhs_i64, rhs_i64, kind);
    builder.CreateBr(fast_ok);
    builder.SetInsertPoint(fast_ok);
    auto* boxed = ir_exec.emit_box_int_value(builder, ctx, result_i64);
    ir_exec.emit_pop(builder, exec);
    ir_exec.emit_push(builder, exec, boxed);
  }
  builder.CreateBr(done);

  builder.SetInsertPoint(slow);
  ir_exec.emit_push(builder, exec, rhs);
  builder.CreateCall(ir_exec.object_fn(fallback_rt_name), {exec, ctx, err});
  builder.CreateBr(done);

  builder.SetInsertPoint(done);
}

// Equivalent C++:
//   auto* rhs = exec->eval_stack.pop();
//   auto* lhs = exec->eval_stack.peek();
//   if (lhs->kind() == object_kind::kInt && rhs->kind() == object_kind::kInt) {
//     int64_t a = lhs->as_int64(), b = rhs->as_int64();
//     bool result = compare(a, b, kind);  // ==, !=, <, <=, >, >=
//     exec->eval_stack.pop();
//     exec->eval_stack.push(result ? ctx->true_value() : ctx->false_value());
//     return;
//   }
//   exec->eval_stack.push(rhs);
//   fallback_rt(exec, ctx, err);  // cmp/fallback replaces top-of-stack with bool
void emit_cmp_with_int_fastpath(lowering_context& lowering,
    llvm::IRBuilderBase& builder,
    llvm::Function* fn,
    llvm::Value* exec,
    llvm::Value* ctx,
    llvm::Value* err,
    int_cmp_kind kind,
    const char* fallback_rt_name) {
  auto& ir_exec = lowering.ir_exec;
  auto* rhs = ir_exec.emit_pop(builder, exec);
  auto* lhs = ir_exec.emit_peek(builder, exec);

  auto* entry = builder.GetInsertBlock();
  auto* slow = llvm::BasicBlock::Create(entry->getContext(), "bin_slow", fn);
  auto* done = llvm::BasicBlock::Create(entry->getContext(), "bin_done", fn);

  auto* lhs_int = emit_obj_is_int(ir_exec, builder, lhs);
  auto* rhs_int = emit_obj_is_int(ir_exec, builder, rhs);
  auto* both_int = builder.CreateAnd(lhs_int, rhs_int);
  auto* fast = llvm::BasicBlock::Create(entry->getContext(), "bin_fast", fn);
  builder.CreateCondBr(both_int, fast, slow);

  builder.SetInsertPoint(fast);
  auto* lhs_i64 = emit_obj_as_int64(ir_exec, builder, lhs);
  auto* rhs_i64 = emit_obj_as_int64(ir_exec, builder, rhs);
  auto* cmp = compare_int64(builder, lhs_i64, rhs_i64, kind);
  auto* bool_obj = builder.CreateSelect(cmp, emit_ctx_true(ir_exec, builder, ctx), emit_ctx_false(ir_exec, builder, ctx));
  ir_exec.emit_pop(builder, exec);
  ir_exec.emit_push(builder, exec, bool_obj);
  builder.CreateBr(done);

  builder.SetInsertPoint(slow);
  ir_exec.emit_push(builder, exec, rhs);
  builder.CreateCall(ir_exec.object_fn(fallback_rt_name), {exec, ctx, err});
  builder.CreateBr(done);

  builder.SetInsertPoint(done);
}

void emit_create_frame_from_meta(lowering_context& lowering, llvm::IRBuilderBase& builder, llvm::Value* exec, int meta_index) {
  assert(meta_index >= 0);
  assert(lowering.options.metadata != nullptr);
  auto* create_fn = lowering.module.getFunction("starlark_rt_exec_create_frame_meta");
  assert(create_fn != nullptr);
  builder.CreateCall(create_fn,
      {exec, llvm::ConstantInt::get(lowering.ir_exec.i32_ty(), static_cast<uint32_t>(meta_index))});
}

void emit_function_prologue_from_meta(lowering_context& lowering,
    llvm::IRBuilderBase& builder,
    llvm::Function* fn,
    llvm::Value* exec,
    int block_idx) {
  (void)fn;
  (void)block_idx;
  auto* entry_fn = lowering.module.getFunction("starlark_rt_exec_jit_function_entry");
  assert(entry_fn != nullptr);
  builder.CreateCall(entry_fn, {exec});
}

// Equivalent C++:
//   if (exec->failed) return false;
//   auto* it = exec->frame_chain.back()->iterators.back();  // nullptr if missing
//   if (it == nullptr) { exec->failed = true; return false; }
//   if (!it->has_next()) return false;
//   if (extended) { it->next_ext(); return true; }
//   exec->eval_stack.push(it->next());
//   return true;
llvm::Value* emit_for_iterator_cond(lowering_context& lowering,
    llvm::IRBuilderBase& builder,
    llvm::Function* fn,
    llvm::Value* exec,
    llvm::Value* ctx,
    llvm::Value* err,
    bool extended) {
  (void)ctx;
  (void)err;
  auto& ir_exec = lowering.ir_exec;
  auto* i1 = ir_exec.i1_ty();
  auto* entry = builder.GetInsertBlock();
  auto* done_bb = llvm::BasicBlock::Create(entry->getContext(), "iter_done", fn);
  auto* phi = llvm::PHINode::Create(i1, 4, "iter_has_next", done_bb);

  auto* already_failed = ir_exec.emit_get_failed(builder, exec);
  auto* iter_bb = llvm::BasicBlock::Create(entry->getContext(), "iter_body", fn);
  builder.CreateCondBr(already_failed, done_bb, iter_bb);
  phi->addIncoming(llvm::ConstantInt::getFalse(entry->getContext()), entry);

  builder.SetInsertPoint(iter_bb);
  auto* it = builder.CreateCall(ir_exec.object_fn("starlark_obj_rt_exec_frame_back_iterator"), {exec});
  auto* null_bb = llvm::BasicBlock::Create(entry->getContext(), "iter_null", fn);
  auto* check_bb = llvm::BasicBlock::Create(entry->getContext(), "iter_check", fn);
  auto* push_bb = llvm::BasicBlock::Create(entry->getContext(), "iter_push", fn);
  auto* no_next_bb = llvm::BasicBlock::Create(entry->getContext(), "iter_no_next", fn);

  auto* is_null = builder.CreateIsNull(it);
  builder.CreateCondBr(is_null, null_bb, check_bb);

  builder.SetInsertPoint(null_bb);
  ir_exec.emit_set_failed(builder, exec, llvm::ConstantInt::getTrue(entry->getContext()));
  builder.CreateBr(done_bb);
  phi->addIncoming(llvm::ConstantInt::getFalse(entry->getContext()), null_bb);

  builder.SetInsertPoint(check_bb);
  auto* has_next = builder.CreateCall(ir_exec.object_fn("starlark_obj_rt_iterator_has_next"), {it});
  builder.CreateCondBr(has_next, push_bb, no_next_bb);

  builder.SetInsertPoint(push_bb);
  llvm::Value* advanced = extended ? builder.CreateCall(ir_exec.object_fn("starlark_obj_rt_iterator_next_ext"), {it})
                                   : builder.CreateCall(ir_exec.object_fn("starlark_obj_rt_iterator_next_push"), {exec, it});
  builder.CreateBr(done_bb);
  phi->addIncoming(advanced, push_bb);

  builder.SetInsertPoint(no_next_bb);
  builder.CreateBr(done_bb);
  phi->addIncoming(llvm::ConstantInt::getFalse(entry->getContext()), no_next_bb);

  builder.SetInsertPoint(done_bb);
  return phi;
}

void emit_make_tuple_inline(lowering_context& lowering,
    llvm::IRBuilderBase& builder,
    llvm::Value* exec,
    llvm::Value* ctx,
    int32_t count) {
  if (count <= 0) {
    builder.CreateCall(lowering.ir_exec.object_fn("starlark_obj_rt_make_tuple"), {exec, llvm::ConstantInt::get(lowering.ir_exec.i32_ty(), 0), ctx});
    return;
  }
  builder.CreateCall(lowering.ir_exec.object_fn("starlark_obj_rt_make_tuple"), {exec, llvm::ConstantInt::get(lowering.ir_exec.i32_ty(), count), ctx});
}

void emit_unpack_inline(lowering_context& lowering,
    llvm::IRBuilderBase& builder,
    llvm::Value* exec,
    llvm::Value* ctx,
    llvm::Value* err,
    int32_t count) {
  builder.CreateCall(lowering.ir_exec.object_fn("starlark_obj_rt_unpack"),
      {exec, llvm::ConstantInt::get(lowering.ir_exec.i32_ty(), count), ctx, err});
}

// Equivalent C++:
//   if (try_predeclared_call_pos(exec, pos_count, ctx, err)) return;
//   auto* callee = exec->eval_stack.at(exec->eval_stack.size - pos_count - 1);
//   if (is_native_function(callee) && native_fn_direct_eligible(callee)) {
//     call_native_direct(exec, pos_count, ctx, err);
//     return;
//   }
//   call_posN(exec, ctx, err);  // or call_pos(exec, pos_count, ctx, err) for N > 3
void emit_call_pos_inline(lowering_context& lowering,
    llvm::IRBuilderBase& builder,
    llvm::Function* fn,
    llvm::Value* exec,
    llvm::Value* ctx,
    llvm::Value* err,
    int32_t pos_count) {
  auto& ir_exec = lowering.ir_exec;
  auto* pos_count_val = llvm::ConstantInt::get(ir_exec.i32_ty(), pos_count);
  auto* entry = builder.GetInsertBlock();
  auto* done = llvm::BasicBlock::Create(entry->getContext(), "call_pos_done", fn);
  auto* after_predecl = llvm::BasicBlock::Create(entry->getContext(), "call_pos_dispatch", fn);
  auto* predecl = llvm::BasicBlock::Create(entry->getContext(), "call_pos_predecl", fn);
  builder.CreateBr(predecl);
  builder.SetInsertPoint(predecl);
  auto* predecl_handled = builder.CreateCall(ir_exec.object_fn("starlark_obj_rt_try_predeclared_call_pos"), {exec, pos_count_val, ctx, err});
  builder.CreateCondBr(predecl_handled, done, after_predecl);

  builder.SetInsertPoint(after_predecl);
  auto* slow = llvm::BasicBlock::Create(entry->getContext(), "call_pos_slow", fn);
  auto* fast = llvm::BasicBlock::Create(entry->getContext(), "call_pos_fast", fn);
  auto* stack_size = ir_exec.emit_stack_size(builder, exec);
  auto* callee_idx = builder.CreateSub(stack_size, builder.CreateAdd(pos_count_val, llvm::ConstantInt::get(ir_exec.i32_ty(), 1)));
  auto* callee = ir_exec.emit_stack_at(builder, exec, callee_idx);
  auto* is_native = builder.CreateCall(ir_exec.object_fn("starlark_obj_rt_is_native_function"), {callee});
  auto* direct_eligible = builder.CreateCall(ir_exec.object_fn("starlark_obj_rt_native_fn_direct_eligible"), {callee});
  auto* can_direct = builder.CreateAnd(is_native, direct_eligible);
  builder.CreateCondBr(can_direct, fast, slow);

  builder.SetInsertPoint(fast);
  builder.CreateCall(ir_exec.object_fn("starlark_obj_rt_call_native_direct"), {exec, pos_count_val, ctx, err});
  builder.CreateBr(done);

  builder.SetInsertPoint(slow);
  const char* slow_fn_name = nullptr;
  switch (pos_count) {
    case 0:
      slow_fn_name = "starlark_obj_rt_call_pos0";
      break;
    case 1:
      slow_fn_name = "starlark_obj_rt_call_pos1";
      break;
    case 2:
      slow_fn_name = "starlark_obj_rt_call_pos2";
      break;
    case 3:
      slow_fn_name = "starlark_obj_rt_call_pos3";
      break;
    default:
      builder.CreateCall(ir_exec.object_fn("starlark_obj_rt_call_pos"), {exec, pos_count_val, ctx, err});
      builder.CreateBr(done);
      builder.SetInsertPoint(done);
      return;
  }
  builder.CreateCall(ir_exec.object_fn(slow_fn_name), {exec, ctx, err});
  builder.CreateBr(done);

  builder.SetInsertPoint(done);
}

}  // namespace ir_object_ops
}  // namespace native
}  // namespace starlark
