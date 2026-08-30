// Copyright 2026 Lucas Mirelmann

#ifndef IO_READ_FILE_HPP_
#define IO_READ_FILE_HPP_

#include <optional>
#include <string>
#include <string_view>

#pragma GCC visibility push(default)

namespace starlark {
namespace io {

// Returns file contents, or std::nullopt on any open/fstat/read error.
std::optional<std::string> read_file(std::string_view path);

}  // namespace io
}  // namespace starlark

#pragma GCC visibility pop

#endif  // IO_READ_FILE_HPP_
