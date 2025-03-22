// Copyright 2024-2025 Lucas Mirelmann

#ifndef GRAMMAR_PARSER_HPP_
#define GRAMMAR_PARSER_HPP_

#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "grammar/lexer.hpp"
#include "grammar/logging.hpp"
#include "grammar/options.hpp"
#include "grammar/proto/starlark.pb.h"

#pragma GCC visibility push(default)

namespace starlark {
namespace grammar {

struct parsing_block {
  std::set<std::string> identifiers;
  google::protobuf::RepeatedPtrField<std::string>* id_store;
  std::vector<std::pair<starlark::ast::Identifier*, int>> to_resolve;
};

class parser {
 public:
  parser(std::string_view input, logger& logging);
  parser(std::string_view input, const options& opts, const std::set<std::string>& bindings, logger& logging);
  parser() = delete;
  parser(const parser&) = delete;
  parser(parser&&) = delete;
  starlark::ast::File* parse_file(google::protobuf::Arena& arena);

  static constexpr std::string module = "Parser";

 private:
  void parse_statement(google::protobuf::RepeatedPtrField<starlark::ast::Statement>& statements);
  void bind_and_resolve(starlark::ast::Expression* expression);
  void bind(const starlark::ast::Identifier& identifier);
  bool set_identifier(starlark::ast::Identifier& id);
  bool capture(token_type expected_token);
  bool is_current(token_type expected_token) const;
  bool expect(token_type expected_token);
  void add_error(std::string_view error_message);
  void add_error(std::string_view error_message, position);
  void add_warning(std::string_view error_message);
  void create_block(const std::set<std::string>& symbols,
                    const std::set<starlark::ast::Identifier*>& identifiers,
                    google::protobuf::RepeatedPtrField<std::string>* binding);
  void drop_block();
  void resolve(starlark::ast::Identifier* identifier);
  void resolve(starlark::ast::Expression* test);
  bool is_top_level_block() const;

  options opts;
  lexer lex;
  logger& logging;
  std::set<std::string> base_bindings;

  std::vector<int> nested_loops;
  bool recover = false;
  bool found_non_load = false;
  std::vector<std::pair<std::set<std::string>, std::set<starlark::ast::Identifier*>>> parse_parameter_identifiers;
  std::vector<parsing_block> parser_blocks;
  std::map<starlark::ast::Identifier*, position> identifier_positions;
};

}  // namespace grammar
}  // namespace starlark

#pragma GCC visibility pop

#endif  // GRAMMAR_PARSER_HPP_

