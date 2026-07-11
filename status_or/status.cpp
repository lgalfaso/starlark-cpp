// Copyright 2026 Lucas Mirelmann

#include "status_or/status.hpp"

namespace starlark {
namespace result {

status::status(status_code code) : code(code) {}

bool status::ok() const {
  return code == status_code::kOk;
}

status ok_status() {
  return status(status_code::kOk);
}

status error_status() {
  return status(status_code::kRuntimeError);
}

}  // namespace result
}  // namespace starlark

