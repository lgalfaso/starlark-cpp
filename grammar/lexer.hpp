// Copyright 2024-2025 Lucas Mirelmann

#ifndef GRAMMAR_LEXER_HPP_
#define GRAMMAR_LEXER_HPP_

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "grammar/options.hpp"
#include "grammar/token.hpp"
#include "logging/logging.hpp"
#include "unicode/utf8_reader.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace grammar {

class lexer {
 public:
  lexer(std::string_view program_name, std::string_view input, starlark::logging::logger& logging);
  lexer(std::string_view program_name, std::string_view input, const grammar_options& options, starlark::logging::logger& logging);
  const token& current_token() const;
  void next_token();
  const std::vector<std::pair<starlark::logging::Position, starlark::logging::Position>>& comments() const;

 private:
  void tokenize();
  void consume_indentation(bool modify_indents);
  void read_operator(char first_char);
  void read_numeric();
  void read_string();
  bool read_escaped_char(std::string& result, bool utf8_encode, int max_value, int min_size, int max_size, int base);
  std::string read_identifier_or_keyword();
  void add_error(std::string_view message, starlark::logging::Position pos);
  void add_warning(std::string_view message, starlark::logging::Position pos);
  void add_comment(starlark::logging::Position start, starlark::logging::Position end);
  void newline();
  starlark::logging::Position get_position() const;

  grammar_options opts;
  std::size_t current_line = 0;
  std::size_t last_begin_of_line = 0;
  std::size_t indent_ignore = 0;
  std::string_view program_name;
  std::string_view input;
  unicode::utf8_reader source_code;
  token current;
  int pending_indents = 0;
  std::vector<int> indent_stack;
  int open_brackets = 0;

  starlark::logging::logger& logging;
  std::vector<std::pair<starlark::logging::Position, starlark::logging::Position>> comments_found;
};

}  // namespace grammar
}  // namespace starlark

#pragma GCC visibility pop

#endif  // GRAMMAR_LEXER_HPP_

