// Copyright 2026 Lucas Mirelmann

#ifndef DIFFERENTIAL_SPEC_SUPPORT_HPP_
#define DIFFERENTIAL_SPEC_SUPPORT_HPP_

#include <string_view>

#include "differential/engine.hpp"
#include "native/runner/native_options.hpp"
#include "runtime/options.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace differential {

compare_result check_source_like_spec_tests(std::string_view source,
    const starlark::runtime::runtime_options& options,
    const starlark::native::native_options& native_options = {});

}  // namespace differential
}  // namespace starlark

#pragma GCC visibility pop

#endif  // DIFFERENTIAL_SPEC_SUPPORT_HPP_
