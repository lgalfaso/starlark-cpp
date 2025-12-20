// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_TESTING_HPP_
#define RUNTIME_STARLARK_TESTING_HPP_

#include <string>
#include <vector>

#include "runtime/error_fn.hpp"

namespace starlark {
namespace testing {

struct error_handler : public starlark::runtime::error_fn {
  void add_error(std::string_view error_msg) override {
    messages.push_back(std::string(error_msg));
  }

  std::vector<std::string> messages;
};

}  // namespace testing
}  // namespace starlark

#endif  // RUNTIME_STARLARK_TESTING_HPP_

