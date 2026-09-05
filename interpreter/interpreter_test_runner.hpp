// Copyright 2026 Lucas Mirelmann

#ifndef INTERPRETER_INTERPRETER_TEST_RUNNER_HPP_
#define INTERPRETER_INTERPRETER_TEST_RUNNER_HPP_

#include <map>
#include <string>

#include "logging/logging.hpp"
#include "status_or/status.hpp"
#include "vm/frame.hpp"
#include "vm/test_case.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace interpreter_runner {

using ::starlark::vm::test::split_test_case;

starlark::result::status_or<starlark::vm::frame*> run_test(std::map<std::string, std::string> programs, starlark::logging::logger& logging);

}  // namespace interpreter_runner
}  // namespace starlark

#pragma GCC visibility pop

#endif  // INTERPRETER_INTERPRETER_TEST_RUNNER_HPP_
