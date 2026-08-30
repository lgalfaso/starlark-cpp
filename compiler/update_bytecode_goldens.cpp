// Copyright 2026 Lucas Mirelmann

#include "proto/bytecode_formatter/bytecode_golden_updater.hpp"

int main() {
  return starlark::proto::update_bytecode_goldens("compiler/bytecode_generator_tests");
}
