// Copyright 2026 Lucas Mirelmann

#ifndef VM_PROGRAM_LIMITS_HPP_
#define VM_PROGRAM_LIMITS_HPP_

#include <optional>
#include <string>
#include <string_view>

#include "proto/starlark_bytecode.pb.h"
#include "proto/starlark_logging.pb.h"
#include "runtime/options.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace vm {

struct program_limit_violation {
  std::string message;
  starlark::logging::Position position;
};

std::optional<program_limit_violation> check_program_limits(const starlark::bytecode::Program& program,
    std::string_view source,
    const starlark::runtime::runtime_options& options);

}  // namespace vm
}  // namespace starlark

#pragma GCC visibility pop

#endif  // VM_PROGRAM_LIMITS_HPP_
