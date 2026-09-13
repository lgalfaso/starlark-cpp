// Copyright 2026 Lucas Mirelmann

#include "vm/module_loader.hpp"

#include <vector>
#include <functional>
#include <map>
#include <utility>
#include <string>

#include "vm/built_in_functions.hpp"
#include "vm/module_metadata.hpp"
#include "runtime/starlark_function.hpp"


using ::google::protobuf::Arena;
using ::starlark::bytecode::OpCode;
using ::starlark::bytecode::Program;
using ::starlark::result::status_code;
using ::starlark::result::status_or;
using ::starlark::runtime::builtin_entrypoints;
using ::starlark::runtime::starlark_built_in_function;
using ::starlark::runtime::starlark_obj;

namespace starlark {
namespace vm {

namespace {

const char builtin_star[] = {
    #embed "vm/builtin.star"
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

starlark::vm::module_info::module_info(std::string_view c_name, std::string_view source, bool inner_module, const bindings_t& bindings) :
    cannonical_name_(c_name),
    source_code_(source),
    inner_(inner_module),
    custom_binding_(bindings),
    frame_and_program(nullptr, nullptr) {}

bool starlark::vm::module_info::ready() const {
  return frame_and_program.first != nullptr;
}

const starlark::vm::module_info::bindings_t& starlark::vm::module_info::custom_binding() const {
  return custom_binding_;
}

Arena& starlark::vm::module_info::arena() {
  return arena_;
}

std::string_view starlark::vm::module_info::cannonical_name() const {
  return cannonical_name_;
}

std::string_view starlark::vm::module_info::source_code() const {
  return source_code_;
}

bool starlark::vm::module_info::inner() const {
  return inner_;
}

void starlark::vm::module_info::loaded(frame* base_frame, const Program* program) {
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

std::pair<frame*, const starlark::bytecode::Program*>& starlark::vm::module_info::get() {
  return frame_and_program;
}

const module_metadata& starlark::vm::module_info::metadata_for(const Program& program) {
  if (metadata_ == nullptr) {
    metadata_ = Arena::Create<module_metadata>(&arena_, module_metadata::build(program));
  }
  return *metadata_;
}

module_loader::module_loader() {
  custom_binding["inner_max"] = Arena::Create<starlark_built_in_function>(&arena, nullptr, builtin_entrypoints{.call = starlark_fn_inner_max}, "inner_max");
  custom_binding["inner_min"] = Arena::Create<starlark_built_in_function>(&arena, nullptr, builtin_entrypoints{.call = starlark_fn_inner_min}, "inner_min");
  custom_binding["inner_sorted"] = Arena::Create<starlark_built_in_function>(&arena, nullptr, builtin_entrypoints{.call = starlark_fn_inner_sorted}, "inner_sorted");
}

status_or<module_info*> module_loader::load_module(std::string_view local_module_name, std::string_view caller_cannonical_name) {
  return load_module(cannonical_name(local_module_name, caller_cannonical_name));
}

status_or<module_info*> module_loader::load_module(std::string_view cannonical_module_name) {
  // If this is one of the built-in modules, then use it.
  if (cannonical_module_name == builtin_star_module) {
    auto it = modules.find(cannonical_module_name);
    if (it != modules.end()) {
      return status_or<module_info*>(&it->second);
    }
    auto result = modules.try_emplace(std::string{cannonical_module_name}, cannonical_module_name, builtin_star, true, custom_binding);
    return status_or<module_info*>(&result.first->second);
  }
  auto it = modules.find(cannonical_module_name);
  if (it != modules.end()) {
    return status_or<module_info*>(&it->second);
  }
  auto source_and_bindings = inner_load(cannonical_module_name);
  if (!source_and_bindings.ok()) {
    return status_or<module_info*>(status_code::kStaticError);
  }
  auto result = modules.try_emplace(std::string{cannonical_module_name}, cannonical_module_name, source_and_bindings->first, false, source_and_bindings->second);
  return status_or<module_info*>(&result.first->second);
}

std::string module_loader::cannonical_name(std::string_view module_name, std::string_view caller_module_name) {
  return std::string{module_name};
}

kv_module_loader::kv_module_loader(const std::map<std::string, std::pair<std::string, const starlark::vm::module_info::bindings_t>, std::less<>>& values) : values(values) {}

status_or<std::pair<std::string, const starlark::vm::module_info::bindings_t>> kv_module_loader::inner_load(std::string_view cannonical_name_) {
  auto it = values.find(cannonical_name_);
  if (it == values.end()) {
    return status_or<std::pair<std::string, const starlark::vm::module_info::bindings_t>>(status_code::kStaticError);
  }
  return status_or<std::pair<std::string, const starlark::vm::module_info::bindings_t>>(it->second);
}

}  // namespace vm
}  // namespace starlark
