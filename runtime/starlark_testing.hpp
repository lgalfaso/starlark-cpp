// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_TESTING_HPP_
#define RUNTIME_STARLARK_TESTING_HPP_

#include <string>
#include <string_view>
#include <vector>

#include "runtime/error_fn.hpp"
#include "runtime/starlark_function.hpp"

namespace starlark {
namespace testing {

struct error_handler : public starlark::runtime::error_fn {
  void add_error(std::string_view error_msg) override;

  std::vector<std::string> messages;
};

class starlark_testing_function : public starlark::runtime::starlark_function {
 public:
  starlark_testing_function();
  explicit starlark_testing_function(std::string_view fn_name);
  starlark_obj* call(const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, starlark::runtime::context& ctx, starlark::runtime::error_fn& error_callback) override;

 protected:
  bool inner_equals(starlark::runtime::equals_comparator& comp, const starlark::runtime::starlark_obj* other) const override;
};

}  // namespace testing
}  // namespace starlark

#endif  // RUNTIME_STARLARK_TESTING_HPP_

