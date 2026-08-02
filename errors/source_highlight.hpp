// Copyright 2026 Lucas Mirelmann

#ifndef ERRORS_SOURCE_HIGHLIGHT_HPP_
#define ERRORS_SOURCE_HIGHLIGHT_HPP_

#include <string>
#include <string_view>

#include "proto/starlark_logging.pb.h"

#pragma GCC visibility push(default)

namespace starlark {
namespace error_messages {

std::string get_line_and_underline(std::string_view program, const starlark::logging::PositionInFile& pos);
std::string get_line_and_underline(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string get_line_and_underline(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end, bool reverse);
std::string get_line_and_underline(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end, bool reverse, std::string_view hint);

}  // namespace error_messages
}  // namespace starlark

#pragma GCC visibility pop

#endif  // ERRORS_SOURCE_HIGHLIGHT_HPP_

