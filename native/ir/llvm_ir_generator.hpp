// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_IR_LLVM_IR_GENERATOR_HPP_
#define NATIVE_IR_LLVM_IR_GENERATOR_HPP_

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "native/ir/irgen_options.hpp"
#include "proto/starlark_bytecode.pb.h"

#pragma GCC visibility push(default)

namespace llvm {
class LLVMContext;
class Module;
}  // namespace llvm

namespace starlark {
namespace native {

class llvm_ir_generator {
 public:
  explicit llvm_ir_generator(llvm::LLVMContext& context);

  std::unique_ptr<llvm::Module> generate(const starlark::bytecode::Program& program,
      const irgen_options& options);

 private:
  llvm::LLVMContext& context_;
};

}  // namespace native
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_IR_LLVM_IR_GENERATOR_HPP_
