// Copyright 2026 Lucas Mirelmann

#include <filesystem>  // NOLINT(build/c++17)
#include <format>
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

TEST(OrcEngineWriteBack, CreatesDylibAndLoadsFromCache) {
  const uint64_t cache_key = 0xdeadbeefULL;
  auto context = std::make_unique<llvm::LLVMContext>();
  llvm_ir_generator generator(*context);
  auto program = empty_module_program();
  starlark::vm::module_metadata metadata;
  auto options = make_test_options(program, cache_key, metadata);
  auto module = generator.generate(program, options);
  ASSERT_NE(nullptr, module);

  auto temp_root = std::filesystem::temp_directory_path() / "starlark_orc_write_back_test";
  std::error_code ec;
  std::filesystem::remove_all(temp_root, ec);
  std::filesystem::create_directories(temp_root, ec);
  ASSERT_FALSE(ec) << ec.message();

  auto dylib_path = temp_root / "module.dylib";
  orc_engine engine;
  auto write_status = engine.write_back(cache_key, *module, dylib_path);
  ASSERT_TRUE(write_status.ok());

  ASSERT_TRUE(std::filesystem::exists(dylib_path));
  EXPECT_GT(std::filesystem::file_size(dylib_path), 0u);

  auto loaded = engine.load_cached(cache_key, dylib_path);
  ASSERT_TRUE(loaded.ok());
  EXPECT_NE(nullptr, *loaded);
  EXPECT_NE(nullptr, (*loaded)->init);

  std::filesystem::remove_all(temp_root, ec);
}

TEST(OrcEngineWriteBack, CanLinkTwiceInSameProcess) {
  auto context = std::make_unique<llvm::LLVMContext>();
  llvm_ir_generator generator(*context);

  auto temp_root = std::filesystem::temp_directory_path() / "starlark_orc_write_back_twice_test";
  std::error_code ec;
  std::filesystem::remove_all(temp_root, ec);
  std::filesystem::create_directories(temp_root, ec);
  ASSERT_FALSE(ec) << ec.message();

  orc_engine engine;
  for (uint64_t cache_key : {0xaaaULL, 0xbbbULL}) {
    auto program = empty_module_program();
    starlark::vm::module_metadata metadata;
    auto options = make_test_options(program, cache_key, metadata);
    auto module = generator.generate(program, options);
    ASSERT_NE(nullptr, module);

    auto dylib_path = temp_root / std::format("module_{:x}.dylib", cache_key);
    auto write_status = engine.write_back(cache_key, *module, dylib_path);
    ASSERT_TRUE(write_status.ok());
    ASSERT_TRUE(std::filesystem::exists(dylib_path));
    EXPECT_GT(std::filesystem::file_size(dylib_path), 0u);
  }

  std::filesystem::remove_all(temp_root, ec);
}
