// Copyright 2024-2025 Lucas Mirelmann

#ifndef GRAMMAR_PARSER_HPP_
#define GRAMMAR_PARSER_HPP_

#include <functional>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "grammar/lexer.hpp"
#include "grammar/options.hpp"
#include "logging/logging.hpp"
#include "proto/starlark_ast.pb.h"

#pragma GCC visibility push(default)

namespace starlark {
namespace grammar {

struct parsing_block {
  std::set<std::string, std::less<>> identifiers;
  google::protobuf::RepeatedPtrField<std::string>* id_store;
  std::vector<std::tuple<starlark::ast::Identifier*, int, std::string>> to_resolve;
};

class parser {
 public:
  parser(std::string_view program_name, std::string_view input, starlark::logging::logger& logging);
  parser(std::string_view program_name, std::string_view input, const grammar_options& opts, const std::set<std::string, std::less<>>& bindings, starlark::logging::logger& logging);
  parser() = delete;
  parser(const parser&) = delete;
  parser(parser&&) = delete;
  starlark::ast::File* parse_file(google::protobuf::Arena& arena);

 private:
  void parse_statement(google::protobuf::RepeatedPtrField<starlark::ast::Statement>& statements);
  void bind_and_resolve(starlark::ast::Expression* expression);
  void bind(const starlark::ast::Identifier& identifier);
  bool set_identifier(starlark::ast::Identifier& id);
  bool capture(token_type expected_token);
  bool is_current(token_type expected_token) const;
  void add_error(std::string_view error_message);
  void add_error(std::string_view error_message, const starlark::logging::Position&);
  void add_warning(std::string_view error_message);
  void create_block(const std::set<std::string, std::less<>>& symbols,
                    const std::set<starlark::ast::Identifier*>& identifiers,
                    google::protobuf::RepeatedPtrField<std::string>* binding);
  void drop_block();
  void resolve(starlark::ast::Identifier* identifier, int base_frame);
  void resolve(starlark::ast::Expression* test, int base_frame);
  bool is_top_level_block() const;

  grammar_options opts;
  std::string_view program_name;
  std::string_view input;
  lexer lex;
  starlark::logging::logger& logging;
  std::set<std::string, std::less<>> base_bindings;

  std::vector<int> nested_loops;
  bool recover = false;
  bool found_non_load = false;
  std::vector<std::pair<std::set<std::string, std::less<>>, std::set<starlark::ast::Identifier*>>> parse_parameter_identifiers;
  std::vector<parsing_block> parser_blocks;
};

}  // namespace grammar
}  // namespace starlark

#pragma GCC visibility pop

#endif  // GRAMMAR_PARSER_HPP_

