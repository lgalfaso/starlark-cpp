// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_ERROR_FN_HPP_
#define RUNTIME_ERROR_FN_HPP_

#include <string_view>

#include "proto/starlark_logging.pb.h"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

class error_fn {
 public:
  virtual void add_error(std::string_view error_msg) = 0;
  virtual void add_error(std::string_view error_msg, const starlark::logging::Position& pos) = 0;
};

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_ERROR_FN_HPP_

