// Copyright 2026 Lucas Mirelmann

#include "io/read_file.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>

#include "third-party/defer.hpp"

namespace starlark {
namespace io {

std::optional<std::string> read_file(std::string_view path) {
  const int fd = open(std::string{path}.c_str(), O_RDONLY);
  if (fd < 0) {
    return std::nullopt;
  }
  defer { close(fd); };
  struct stat sb;
  if (fstat(fd, &sb) < 0) {
    return std::nullopt;
  }
  std::string out;
  out.resize(static_cast<std::size_t>(sb.st_size));
  if (read(fd, out.data(), out.size()) != static_cast<ssize_t>(out.size())) {
    return std::nullopt;
  }
  return out;
}

}  // namespace io
}  // namespace starlark
