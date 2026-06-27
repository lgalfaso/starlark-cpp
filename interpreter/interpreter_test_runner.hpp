// Copyright 2026 Lucas Mirelmann

#ifndef INTERPRETER_INTERPRETER_TEST_RUNNER_HPP_
#define INTERPRETER_INTERPRETER_TEST_RUNNER_HPP_

#include <map>
#include <string>

#include "interpreter/frame.hpp"
#include "logging/logging.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace interpreter_runner {

starlark::interpreter::frame* run_test(std::map<std::string, std::string> programs, starlark::logging::logger& logging);
std::map<std::string, std::string> split_test_case(std::string_view source);

}  // namespace interpreter_runner
}  // namespace starlark

#pragma GCC visibility pop

#endif  // INTERPRETER_INTERPRETER_TEST_RUNNER_HPP_

