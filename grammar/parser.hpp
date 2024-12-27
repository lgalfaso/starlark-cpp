// Copyright 2024 Lucas Mirelmann

#ifndef GRAMMAR_PARSER_HPP_
#define GRAMMAR_PARSER_HPP_

#include <string_view>

#include "grammar/lexer.hpp"
#include "grammar/options.hpp"
#include "grammar/proto/starlark.pb.h"

#pragma GCC visibility push(default)

namespace grammar {

class parser {
 public:
  parser(std::string_view input);
  parser(std::string_view input, const grammar_options& options);
  parser() = delete;
  parser(const parser&) = delete;
  parser(parser&&) = delete;
  starlark::File parse_file();
  // TODO(lmirelmann): These two errors should be merged.
  const std::vector<std::pair<std::string, position>>& parser_errors() const;
  const std::vector<std::pair<std::string, position>>& lexer_errors() const;

 private:
  grammar_options options;
  lexer lex;
  std::vector<std::pair<std::string, position>> errors;
  std::vector<int> nested_loops;
  bool recover = false;

  bool capture(token_type expected_token);
  bool is_current(token_type expected_token) const;
  bool expect(token_type expected_token);
  void add_error(const std::string& error_message);
  void parse_statement(google::protobuf::RepeatedPtrField<starlark::Statement>& statements);
  bool set_identifier(starlark::Identifier& id);
};

}  // namespace grammar

#pragma GCC visibility pop

#endif  // GRAMMAR_PARSER_HPP_

