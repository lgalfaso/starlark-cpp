// Copyright 2026 Lucas Mirelmann

#include "compiler/tools/bytecode_golden_updater.hpp"

int main() {
  return starlark::compiler::tools::update_bytecode_goldens("compiler/bytecode_generator_tests");
}
