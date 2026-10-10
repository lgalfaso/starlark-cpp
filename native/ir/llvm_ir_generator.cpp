// Copyright 2026 Lucas Mirelmann

#include "native/ir/llvm_ir_generator.hpp"

#include <cassert>

#include <format>
#include <set>
#include <memory>

#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "native/ir/bytecode_lowering.hpp"

namespace starlark {
namespace native {

llvm_ir_generator::llvm_ir_generator(llvm::LLVMContext& context) : context_(context) {}

std::unique_ptr<llvm::Module> llvm_ir_generator::generate(const starlark::bytecode::Program& program,
    const irgen_options& options) {
  assert(options.metadata != nullptr);
  assert(program.max_eval_stack_depth() > 0);

  auto module_name = std::format("starlark_mod_{:x}", options.cache_key);
  auto module = std::make_unique<llvm::Module>(module_name, context_);

  auto* ptr_ty = llvm::PointerType::getUnqual(context_);

  irgen_options lowering_options = options;
  if (lowering_options.max_stack_depth == 0) {
    lowering_options.max_stack_depth = program.max_eval_stack_depth();
  }

  bytecode_lowering lowering(context_, *module, program, lowering_options);

  std::set<int> function_blocks;
  if (!program.block().empty()) {
    for (const auto& op : program.block(0).op_code()) {
      if (op.op_code_case() == starlark::bytecode::OpCode::kMakeFunction) {
        function_blocks.insert(op.make_function().entrypoint());
      }
    }
  }
  for (int block_idx = 1; block_idx < program.block().size(); ++block_idx) {
    function_blocks.insert(block_idx);
  }
  for (int block_idx : function_blocks) {
    if (block_idx > 0 && block_idx < program.block().size()) {
      lowering.declare_function_block(block_idx);
    }
  }
  for (int block_idx : function_blocks) {
    if (block_idx > 0 && block_idx < program.block().size()) {
      lowering.lower_function_block(block_idx);
    }
  }

  auto* init_fn = lowering.lower_module_init();
  auto* init_ptr = llvm::ConstantExpr::getPointerCast(init_fn, ptr_ty);
  llvm::SmallVector<llvm::Type*, 1> desc_field_types;
  desc_field_types.push_back(ptr_ty);
  auto* desc_ty = llvm::StructType::get(context_, desc_field_types);
  auto* desc_init = llvm::ConstantStruct::get(desc_ty, {init_ptr});
  new llvm::GlobalVariable(*module,
      desc_ty,
      true,
      llvm::GlobalValue::ExternalLinkage,
      desc_init,
      std::format("starlark_module_descriptor_{:x}", options.cache_key));

  return module;
}

}  // namespace native
}  // namespace starlark
