// Copyright 2026 Lucas Mirelmann

#include "vm/module_metadata.hpp"
#include <utility>

namespace starlark {
namespace vm {

using ::starlark::bytecode::BlockType;
using ::starlark::bytecode::OpCode;

module_metadata module_metadata::build(const starlark::bytecode::Program& program) {
  module_metadata result;
  if (!program.block().empty()) {
    for (int ip = 0; ip < program.block(0).op_code().size(); ++ip) {
      const auto& op = program.block(0).op_code(ip);
      if (op.op_code_case() == OpCode::kCreateFrame) {
        frame_metadata frame;
        frame.block_type = op.create_frame().block_type();
        frame.block_idx = 0;
        frame.ip = ip;
        for (const auto& symbol : op.create_frame().symbol()) {
          frame.symbols.push_back(symbol);
        }
        result.frames.push_back(std::move(frame));
      }
    }
  }
  for (int block_idx = 0; block_idx < program.block().size(); ++block_idx) {
    const auto& block = program.block(block_idx);
    for (int ip = 0; ip < block.op_code().size(); ++ip) {
      const auto& op = block.op_code(ip);
      if (op.op_code_case() != OpCode::kCreateFrame) {
        continue;
      }
      if (block_idx == 0 || op.create_frame().block_type() == BlockType::FUNCTION_BLOCK) {
        continue;
      }
      frame_metadata frame;
      frame.block_type = op.create_frame().block_type();
      frame.block_idx = block_idx;
      frame.ip = ip;
      for (const auto& symbol : op.create_frame().symbol()) {
        frame.symbols.push_back(symbol);
      }
      result.frames.push_back(std::move(frame));
    }
  }
  for (int block_idx = 1; block_idx < program.block().size(); ++block_idx) {
    const auto& block = program.block(block_idx);
    function_signature_metadata fn;
    fn.fn_name = block.function_signature().fn_name();
    fn.keyword_only_parameter_count = block.function_signature().keyword_only_parameter_count();
    fn.has_star_argument = block.function_signature().has_star_argument();
    fn.has_star_star_argument = block.function_signature().has_star_star_argument();
    fn.param_count = block.function_signature().param().size();
    if (!block.op_code().empty() && block.op_code(0).op_code_case() == OpCode::kCreateFrame) {
      for (const auto& symbol : block.op_code(0).create_frame().symbol()) {
        fn.frame_symbols.push_back(symbol);
      }
    }
    for (const auto& param : block.function_signature().param()) {
      function_param_metadata param_meta;
      param_meta.name = param.name();
      param_meta.pos_in_frame = param.pos().pos_in_frame();
      param_meta.default_initialization = param.default_initialization();
      fn.params.push_back(std::move(param_meta));
    }
    int number_of_standard_params = fn.param_count;
    if (fn.has_star_argument) {
      number_of_standard_params--;
    }
    if (fn.has_star_star_argument) {
      number_of_standard_params--;
    }
    fn.positional_param_count = number_of_standard_params - fn.keyword_only_parameter_count;
    result.functions.push_back(std::move(fn));
  }
  return result;
}

const function_signature_metadata* function_metadata_for_block(const module_metadata& metadata, int32_t block_idx) {
  if (block_idx <= 0) {
    return nullptr;
  }
  const auto fn_idx = static_cast<std::size_t>(block_idx - 1);
  if (fn_idx >= metadata.functions.size()) {
    return nullptr;
  }
  return &metadata.functions[fn_idx];
}

const frame_metadata* frame_metadata_for(const module_metadata& metadata, BlockType block_type) {
  for (const auto& frame : metadata.frames) {
    if (frame.block_type == block_type && frame.block_idx == 0) {
      return &frame;
    }
  }
  return nullptr;
}

int frame_meta_index(const module_metadata& metadata, BlockType block_type) {
  for (std::size_t i = 0; i < metadata.frames.size(); ++i) {
    if (metadata.frames[i].block_type == block_type && metadata.frames[i].block_idx == 0) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

int frame_meta_index_at(const module_metadata& metadata, int32_t block_idx, int32_t ip) {
  for (std::size_t i = 0; i < metadata.frames.size(); ++i) {
    if (metadata.frames[i].block_idx == block_idx && metadata.frames[i].ip == ip) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

}  // namespace vm
}  // namespace starlark
