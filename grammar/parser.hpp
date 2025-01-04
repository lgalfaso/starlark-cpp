// Copyright 2024 Lucas Mirelmann

#ifndef GRAMMAR_PARSER_HPP_
#define GRAMMAR_PARSER_HPP_

#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "grammar/lexer.hpp"
#include "grammar/logging.hpp"
#include "grammar/options.hpp"
#include "grammar/proto/starlark.pb.h"

#pragma GCC visibility push(default)

namespace grammar {

struct expression_frame {
  const starlark::Expression* expression = nullptr;
  const starlark::PrimaryExpr* primary_expression = nullptr;
  const starlark::Test* test = nullptr;
};

class parser {
 public:
  parser(std::string_view input, logger& logging);
  parser(std::string_view input, const grammar_options& options, logger& logging);
  parser() = delete;
  parser(const parser&) = delete;
  parser(parser&&) = delete;
  starlark::File* parse_file(google::protobuf::Arena& arena);

  static constexpr std::string module = "Parser";

 private:
  void parse_statement(google::protobuf::RepeatedPtrField<starlark::Statement>& statements);
  void bind(const starlark::PrimaryExpr* primary_expression);
  void bind(const starlark::Expression* expression);
  void bind(expression_frame frame);
  bool set_identifier(starlark::Identifier& id);
  bool capture(token_type expected_token);
  bool is_current(token_type expected_token) const;
  bool expect(token_type expected_token);
  void add_error(const std::string& error_message);
  void add_warning(const std::string& error_message);

  grammar_options options;
  lexer lex;
  logger& logging;

  // TODO(lmirelmann): Move outside the class the fields that are only needed when creating the AST.
  std::vector<int> nested_loops;
  bool recover = false;
  bool found_non_load = false;
  std::vector<std::set<std::string>> parse_parameter_identifiers;
  std::vector<std::pair<google::protobuf::Message*, std::set<std::string>>> parser_blocks;
};

}  // namespace grammar

#pragma GCC visibility pop

#endif  // GRAMMAR_PARSER_HPP_

