// Copyright 2026 Lucas Mirelmann

#include <map>
#include <string>
#include <vector>

#include "compiler/analysis/stack_depth_analysis.hpp"
#include "google/protobuf/arena.h"
#include "gtest/gtest.h"
#include "logging/logging.hpp"
#include "compiler/test/stack_depth_test_support.hpp"

using ::google::protobuf::Arena;
using ::starlark::bytecode::OpCode;
using ::starlark::bytecode::Program;
using ::starlark::compiler::analysis::analyze_block_stack_peak;
using ::starlark::compiler::analysis::opcode_stack_effect;
using ::starlark::logging::logger;
using ::starlark::compiler::analysis::test::all_defined_opcode_cases;
using ::starlark::compiler::analysis::test::all_star_test_files;
using ::starlark::compiler::analysis::test::collect_opcode_cases;
using ::starlark::compiler::analysis::test::collect_opcodes_from_files;
using ::starlark::compiler::analysis::test::compilable_opcode_inventory;
using ::starlark::compiler::analysis::test::compile_test_file;
using ::starlark::compiler::analysis::test::compiler_internal_opcode_inventory;
using ::starlark::compiler::analysis::test::function_block_stack_peak;
using ::starlark::compiler::analysis::test::make_minimal_opcode;
using ::starlark::compiler::analysis::test::map_opcodes_to_source_files;
using ::starlark::compiler::analysis::test::opcode_case_name;
using ::starlark::compiler::analysis::test::program_stack_peak;
using ::starlark::compiler::analysis::test::required_stack_capacity;
using ::starlark::compiler::analysis::test::runtime_only_opcode_inventory;
using ::starlark::compiler::analysis::test::stack_depth_feature_files;

namespace {

Program* compile_file(const std::string& filename, Arena& arena, logger& logging) {
  Program* program = compile_test_file(filename, arena, logging);
  EXPECT_NE(program, nullptr) << filename;
  return program;
}

}  // namespace

TEST(StackDepthStarlark, FeatureFilesCompile) {
  Arena arena;
  logger logging;
  for (const auto& filename : stack_depth_feature_files()) {
    compile_file(filename, arena, logging);
  }
}

TEST(StackDepthStarlark, FeatureFilesCoverCompilableOpcodes) {
  Arena arena;
  logger logging;
  const auto mapping = map_opcodes_to_source_files(all_star_test_files(), arena, logging);

  for (const auto case_ : compilable_opcode_inventory()) {
    const auto it = mapping.find(case_);
    EXPECT_NE(it, mapping.end()) << opcode_case_name(case_);
    EXPECT_FALSE(it->second.empty()) << opcode_case_name(case_) << " was not emitted by any .star test file";
  }
  for (const auto case_ : runtime_only_opcode_inventory()) {
    const auto it = mapping.find(case_);
    EXPECT_TRUE(it == mapping.end() || it->second.empty())
        << opcode_case_name(case_) << " should not be emitted from Starlark compiler output";
  }
  for (const auto case_ : compiler_internal_opcode_inventory()) {
    const auto it = mapping.find(case_);
    EXPECT_TRUE(it == mapping.end() || it->second.empty())
        << opcode_case_name(case_) << " should be stripped from Starlark compiler output";
  }
}

TEST(StackDepthStarlark, OpcodeInventoryMatchesProtoDefinition) {
  EXPECT_EQ(compilable_opcode_inventory().size() + runtime_only_opcode_inventory().size() + compiler_internal_opcode_inventory().size(),
      all_defined_opcode_cases().size());
}

TEST(StackDepthStarlark, OpcodeStackEffectsAreExhaustive) {
  for (const auto case_ : compilable_opcode_inventory()) {
    const auto op = make_minimal_opcode(case_);
    EXPECT_EQ(op.op_code_case(), case_) << opcode_case_name(case_);
    EXPECT_NO_FATAL_FAILURE(static_cast<void>(opcode_stack_effect(op))) << opcode_case_name(case_);
  }
  for (const auto case_ : runtime_only_opcode_inventory()) {
    const auto op = make_minimal_opcode(case_);
    EXPECT_NO_FATAL_FAILURE(static_cast<void>(opcode_stack_effect(op))) << opcode_case_name(case_);
  }
}

TEST(StackDepthStarlark, AllStarProgramsAnalyzeWithoutStackUnderflow) {
  Arena arena;
  logger logging;
  for (const auto& filename : all_star_test_files()) {
    Program* program = compile_file(filename, arena, logging);
    ASSERT_NE(program, nullptr) << filename;
    EXPECT_GT(required_stack_capacity(*program), 0u) << filename;
    for (const auto& block : program->block()) {
      EXPECT_NO_FATAL_FAILURE(static_cast<void>(analyze_block_stack_peak(block))) << filename;
    }
  }
}

TEST(StackDepthStarlark, RequiredCapacityIsPeakPlusOne) {
  Arena arena;
  logger logging;
  for (const auto& filename : all_star_test_files()) {
    Program* program = compile_file(filename, arena, logging);
    ASSERT_NE(program, nullptr) << filename;
    EXPECT_EQ(required_stack_capacity(*program), static_cast<uint32_t>(program_stack_peak(*program)) + 1u)
        << filename;
  }
}

TEST(StackDepthStarlark, UnaryOperatorsAreInPlace) {
  Arena arena;
  logger logging;
  Program* plain = compile_file("peaks_call_plain.star", arena, logging);
  Program* negated = compile_file("peaks_call_negated.star", arena, logging);
  ASSERT_NE(plain, nullptr);
  ASSERT_NE(negated, nullptr);
  EXPECT_EQ(program_stack_peak(*plain), 19);
  EXPECT_EQ(program_stack_peak(*negated), 19);
  EXPECT_EQ(required_stack_capacity(*plain), 20u);
  EXPECT_EQ(required_stack_capacity(*negated), 20u);

  Program* peaks = compile_file("peaks_unary.star", arena, logging);
  ASSERT_NE(peaks, nullptr);
  const auto binary_peak = function_block_stack_peak(*peaks, "binary_peak");
  const auto unary_peak = function_block_stack_peak(*peaks, "unary_neg_peak");
  ASSERT_TRUE(binary_peak.has_value());
  ASSERT_TRUE(unary_peak.has_value());
  EXPECT_EQ(*unary_peak, *binary_peak);
}

TEST(StackDepthStarlark, MakeFunctionWithDefaultParameters) {
  Arena arena;
  logger logging;
  Program* defaults = compile_file("peaks_defaults.star", arena, logging);
  ASSERT_NE(defaults, nullptr);

  EXPECT_EQ(program_stack_peak(*defaults), 17);
  EXPECT_EQ(required_stack_capacity(*defaults), 18u);
}

TEST(StackDepthStarlark, TupleConstructionPeak) {
  Arena arena;
  logger logging;
  Program* program = compile_file("peaks_unary.star", arena, logging);
  ASSERT_NE(program, nullptr);

  const auto tuple_peak = function_block_stack_peak(*program, "tuple_peak");
  ASSERT_TRUE(tuple_peak.has_value());
  EXPECT_EQ(*tuple_peak, 3);
}

TEST(StackDepthStarlark, ForLoopWithIndexCompoundAssign) {
  Arena arena;
  logger logging;
  Program* program = compile_file("peaks_for.star", arena, logging);
  ASSERT_NE(program, nullptr);
  EXPECT_GE(required_stack_capacity(*program), 2u);
  EXPECT_TRUE(collect_opcode_cases(*program).contains(OpCode::kAssignIndexMemberPlusEquals));
}
