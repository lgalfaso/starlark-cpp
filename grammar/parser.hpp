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
  starlark::Expression* expression = nullptr;
  starlark::PrimaryExpr* primary_expression = nullptr;
  starlark::Test* test = nullptr;
};

struct parsing_block {
  std::set<std::string> identifiers;
  google::protobuf::RepeatedPtrField<std::string>* id_store;
  std::vector<std::pair<starlark::Identifier*, int>> to_resolve;
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
  void bind_and_resolve(starlark::PrimaryExpr* primary_expression);
  void bind_and_resolve(starlark::Expression* expression);
  void bind_and_resolve(expression_frame frame);
  void bind(const starlark::Identifier& identifier);
  bool set_identifier(starlark::Identifier& id);
  bool capture(token_type expected_token);
  bool is_current(token_type expected_token) const;
  bool expect(token_type expected_token);
  void add_error(const std::string& error_message);
  void add_error(const std::string& error_message, position);
  void add_warning(const std::string& error_message);
  void create_block(const std::set<std::string>& symbols,
                    const std::set<starlark::Identifier*>& identifiers,
                    google::protobuf::RepeatedPtrField<std::string>* binding);
  void drop_block();
  void resolve(starlark::Identifier* identifier);
  void resolve(starlark::Test* test);
  void resolve(starlark::Expression* test);
  void resolve(expression_frame frame);
  bool is_top_level_block() const;

  grammar_options options;
  lexer lex;
  logger& logging;

  std::vector<int> nested_loops;
  bool recover = false;
  bool found_non_load = false;
  std::vector<std::pair<std::set<std::string>, std::set<starlark::Identifier*>>> parse_parameter_identifiers;
  std::vector<parsing_block> parser_blocks;
  std::map<starlark::Identifier*, position> identifier_positions;
};

}  // namespace grammar

#pragma GCC visibility pop

#endif  // GRAMMAR_PARSER_HPP_

