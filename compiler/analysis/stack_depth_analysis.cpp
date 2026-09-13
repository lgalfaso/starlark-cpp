// Copyright 2026 Lucas Mirelmann

#include "compiler/analysis/stack_depth_analysis.hpp"

#include <cassert>
#include <cstdint>

#include <algorithm>
#include <deque>
#include <limits>
#include <utility>
#include <vector>

namespace starlark {
namespace compiler {
namespace analysis {

namespace {

using ::starlark::bytecode::OpCode;

int stack_effect(const OpCode& op) {
  switch (op.op_code_case()) {
    case OpCode::kConstBigInt:
    case OpCode::kConstInt:
    case OpCode::kConstFloat:
    case OpCode::kConstString:
    case OpCode::kConstStringView:
    case OpCode::kConstBytes:
    case OpCode::kConstNone:
    case OpCode::kLoad:
    case OpCode::kMakeList:
    case OpCode::kMakeDictionary:
      return 1;
    case OpCode::kGetIterator:
      return -1;
    case OpCode::kPop:
    case OpCode::kStore:
      return -1;
    case OpCode::kMakeFunction:
      return 1 - op.make_function().default_values_count();
    case OpCode::kMakeTuple:
      return 1 - op.make_tuple().number_of_elements();
    case OpCode::kAddToList:
      return -op.add_to_list().number_of_elements();
    case OpCode::kAddToDictionary:
      return -2 * op.add_to_dictionary().number_of_elements();
    case OpCode::kBinaryEqualsEquals:
    case OpCode::kBinaryBangEquals:
    case OpCode::kBinaryLessThan:
    case OpCode::kBinaryGreaterThan:
    case OpCode::kBinaryLessThanEquals:
    case OpCode::kBinaryGreaterThanEquals:
    case OpCode::kBinaryIn:
    case OpCode::kBinaryNotIn:
    case OpCode::kBinaryPipe:
    case OpCode::kBinaryHat:
    case OpCode::kBinaryAmpersand:
    case OpCode::kBinaryLessThanLessThan:
    case OpCode::kBinaryGreaterThanGreaterThan:
    case OpCode::kBinaryMinus:
    case OpCode::kBinaryPlus:
    case OpCode::kBinaryStar:
    case OpCode::kBinaryPercent:
    case OpCode::kBinarySlash:
    case OpCode::kBinarySlashSlash:
      return -1;
    case OpCode::kUnpack:
      return op.unpack().number_of_elements() - 1;
    case OpCode::kCallPos0:
      return 0;
    case OpCode::kCallPos1:
      return -1;
    case OpCode::kCallPos2:
      return -2;
    case OpCode::kCallPos3:
      return -3;
    case OpCode::kCallPos:
      return -op.call_pos().positional_count();
    case OpCode::kCallMethodPos0:
      return 0;
    case OpCode::kCallMethodPos1:
      return -1;
    case OpCode::kCallMethodPos2:
      return -2;
    case OpCode::kCallMethodPos3:
      return -3;
    case OpCode::kCallMethodPos:
      return -op.call_method_pos().positional_count();
    case OpCode::kCallNamed: {
      int n = op.call_named().positional_arguments_count() + 2 * op.call_named().named_arguments_count();
      return -n;
    }
    case OpCode::kCallPosStar:
      return -op.call_pos_star().positional_arguments_count() - 1;
    case OpCode::kCall: {
      const auto& call = op.call();
      int n = call.positional_arguments_count() + 2 * call.named_arguments_count();
      if (call.has_variadic_positional_argument()) {
        ++n;
      }
      if (call.has_variadic_named_argument()) {
        ++n;
      }
      return -n;
    }
    case OpCode::kIndexMember:
      return -1;
    case OpCode::kAssignIndexMember:
      return -3;
    case OpCode::kDotMember:
      return 0;
    case OpCode::kAssignDotMember:
      return -2;
    case OpCode::kSliceRange:
      return -3;
    case OpCode::kAssignSliceRange:
      return -5;
    case OpCode::kAssignSliceRangePlusEquals:
    case OpCode::kAssignSliceRangeMinusEquals:
    case OpCode::kAssignSliceRangeStarEquals:
    case OpCode::kAssignSliceRangeSlashEquals:
    case OpCode::kAssignSliceRangeSlashSlashEquals:
    case OpCode::kAssignSliceRangePercentEquals:
    case OpCode::kAssignSliceRangeAmpersandEquals:
    case OpCode::kAssignSliceRangePipeEquals:
    case OpCode::kAssignSliceRangeHatEquals:
    case OpCode::kAssignSliceRangeLessLessEquals:
    case OpCode::kAssignSliceRangeGreaterGreaterEquals:
      return -5;
    case OpCode::kAssignPlusEquals:
    case OpCode::kAssignMinusEquals:
    case OpCode::kAssignStarEquals:
    case OpCode::kAssignSlashEquals:
    case OpCode::kAssignSlashSlashEquals:
    case OpCode::kAssignPercentEquals:
    case OpCode::kAssignAmpersandEquals:
    case OpCode::kAssignPipeEquals:
    case OpCode::kAssignHatEquals:
    case OpCode::kAssignLessLessEquals:
    case OpCode::kAssignGreaterGreaterEquals:
      return -1;
    case OpCode::kAssignIndexMemberPlusEquals:
    case OpCode::kAssignIndexMemberMinusEquals:
    case OpCode::kAssignIndexMemberStarEquals:
    case OpCode::kAssignIndexMemberSlashEquals:
    case OpCode::kAssignIndexMemberSlashSlashEquals:
    case OpCode::kAssignIndexMemberPercentEquals:
    case OpCode::kAssignIndexMemberAmpersandEquals:
    case OpCode::kAssignIndexMemberPipeEquals:
    case OpCode::kAssignIndexMemberHatEquals:
    case OpCode::kAssignIndexMemberLessLessEquals:
    case OpCode::kAssignIndexMemberGreaterGreaterEquals:
      return -3;
    case OpCode::kAssignDotMemberPlusEquals:
    case OpCode::kAssignDotMemberMinusEquals:
    case OpCode::kAssignDotMemberStarEquals:
    case OpCode::kAssignDotMemberSlashEquals:
    case OpCode::kAssignDotMemberSlashSlashEquals:
    case OpCode::kAssignDotMemberPercentEquals:
    case OpCode::kAssignDotMemberAmpersandEquals:
    case OpCode::kAssignDotMemberPipeEquals:
    case OpCode::kAssignDotMemberHatEquals:
    case OpCode::kAssignDotMemberLessLessEquals:
    case OpCode::kAssignDotMemberGreaterGreaterEquals:
      return -2;
    case OpCode::kJumpIfFalse:
      return -1;
    case OpCode::kJumpIfFalseOrPop:
    case OpCode::kJumpIfTrueOrPop:
      return 0;
    case OpCode::kUnaryPlus:
    case OpCode::kUnaryMinus:
    case OpCode::kUnaryTilde:
    case OpCode::kUnaryNot:
    case OpCode::kCreateFrame:
    case OpCode::kPopFrame:
    case OpCode::kLoadModule:
    case OpCode::kEndIterator:
    case OpCode::kGoto:
    case OpCode::kForIterator:
    case OpCode::kForIteratorExt:
    case OpCode::kEnd:
    case OpCode::kReturn:
    case OpCode::kFail:
    case OpCode::kNop:
      return 0;
    case OpCode::OP_CODE_NOT_SET:
    default:
      assert(false && "missing opcode stack effect");
      return 0;
  }
}

void enqueue(std::deque<std::pair<int, int>>& worklist, std::vector<int>& best_depth, int ip, int depth, int& global_max) {
  if (ip < 0) {
    return;
  }
  assert(depth >= 0 && "eval-stack depth underflow during analysis");
  global_max = std::max(global_max, depth);
  if (ip >= static_cast<int>(best_depth.size())) {
    return;
  }
  if (best_depth[static_cast<std::size_t>(ip)] >= depth) {
    return;
  }
  best_depth[static_cast<std::size_t>(ip)] = depth;
  worklist.emplace_back(ip, depth);
}

void propagate_successors(const starlark::bytecode::Block& block,
    int ip,
    int depth,
    std::deque<std::pair<int, int>>& worklist,
    std::vector<int>& best_depth,
    int& global_max) {
  const int op_count = block.op_code().size();
  if (ip < 0 || ip >= op_count) {
    return;
  }
  assert(depth >= 0 && "eval-stack depth underflow during analysis");

  const auto& op = block.op_code(ip);
  switch (op.op_code_case()) {
    case OpCode::kGoto:
      enqueue(worklist, best_depth, ip + op.goto_().address_delta(), depth, global_max);
      return;
    case OpCode::kJumpIfFalse: {
      const int next_depth = depth + stack_effect(op);
      enqueue(worklist, best_depth, ip + 1, next_depth, global_max);
      enqueue(worklist, best_depth, ip + op.jump_if_false().address_delta(), next_depth, global_max);
      return;
    }
    case OpCode::kJumpIfFalseOrPop: {
      enqueue(worklist, best_depth, ip + op.jump_if_false_or_pop().address_delta(), depth, global_max);
      enqueue(worklist, best_depth, ip + 1, depth - 1, global_max);
      return;
    }
    case OpCode::kJumpIfTrueOrPop: {
      enqueue(worklist, best_depth, ip + op.jump_if_true_or_pop().address_delta(), depth, global_max);
      enqueue(worklist, best_depth, ip + 1, depth - 1, global_max);
      return;
    }
    case OpCode::kForIterator:
      enqueue(worklist, best_depth, ip + 1, depth + 1, global_max);
      enqueue(worklist, best_depth, ip + op.for_iterator().address_delta(), depth, global_max);
      return;
    case OpCode::kForIteratorExt:
      enqueue(worklist, best_depth, ip + 1, depth, global_max);
      enqueue(worklist, best_depth, ip + op.for_iterator_ext().address_delta(), depth, global_max);
      return;
    case OpCode::kEnd:
    case OpCode::kReturn:
    case OpCode::kFail:
      return;
    default:
      enqueue(worklist, best_depth, ip + 1, depth + stack_effect(op), global_max);
      return;
  }
}

int block_stack_peak_impl(const starlark::bytecode::Block& block) {
  const int op_count = block.op_code().size();
  assert(op_count > 0);

  std::vector<int> best_depth(static_cast<std::size_t>(op_count), std::numeric_limits<int>::min());
  std::deque<std::pair<int, int>> worklist;
  int global_max = 0;
  enqueue(worklist, best_depth, 0, 0, global_max);

  while (!worklist.empty()) {
    const auto [ip, depth] = worklist.front();
    worklist.pop_front();
    if (best_depth[static_cast<std::size_t>(ip)] != depth) {
      continue;
    }
    propagate_successors(block, ip, depth, worklist, best_depth, global_max);
  }

  return global_max;
}

}  // namespace

int opcode_stack_effect(const OpCode& op) {
  return stack_effect(op);
}

int analyze_block_stack_peak(const starlark::bytecode::Block& block) {
  return block_stack_peak_impl(block);
}

uint32_t analyze_max_stack_depth(const starlark::bytecode::Program& program) {
  int max_peak = 0;
  for (const auto& block : program.block()) {
    max_peak = std::max(max_peak, analyze_block_stack_peak(block));
  }
  // Stack capacity must strictly exceed the maximum simultaneous eval-stack depth.
  return static_cast<uint32_t>(max_peak) + 1;
}

}  // namespace analysis
}  // namespace compiler
}  // namespace starlark
