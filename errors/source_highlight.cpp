// Copyright 2026 Lucas Mirelmann

#include "errors/source_highlight.hpp"

#include <format>
#include <string>
#include <string_view>

#include "proto/starlark_logging.pb.h"

using ::starlark::logging::Position;
using ::starlark::logging::PositionInFile;

namespace starlark {
namespace error_messages {

std::string get_line_and_underline(std::string_view program, const PositionInFile& pos) {
  return get_line_and_underline(program, pos.start(), pos.end());
}

std::string get_line_and_underline(std::string_view program, const Position& start, const Position& end) {
  return get_line_and_underline(program, start, end, false);
}

std::string get_line_and_underline(std::string_view program, const Position& start, const Position& end, bool reverse) {
  return get_line_and_underline(program, start, end, reverse, "");
}

std::string get_line_and_underline(std::string_view program, const Position& start, const Position& end, bool reverse, std::string_view hint) {
  std::string result;
  // TODO(lmirelmann): This can be improved as this information can be part of the program while being parsed.
  // We cannot use the information from `column` directly as this would not be taking into consideration Unicode characters
  // that their UTF8 representation is 2 or more characters.
  // TODO(lmirelmann): Whenever `reverse` is true, and `end.row() != start.row()`, an alternative would be to print the line where `end` is at
  //   This changes where the error is presented.
  //   An alternative would be to show both the `start.row()` and `end.row()` in two lines. We would need to take a look at how this is presented
  //   in cases line unterminated triple-quoted strings as we have to make sure that we do not pick a line that has zero length.
  //   This needs a lot more thought.
  auto start_pos = start.pos();
  while (start_pos > 0 && program[start_pos] != '\n' && program[start_pos] != '\r') {
    --start_pos;
  }
  if (program[start_pos] == '\n' || program[start_pos] == '\r') {
    ++start_pos;
  }
  auto end_pos = program.find_first_of("\n\r", start.pos());
  std::string_view line;
  if (end_pos == std::string_view::npos) {
    line = program.substr(start_pos);
  } else {
    line = program.substr(start_pos, end_pos - start_pos);
  }
  result += std::format("{:5} | {}\n", start.row(), line);
  if (start.row() == end.row()) {
    if (reverse) {
      result += std::format("      |{:{}}{:~>{}}\n", ' ', start.column(), '^', end.column() - start.column());
    } else {
      result += std::format("      |{:{}}{:~<{}}\n", ' ', start.column(), '^', end.column() - start.column());
    }
  } else {
    if (reverse) {
      result += std::format("      |{:{}}{:~>{}}\n", ' ', start.column(), '^', line.size() - start.column() + 1);
    } else {
      result += std::format("      |{:{}}{:~<{}}\n", ' ', start.column(), '^', line.size() - start.column() + 1);
    }
  }
  if (!hint.empty()) {
    result += std::format("      |{:{}}{}\n", ' ', start.column(), hint);
  }
  return result;
}

}  // namespace error_messages
}  // namespace starlark

