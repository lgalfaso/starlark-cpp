// Copyright 2026 Lucas Mirelmann

#ifndef COMPILER_ANALYSIS_STACK_DEPTH_ANALYSIS_HPP_
#define COMPILER_ANALYSIS_STACK_DEPTH_ANALYSIS_HPP_

#include <cstdint>

#include "proto/starlark_bytecode.pb.h"

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {
namespace analysis {

// Returns eval-stack capacity: max simultaneous stack depth across all bytecode blocks, plus one.
uint32_t analyze_max_stack_depth(const starlark::bytecode::Program& program);

// Returns the net eval-stack delta for a single opcode in a straight-line sequence.
int opcode_stack_effect(const starlark::bytecode::OpCode& op);

// Returns the maximum eval-stack depth reached in one bytecode block, without padding.
int analyze_block_stack_peak(const starlark::bytecode::Block& block);

}  // namespace analysis
}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_ANALYSIS_STACK_DEPTH_ANALYSIS_HPP_
