// Copyright 2026 Lucas Mirelmann

#include <memory>
#include <string>

#include "gtest/gtest.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "native/ir/llvm_ir_generator.hpp"
#include "vm/module_metadata.hpp"

using ::starlark::native::irgen_options;
using ::starlark::native::llvm_ir_generator;

namespace {

irgen_options make_test_options(starlark::bytecode::Program& program, uint64_t cache_key, starlark::vm::module_metadata& metadata_storage) {
  metadata_storage = starlark::vm::module_metadata::build(program);
  if (program.max_eval_stack_depth() == 0) {
    program.set_max_eval_stack_depth(32);
  }
  irgen_options options{
      .cache_key = cache_key,
      .max_stack_depth = program.max_eval_stack_depth(),
      .metadata = &metadata_storage,
  };
  return options;
}

int count_calls(const llvm::Module& module, llvm::StringRef callee_name) {
  int count = 0;
  for (const auto& fn : module.functions()) {
    for (const auto& block : fn) {
      for (const auto& inst : block) {
        if (auto* call = llvm::dyn_cast<llvm::CallBase>(&inst)) {
          if (auto* callee = call->getCalledFunction()) {
            if (callee->getName() == callee_name) {
              ++count;
            }
          }
        }
      }
    }
  }
  return count;
}

bool module_contains_ir(const llvm::Module& module, llvm::StringRef needle) {
  std::string ir;
  llvm::raw_string_ostream stream(ir);
  module.print(stream, nullptr);
  return ir.find(needle.str()) != std::string::npos;
}

}  // namespace

TEST(BytecodeLowering, EmitsModuleInitAndFunctionSymbols) {
  starlark::bytecode::Program program;
  auto* module_block = program.add_block();
  {
    auto* op = module_block->add_op_code()->mutable_create_frame();
    op->set_block_type(starlark::bytecode::BlockType::PREDECLARED_BLOCK);
  }
  {
    auto* op = module_block->add_op_code()->mutable_create_frame();
    op->set_block_type(starlark::bytecode::BlockType::MODULE_BLOCK);
    op->add_symbol("foo");
  }
  module_block->add_op_code()->mutable_create_frame()->set_block_type(starlark::bytecode::BlockType::FILE_BLOCK);
  module_block->add_op_code()->mutable_make_function()->set_entrypoint(1);
  module_block->add_op_code()->mutable_end();

  auto* fn_block = program.add_block();
  fn_block->add_op_code()->mutable_create_frame()->set_block_type(starlark::bytecode::BlockType::FUNCTION_BLOCK);
  fn_block->add_op_code()->mutable_const_int()->set_value(1);
  fn_block->add_op_code()->mutable_return_();
  fn_block->mutable_function_signature()->set_fn_name("foo");

  auto context = std::make_unique<llvm::LLVMContext>();
  llvm_ir_generator generator(*context);
  starlark::vm::module_metadata metadata;
  auto options = make_test_options(program, 0x42, metadata);
  auto module = generator.generate(program, options);
  ASSERT_NE(nullptr, module);
  EXPECT_NE(nullptr, module->getFunction("starlark_module_init"));
  EXPECT_NE(nullptr, module->getFunction("starlark_fn_42_1"));
}

TEST(BytecodeLowering, InlinesConstNoneAndIntFastPaths) {
  starlark::bytecode::Program program;
  auto* module_block = program.add_block();
  module_block->add_op_code()->mutable_create_frame()->set_block_type(starlark::bytecode::BlockType::MODULE_BLOCK);
  module_block->add_op_code()->mutable_end();

  auto* fn_block = program.add_block();
  fn_block->add_op_code()->mutable_create_frame()->set_block_type(starlark::bytecode::BlockType::FUNCTION_BLOCK);
  fn_block->add_op_code()->mutable_const_none();
  fn_block->add_op_code()->mutable_const_int()->set_value(1);
  fn_block->add_op_code()->mutable_const_int()->set_value(2);
  fn_block->add_op_code()->mutable_binary_plus();
  fn_block->add_op_code()->mutable_return_();
  fn_block->mutable_function_signature()->set_fn_name("add");

  auto context = std::make_unique<llvm::LLVMContext>();
  llvm_ir_generator generator(*context);
  starlark::vm::module_metadata metadata;
  auto options = make_test_options(program, 0x99, metadata);
  auto module = generator.generate(program, options);
  ASSERT_NE(nullptr, module);
  EXPECT_EQ(0, count_calls(*module, "starlark_obj_rt_push_none"));
  EXPECT_TRUE(module_contains_ir(*module, "bin_fast"));
  EXPECT_TRUE(module_contains_ir(*module, "starlark_obj_rt_ctx_none_value"));
}

TEST(BytecodeLowering, UsesMetadataForMakeFunctionAndFrames) {
  starlark::bytecode::Program program;
  auto* module_block = program.add_block();
  module_block->add_op_code()->mutable_create_frame()->set_block_type(starlark::bytecode::BlockType::MODULE_BLOCK);
  module_block->add_op_code()->mutable_make_function()->set_entrypoint(1);
  module_block->add_op_code()->mutable_end();

  auto* fn_block = program.add_block();
  fn_block->add_op_code()->mutable_create_frame()->set_block_type(starlark::bytecode::BlockType::FUNCTION_BLOCK);
  fn_block->add_op_code()->mutable_return_();
  fn_block->mutable_function_signature()->set_fn_name("exported_fn");

  auto context = std::make_unique<llvm::LLVMContext>();
  llvm_ir_generator generator(*context);
  starlark::vm::module_metadata metadata;
  auto options = make_test_options(program, 0xab, metadata);
  auto module = generator.generate(program, options);
  ASSERT_NE(nullptr, module);
  EXPECT_GE(count_calls(*module, "starlark_obj_rt_make_native_function_meta"), 1);
  EXPECT_TRUE(module_contains_ir(*module, "starlark_rt_exec_create_frame_meta"));
  EXPECT_TRUE(module_contains_ir(*module, "starlark_obj_rt_ensure_stack_capacity"));
}

TEST(BytecodeLowering, RoutesSpecializedCallAndIteratorOps) {
  starlark::bytecode::Program program;
  auto* module_block = program.add_block();
  module_block->add_op_code()->mutable_create_frame()->set_block_type(starlark::bytecode::BlockType::MODULE_BLOCK);
  module_block->add_op_code()->mutable_end();

  auto* fn_block = program.add_block();
  fn_block->add_op_code()->mutable_create_frame()->set_block_type(starlark::bytecode::BlockType::FUNCTION_BLOCK);
  fn_block->add_op_code()->mutable_call_pos0();
  fn_block->add_op_code()->mutable_make_tuple()->set_number_of_elements(2);
  fn_block->add_op_code()->mutable_unpack()->set_number_of_elements(2);
  fn_block->add_op_code()->mutable_return_();
  fn_block->mutable_function_signature()->set_fn_name("ops");

  auto context = std::make_unique<llvm::LLVMContext>();
  llvm_ir_generator generator(*context);
  starlark::vm::module_metadata metadata;
  auto options = make_test_options(program, 0xcd, metadata);
  auto module = generator.generate(program, options);
  ASSERT_NE(nullptr, module);
  EXPECT_GE(count_calls(*module, "starlark_obj_rt_call_pos0"), 1);
  EXPECT_GE(count_calls(*module, "starlark_obj_rt_unpack"), 1);
}
