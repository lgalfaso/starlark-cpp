// Copyright 2024 Lucas Mirelmann

#ifndef GRAMMAR_PARSER_HPP_
#define GRAMMAR_PARSER_HPP_

#include <string_view>

#include "grammar/lexer.hpp"
#include "grammar/proto/starlark.pb.h"

#pragma GCC visibility push(default)

namespace grammar {

class parser {
 public:
  parser(std::string_view input);
  starlark::File parse_file();
  // TODO(lmirelmann): These two errors should be merged.
  const std::vector<std::pair<std::string, std::size_t>>& parser_errors() const;
  const std::vector<std::pair<std::string, std::size_t>>& lexer_errors() const;

 private:
  lexer lex;
  std::vector<std::pair<std::string, std::size_t>> errors;
  std::vector<int> nested_loops;
  bool recover = false;

  bool capture(token_type expected_token);
  bool is_current(token_type expected_token) const;
  bool expect(token_type expected_token);
  void add_error(const std::string& error_message);
  void parse_statement(google::protobuf::RepeatedPtrField<starlark::Statement>& statements);
  void parse_suite(google::protobuf::RepeatedPtrField<starlark::Statement>& statements);
  void parse_simple_statement(google::protobuf::RepeatedPtrField<starlark::Statement>& statements);
  starlark::Statement parse_small_statement();
  starlark::Expression parse_expression(bool allow_trailing_comma);
  starlark::Test parse_test();
  starlark::Test parse_test(int precedence);
  starlark::PrimaryExpr parse_primary();
  starlark::PrimaryExpr::Operand parse_operand();
  starlark::PrimaryExpr::Operand parse_list();
  starlark::PrimaryExpr::Operand parse_dict();
  starlark::PrimaryExpr::Operand::Entry parse_entry();
  starlark::PrimaryExpr::Operand::CompClause parse_comp_clause();
  starlark::PrimaryExpr::CallExpr::Argument parse_argument();
  starlark::Test::LambdaExpr parse_lambda();
  void parse_parameters(google::protobuf::RepeatedPtrField<starlark::Parameter>& parameters, bool allow_trailing_comma);
};

}  // namespace grammar

#pragma GCC visibility pop

#endif  // GRAMMAR_PARSER_HPP_

