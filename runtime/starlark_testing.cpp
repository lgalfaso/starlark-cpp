// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_testing.hpp"

#include <string>
#include <string_view>

namespace starlark {
namespace testing {

void error_handler::add_error(std::string_view error_msg) {
  messages.push_back(std::string(error_msg));
}

}  // namespace testing
}  // namespace starlark

