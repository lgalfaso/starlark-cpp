// Copyright 2026 Lucas Mirelmann

#include "errors/source_highlight.hpp"

#include <format>
#include <string>
#include <string_view>

#include "proto/starlark_logging.pb.h"
#include "unicode/utf8_reader.hpp"

using ::starlark::logging::Position;
using ::starlark::unicode::is_utf8_continue;
using ::starlark::unicode::utf8_reader;

namespace starlark {
namespace error_messages {

namespace {

int64_t count_chars(std::string_view input) {
  int64_t result = 0;
  for (auto c : input) {
    // We do not count Unicode continuation characters as we are interested in the Unicode length.
    // TODO(lmirelmann): This is not taking into consideration if the character is a combining mark. Doing this would be a lot of work.
    if (!is_utf8_continue(c)) {
      ++result;
    }
  }
  return result;
}

int64_t safe_dec(int64_t a, int64_t b) {
  if (a >= b) {
    return a - b;
  }
  return 0;
}

}  // namespace

starlark::logging::Position previous_position(std::string_view program, const Position& input) {
  if (input.column() == 1 && input.row() == 1) {
    return input;
  }
  Position result = input;
  if (input.column() != 1) {
    int delta = 1;
    while (delta < input.pos() && is_utf8_continue(program[input.pos() - delta + 1])) {
      delta++;
    }
    result.set_column(input.column() - 1);
    result.set_pos(input.pos() - delta);
    result.set_row(input.row());
  } else {
    // TODO(lmirelmann): This could be optimized if we knew every line and its size.
    int previous_line_length = 1;
    while (previous_line_length < input.pos() && program[input.pos() - previous_line_length - 1] != '\n') {
      previous_line_length++;
    }
    result.set_column(count_chars(program.substr(input.pos() - previous_line_length, previous_line_length)) - 1);
    result.set_pos(input.pos() - 1);
    result.set_row(input.row() - 1);
  }
  return result;
}

starlark::logging::Position next_position(std::string_view program, const Position& input) {
  if (input.pos() == program.size()) {
    return input;
  }
  Position result;
  if (program[input.pos()] == '\n') {
    result.set_column(1);
    result.set_pos(input.pos() + 1);
    result.set_row(input.row() + 1);
  } else {
    // This is somehow of an overkill, but better safe that sorry.
    utf8_reader reader(program.substr(input.pos()), /*strict=*/ false, /*remove_bom=*/ false);
    auto ch = reader.peek_code_point();
    result.set_column(input.column() + 1);
    result.set_pos(input.pos() + ch.second);
    result.set_row(input.row());
  }
  return result;
}

std::string get_line_and_underline(std::string_view program, const Position& start, const Position& start_underline, const Position& end_underline, const Position& end, std::string_view hint) {
  assert(start.pos() <= start_underline.pos());
  // Ideally, this should be `start_underline.pos() < end_underline.pos()`, but this is not possible yet as sometimes we produce an error at the last character of a program.
  assert(start_underline.pos() <= end_underline.pos());
  assert(end_underline.pos() <= end.pos());
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
  auto first_curly_size = safe_dec(start_underline.column(), padding_size);
  if (first_curly_size > 0) {
    result += std::format("{:~>{}}", '~', first_curly_size);
  }
  result += std::format("{:^>{}}", '^', start_underline.row() == end_underline.row() ? safe_dec(end_underline.column(), start_underline.column()) : safe_dec(count_chars(line), start_underline.column()));
  if (start_underline.row() == end_underline.row()) {
    auto second_curly_size = end_underline.row() == end.row() ? safe_dec(end.column(), end_underline.column()) : safe_dec(count_chars(line) + 1, end_underline.column());
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
    return get_line_and_underline(program, start, previous_position(program, end), end, end, hint);
  } else {
    Position underline_end = next_position(program, start);
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

