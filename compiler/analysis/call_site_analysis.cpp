// Copyright 2026 Lucas Mirelmann

#include "compiler/analysis/call_site_analysis.hpp"

#include <cassert>

#include <optional>
#include <utility>

#include "compiler/analysis/stack_depth_analysis.hpp"

namespace starlark {
namespace compiler {
namespace analysis {

namespace {

using ::starlark::bytecode::OpCode;
using ::starlark::bytecode::Program;

using slot_key = std::pair<int, int>;

constexpr int kModuleInitFrameStackDepth = 0;

int frame_stack_depth_for_block(int block_idx) {
  if (block_idx == 0) {
    return kModuleInitFrameStackDepth;
  }
  assert(block_idx > 0);
  return kModuleInitFrameStackDepth + 1;
}

slot_key canonical_slot_key(int frame_stack_depth, int frame_from_top, int pos_in_frame) {
  return {frame_stack_depth - 1 - frame_from_top, pos_in_frame};
}

int call_pos_count(const OpCode& op) {
  switch (op.op_code_case()) {
    case OpCode::kCallPos0:
      return 0;
    case OpCode::kCallPos1:
      return 1;
    case OpCode::kCallPos2:
      return 2;
    case OpCode::kCallPos3:
      return 3;
    case OpCode::kCallPos:
      return op.call_pos().positional_count();
    default:
      return -1;
  }
}

std::map<slot_key, int> build_slot_to_entry(const Program& program) {
  std::map<slot_key, int> slot_to_entry;
  for (int block_idx = 0; block_idx < program.block().size(); ++block_idx) {
    const auto& block = program.block(block_idx);
    for (int ip = 0; ip + 1 < block.op_code().size(); ++ip) {
      const auto& op = block.op_code(ip);
      const auto& next = block.op_code(ip + 1);
      if (op.op_code_case() == OpCode::kMakeFunction && next.op_code_case() == OpCode::kStore) {
        const int frame_stack_depth = frame_stack_depth_for_block(block_idx);
        slot_to_entry[canonical_slot_key(frame_stack_depth, next.store().frame(), next.store().pos_in_frame())] =
            op.make_function().entrypoint();
      }
    }
  }
  return slot_to_entry;
}

std::optional<int> callee_entry_for_call(const Program& program,
    int block_idx,
    int call_ip,
    int pos_count,
    const std::map<slot_key, int>& slot_to_entry) {
  const auto& block = program.block(block_idx);
  const int frame_stack_depth = frame_stack_depth_for_block(block_idx);
  int depth = pos_count + 1;
  for (int ip = call_ip - 1; ip >= 0; --ip) {
    const auto& op = block.op_code(ip);
    if (op.op_code_case() == OpCode::kLoad) {
      depth--;
      if (depth == 0) {
        const auto key = canonical_slot_key(frame_stack_depth, op.load().frame(), op.load().pos_in_frame());
        auto it = slot_to_entry.find(key);
        if (it == slot_to_entry.end()) {
          return std::nullopt;
        }
        return it->second;
      }
      continue;
    }
    depth -= opcode_stack_effect(op);
    assert(depth >= 0 && "call site backward walk stack depth underflow");
  }
  return std::nullopt;
}

void set_known_callee_block(OpCode& op, int32_t block_idx) {
  switch (op.op_code_case()) {
    case OpCode::kCallPos0:
      op.mutable_call_pos0()->set_known_callee_block(block_idx);
      return;
    case OpCode::kCallPos1:
      op.mutable_call_pos1()->set_known_callee_block(block_idx);
      return;
    case OpCode::kCallPos2:
      op.mutable_call_pos2()->set_known_callee_block(block_idx);
      return;
    case OpCode::kCallPos3:
      op.mutable_call_pos3()->set_known_callee_block(block_idx);
      return;
    case OpCode::kCallPos:
      op.mutable_call_pos()->set_known_callee_block(block_idx);
      return;
    default:
      return;
  }
}

}  // namespace

std::map<int, int> analyze_static_self_calls(const Program& program, int block_idx) {
  std::map<int, int> known_calls;
  assert(block_idx > 0 && block_idx < program.block().size());
  const auto slot_to_entry = build_slot_to_entry(program);
  const auto& block = program.block(block_idx);
  for (int ip = 0; ip < block.op_code().size(); ++ip) {
    const auto& op = block.op_code(ip);
    const int pos_count = call_pos_count(op);
    if (pos_count < 0) {
      continue;
    }
    if (auto entry = callee_entry_for_call(program, block_idx, ip, pos_count, slot_to_entry)) {
      if (*entry == block_idx) {
        known_calls[ip] = *entry;
      }
    }
  }
  return known_calls;
}

void annotate_static_self_calls(Program& program) {
  for (int block_idx = 1; block_idx < program.block().size(); ++block_idx) {
    const auto known_calls = analyze_static_self_calls(program, block_idx);
    auto* block = program.mutable_block(block_idx);
    for (const auto& [call_ip, entry] : known_calls) {
      set_known_callee_block(*block->mutable_op_code(call_ip), entry);
    }
  }
}

}  // namespace analysis
}  // namespace compiler
}  // namespace starlark
