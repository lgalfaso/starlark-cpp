// Copyright 2026 Lucas Mirelmann

#include "vm/module_metadata.hpp"

namespace starlark {
namespace vm {

using ::starlark::bytecode::OpCode;

module_metadata module_metadata::build(const starlark::bytecode::Program& program) {
  module_metadata result;
  if (!program.block().empty()) {
    for (const auto& op : program.block(0).op_code()) {
      if (op.op_code_case() == OpCode::kCreateFrame) {
        frame_metadata frame;
        frame.block_type = op.create_frame().block_type();
        for (const auto& symbol : op.create_frame().symbol()) {
          frame.symbols.push_back(symbol);
        }
        result.frames.push_back(std::move(frame));
      }
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

}  // namespace vm
}  // namespace starlark
