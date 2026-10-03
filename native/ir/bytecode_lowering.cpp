// Copyright 2026 Lucas Mirelmann

#include "native/ir/bytecode_lowering.hpp"

#include <cassert>

#include <format>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "native/ir/ir_object_ops.hpp"
#include "native/ir/lowering_context.hpp"
#include "native/ir/opcode_lowering.hpp"

namespace starlark {
namespace native {

namespace {

using ::starlark::bytecode::OpCode;

llvm::FunctionCallee declare_fn(llvm::Module& module, llvm::StringRef name, llvm::FunctionType* ty) {
  return module.getOrInsertFunction(name, ty);
}

int max_label_index(const starlark::bytecode::Block& block) {
  int max_ip = block.op_code().size();
  for (int ip = 0; ip < block.op_code().size(); ++ip) {
    const auto& op = block.op_code(ip);
    switch (op.op_code_case()) {
      case OpCode::kGoto:
        max_ip = std::max(max_ip, ip + op.goto_().address_delta());
        break;
      case OpCode::kJumpIfFalse:
        max_ip = std::max(max_ip, ip + 1);
        max_ip = std::max(max_ip, ip + op.jump_if_false().address_delta());
        break;
      case OpCode::kJumpIfFalseOrPop:
        max_ip = std::max(max_ip, ip + 1);
        max_ip = std::max(max_ip, ip + op.jump_if_false_or_pop().address_delta());
        break;
      case OpCode::kJumpIfTrueOrPop:
        max_ip = std::max(max_ip, ip + 1);
        max_ip = std::max(max_ip, ip + op.jump_if_true_or_pop().address_delta());
        break;
      case OpCode::kForIterator:
        max_ip = std::max(max_ip, ip + 1);
        max_ip = std::max(max_ip, ip + op.for_iterator().address_delta());
        break;
      case OpCode::kForIteratorExt:
        max_ip = std::max(max_ip, ip + 1);
        max_ip = std::max(max_ip, ip + op.for_iterator_ext().address_delta());
        break;
      default:
        max_ip = std::max(max_ip, ip + 1);
        break;
    }
  }
  return max_ip;
}

void ensure_block_labels(llvm::LLVMContext& context,
    llvm::Function* fn,
    std::map<int, llvm::BasicBlock*>& labels,
    int min_ip,
    int max_ip) {
  for (int ip = min_ip; ip <= max_ip; ++ip) {
    if (!labels.contains(ip)) {
      labels[ip] = llvm::BasicBlock::Create(context, std::format("bb_{}", ip), fn);
    }
  }
}

void close_open_labels(std::map<int, llvm::BasicBlock*>& labels, int last_label, llvm::BasicBlock* fallback) {
  for (int ip = 0; ip <= last_label; ++ip) {
    auto it = labels.find(ip);
    if (it == labels.end() || it->second->hasTerminator()) {
      continue;
    }
    llvm::IRBuilder<> builder(it->second);
    builder.CreateBr(fallback);
  }
}

}  // namespace

bytecode_lowering::bytecode_lowering(llvm::LLVMContext& context,
    llvm::Module& module,
    const starlark::bytecode::Program& program,
    const irgen_options& options) :
    context_(context), module_(module), program_(program), options_(options), ir_exec_(context, module) {
  exec_ty_ = ir_exec_.exec_ty();
  obj_ptr_ty_ = ir_exec_.obj_ptr_ty();
  ctx_ptr_ty_ = ir_exec_.ctx_ptr_ty();
  err_ptr_ty_ = ir_exec_.err_ptr_ty();
  i1_ty_ = ir_exec_.i1_ty();
  i32_ty_ = ir_exec_.i32_ty();
  declare_runtime_functions();
}

void bytecode_lowering::declare_runtime_functions() {
  auto* void_ty = llvm::Type::getVoidTy(context_);
  declare_fn(module_, "starlark_rt_exec_create_predeclared", llvm::FunctionType::get(i1_ty_, {exec_ty_, exec_ty_, ctx_ptr_ty_, err_ptr_ty_}, false));
  auto* i32_ty = llvm::Type::getInt32Ty(context_);
  declare_fn(module_, "starlark_rt_exec_create_frame_meta", llvm::FunctionType::get(void_ty, {exec_ty_, i32_ty}, false));
  declare_fn(module_, "starlark_rt_exec_jit_function_entry", llvm::FunctionType::get(i1_ty_, {exec_ty_}, false));
  declare_fn(module_, "starlark_rt_module_init_begin", llvm::FunctionType::get(exec_ty_, {exec_ty_, err_ptr_ty_}, false));
  declare_fn(module_, "starlark_rt_module_init_end", llvm::FunctionType::get(i1_ty_, {exec_ty_}, false));
}

llvm::Value* bytecode_lowering::emit_truthy_pop(llvm::IRBuilderBase& builder, llvm::Function* fn, llvm::Value* exec, llvm::Value* ctx) {
  auto* value = ir_exec_.emit_pop(builder, exec);
  return ir_object_ops::emit_obj_truthy(ir_exec_, builder, fn, value, ctx);
}

llvm::Value* bytecode_lowering::emit_peek_truthy(llvm::IRBuilderBase& builder, llvm::Function* fn, llvm::Value* exec, llvm::Value* ctx) {
  auto* value = ir_exec_.emit_peek(builder, exec);
  return ir_object_ops::emit_obj_truthy(ir_exec_, builder, fn, value, ctx);
}

bool bytecode_lowering::emit_control_flow_opcode(llvm::Function* fn,
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
    const std::function<llvm::BasicBlock*(int ip)>& next_bb_for) {
  switch (op.op_code_case()) {
    case OpCode::kEnd:
      builder.CreateBr(exit_bb);
      return true;
    case OpCode::kReturn: {
      auto* result = ir_exec_.emit_pop(builder, exec);
      builder.CreateRet(result);
      return true;
    }
    case OpCode::kGoto:
      builder.CreateBr(labels[ip + op.goto_().address_delta()]);
      return true;
    case OpCode::kJumpIfFalse: {
      auto* truthy = emit_truthy_pop(builder, fn, exec, ctx);
      builder.CreateCondBr(truthy, labels[ip + 1], labels[ip + op.jump_if_false().address_delta()]);
      return true;
    }
    case OpCode::kJumpIfFalseOrPop: {
      auto* truthy = emit_peek_truthy(builder, fn, exec, ctx);
      auto* pop_bb = llvm::BasicBlock::Create(context_, std::format("pop_{}", ip), fn);
      builder.CreateCondBr(truthy, pop_bb, labels[ip + op.jump_if_false_or_pop().address_delta()]);
      builder.SetInsertPoint(pop_bb);
      ir_exec_.emit_pop(builder, exec);
      builder.CreateBr(labels[ip + 1]);
      return true;
    }
    case OpCode::kJumpIfTrueOrPop: {
      auto* truthy = emit_peek_truthy(builder, fn, exec, ctx);
      auto* pop_bb = llvm::BasicBlock::Create(context_, std::format("pop_true_{}", ip), fn);
      builder.CreateCondBr(truthy, labels[ip + op.jump_if_true_or_pop().address_delta()], pop_bb);
      builder.SetInsertPoint(pop_bb);
      ir_exec_.emit_pop(builder, exec);
      builder.CreateBr(labels[ip + 1]);
      return true;
    }
    case OpCode::kForIterator:
    case OpCode::kForIteratorExt: {
      int delta = op.op_code_case() == OpCode::kForIterator ? op.for_iterator().address_delta() : op.for_iterator_ext().address_delta();
      bool extended = op.op_code_case() == OpCode::kForIteratorExt;
      lowering_context lowering_ctx{ir_exec_, module_, options_, function_blocks_};
      auto* has_next = ir_object_ops::emit_for_iterator_cond(lowering_ctx, builder, fn, exec, ctx, err, extended);
      builder.CreateCondBr(has_next, labels[ip + 1], labels[ip + delta]);
      return true;
    }
    default:
      emit_opcode(fn,
          labels[ip],
          error_bb,
          next_bb_for(ip),
          block_idx,
          ip,
          exec);
      return true;
  }
}

llvm::Function* bytecode_lowering::lower_module_init() {
  auto* init_fn_ty = llvm::FunctionType::get(i1_ty_, {exec_ty_, err_ptr_ty_}, false);
  auto* init_fn = llvm::Function::Create(init_fn_ty, llvm::Function::ExternalLinkage, "starlark_module_init", &module_);
  auto* entry = llvm::BasicBlock::Create(context_, "entry", init_fn);
  llvm::IRBuilder<> builder(entry);
  assert(!program_.block().empty());
  auto* exec = builder.CreateCall(module_.getFunction("starlark_rt_module_init_begin"),
      {init_fn->getArg(0), init_fn->getArg(1)},
      "exec");
  builder.CreateCall(ir_exec_.object_fn("starlark_obj_rt_ensure_stack_capacity"),
      {exec, llvm::ConstantInt::get(i32_ty_, options_.max_stack_depth)});
  auto* ctx = ir_exec_.emit_load_ctx(builder, exec);
  const auto& block = program_.block(0);
  auto& labels = block_labels_[0];
  ensure_block_labels(context_, init_fn, labels, 0, max_label_index(block));
  auto* exit_bb = llvm::BasicBlock::Create(context_, "exit", init_fn);
  auto* error_bb = llvm::BasicBlock::Create(context_, "error", init_fn);
  builder.CreateBr(labels[0]);
  auto next_bb_for = [&](int current_ip) {
    return labels.contains(current_ip + 1) ? labels[current_ip + 1] : exit_bb;
  };
  for (int ip = 0; ip < block.op_code().size(); ++ip) {
    builder.SetInsertPoint(labels[ip]);
    emit_control_flow_opcode(init_fn,
        builder,
        exec,
        ctx,
        init_fn->getArg(1),
        block.op_code(ip),
        ip,
        0,
        labels,
        exit_bb,
        error_bb,
        next_bb_for);
  }
  close_open_labels(labels, max_label_index(block), exit_bb);
  builder.SetInsertPoint(error_bb);
  builder.CreateRet(llvm::ConstantInt::getFalse(context_));
  builder.SetInsertPoint(exit_bb);
  auto* ok = builder.CreateCall(module_.getFunction("starlark_rt_module_init_end"), {exec});
  builder.CreateRet(ok);
  return init_fn;
}

void bytecode_lowering::declare_function_block(int block_idx) {
  if (function_blocks_.contains(block_idx)) {
    return;
  }
  auto* fn_ty = llvm::FunctionType::get(obj_ptr_ty_,
      {exec_ty_, llvm::PointerType::getUnqual(context_), i32_ty_, ctx_ptr_ty_, err_ptr_ty_},
      false);
  auto name = std::format("starlark_fn_{:x}_{}", options_.cache_key, block_idx);
  function_blocks_[block_idx] = llvm::Function::Create(fn_ty, llvm::Function::ExternalLinkage, name, &module_);
}

llvm::Function* bytecode_lowering::lower_function_block(int block_idx) {
  declare_function_block(block_idx);
  auto* fn = function_blocks_[block_idx];
  auto* entry = llvm::BasicBlock::Create(context_, "entry", fn);
  llvm::IRBuilder<> builder(entry);
  auto* exec = fn->getArg(0);
  auto* ctx = fn->getArg(3);
  lowering_context lowering_ctx{ir_exec_, module_, options_, function_blocks_};
  ir_object_ops::emit_function_prologue_from_meta(lowering_ctx, builder, fn, exec, block_idx);
  const auto& block = program_.block(block_idx);
  auto& labels = block_labels_[block_idx];
  constexpr int start_ip = 1;
  assert(static_cast<int>(block.op_code().size()) > start_ip);
  ensure_block_labels(context_, fn, labels, 0, max_label_index(block));
  auto* error_bb = llvm::BasicBlock::Create(context_, "error", fn);
  if (labels.contains(0)) {
    llvm::IRBuilder<> ip0_builder(labels[0]);
    ip0_builder.CreateBr(labels[start_ip]);
  }
  builder.CreateBr(labels[start_ip]);
  auto next_bb_for = [&](int current_ip) -> llvm::BasicBlock* {
    if (labels.contains(current_ip + 1)) {
      return labels[current_ip + 1];
    }
    auto* tail_bb = llvm::BasicBlock::Create(context_, std::format("tail_{}", current_ip), fn);
    llvm::IRBuilder<> tail_builder(tail_bb);
    auto* result = ir_exec_.emit_pop(tail_builder, exec);
    tail_builder.CreateRet(result);
    return tail_bb;
  };
  for (int ip = start_ip; ip < block.op_code().size(); ++ip) {
    builder.SetInsertPoint(labels[ip]);
    emit_control_flow_opcode(fn,
        builder,
        exec,
        ctx,
        fn->getArg(4),
        block.op_code(ip),
        ip,
        block_idx,
        labels,
        nullptr,
        error_bb,
        next_bb_for);
  }
  const int last_label = max_label_index(block);
  for (int ip = block.op_code().size(); ip <= last_label; ++ip) {
    auto it = labels.find(ip);
    if (it == labels.end() || it->second->hasTerminator()) {
      continue;
    }
    llvm::IRBuilder<> merge_builder(it->second);
    auto* result = ir_exec_.emit_pop(merge_builder, exec);
    merge_builder.CreateRet(result);
  }
  builder.SetInsertPoint(error_bb);
  builder.CreateRet(llvm::ConstantPointerNull::get(obj_ptr_ty_));
  return fn;
}

void bytecode_lowering::emit_opcode(llvm::Function* fn,
    llvm::BasicBlock* bb,
    llvm::BasicBlock* error_bb,
    llvm::BasicBlock* next_bb,
    int block_idx,
    int ip,
    llvm::Value* exec) {
  llvm::IRBuilder<> builder(bb);
  ir_exec_.emit_set_location(builder, exec, block_idx, ip);
  llvm::Value* ctx = fn->arg_size() >= 4 ? static_cast<llvm::Value*>(fn->getArg(3)) : ir_exec_.emit_load_ctx(builder, exec);
  llvm::Value* err = fn->arg_size() >= 5 ? static_cast<llvm::Value*>(fn->getArg(4)) : ir_exec_.emit_load_err(builder, exec);
  lowering_context lowering_ctx{ir_exec_, module_, options_, function_blocks_};
  lower_opcode(lowering_ctx, builder, fn, exec, ctx, err, error_bb, next_bb, block_idx, ip, program_.block(block_idx).op_code(ip), program_);
  llvm::BasicBlock* tail = builder.GetInsertBlock();
  if (!tail->hasTerminator()) {
    ir_exec_.emit_branch_if_failed(builder, exec, error_bb, next_bb);
  }
}

}  // namespace native
}  // namespace starlark
