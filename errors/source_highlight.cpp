// Copyright 2026 Lucas Mirelmann

#include "errors/source_highlight.hpp"

#include <format>
#include <string>
#include <string_view>

#include "proto/starlark_logging.pb.h"

using ::starlark::logging::Position;

namespace starlark {
namespace error_messages {

std::string get_line_and_underline(std::string_view program, const Position& start, const Position& start_underline, const Position& end_underline, const Position& end, std::string_view hint) {
  std::string result;
  auto start_pos = start_underline.pos();
  while (start_pos > 0 && program[start_pos - 1] != '\n' && program[start_pos - 1] != '\r') {
    --start_pos;
  }
  std::string_view line = program.substr(start_pos, program.find_first_of("\n\r", start_underline.pos()) - start_pos);

  // Write the first line.
  result += std::format("{:5} | {}\n", start_underline.row(), line);

  // Write the second line.
  auto padding_size = start.row() == start_underline.row() ? start.column() : 1;
  result += std::format("      |{:{}}", ' ', padding_size);
  auto first_curly_size = start_underline.column() - padding_size;
  if (first_curly_size > 0) {
    result += std::format("{:~>{}}", '~', first_curly_size);
  }
  result += std::format("{:^>{}}", '^', start_underline.row() == end_underline.row() ? end_underline.column() - start_underline.column() : line.size() - start_underline.column());
  if (start_underline.row() == end_underline.row()) {
    auto second_curly_size = end_underline.row() == end.row() ? end.column() - end_underline.column() : line.size() - end_underline.column() + 1;
    if (second_curly_size > 0) {
      result += std::format("{:~>{}}", '~', second_curly_size);
    }
  }
  result += '\n';

  // Write the third line.
  if (!hint.empty()) {
    result += std::format("      |{:{}}{}\n", ' ', start_underline.column(), hint);
  }
  return result;
}

std::string get_line_and_underline(std::string_view program, const Position& start, const Position& end) {
  return get_line_and_underline(program, start, end, false);
}

std::string get_line_and_underline(std::string_view program, const Position& start, const Position& end, bool reverse) {
  return get_line_and_underline(program, start, end, reverse, "");
}

std::string get_line_and_underline(std::string_view program, const Position& start, const Position& end, bool reverse, std::string_view hint) {
  if (reverse) {
    Position underline_start = end;
    if (end.column() != 1) {
      underline_start.set_column(end.column() - 1);
      underline_start.set_pos(end.pos() - 1);
      underline_start.set_row(end.row());
    } else if (end.row() != 1) {
      // TODO(lmirelmann): This could be optimized if we knew every line and its size.
      int previous_line_length = 1;
      while (previous_line_length < end.pos() && program[end.pos() - previous_line_length - 1] != '\n') {
        previous_line_length++;
      }
      underline_start.set_column(previous_line_length - 1);
      underline_start.set_pos(end.pos() - 1);
      underline_start.set_row(end.row() - 1);
    }
    return get_line_and_underline(program, start, underline_start, end, end, hint);
  } else {
    Position underline_end;
    underline_end.set_column(start.column() + 1);
    underline_end.set_pos(start.pos() + 1);
    underline_end.set_row(start.row());
    if (underline_end.pos() > end.pos()) {
      return get_line_and_underline(program, start, start, underline_end, underline_end, hint);
    } else {
      return get_line_and_underline(program, start, start, underline_end, end, hint);
    }
  }
}

std::string get_line_and_underline(std::string_view program, const Position& start, const Position& mid, const Position& end) {
  return get_line_and_underline(program, start, mid, end, end, "");
}


}  // namespace error_messages
}  // namespace starlark

