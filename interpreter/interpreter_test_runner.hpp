// Copyright 2026 Lucas Mirelmann

#ifndef INTERPRETER_TEST_RUNNER_HPP_
#define INTERPRETER_TEST_RUNNER_HPP_

#include <string_view>

#include "interpreter/frame.hpp"
#include "logging/logging.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace interpreter_runner {

starlark::interpreter::frame* run_test(std::string_view program, starlark::logging::logger& logging);

}  // namespace interpreter_runner
}  // namespace starlark

#pragma GCC visibility pop

#endif  // INTERPRETER_TEST_RUNNER_HPP_

