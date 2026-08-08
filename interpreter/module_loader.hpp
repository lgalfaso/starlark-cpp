// Copyright 2026 Lucas Mirelmann

#ifndef INTERPRETER_MODULE_LOADER_HPP_
#define INTERPRETER_MODULE_LOADER_HPP_

#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "interpreter/frame.hpp"
#include "proto/starlark_bytecode.pb.h"
#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace interpreter {

static const char builtin_star_module[] = "@@//:builtin.star";

class module_info {
 public:
  typedef std::map<std::string, starlark::runtime::starlark_obj*, std::less<>> bindings_t;

  module_info(std::string_view c_name, std::string_view source, bool inner_, const bindings_t& bindings);
  bool ready() const;
  const bindings_t& custom_binding() const;
  google::protobuf::Arena& arena();
  std::string_view cannonical_name() const;
  std::string_view source_code() const;
  bool inner() const;
  void loaded(frame* base_frame, const starlark::bytecode::Program* program);
  std::pair<frame*, const starlark::bytecode::Program*>& get();

 private:
  std::string cannonical_name_;
  std::string source_code_;
  const bool inner_;
  const bindings_t custom_binding_;
  google::protobuf::Arena arena_;
  std::pair<frame*, const starlark::bytecode::Program*> frame_and_program;
};

class module_loader {
 public:
  module_loader();
  starlark::result::status_or<module_info*> load_module(std::string_view module_name, std::string_view caller_cannonical_name);
  starlark::result::status_or<module_info*> load_module(std::string_view cannonical_module_name);

 protected:
  virtual std::string cannonical_name(std::string_view module_name, std::string_view caller_module_name);
  virtual starlark::result::status_or<std::pair<std::string, const module_info::bindings_t>> inner_load(std::string_view cannonical_name_) = 0;

 private:
  std::map<std::string, module_info, std::less<>> modules;
  module_info::bindings_t custom_binding;
  google::protobuf::Arena arena;
};

class kv_module_loader : public module_loader {
 public:
  explicit kv_module_loader(const std::map<std::string, std::pair<std::string, const module_info::bindings_t>, std::less<>>& values);

 protected:
  starlark::result::status_or<std::pair<std::string, const module_info::bindings_t>> inner_load(std::string_view cannonical_name_) override;

 private:
  std::map<std::string, std::pair<std::string, const module_info::bindings_t>, std::less<>> values;
};

std::vector<std::string_view> get_dependencies(const starlark::bytecode::Program* program);

}  // namespace interpreter
}  // namespace starlark

#pragma GCC visibility pop

#endif  // INTERPRETER_MODULE_LOADER_HPP_


