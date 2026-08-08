// Copyright 2026 Lucas Mirelmann

#include "errors/source_highlight.hpp"

#include <format>
#include <string>
#include <string_view>

#include "proto/starlark_logging.pb.h"

using ::starlark::logging::Position;

namespace starlark {
namespace error_messages {

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
  while (start_pos > 0 && program[start_pos - 1] != '\n' && program[start_pos - 1] != '\r') {
    --start_pos;
  }
  std::string_view line = program.substr(start_pos, program.find_first_of("\n\r", start.pos()) - start_pos);
  result += std::format("{:5} | {}\n", start.row(), line);
  auto padding_size = start.column();
  auto underline_size = start.row() == end.row() ? end.column() - start.column() : line.size() - start.column() + 1;
  if (reverse) {
    result += std::format("      |{:{}}{:~>{}}\n", ' ', padding_size, '^', underline_size);
  } else {
    result += std::format("      |{:{}}{:~<{}}\n", ' ', padding_size, '^', underline_size);
  }
  if (!hint.empty()) {
    result += std::format("      |{:{}}{}\n", ' ', padding_size, hint);
  }
  return result;
}

std::string get_line_and_underline(std::string_view program, const Position& start, const Position& mid, const Position& end) {
  std::string result;
  // TODO(lmirelmann): Merge into a single implementation.
  auto start_pos = start.pos();
  while (start_pos > 0 && program[start_pos - 1] != '\n' && program[start_pos - 1] != '\r') {
    --start_pos;
  }
  std::string_view line = program.substr(start_pos, program.find_first_of("\n\r", start.pos()) - start_pos);
  result += std::format("{:5} | {}\n", start.row(), line);
  auto padding_size = start.column();
  auto underline_size = start.row() == mid.row() ? mid.column() - start.column() : line.size() - start.column() + 1;
  result += std::format("      |{:{}}{:~>{}}", ' ', padding_size, '~', underline_size);
  if (start.row() == end.row() && mid.column() != end.column()) {
    result += std::format("{:^>{}}", '^', end.column() - mid.column());
  }
  result += "\n";
  return result;
}

}  // namespace error_messages
}  // namespace starlark

