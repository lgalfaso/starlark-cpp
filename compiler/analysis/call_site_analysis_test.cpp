// Copyright 2026 Lucas Mirelmann

#include "compiler/analysis/call_site_analysis.hpp"
#include "google/protobuf/arena.h"
#include "gtest/gtest.h"
#include "logging/logging.hpp"
#include "compiler/analysis/stack_depth_test_support.hpp"
#include "proto/starlark_bytecode.pb.h"

using ::google::protobuf::Arena;
using ::starlark::bytecode::OpCode;
using ::starlark::bytecode::Program;
using ::starlark::compiler::analysis::analyze_static_self_calls;
using ::starlark::logging::logger;
using ::starlark::compiler::analysis::test::compile_test_file;
using ::starlark::compiler::analysis::test::function_block_index;

namespace {

Program* compile_file(const std::string& filename, Arena& arena, logger& logging) {
  Program* program = compile_test_file(filename, arena, logging);
  EXPECT_NE(program, nullptr) << filename;
  return program;
}

bool block_contains_call_opcode(const Program& program, int block_idx) {
  const auto& block = program.block(block_idx);
  for (const auto& op : block.op_code()) {
    switch (op.op_code_case()) {
      case OpCode::kCallPos0:
      case OpCode::kCallPos1:
      case OpCode::kCallPos2:
      case OpCode::kCallPos3:
      case OpCode::kCallPos:
        return true;
      default:
        break;
    }
  }
  return false;
}

}  // namespace

TEST(CallSiteAnalysis, DetectsSelfRecursiveCallWithComputedArgument) {
  Arena arena;
  logger logging;
  Program* program = compile_file("self_recursive.star", arena, logging);
  ASSERT_NE(program, nullptr);

  const auto block_idx = function_block_index(*program, "countdown");
  ASSERT_TRUE(block_idx.has_value());
  EXPECT_TRUE(block_contains_call_opcode(*program, *block_idx));

  const auto known_calls = analyze_static_self_calls(*program, *block_idx);
  EXPECT_FALSE(known_calls.empty());
  for (const auto& [call_ip, entrypoint] : known_calls) {
    EXPECT_EQ(entrypoint, *block_idx) << "call at ip " << call_ip;
  }
}
