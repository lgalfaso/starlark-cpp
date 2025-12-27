// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_ERROR_FN_HPP_
#define RUNTIME_ERROR_FN_HPP_

#include <map>
#include <span>
#include <string>
#include <string_view>
#include <unordered_set>
#include <variant>
#include <vector>

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

class error_fn {
 public:
  virtual void add_error(std::string_view error_msg) = 0;
  virtual void replace_last_error(std::string_view error_msg) = 0;
};

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_ERROR_FN_HPP_

