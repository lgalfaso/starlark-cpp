// Copyright 2026 Lucas Mirelmann

#include "errors/source_highlight.hpp"

#include <format>
#include <string>
#include <string_view>

#include "proto/starlark_logging.pb.h"

using ::starlark::logging::PositionInFile;

namespace starlark {
namespace error_messages {

std::string get_line_and_underline(std::string_view program, const PositionInFile& pos) {
  std::string result;
  // TODO(lmirelmann): This can be improved as this information can be part of the program while being parsed.
  // We cannot use the information from `column` directly as this would not be taking into consideration Unicode characters
  // that their UTF8 representation is 2 or more characters.
  auto start = pos.start().pos();
  while (start > 0 && program[start] != '\n' && program[start] != '\r') {
    --start;
  }
  if (program[start] == '\n' || program[start] == '\r') {
    ++start;
  }
  auto end = program.find_first_of("\n\r", pos.start().pos());
  std::string_view line;
  if (end == std::string_view::npos) {
    line = program.substr(start);
  } else {
    line = program.substr(start, end - start);
  }
  result += std::format("{:5} | {}\n", pos.start().row(), line);
  if (pos.start().row() == pos.end().row()) {
    result += std::format("      |{:{}}{:~<{}}\n", ' ', pos.start().column(), '^', pos.end().column() - pos.start().column());
  } else {
    result += std::format("      |{:{}}{:~<{}}\n", ' ', pos.start().column(), '^', line.size() - pos.start().column() + 1);
  }
  return result;
}

}  // namespace error_messages
}  // namespace starlark

