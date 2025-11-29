// Copyright 2025 Lucas Mirelmann

#include "bigint/number_stream_for_test.hpp"

#include "bigint/number.hpp"

using ::starlark::bigint::number;

namespace starlark {
namespace bigint {

std::ostream& operator<<(std::ostream& os, const number& n) {
  os << n.to_string(10);
  return os;
}

}  // namespace bigint
}  // namespace starlark

