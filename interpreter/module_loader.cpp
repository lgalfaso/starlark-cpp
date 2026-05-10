// Copyright 2026 Lucas Mirelmann

#include "interpreter/module_loader.hpp"

#include <vector>
#include <functional>
#include <map>
#include <utility>
#include <string>

using ::google::protobuf::Arena;
using ::starlark::bytecode::OpCode;
using ::starlark::bytecode::Program;
using ::starlark::runtime::starlark_obj;

namespace starlark {
namespace interpreter {

namespace {

const char builtin_star_module[] = "@@//:builtin.star";

const unsigned char builtin_star[] = {
    #embed "interpreter/builtin.star"
};

}  // namespace

std::vector<std::string_view> get_dependencies(const starlark::bytecode::Program* program) {
  std::vector<std::string_view> result;
  if (!program->block().empty()) {
    // The built-in functions `min`, `max` and `sorted` have special variations.
    if (!program->block(0).op_code().empty() &&
        program->block(0).op_code(0).op_code_case() == OpCode::kCreateFrame) {
      for (const auto& symbol : program->block(0).op_code(0).create_frame().symbol()) {
        if (symbol == "min" || symbol == "max" || symbol == "sorted") {
          result.push_back(builtin_star_module);
          break;
        }
      }
    }
    // This code makes the assumption that the `load` statement are only present in the first block and
    // that are at the top with the possible exception of some string comments.
    for (const auto& op_code : program->block(0).op_code()) {
      if (op_code.op_code_case() == OpCode::kConstString || op_code.op_code_case() == OpCode::kCreateFrame) {
        continue;
      }
      if (op_code.op_code_case() == OpCode::kLoadModule) {
        result.push_back(op_code.load_module().module());
        continue;
      }
      break;
    }
  }
  return result;
}

module_info::module_info(std::string_view c_name, std::string_view source, const bindings_t& bindings) :
    cannonical_name_(c_name),
    source_code_(source),
    custom_binding_(bindings),
    frame_and_program(nullptr, nullptr) {
}

bool module_info::ready() const {
  return frame_and_program.first != nullptr;
}

const module_info::bindings_t& module_info::custom_binding() const {
  return custom_binding_;
}

Arena& module_info::arena() {
  return arena_;
}

std::string_view module_info::cannonical_name() const {
  return cannonical_name_;
}

std::string_view module_info::source_code() const {
  return source_code_;
}

void module_info::loaded(frame* base_frame, const Program* program) {
  assert(frame_and_program.first == nullptr);
  assert(frame_and_program.second == nullptr);
  frame_and_program.first = base_frame;
  frame_and_program.second = program;
  for (auto& element : base_frame->elements) {
    if (element != nullptr) {
      element->freeze();
    }
  }
}

std::pair<frame*, const starlark::bytecode::Program*>& module_info::get() {
  return frame_and_program;
}

starlark::result::status_or<module_info*> module_loader::load_module(std::string_view module_name, std::string_view caller_module_name) {
  // TODO(lmirelmann): If this is one of the built-in modules, then use it.
  auto c_name = cannonical_name(module_name, caller_module_name);
  auto it = modules.find(c_name);
  if (it != modules.end()) {
    return starlark::result::status_or<module_info*>(&it->second);
  }
  auto source_and_bindings = inner_load(c_name);
  if (!source_and_bindings.ok()) {
    return starlark::result::status_or<module_info*>(starlark::result::status_code::kError);
  }
  auto result = modules.try_emplace(c_name, c_name, source_and_bindings->first, source_and_bindings->second);
  return starlark::result::status_or<module_info*>(&result.first->second);
}

std::string module_loader::cannonical_name(std::string_view module_name, std::string_view caller_module_name) {
  return std::string{module_name};
}

kv_module_loader::kv_module_loader(const std::map<std::string, std::pair<std::string, const module_info::bindings_t>, std::less<>>& values) : values(values) {}

starlark::result::status_or<std::pair<std::string, const module_info::bindings_t>> kv_module_loader::inner_load(std::string_view cannonical_name_) {
  auto it = values.find(cannonical_name_);
  if (it == values.end()) {
    return starlark::result::status_or<std::pair<std::string, const module_info::bindings_t>>(starlark::result::status_code::kError);
  }
  return starlark::result::status_or<std::pair<std::string, const module_info::bindings_t>>(it->second);
}

}  // namespace interpreter
}  // namespace starlark

