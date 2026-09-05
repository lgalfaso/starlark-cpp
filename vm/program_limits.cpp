// Copyright 2026 Lucas Mirelmann

#include "vm/program_limits.hpp"

#include "errors/runtime_error_messages.hpp"

namespace starlark {
namespace vm {

namespace {

using ::starlark::bytecode::OpCode;

}  // namespace

std::optional<program_limit_violation> check_program_limits(const starlark::bytecode::Program& program,
    std::string_view source,
    const starlark::runtime::runtime_options& options) {
  for (const auto& block : program.block()) {
    for (const auto& op : block.op_code()) {
      switch (op.op_code_case()) {
        case OpCode::kConstString:
          if (program.const_string(op.const_string().const_string_pos()).length() > options.max_string_length) {
            return program_limit_violation{
                starlark::error_messages::error_v2_max_string_length(
                    options.max_string_length, source, op.sh().highlight_start(), op.sh().highlight_end()),
                op.sh().highlight_start(),
            };
          }
          break;
        case OpCode::kConstBytes:
          if (op.const_bytes().value().length() > options.max_string_length) {
            return program_limit_violation{
                starlark::error_messages::error_v2_max_bytes_length(
                    options.max_string_length, source, op.sh().highlight_start(), op.sh().highlight_end()),
                op.sh().highlight_start(),
            };
          }
          break;
        case OpCode::kMakeTuple:
          if (op.make_tuple().number_of_elements() > options.max_sequence_size) {
            return program_limit_violation{
                starlark::error_messages::error_v2_max_sequence_length(
                    options.max_sequence_size, source, op.sh().highlight_start(), op.sh().highlight_end()),
                op.sh().highlight_start(),
            };
          }
          break;
        default:
          break;
      }
    }
  }
  return std::nullopt;
}

}  // namespace vm
}  // namespace starlark
