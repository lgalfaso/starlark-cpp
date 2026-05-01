// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_testing.hpp"

#include <string>
#include <string_view>

using ::starlark::runtime::context;
using ::starlark::runtime::equals_comparator;
using ::starlark::runtime::error_fn;
using ::starlark::runtime::starlark_obj;

namespace starlark {
namespace testing {

void error_handler::add_error(std::string_view error_msg) {
  messages.push_back(std::string(error_msg));
}

starlark_testing_function::starlark_testing_function() : starlark_testing_function("test_fn") {}

starlark_testing_function::starlark_testing_function(std::string_view fn_name) : starlark_function(fn_name) {}

starlark_obj* starlark_testing_function::call(const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  return starlark_obj::call(pos_args, named_args, ctx, error_callback);
}

bool starlark_testing_function::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
  return false;
}

}  // namespace testing
}  // namespace starlark

