// Copyright 2025 Lucas Mirelmann

#ifndef INTERPRETER_INTERPRETER_HPP_
#define INTERPRETER_INTERPRETER_HPP_

#include <functional>
#include <map>
#include <string>

#include "grammar/options.hpp"
#include "interpreter/frame.hpp"
#include "logging/logging.hpp"
#include "interpreter/module_loader.hpp"
#include "runtime/options.hpp"
#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace interpreter {

class interpreter {
 public:
  interpreter();
  frame* run(module_loader& loader,
      std::string_view module_name,
      const starlark::grammar::grammar_options& g_options,
      const starlark::runtime::runtime_options& r_options,
      starlark::logging::logger& logging);

 private:
  void add_base_global_context(std::map<std::string, starlark::runtime::starlark_obj*, std::less<>>& global_context, starlark::runtime::context& ctx) const;
};

}  // namespace interpreter
}  // namespace starlark

#pragma GCC visibility pop

#endif  // INTERPRETER_INTERPRETER_HPP_

