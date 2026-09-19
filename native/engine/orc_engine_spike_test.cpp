// Copyright 2026 Lucas Mirelmann

#include <memory>

#include "gtest/gtest.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "native/engine/orc_engine.hpp"
#include "native/ir/llvm_ir_generator.hpp"
#include "vm/module_metadata.hpp"

using ::starlark::native::irgen_options;
using ::starlark::native::llvm_ir_generator;
using ::starlark::native::orc_engine;

namespace {

starlark::bytecode::Program empty_module_program() {
  starlark::bytecode::Program program;
  program.add_block()->add_op_code()->mutable_end();
  return program;
}

irgen_options make_test_options(starlark::bytecode::Program& program, uint64_t cache_key, starlark::vm::module_metadata& metadata_storage) {
  metadata_storage = starlark::vm::module_metadata::build(program);
  if (program.max_eval_stack_depth() == 0) {
    program.set_max_eval_stack_depth(32);
  }
  return irgen_options{
      .cache_key = cache_key,
      .max_stack_depth = program.max_eval_stack_depth(),
      .metadata = &metadata_storage,
  };
}

}  // namespace

TEST(OrcEngineSpike, LoadsJitModuleDescriptor) {
  orc_engine engine;
  auto context = std::make_unique<llvm::LLVMContext>();
  llvm_ir_generator generator(*context);
  auto program = empty_module_program();
  starlark::vm::module_metadata metadata;
  auto options = make_test_options(program, 0xabc123, metadata);
  auto module = generator.generate(program, options);
  ASSERT_NE(nullptr, module);

  auto loaded = engine.load_jit(0xabc123, std::move(module), std::move(context));
  ASSERT_TRUE(loaded.ok());
  EXPECT_NE(nullptr, *loaded);
  EXPECT_NE(nullptr, (*loaded)->init);
}

TEST(OrcEngineSpike, LoadsSecondModuleAfterFirst) {
  orc_engine engine;
  for (uint64_t key : {0xabc124ULL, 0xabc125ULL}) {
    auto context = std::make_unique<llvm::LLVMContext>();
    llvm_ir_generator generator(*context);
    auto program = empty_module_program();
    starlark::vm::module_metadata metadata;
    auto options = make_test_options(program, key, metadata);
    auto module = generator.generate(program, options);
    ASSERT_NE(nullptr, module);
    auto loaded = engine.load_jit(key, std::move(module), std::move(context));
    ASSERT_TRUE(loaded.ok()) << key;
    EXPECT_NE(nullptr, (*loaded)->init);
  }
}
