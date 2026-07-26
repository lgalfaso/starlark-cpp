// Copyright 2026 Lucas Mirelmann

#ifndef RUNTIME_PARSING_OPTIONS_HPP_
#define RUNTIME_PARSING_OPTIONS_HPP_

#include <string_view>

#include "runtime/options.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

runtime_options get_runtime_options(std::string_view starlark_program, std::ostream& out);

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_PARSING_OPTIONS_HPP_

