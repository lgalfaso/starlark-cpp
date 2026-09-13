// Copyright 2026 Lucas Mirelmann

#ifndef COMPILER_ANALYSIS_STACK_DEPTH_TEST_SUPPORT_HPP_
#define COMPILER_ANALYSIS_STACK_DEPTH_TEST_SUPPORT_HPP_

#include <cstdlib>

#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "compiler/analysis/stack_depth_analysis.hpp"
#include "compiler/compiler.hpp"
#include "google/protobuf/arena.h"
#include "grammar/options.hpp"
#include "logging/logging.hpp"
#include "proto/starlark_bytecode.pb.h"

namespace starlark {
namespace compiler {
namespace analysis {
namespace test {

inline grammar::grammar_options test_grammar_options() {
  return grammar::grammar_options{
      .allow_top_level_rebinding = true,
      .allow_top_level_for = true,
      .allow_top_level_if = true,
  };
}

inline std::string read_stack_depth_test_file(const std::string& filename) {
  const char* test_srcdir = std::getenv("TEST_SRCDIR");
  const char* test_workspace = std::getenv("TEST_WORKSPACE");
  std::vector<std::filesystem::path> candidates;
  if (test_srcdir != nullptr && test_workspace != nullptr) {
    candidates.push_back(std::filesystem::path(test_srcdir) / test_workspace / "compiler/analysis/stack_depth_tests" / filename);
  }
  candidates.push_back(std::filesystem::path("compiler/analysis/stack_depth_tests") / filename);

  for (const auto& path : candidates) {
    std::ifstream input(path);
    if (!input.is_open()) {
      continue;
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
  }

  return {};
}

inline bytecode::Program* compile_source(const std::string& module_name,
    const std::string& source,
    google::protobuf::Arena& arena,
    logging::logger& logging) {
  std::set<std::string, std::less<>> binding;
  ::starlark::compiler::compiler star_compiler(binding);
  return star_compiler.compile(module_name, source, test_grammar_options(), logging, arena);
}

inline bytecode::Program* compile_test_file(const std::string& filename,
    google::protobuf::Arena& arena,
    logging::logger& logging) {
  const std::string source = read_stack_depth_test_file(filename);
  return source.empty() ? nullptr : compile_source(filename, source, arena, logging);
}

inline std::set<bytecode::OpCode::OpCodeCase> collect_opcode_cases(const bytecode::Program& program) {
  std::set<bytecode::OpCode::OpCodeCase> cases;
  for (const auto& block : program.block()) {
    for (const auto& op : block.op_code()) {
      if (op.op_code_case() != bytecode::OpCode::OP_CODE_NOT_SET) {
        cases.insert(op.op_code_case());
      }
    }
  }
  return cases;
}

inline int max_block_stack_peak(const bytecode::Program& program) {
  int peak = 0;
  for (const auto& block : program.block()) {
    peak = std::max(peak, analyze_block_stack_peak(block));
  }
  return peak;
}

inline std::optional<int> function_block_stack_peak(const bytecode::Program& program, const std::string& fn_name) {
  std::optional<int> peak;
  for (const auto& block : program.block()) {
    if (block.function_signature().fn_name() != fn_name) {
      continue;
    }
    const int block_peak = analyze_block_stack_peak(block);
    peak = peak.has_value() ? std::max(*peak, block_peak) : block_peak;
  }
  return peak;
}

inline int program_stack_peak(const bytecode::Program& program) {
  return max_block_stack_peak(program);
}

inline uint32_t required_stack_capacity(const bytecode::Program& program) {
  return analyze_max_stack_depth(program);
}

inline std::optional<int> function_block_index(const bytecode::Program& program, const std::string& fn_name) {
  for (int block_idx = 0; block_idx < program.block().size(); ++block_idx) {
    if (program.block(block_idx).function_signature().fn_name() == fn_name) {
      return block_idx;
    }
  }
  return std::nullopt;
}

inline bytecode::OpCode make_minimal_opcode(bytecode::OpCode::OpCodeCase case_) {
  bytecode::OpCode op;
  switch (case_) {
    case bytecode::OpCode::kConstBigInt:
      op.mutable_const_big_int();
      break;
    case bytecode::OpCode::kConstInt:
      op.mutable_const_int();
      break;
    case bytecode::OpCode::kConstFloat:
      op.mutable_const_float();
      break;
    case bytecode::OpCode::kConstString:
      op.mutable_const_string();
      break;
    case bytecode::OpCode::kConstStringView:
      op.mutable_const_string_view();
      break;
    case bytecode::OpCode::kConstBytes:
      op.mutable_const_bytes();
      break;
    case bytecode::OpCode::kConstNone:
      op.mutable_const_none();
      break;
    case bytecode::OpCode::kPop:
      op.mutable_pop();
      break;
    case bytecode::OpCode::kUnaryPlus:
      op.mutable_unary_plus();
      break;
    case bytecode::OpCode::kUnaryMinus:
      op.mutable_unary_minus();
      break;
    case bytecode::OpCode::kUnaryTilde:
      op.mutable_unary_tilde();
      break;
    case bytecode::OpCode::kUnaryNot:
      op.mutable_unary_not();
      break;
    case bytecode::OpCode::kBinaryEqualsEquals:
      op.mutable_binary_equals_equals();
      break;
    case bytecode::OpCode::kBinaryBangEquals:
      op.mutable_binary_bang_equals();
      break;
    case bytecode::OpCode::kBinaryLessThan:
      op.mutable_binary_less_than();
      break;
    case bytecode::OpCode::kBinaryGreaterThan:
      op.mutable_binary_greater_than();
      break;
    case bytecode::OpCode::kBinaryLessThanEquals:
      op.mutable_binary_less_than_equals();
      break;
    case bytecode::OpCode::kBinaryGreaterThanEquals:
      op.mutable_binary_greater_than_equals();
      break;
    case bytecode::OpCode::kBinaryIn:
      op.mutable_binary_in();
      break;
    case bytecode::OpCode::kBinaryNotIn:
      op.mutable_binary_not_in();
      break;
    case bytecode::OpCode::kBinaryPipe:
      op.mutable_binary_pipe();
      break;
    case bytecode::OpCode::kBinaryHat:
      op.mutable_binary_hat();
      break;
    case bytecode::OpCode::kBinaryAmpersand:
      op.mutable_binary_ampersand();
      break;
    case bytecode::OpCode::kBinaryLessThanLessThan:
      op.mutable_binary_less_than_less_than();
      break;
    case bytecode::OpCode::kBinaryGreaterThanGreaterThan:
      op.mutable_binary_greater_than_greater_than();
      break;
    case bytecode::OpCode::kBinaryMinus:
      op.mutable_binary_minus();
      break;
    case bytecode::OpCode::kBinaryPlus:
      op.mutable_binary_plus();
      break;
    case bytecode::OpCode::kBinaryStar:
      op.mutable_binary_star();
      break;
    case bytecode::OpCode::kBinaryPercent:
      op.mutable_binary_percent();
      break;
    case bytecode::OpCode::kBinarySlash:
      op.mutable_binary_slash();
      break;
    case bytecode::OpCode::kBinarySlashSlash:
      op.mutable_binary_slash_slash();
      break;
    case bytecode::OpCode::kGoto:
      op.mutable_goto_();
      break;
    case bytecode::OpCode::kJumpIfFalse:
      op.mutable_jump_if_false();
      break;
    case bytecode::OpCode::kJumpIfTrueOrPop:
      op.mutable_jump_if_true_or_pop();
      break;
    case bytecode::OpCode::kJumpIfFalseOrPop:
      op.mutable_jump_if_false_or_pop();
      break;
    case bytecode::OpCode::kMakeTuple:
      op.mutable_make_tuple()->set_number_of_elements(0);
      break;
    case bytecode::OpCode::kMakeList:
      op.mutable_make_list();
      break;
    case bytecode::OpCode::kAddToList:
      op.mutable_add_to_list()->set_number_of_elements(0);
      break;
    case bytecode::OpCode::kMakeDictionary:
      op.mutable_make_dictionary();
      break;
    case bytecode::OpCode::kAddToDictionary:
      op.mutable_add_to_dictionary()->set_number_of_elements(0);
      break;
    case bytecode::OpCode::kLoad:
      op.mutable_load();
      break;
    case bytecode::OpCode::kStore:
      op.mutable_store();
      break;
    case bytecode::OpCode::kGetIterator:
      op.mutable_get_iterator();
      break;
    case bytecode::OpCode::kForIterator:
      op.mutable_for_iterator();
      break;
    case bytecode::OpCode::kForIteratorExt:
      op.mutable_for_iterator_ext();
      break;
    case bytecode::OpCode::kEndIterator:
      op.mutable_end_iterator();
      break;
    case bytecode::OpCode::kUnpack:
      op.mutable_unpack()->set_number_of_elements(0);
      break;
    case bytecode::OpCode::kAssignPlusEquals:
      op.mutable_assign_plus_equals();
      break;
    case bytecode::OpCode::kAssignMinusEquals:
      op.mutable_assign_minus_equals();
      break;
    case bytecode::OpCode::kAssignStarEquals:
      op.mutable_assign_star_equals();
      break;
    case bytecode::OpCode::kAssignSlashEquals:
      op.mutable_assign_slash_equals();
      break;
    case bytecode::OpCode::kAssignSlashSlashEquals:
      op.mutable_assign_slash_slash_equals();
      break;
    case bytecode::OpCode::kAssignPercentEquals:
      op.mutable_assign_percent_equals();
      break;
    case bytecode::OpCode::kAssignAmpersandEquals:
      op.mutable_assign_ampersand_equals();
      break;
    case bytecode::OpCode::kAssignPipeEquals:
      op.mutable_assign_pipe_equals();
      break;
    case bytecode::OpCode::kAssignHatEquals:
      op.mutable_assign_hat_equals();
      break;
    case bytecode::OpCode::kAssignLessLessEquals:
      op.mutable_assign_less_less_equals();
      break;
    case bytecode::OpCode::kAssignGreaterGreaterEquals:
      op.mutable_assign_greater_greater_equals();
      break;
    case bytecode::OpCode::kDotMember:
      op.mutable_dot_member();
      break;
    case bytecode::OpCode::kAssignDotMember:
      op.mutable_assign_dot_member();
      break;
    case bytecode::OpCode::kAssignDotMemberPlusEquals:
      op.mutable_assign_dot_member_plus_equals();
      break;
    case bytecode::OpCode::kAssignDotMemberMinusEquals:
      op.mutable_assign_dot_member_minus_equals();
      break;
    case bytecode::OpCode::kAssignDotMemberStarEquals:
      op.mutable_assign_dot_member_star_equals();
      break;
    case bytecode::OpCode::kAssignDotMemberSlashEquals:
      op.mutable_assign_dot_member_slash_equals();
      break;
    case bytecode::OpCode::kAssignDotMemberSlashSlashEquals:
      op.mutable_assign_dot_member_slash_slash_equals();
      break;
    case bytecode::OpCode::kAssignDotMemberPercentEquals:
      op.mutable_assign_dot_member_percent_equals();
      break;
    case bytecode::OpCode::kAssignDotMemberAmpersandEquals:
      op.mutable_assign_dot_member_ampersand_equals();
      break;
    case bytecode::OpCode::kAssignDotMemberPipeEquals:
      op.mutable_assign_dot_member_pipe_equals();
      break;
    case bytecode::OpCode::kAssignDotMemberHatEquals:
      op.mutable_assign_dot_member_hat_equals();
      break;
    case bytecode::OpCode::kAssignDotMemberLessLessEquals:
      op.mutable_assign_dot_member_less_less_equals();
      break;
    case bytecode::OpCode::kAssignDotMemberGreaterGreaterEquals:
      op.mutable_assign_dot_member_greater_greater_equals();
      break;
    case bytecode::OpCode::kIndexMember:
      op.mutable_index_member();
      break;
    case bytecode::OpCode::kAssignIndexMember:
      op.mutable_assign_index_member();
      break;
    case bytecode::OpCode::kAssignIndexMemberPlusEquals:
      op.mutable_assign_index_member_plus_equals();
      break;
    case bytecode::OpCode::kAssignIndexMemberMinusEquals:
      op.mutable_assign_index_member_minus_equals();
      break;
    case bytecode::OpCode::kAssignIndexMemberStarEquals:
      op.mutable_assign_index_member_star_equals();
      break;
    case bytecode::OpCode::kAssignIndexMemberSlashEquals:
      op.mutable_assign_index_member_slash_equals();
      break;
    case bytecode::OpCode::kAssignIndexMemberSlashSlashEquals:
      op.mutable_assign_index_member_slash_slash_equals();
      break;
    case bytecode::OpCode::kAssignIndexMemberPercentEquals:
      op.mutable_assign_index_member_percent_equals();
      break;
    case bytecode::OpCode::kAssignIndexMemberAmpersandEquals:
      op.mutable_assign_index_member_ampersand_equals();
      break;
    case bytecode::OpCode::kAssignIndexMemberPipeEquals:
      op.mutable_assign_index_member_pipe_equals();
      break;
    case bytecode::OpCode::kAssignIndexMemberHatEquals:
      op.mutable_assign_index_member_hat_equals();
      break;
    case bytecode::OpCode::kAssignIndexMemberLessLessEquals:
      op.mutable_assign_index_member_less_less_equals();
      break;
    case bytecode::OpCode::kAssignIndexMemberGreaterGreaterEquals:
      op.mutable_assign_index_member_greater_greater_equals();
      break;
    case bytecode::OpCode::kSliceRange:
      op.mutable_slice_range();
      break;
    case bytecode::OpCode::kAssignSliceRange:
      op.mutable_assign_slice_range();
      break;
    case bytecode::OpCode::kAssignSliceRangePlusEquals:
      op.mutable_assign_slice_range_plus_equals();
      break;
    case bytecode::OpCode::kAssignSliceRangeMinusEquals:
      op.mutable_assign_slice_range_minus_equals();
      break;
    case bytecode::OpCode::kAssignSliceRangeStarEquals:
      op.mutable_assign_slice_range_star_equals();
      break;
    case bytecode::OpCode::kAssignSliceRangeSlashEquals:
      op.mutable_assign_slice_range_slash_equals();
      break;
    case bytecode::OpCode::kAssignSliceRangeSlashSlashEquals:
      op.mutable_assign_slice_range_slash_slash_equals();
      break;
    case bytecode::OpCode::kAssignSliceRangePercentEquals:
      op.mutable_assign_slice_range_percent_equals();
      break;
    case bytecode::OpCode::kAssignSliceRangeAmpersandEquals:
      op.mutable_assign_slice_range_ampersand_equals();
      break;
    case bytecode::OpCode::kAssignSliceRangePipeEquals:
      op.mutable_assign_slice_range_pipe_equals();
      break;
    case bytecode::OpCode::kAssignSliceRangeHatEquals:
      op.mutable_assign_slice_range_hat_equals();
      break;
    case bytecode::OpCode::kAssignSliceRangeLessLessEquals:
      op.mutable_assign_slice_range_less_less_equals();
      break;
    case bytecode::OpCode::kAssignSliceRangeGreaterGreaterEquals:
      op.mutable_assign_slice_range_greater_greater_equals();
      break;
    case bytecode::OpCode::kCall:
      op.mutable_call();
      break;
    case bytecode::OpCode::kCallPos0:
      op.mutable_call_pos0();
      break;
    case bytecode::OpCode::kCallPos1:
      op.mutable_call_pos1();
      break;
    case bytecode::OpCode::kCallPos2:
      op.mutable_call_pos2();
      break;
    case bytecode::OpCode::kCallPos3:
      op.mutable_call_pos3();
      break;
    case bytecode::OpCode::kCallPos:
      op.mutable_call_pos()->set_positional_count(0);
      break;
    case bytecode::OpCode::kCallNamed:
      op.mutable_call_named();
      break;
    case bytecode::OpCode::kCallPosStar:
      op.mutable_call_pos_star()->set_positional_arguments_count(0);
      break;
    case bytecode::OpCode::kCallMethodPos0:
      op.mutable_call_method_pos0();
      break;
    case bytecode::OpCode::kCallMethodPos1:
      op.mutable_call_method_pos1();
      break;
    case bytecode::OpCode::kCallMethodPos2:
      op.mutable_call_method_pos2();
      break;
    case bytecode::OpCode::kCallMethodPos3:
      op.mutable_call_method_pos3();
      break;
    case bytecode::OpCode::kCallMethodPos:
      op.mutable_call_method_pos()->set_positional_count(0);
      break;
    case bytecode::OpCode::kLoadModule:
      op.mutable_load_module();
      break;
    case bytecode::OpCode::kReturn:
      op.mutable_return_();
      break;
    case bytecode::OpCode::kMakeFunction:
      op.mutable_make_function()->set_default_values_count(0);
      break;
    case bytecode::OpCode::kCreateFrame:
      op.mutable_create_frame();
      break;
    case bytecode::OpCode::kPopFrame:
      op.mutable_pop_frame();
      break;
    case bytecode::OpCode::kEnd:
      op.mutable_end();
      break;
    case bytecode::OpCode::kNop:
      op.mutable_nop();
      break;
    case bytecode::OpCode::kFail:
      op.mutable_fail();
      break;
    case bytecode::OpCode::OP_CODE_NOT_SET:
      break;
  }
  return op;
}

inline std::set<bytecode::OpCode::OpCodeCase> collect_opcodes_from_files(const std::vector<std::string>& filenames,
    google::protobuf::Arena& arena,
    logging::logger& logging) {
  std::set<bytecode::OpCode::OpCodeCase> cases;
  for (const auto& filename : filenames) {
    bytecode::Program* program = compile_test_file(filename, arena, logging);
    if (program == nullptr) {
      continue;
    }
    const auto file_cases = collect_opcode_cases(*program);
    cases.insert(file_cases.begin(), file_cases.end());
  }
  return cases;
}

inline std::string opcode_case_name(bytecode::OpCode::OpCodeCase case_) {
  return std::to_string(static_cast<int>(case_));
}

inline std::map<bytecode::OpCode::OpCodeCase, std::vector<std::string>> map_opcodes_to_source_files(
    const std::vector<std::string>& filenames,
    google::protobuf::Arena& arena,
    logging::logger& logging) {
  std::map<bytecode::OpCode::OpCodeCase, std::vector<std::string>> mapping;
  for (const auto& filename : filenames) {
    bytecode::Program* program = compile_test_file(filename, arena, logging);
    if (program == nullptr) {
      continue;
    }
    for (const auto case_ : collect_opcode_cases(*program)) {
      mapping[case_].push_back(filename);
    }
  }
  return mapping;
}

inline const std::vector<std::string>& all_star_test_files() {
  static const std::vector<std::string> files = {
      "binary.star",
      "unary.star",
      "constants.star",
      "calls.star",
      "collections.star",
      "assignments.star",
      "slices.star",
      "control_flow.star",
      "defs.star",
      "modules.star",
      "peaks_unary.star",
      "peaks_call_plain.star",
      "peaks_call_negated.star",
      "peaks_defaults.star",
      "peaks_for.star",
      "self_recursive.star",
  };
  return files;
}

inline const std::vector<std::string>& stack_depth_feature_files() {
  static const std::vector<std::string> files = {
      "binary.star",
      "unary.star",
      "constants.star",
      "calls.star",
      "collections.star",
      "assignments.star",
      "slices.star",
      "control_flow.star",
      "defs.star",
      "modules.star",
  };
  return files;
}

inline const std::vector<bytecode::OpCode::OpCodeCase>& compilable_opcode_inventory() {
  static const std::vector<bytecode::OpCode::OpCodeCase> inventory = {
      bytecode::OpCode::kConstBigInt,
      bytecode::OpCode::kConstInt,
      bytecode::OpCode::kConstFloat,
      bytecode::OpCode::kConstString,
      bytecode::OpCode::kConstStringView,
      bytecode::OpCode::kConstBytes,
      bytecode::OpCode::kConstNone,
      bytecode::OpCode::kPop,
      bytecode::OpCode::kUnaryPlus,
      bytecode::OpCode::kUnaryMinus,
      bytecode::OpCode::kUnaryTilde,
      bytecode::OpCode::kUnaryNot,
      bytecode::OpCode::kBinaryEqualsEquals,
      bytecode::OpCode::kBinaryBangEquals,
      bytecode::OpCode::kBinaryLessThan,
      bytecode::OpCode::kBinaryGreaterThan,
      bytecode::OpCode::kBinaryLessThanEquals,
      bytecode::OpCode::kBinaryGreaterThanEquals,
      bytecode::OpCode::kBinaryIn,
      bytecode::OpCode::kBinaryNotIn,
      bytecode::OpCode::kBinaryPipe,
      bytecode::OpCode::kBinaryHat,
      bytecode::OpCode::kBinaryAmpersand,
      bytecode::OpCode::kBinaryLessThanLessThan,
      bytecode::OpCode::kBinaryGreaterThanGreaterThan,
      bytecode::OpCode::kBinaryMinus,
      bytecode::OpCode::kBinaryPlus,
      bytecode::OpCode::kBinaryStar,
      bytecode::OpCode::kBinaryPercent,
      bytecode::OpCode::kBinarySlash,
      bytecode::OpCode::kBinarySlashSlash,
      bytecode::OpCode::kGoto,
      bytecode::OpCode::kJumpIfFalse,
      bytecode::OpCode::kJumpIfTrueOrPop,
      bytecode::OpCode::kJumpIfFalseOrPop,
      bytecode::OpCode::kMakeTuple,
      bytecode::OpCode::kMakeList,
      bytecode::OpCode::kAddToList,
      bytecode::OpCode::kMakeDictionary,
      bytecode::OpCode::kAddToDictionary,
      bytecode::OpCode::kLoad,
      bytecode::OpCode::kStore,
      bytecode::OpCode::kGetIterator,
      bytecode::OpCode::kForIterator,
      bytecode::OpCode::kForIteratorExt,
      bytecode::OpCode::kEndIterator,
      bytecode::OpCode::kUnpack,
      bytecode::OpCode::kAssignPlusEquals,
      bytecode::OpCode::kAssignMinusEquals,
      bytecode::OpCode::kAssignStarEquals,
      bytecode::OpCode::kAssignSlashEquals,
      bytecode::OpCode::kAssignSlashSlashEquals,
      bytecode::OpCode::kAssignPercentEquals,
      bytecode::OpCode::kAssignAmpersandEquals,
      bytecode::OpCode::kAssignPipeEquals,
      bytecode::OpCode::kAssignHatEquals,
      bytecode::OpCode::kAssignLessLessEquals,
      bytecode::OpCode::kAssignGreaterGreaterEquals,
      bytecode::OpCode::kDotMember,
      bytecode::OpCode::kAssignDotMember,
      bytecode::OpCode::kAssignDotMemberPlusEquals,
      bytecode::OpCode::kAssignDotMemberMinusEquals,
      bytecode::OpCode::kAssignDotMemberStarEquals,
      bytecode::OpCode::kAssignDotMemberSlashEquals,
      bytecode::OpCode::kAssignDotMemberSlashSlashEquals,
      bytecode::OpCode::kAssignDotMemberPercentEquals,
      bytecode::OpCode::kAssignDotMemberAmpersandEquals,
      bytecode::OpCode::kAssignDotMemberPipeEquals,
      bytecode::OpCode::kAssignDotMemberHatEquals,
      bytecode::OpCode::kAssignDotMemberLessLessEquals,
      bytecode::OpCode::kAssignDotMemberGreaterGreaterEquals,
      bytecode::OpCode::kIndexMember,
      bytecode::OpCode::kAssignIndexMember,
      bytecode::OpCode::kAssignIndexMemberPlusEquals,
      bytecode::OpCode::kAssignIndexMemberMinusEquals,
      bytecode::OpCode::kAssignIndexMemberStarEquals,
      bytecode::OpCode::kAssignIndexMemberSlashEquals,
      bytecode::OpCode::kAssignIndexMemberSlashSlashEquals,
      bytecode::OpCode::kAssignIndexMemberPercentEquals,
      bytecode::OpCode::kAssignIndexMemberAmpersandEquals,
      bytecode::OpCode::kAssignIndexMemberPipeEquals,
      bytecode::OpCode::kAssignIndexMemberHatEquals,
      bytecode::OpCode::kAssignIndexMemberLessLessEquals,
      bytecode::OpCode::kAssignIndexMemberGreaterGreaterEquals,
      bytecode::OpCode::kSliceRange,
      bytecode::OpCode::kAssignSliceRange,
      bytecode::OpCode::kAssignSliceRangePlusEquals,
      bytecode::OpCode::kAssignSliceRangeMinusEquals,
      bytecode::OpCode::kAssignSliceRangeStarEquals,
      bytecode::OpCode::kAssignSliceRangeSlashEquals,
      bytecode::OpCode::kAssignSliceRangeSlashSlashEquals,
      bytecode::OpCode::kAssignSliceRangePercentEquals,
      bytecode::OpCode::kAssignSliceRangeAmpersandEquals,
      bytecode::OpCode::kAssignSliceRangePipeEquals,
      bytecode::OpCode::kAssignSliceRangeHatEquals,
      bytecode::OpCode::kAssignSliceRangeLessLessEquals,
      bytecode::OpCode::kAssignSliceRangeGreaterGreaterEquals,
      bytecode::OpCode::kCall,
      bytecode::OpCode::kCallPos0,
      bytecode::OpCode::kCallPos1,
      bytecode::OpCode::kCallPos2,
      bytecode::OpCode::kCallPos3,
      bytecode::OpCode::kCallPos,
      bytecode::OpCode::kCallNamed,
      bytecode::OpCode::kCallPosStar,
      bytecode::OpCode::kCallMethodPos0,
      bytecode::OpCode::kCallMethodPos1,
      bytecode::OpCode::kCallMethodPos2,
      bytecode::OpCode::kCallMethodPos3,
      bytecode::OpCode::kCallMethodPos,
      bytecode::OpCode::kLoadModule,
      bytecode::OpCode::kReturn,
      bytecode::OpCode::kMakeFunction,
      bytecode::OpCode::kCreateFrame,
      bytecode::OpCode::kPopFrame,
      bytecode::OpCode::kEnd,
  };
  return inventory;
}

inline const std::vector<bytecode::OpCode::OpCodeCase>& compiler_internal_opcode_inventory() {
  static const std::vector<bytecode::OpCode::OpCodeCase> inventory = {
      bytecode::OpCode::kNop,
  };
  return inventory;
}

inline const std::vector<bytecode::OpCode::OpCodeCase>& runtime_only_opcode_inventory() {
  static const std::vector<bytecode::OpCode::OpCodeCase> inventory = {
      bytecode::OpCode::kFail,
  };
  return inventory;
}

inline const std::vector<bytecode::OpCode::OpCodeCase>& all_defined_opcode_cases() {
  static const std::vector<bytecode::OpCode::OpCodeCase> cases = {
      bytecode::OpCode::kConstBigInt,
      bytecode::OpCode::kConstInt,
      bytecode::OpCode::kConstFloat,
      bytecode::OpCode::kConstString,
      bytecode::OpCode::kConstStringView,
      bytecode::OpCode::kConstBytes,
      bytecode::OpCode::kConstNone,
      bytecode::OpCode::kPop,
      bytecode::OpCode::kUnaryPlus,
      bytecode::OpCode::kUnaryMinus,
      bytecode::OpCode::kUnaryTilde,
      bytecode::OpCode::kUnaryNot,
      bytecode::OpCode::kBinaryEqualsEquals,
      bytecode::OpCode::kBinaryBangEquals,
      bytecode::OpCode::kBinaryLessThan,
      bytecode::OpCode::kBinaryGreaterThan,
      bytecode::OpCode::kBinaryLessThanEquals,
      bytecode::OpCode::kBinaryGreaterThanEquals,
      bytecode::OpCode::kBinaryIn,
      bytecode::OpCode::kBinaryNotIn,
      bytecode::OpCode::kBinaryPipe,
      bytecode::OpCode::kBinaryHat,
      bytecode::OpCode::kBinaryAmpersand,
      bytecode::OpCode::kBinaryLessThanLessThan,
      bytecode::OpCode::kBinaryGreaterThanGreaterThan,
      bytecode::OpCode::kBinaryMinus,
      bytecode::OpCode::kBinaryPlus,
      bytecode::OpCode::kBinaryStar,
      bytecode::OpCode::kBinaryPercent,
      bytecode::OpCode::kBinarySlash,
      bytecode::OpCode::kBinarySlashSlash,
      bytecode::OpCode::kGoto,
      bytecode::OpCode::kJumpIfFalse,
      bytecode::OpCode::kJumpIfTrueOrPop,
      bytecode::OpCode::kJumpIfFalseOrPop,
      bytecode::OpCode::kMakeTuple,
      bytecode::OpCode::kMakeList,
      bytecode::OpCode::kAddToList,
      bytecode::OpCode::kMakeDictionary,
      bytecode::OpCode::kAddToDictionary,
      bytecode::OpCode::kLoad,
      bytecode::OpCode::kStore,
      bytecode::OpCode::kGetIterator,
      bytecode::OpCode::kForIterator,
      bytecode::OpCode::kForIteratorExt,
      bytecode::OpCode::kEndIterator,
      bytecode::OpCode::kUnpack,
      bytecode::OpCode::kAssignPlusEquals,
      bytecode::OpCode::kAssignMinusEquals,
      bytecode::OpCode::kAssignStarEquals,
      bytecode::OpCode::kAssignSlashEquals,
      bytecode::OpCode::kAssignSlashSlashEquals,
      bytecode::OpCode::kAssignPercentEquals,
      bytecode::OpCode::kAssignAmpersandEquals,
      bytecode::OpCode::kAssignPipeEquals,
      bytecode::OpCode::kAssignHatEquals,
      bytecode::OpCode::kAssignLessLessEquals,
      bytecode::OpCode::kAssignGreaterGreaterEquals,
      bytecode::OpCode::kDotMember,
      bytecode::OpCode::kAssignDotMember,
      bytecode::OpCode::kAssignDotMemberPlusEquals,
      bytecode::OpCode::kAssignDotMemberMinusEquals,
      bytecode::OpCode::kAssignDotMemberStarEquals,
      bytecode::OpCode::kAssignDotMemberSlashEquals,
      bytecode::OpCode::kAssignDotMemberSlashSlashEquals,
      bytecode::OpCode::kAssignDotMemberPercentEquals,
      bytecode::OpCode::kAssignDotMemberAmpersandEquals,
      bytecode::OpCode::kAssignDotMemberPipeEquals,
      bytecode::OpCode::kAssignDotMemberHatEquals,
      bytecode::OpCode::kAssignDotMemberLessLessEquals,
      bytecode::OpCode::kAssignDotMemberGreaterGreaterEquals,
      bytecode::OpCode::kIndexMember,
      bytecode::OpCode::kAssignIndexMember,
      bytecode::OpCode::kAssignIndexMemberPlusEquals,
      bytecode::OpCode::kAssignIndexMemberMinusEquals,
      bytecode::OpCode::kAssignIndexMemberStarEquals,
      bytecode::OpCode::kAssignIndexMemberSlashEquals,
      bytecode::OpCode::kAssignIndexMemberSlashSlashEquals,
      bytecode::OpCode::kAssignIndexMemberPercentEquals,
      bytecode::OpCode::kAssignIndexMemberAmpersandEquals,
      bytecode::OpCode::kAssignIndexMemberPipeEquals,
      bytecode::OpCode::kAssignIndexMemberHatEquals,
      bytecode::OpCode::kAssignIndexMemberLessLessEquals,
      bytecode::OpCode::kAssignIndexMemberGreaterGreaterEquals,
      bytecode::OpCode::kSliceRange,
      bytecode::OpCode::kAssignSliceRange,
      bytecode::OpCode::kAssignSliceRangePlusEquals,
      bytecode::OpCode::kAssignSliceRangeMinusEquals,
      bytecode::OpCode::kAssignSliceRangeStarEquals,
      bytecode::OpCode::kAssignSliceRangeSlashEquals,
      bytecode::OpCode::kAssignSliceRangeSlashSlashEquals,
      bytecode::OpCode::kAssignSliceRangePercentEquals,
      bytecode::OpCode::kAssignSliceRangeAmpersandEquals,
      bytecode::OpCode::kAssignSliceRangePipeEquals,
      bytecode::OpCode::kAssignSliceRangeHatEquals,
      bytecode::OpCode::kAssignSliceRangeLessLessEquals,
      bytecode::OpCode::kAssignSliceRangeGreaterGreaterEquals,
      bytecode::OpCode::kCall,
      bytecode::OpCode::kCallPos0,
      bytecode::OpCode::kCallPos1,
      bytecode::OpCode::kCallPos2,
      bytecode::OpCode::kCallPos3,
      bytecode::OpCode::kCallPos,
      bytecode::OpCode::kCallNamed,
      bytecode::OpCode::kCallPosStar,
      bytecode::OpCode::kCallMethodPos0,
      bytecode::OpCode::kCallMethodPos1,
      bytecode::OpCode::kCallMethodPos2,
      bytecode::OpCode::kCallMethodPos3,
      bytecode::OpCode::kCallMethodPos,
      bytecode::OpCode::kLoadModule,
      bytecode::OpCode::kReturn,
      bytecode::OpCode::kMakeFunction,
      bytecode::OpCode::kCreateFrame,
      bytecode::OpCode::kPopFrame,
      bytecode::OpCode::kEnd,
      bytecode::OpCode::kFail,
      bytecode::OpCode::kNop,
  };
  return cases;
}

inline std::string format_opcode_coverage_report(const std::map<bytecode::OpCode::OpCodeCase, std::vector<std::string>>& mapping) {
  std::ostringstream out;
  out << "opcode coverage report\n";
  out << "opcode_case,opcode_name,source_files\n";
  for (const auto case_ : all_defined_opcode_cases()) {
    out << static_cast<int>(case_) << "," << opcode_case_name(case_) << ",";
    const auto it = mapping.find(case_);
    if (it == mapping.end() || it->second.empty()) {
      out << "(not emitted)\n";
      continue;
    }
    for (std::size_t i = 0; i < it->second.size(); ++i) {
      if (i != 0) {
        out << "|";
      }
      out << it->second[i];
    }
    out << "\n";
  }
  return out.str();
}

}  // namespace test
}  // namespace analysis
}  // namespace compiler
}  // namespace starlark

#endif  // COMPILER_ANALYSIS_STACK_DEPTH_TEST_SUPPORT_HPP_
