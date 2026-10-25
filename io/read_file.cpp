// Copyright 2026 Lucas Mirelmann

#include "io/read_file.hpp"

#include <fstream>
#include <string>

namespace starlark {
namespace io {

std::optional<std::string> read_file(std::string_view path) {
  std::ifstream in(std::string{path}, std::ios::binary);
  if (!in) {
    return std::nullopt;
  }
  in.seekg(0, std::ios::end);
  const auto size = in.tellg();
  if (size < 0) {
    return std::nullopt;
  }
  std::string out(static_cast<std::size_t>(size), '\0');
  in.seekg(0, std::ios::beg);
  if (!in.read(out.data(), static_cast<std::streamsize>(out.size()))) {
    return std::nullopt;
  }
  return out;
}

}  // namespace io
}  // namespace starlark
