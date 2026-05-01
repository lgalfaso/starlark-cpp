// Copyright 2025 Lucas Mirelmann

#ifndef INTERPRETER_INTERPRETER_HPP_
#define INTERPRETER_INTERPRETER_HPP_

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "grammar/options.hpp"
#include "interpreter/frame.hpp"
#include "logging/logging.hpp"
#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace interpreter {

class interpreter {
 public:
  interpreter();
  frame* run(std::string_view starlark_program,
      const starlark::grammar::grammar_options& g_options,
      const std::map<std::string, starlark::runtime::starlark_obj*, std::less<>>& custom_binding,
      google::protobuf::Arena& arena,
      starlark::logging::logger& logging);
};

}  // namespace interpreter
}  // namespace starlark

#pragma GCC visibility pop

#endif  // INTERPRETER_INTERPRETER_HPP_

