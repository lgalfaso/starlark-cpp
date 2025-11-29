// Copyright 2025 Lucas Mirelmann

#ifndef BIGINT_NUMBER_STREAM_FOR_TEST_HPP_
#define BIGINT_NUMBER_STREAM_FOR_TEST_HPP_

#include <ostream>

#include "bigint/number.hpp"

namespace starlark {
namespace bigint {

std::ostream& operator<<(std::ostream& os, const starlark::bigint::number& n);

}  // namespace bigint
}  // namespace starlark

#endif  // BIGINT_NUMBER_STREAM_FOR_TEST_HPP_

