// Copyright 2026 Lucas Mirelmann

#ifndef VM_MODULE_METADATA_HPP_
#define VM_MODULE_METADATA_HPP_

#include <cstdint>
#include <string>
#include <vector>

#include "proto/starlark_bytecode.pb.h"

#pragma GCC visibility push(default)

namespace starlark {
namespace vm {

struct frame_metadata {
  starlark::bytecode::BlockType block_type = starlark::bytecode::BlockType::FUNCTION_BLOCK;
  std::vector<std::string> symbols;
};

struct function_param_metadata {
  std::string name;
  int32_t pos_in_frame = 0;
  bool default_initialization = false;
};

struct function_signature_metadata {
  std::string fn_name;
  int32_t keyword_only_parameter_count = 0;
  bool has_star_argument = false;
  bool has_star_star_argument = false;
  int32_t param_count = 0;
  int32_t positional_param_count = 0;
  std::vector<std::string> frame_symbols;
  std::vector<function_param_metadata> params;
};

struct module_metadata {
  std::vector<frame_metadata> frames;
  std::vector<function_signature_metadata> functions;

  static module_metadata build(const starlark::bytecode::Program& program);
};

const function_signature_metadata* function_metadata_for_block(const module_metadata& metadata, int32_t block_idx);

}  // namespace vm
}  // namespace starlark

#pragma GCC visibility pop

#endif  // VM_MODULE_METADATA_HPP_
