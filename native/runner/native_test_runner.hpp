// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_RUNNER_NATIVE_TEST_RUNNER_HPP_
#define NATIVE_RUNNER_NATIVE_TEST_RUNNER_HPP_

#include <map>
#include <string>

#include "logging/logging.hpp"
#include "status_or/status.hpp"
#include "vm/frame.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace native_test_runner {

starlark::result::status_or<starlark::vm::frame*> run_native_test(std::map<std::string, std::string> programs, starlark::logging::logger& logging);

}  // namespace native_test_runner
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_RUNNER_NATIVE_TEST_RUNNER_HPP_
