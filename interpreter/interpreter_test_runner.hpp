// Copyright 2026 Lucas Mirelmann

#ifndef INTERPRETER_TEST_RUNNER_HPP_
#define INTERPRETER_TEST_RUNNER_HPP_

#include <map>
#include <string_view>

#include "interpreter/frame.hpp"
#include "logging/logging.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace interpreter_runner {

starlark::interpreter::frame* run_test(std::map<std::string_view, std::string_view> programs, starlark::logging::logger& logging);

}  // namespace interpreter_runner
}  // namespace starlark

#pragma GCC visibility pop

#endif  // INTERPRETER_TEST_RUNNER_HPP_

