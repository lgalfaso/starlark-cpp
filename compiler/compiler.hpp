// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_COMPILER_HPP_
#define COMPILER_COMPILER_HPP_

#include <functional>
#include <set>
#include <string>
#include <string_view>

#include "grammar/options.hpp"
#include "logging/logging.hpp"
#include "proto/starlark_bytecode.pb.h"

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {

class compiler {
 public:
  explicit compiler(std::set<std::string, std::less<>> &binding);
  starlark::bytecode::Program* compile(
      std::string_view program_name,
      std::string_view starlark_program,
      starlark::grammar::grammar_options options,
      starlark::logging::logger& logging,
      google::protobuf::Arena& arena);

 private:
  const std::set<std::string, std::less<>> binding;
};

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_COMPILER_HPP_

