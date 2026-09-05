// Copyright 2026 Lucas Mirelmann

#ifndef VM_TEST_ASSERTIONS_HPP_
#define VM_TEST_ASSERTIONS_HPP_

#include <functional>
#include <ostream>
#include <sstream>
#include <string_view>

#include "google/protobuf/arena.h"
#include "logging/logging.hpp"
#include "runtime/options.hpp"
#include "runtime/starlark_object.hpp"
#include "status_or/status.hpp"
#include "vm/frame.hpp"
#include "vm/module_loader.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace vm {
namespace test {

starlark::vm::module_info::bindings_t make_common_test_bindings(google::protobuf::Arena& arena);

starlark::runtime::starlark_obj* assert_eq_fn(starlark::runtime::starlark_obj* this_obj,
    const starlark::runtime::starlark_obj::pos_args_t& pos_args,
    const starlark::runtime::starlark_obj::named_args_t& named_args,
    starlark::runtime::context& ctx,
    starlark::runtime::error_fn& error_callback);

starlark::runtime::starlark_obj* assert_ne_fn(starlark::runtime::starlark_obj* this_obj,
    const starlark::runtime::starlark_obj::pos_args_t& pos_args,
    const starlark::runtime::starlark_obj::named_args_t& named_args,
    starlark::runtime::context& ctx,
    starlark::runtime::error_fn& error_callback);

starlark::runtime::starlark_obj* assert_true_fn(starlark::runtime::starlark_obj* this_obj,
    const starlark::runtime::starlark_obj::pos_args_t& pos_args,
    const starlark::runtime::starlark_obj::named_args_t& named_args,
    starlark::runtime::context& ctx,
    starlark::runtime::error_fn& error_callback);

starlark::runtime::starlark_obj* assert_false_fn(starlark::runtime::starlark_obj* this_obj,
    const starlark::runtime::starlark_obj::pos_args_t& pos_args,
    const starlark::runtime::starlark_obj::named_args_t& named_args,
    starlark::runtime::context& ctx,
    starlark::runtime::error_fn& error_callback);

starlark::result::status_or<starlark::runtime::runtime_options> parse_runtime_options(starlark::runtime::starlark_obj** error,
    starlark::runtime::starlark_obj** print,
    bool* allow_static_error,
    std::ostream& out,
    const starlark::runtime::starlark_obj::named_args_t& named_args,
    starlark::runtime::context& ctx,
    starlark::runtime::error_fn& error_callback);

using program_runner = std::function<starlark::result::status_or<starlark::vm::frame*>(
    starlark::vm::module_loader&, starlark::logging::logger&, const starlark::runtime::runtime_options&)>;

starlark::runtime::starlark_obj* check_assert_fail_with_loader(starlark::vm::module_loader& loader,
    std::string_view source,
    const starlark::runtime::runtime_options& r_options,
    starlark::runtime::starlark_obj* expected_error,
    bool allow_static_error,
    const program_runner& run,
    starlark::logging::logger& logging,
    starlark::runtime::context& ctx,
    starlark::runtime::error_fn& error_callback);

starlark::runtime::starlark_obj* check_assert_fail(std::string_view source,
    const starlark::runtime::runtime_options& r_options,
    starlark::runtime::starlark_obj* expected_error,
    bool allow_static_error,
    const program_runner& run,
    starlark::logging::logger& logging,
    starlark::runtime::context& ctx,
    starlark::runtime::error_fn& error_callback);

starlark::runtime::starlark_obj* check_assert_succeed_with_loader(starlark::vm::module_loader& loader,
    std::string_view source,
    const starlark::runtime::runtime_options& r_options,
    starlark::runtime::starlark_obj* expected_print,
    const program_runner& run,
    starlark::logging::logger& logging,
    std::ostringstream& out,
    starlark::runtime::context& ctx,
    starlark::runtime::error_fn& error_callback);

starlark::runtime::starlark_obj* check_assert_succeed(std::string_view source,
    const starlark::runtime::runtime_options& r_options,
    starlark::runtime::starlark_obj* expected_print,
    const program_runner& run,
    starlark::logging::logger& logging,
    std::ostringstream& out,
    starlark::runtime::context& ctx,
    starlark::runtime::error_fn& error_callback);

}  // namespace test
}  // namespace vm
}  // namespace starlark

#pragma GCC visibility pop

#endif  // VM_TEST_ASSERTIONS_HPP_
