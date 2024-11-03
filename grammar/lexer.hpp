// Copyright 2024 Lucas Mirelmann

#ifndef GRAMMAR_LEXER_HPP_
#define GRAMMAR_LEXER_HPP_

#include <string_view>
#include <utility>
#include <vector>

#include "grammar/token.hpp"
#include "unicode/utf8_reader.hpp"

#pragma GCC visibility push(default)

namespace grammar {

class lexer {
 public:
  lexer(std::string_view input);
  const token& current_token() const;
  void next_token();
  const std::vector<std::pair<std::size_t, std::size_t>>& comments() const;
  const std::vector<std::pair<std::string, std::size_t>>& errors() const;

 private:
  void tokenize();
  void consume_indentation(bool modify_indents);
  void read_operator(char first_char);
  void read_numeric();
  void read_string();
  bool read_escaped_char(std::string& result, bool utf8_encode, int max_value, int min_size, int max_size, int base);
  std::string read_identifier_or_keyword();
  void add_error(std::string_view message, std::size_t pos);
  void add_comment(std::size_t start, std::size_t end);

  token current;
  unicode::utf8_reader source_code;
  int pending_indents = 0;
  std::vector<int> indent_stack;
  int open_brackets = 0;

  std::vector<std::pair<std::string, std::size_t>> errors_found;
  std::vector<std::pair<std::size_t, std::size_t>> comments_found;
};

}  // namespace grammar

#pragma GCC visibility pop

#endif  // GRAMMAR_LEXER_HPP_

