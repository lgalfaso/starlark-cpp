// Copyright 2024 Lucas Mirelmann

#include "grammar/parser.hpp"

#include <map>
#include <utility>
#include <vector>

#include "unicode/normalization.hpp"
#include "third-party/defer.hpp"

using google::protobuf::Arena;
using google::protobuf::RepeatedPtrField;
using starlark::Argument;
using starlark::AssignStmt;
using starlark::BinaryExpr;
using starlark::BinaryExpr;
using starlark::CompClause;
using starlark::DefStmt;
using starlark::Entry;
using starlark::Expression;
using starlark::File;
using starlark::ForStmt;
using starlark::Identifier;
using starlark::IfStmt;
using starlark::LambdaExpr;
using starlark::Parameter;
using starlark::PrimaryExpr;
using starlark::Statement;
using starlark::Test;
using starlark::UnaryExpr;
using unicode::to_nfkc;

namespace grammar {

namespace {

const std::map<token_type, std::pair<int, BinaryExpr::BinaryOperator>> operator_precedence = {
  {token_type::or_, {1, BinaryExpr::OR}},
  {token_type::and_, {2, BinaryExpr::AND}},
  {token_type::not_, {3, BinaryExpr::UNKNOWN}},  // As a prefix.
  {token_type::equals_equals, {4, BinaryExpr::EQUALS_EQUALS}},
  {token_type::not_equals, {4, BinaryExpr::BANG_EQUALS}},
  {token_type::less, {4, BinaryExpr::LESS_THAN}},
  {token_type::greater, {4, BinaryExpr::GREATER_THAN}},
  {token_type::less_equals, {4, BinaryExpr::LESS_THAN_EQUALS}},
  {token_type::greater_equals, {4, BinaryExpr::GREATER_THAN_EQUALS}},
  {token_type::in, {4, BinaryExpr::IN}},
  {token_type::pipe, {5, BinaryExpr::PIPE}},
  {token_type::caret, {6, BinaryExpr::HAT}},
  {token_type::ampersand, {7, BinaryExpr::AMPERSAND}},
  {token_type::less_less, {8, BinaryExpr::LESS_THAN_LESS_THAN}},
  {token_type::greater_greater, {8, BinaryExpr::GREATER_THAN_GREATER_THAN}},
  {token_type::minus, {9, BinaryExpr::MINUS}},
  {token_type::plus, {9, BinaryExpr::PLUS}},
  {token_type::star, {10, BinaryExpr::STAR}},
  {token_type::percent, {10, BinaryExpr::PERCENT}},
  {token_type::slash, {10, BinaryExpr::SLASH}},
  {token_type::slash_slash, {10, BinaryExpr::SLASH_SLASH}},
};

constexpr int MAX_PRECEDENCE = 11;

const std::map<token_type, AssignStmt::AssignOperator> assign_ops = {
  {token_type::equals, AssignStmt::EQUALS},
  {token_type::plus_equals, AssignStmt::PLUS_EQUALS},
  {token_type::minus_equals, AssignStmt::MINUS_EQUALS},
  {token_type::star_equals, AssignStmt::STAR_EQUALS},
  {token_type::slash_equals, AssignStmt::SLASH_EQUALS},
  {token_type::slash_slash_equals, AssignStmt::SLASH_SLASH_EQUALS},
  {token_type::percent_equals, AssignStmt::PERCENT_EQUALS},
  {token_type::ampersand_equals, AssignStmt::AMP_EQUALS},
  {token_type::pipe_equals, AssignStmt::PIPE_EQUALS},
  {token_type::caret_equals, AssignStmt::HAT_EQUALS},
  {token_type::less_less_equals, AssignStmt::LESS_LESS_EQUALS},
  {token_type::greater_greater_equals, AssignStmt::GREATER_GREATER_EQUALS},
};

struct target_frame {
  const starlark::Expression* expression = nullptr;
  const starlark::PrimaryExpr* primary_expression = nullptr;
  const starlark::Test* test = nullptr;
};

bool is_target(const target_frame& frame) {
  std::vector<target_frame> frames{frame};
  do {
    auto top = frames.back();
    frames.pop_back();
    if (top.primary_expression != nullptr) {
      switch (top.primary_expression->primary_expression_type_case()) {
        case PrimaryExpr::kDotExpression:
        case PrimaryExpr::kSliceExpression:
        case PrimaryExpr::kIdentifier:
          break;
        case PrimaryExpr::kCallExpression:
        case PrimaryExpr::kIntValue:
        case PrimaryExpr::kFloatValue:
        case PrimaryExpr::kStringValue:
        case PrimaryExpr::kBytesValue:
        case PrimaryExpr::kListComprehension:
        case PrimaryExpr::kDictionaryExpression:
        case PrimaryExpr::kDictionaryComprehension:
        case PrimaryExpr::PRIMARY_EXPRESSION_TYPE_NOT_SET:
        default:
          return false;
        case PrimaryExpr::kListExpression:
          for (const auto& item : top.primary_expression->list_expression().element()) {
            frames.push_back(target_frame{
              .test = &item,
            });
          }
          break;
        case PrimaryExpr::kExpression:
          frames.push_back(target_frame{
            .expression = &top.primary_expression->expression(),
          });
          break;
      }
    }
    if (top.expression != nullptr) {
      switch (top.expression->expression_type_case()) {
        case Expression::kValue:
        case Expression::EXPRESSION_TYPE_NOT_SET:
        default:
          frames.push_back(target_frame{
            .test = &top.expression->value(),
          });
          break;
        case Expression::kTuple:
          for (const auto& element : top.expression->tuple().value()) {
            frames.push_back(target_frame{
              .test = &element,
            });
          }
          break;
      }
    }
    if (top.test != nullptr) {
      if (!top.test->has_primary_expression()) {
        return false;
      }
      frames.push_back(target_frame{
        .primary_expression = &top.test->primary_expression(),
      });
    }
  } while (!frames.empty());
  return true;
}

bool is_target(const PrimaryExpr& primary_expression) {
  return is_target(target_frame{
    .primary_expression = &primary_expression,
  });
}

bool is_target(const Expression& expression) {
  return is_target(target_frame{
    .expression = &expression,
  });
}

enum class parser_state {
  parse_statement,
  parse_statement_def_0,
  parse_statement_def_final,
  parse_statement_if_0,
  parse_statement_if_elif,
  parse_statement_if_else,
  parse_statement_for_0,
  parse_statement_for_1,
  parse_statement_for_2,
  parse_statement_for_final,
  parse_statement_expression_0,
  parse_suite,
  parse_suite_statement_list,
  parse_simple_statement,
  parse_simple_statement_0,
  parse_simple_statement_final,
  parse_small_statement,
  parse_parameters,
  parse_expression,
  parse_expression_0,
  parse_expression_1,
  parse_test,
  parse_test_0,
  parse_test_1,
  parse_test_p,
  parse_test_p_0,
  parse_lambda,
  parse_lambda_0,
  parse_lambda_final,
  parse_primary,
  parse_primary_0,
  parse_primary_call_0,
  parse_primary_index_0,
  parse_primary_index_1,
  parse_primary_index_2,
  parse_primary_index_final,
  parse_operand,
  parse_operand_expression_0,
  parse_list,
  parse_list_0,
  parse_list_index_0,
  parse_list_comprehension_final,
  parse_list_final,
  parse_dict,
  parse_dict_0,
  parse_dict_index_0,
  parse_dict_comprehension_final,
  parse_dict_final,
  parse_entry,
  parse_entry_0,
  parse_comp_clauses,
  parse_comp_clauses_0,
  parse_argument,
  parse_argument_0,
  parse_resolve_test,
  parse_resolve_expression,
};

struct frame {
  parser_state state;
  union {
    RepeatedPtrField<Statement>* statements;
    Statement* statement;
    DefStmt* def_statement;
    IfStmt* if_statement;
    struct {
      ForStmt* for_statement;
      PrimaryExpr* for_loop_variable;
    };
    LambdaExpr* lambda;
    struct {
      Argument* argument;
      Argument* previous_argument;
    };
    Entry* entry;
    RepeatedPtrField<CompClause>* comp_clauses;
    struct {
      CompClause* comp_clause;
      PrimaryExpr* comp_clause_primary;
    };
    struct {
      RepeatedPtrField<Parameter>* parameters;
      bool parse_parameters_allow_trailing_comma;
      bool parse_parameters_first;
      bool found_star_parameter;
      bool found_star_star_parameter;
      bool previous_parameter_was_bare_star;
    };
    struct {
      Expression* expression;
      bool expression_allow_trailing_comma;
    };
    struct {
      Test* test;
      int test_p_precedence;
      bool test_p_0_first;
    };
    struct {
      PrimaryExpr* primary = nullptr;
      Argument* previous_call_argument = nullptr;
      bool primary_must_be_target = false;
    };
  };
};

const std::set<std::string> predeclared_symbols = {
    "None",     "True",      "False",    "abs",     "any",       "all",
    "bool",     "bytes",     "dict",     "dir",     "enumerate", "float",
    "fail",     "getattr",   "hasattr",  "hash",    "int",       "len",
    "list",     "max",       "min",      "print",   "range",     "repr",
    "reversed", "set",       "sorted",   "str",     "tuple",     "type",
    "zip",
};

}  // namespace

parser::parser(std::string_view input, logger& logging) : parser(input, grammar_options{}, logging) {
}

parser::parser(std::string_view input, const grammar_options& options, logger& logging)
    : options(options), lex(input, options, logging), logging(logging), nested_loops(1) {
  lex.next_token();
}

File* parser::parse_file(Arena& arena) {
  // TODO(lmirelmann): Allow a mechanism to define custom bindings.
  File* result = Arena::Create<File>(&arena);

  // Predeclared block.
  create_block(predeclared_symbols, {}, nullptr);
  // The "file block" and "module block" are defined the other way around
  // than the spec. Given that there is no overlap between these two, this should
  // not have any side-effects.
  // Context: https://github.com/bazelbuild/starlark/issues/293
  // File block.
  create_block({}, {}, result->mutable_file_binding());
  // Module block.
  create_block({}, {}, result->mutable_module_binding());

  while (lex.current_token().type() != token_type::eof) {
    if (lex.current_token().type() == token_type::newline) {
      lex.next_token();
    } else {
      parse_statement(*result->mutable_statement());
    }
  }
  assert(parse_parameter_identifiers.empty());
  assert(nested_loops.empty());
  assert(is_top_level_block());

  // Force the resolution of the predefined, module and file blocks.
  while (!parser_blocks.empty()) {
    drop_block();
  }
  identifier_positions.clear();
  return result;
}

bool parser::capture(token_type expected_token) {
  if (!is_current(expected_token)) {
    return false;
  }
  lex.next_token();
  return true;
}

bool parser::is_current(token_type expected_token) const {
  return lex.current_token().type() == expected_token;
}

bool parser::expect(token_type expected_token) {
  if (capture(expected_token)) {
    return true;
  }
  switch (expected_token) {
    case token_type::ampersand:              add_error("Expected AMPERSAND");              break;
    case token_type::ampersand_equals:       add_error("Expected AMPERSAND_EQUALS");       break;
    case token_type::and_:                   add_error("Expected AND");                    break;
    case token_type::as:                     add_error("Expected AS");                     break;
    case token_type::assert:                 add_error("Expected ASSERT");                 break;
    case token_type::async:                  add_error("Expected ASYNC");                  break;
    case token_type::await:                  add_error("Expected AWAIT");                  break;
    case token_type::bof:                    add_error("Expected BOF");                    break;
    case token_type::break_:                 add_error("Expected BREAK");                  break;
    case token_type::bytes:                  add_error("Expected BYTES");                  break;
    case token_type::caret:                  add_error("Expected CARET");                  break;
    case token_type::caret_equals:           add_error("Expected CARET_EQUALS");           break;
    case token_type::class_:                 add_error("Expected CLASS");                  break;
    case token_type::colon:                  add_error("Expected COLON");                  break;
    case token_type::comma:                  add_error("Expected COMMA");                  break;
    case token_type::continue_:              add_error("Expected CONTINUE");               break;
    case token_type::def:                    add_error("Expected DEF");                    break;
    case token_type::del:                    add_error("Expected DEL");                    break;
    case token_type::dot:                    add_error("Expected DOT");                    break;
    case token_type::elif:                   add_error("Expected ELIF");                   break;
    case token_type::else_:                  add_error("Expected ELSE");                   break;
    case token_type::eof:                    add_error("Expected EOF");                    break;
    case token_type::equals:                 add_error("Expected EQUALS");                 break;
    case token_type::equals_equals:          add_error("Expected EQUALS_EQUALS");          break;
    case token_type::except:                 add_error("Expected EXCEPT");                 break;
    case token_type::finally:                add_error("Expected FINALLY");                break;
    case token_type::float_:                 add_error("Expected FLOAT");                  break;
    case token_type::for_:                   add_error("Expected FOR");                    break;
    case token_type::from:                   add_error("Expected FROM");                   break;
    case token_type::global:                 add_error("Expected GLOBAL");                 break;
    case token_type::greater:                add_error("Expected GREATER");                break;
    case token_type::greater_equals:         add_error("Expected GREATER_EQUALS");         break;
    case token_type::greater_greater:        add_error("Expected GREATER_GREATER");        break;
    case token_type::greater_greater_equals: add_error("Expected GREATER_GREATER_EQUALS"); break;
    case token_type::identifier:             add_error("Expected IDENTIFIER");             break;
    case token_type::if_:                    add_error("Expected IF");                     break;
    case token_type::illegal:                add_error("Expected ILLEGAL");                break;
    case token_type::import:                 add_error("Expected IMPORT");                 break;
    case token_type::in:                     add_error("Expected IN");                     break;
    case token_type::indent:                 add_error("Expected INDENT");                 break;
    case token_type::int_:                   add_error("Expected INT");                    break;
    case token_type::is:                     add_error("Expected IS");                     break;
    case token_type::lambda:                 add_error("Expected LAMBDA");                 break;
    case token_type::lbrace:                 add_error("Expected LBRACE");                 break;
    case token_type::lbracket:               add_error("Expected LBRACKET");               break;
    case token_type::less:                   add_error("Expected LESS");                   break;
    case token_type::less_equals:            add_error("Expected LESS_EQUALS");            break;
    case token_type::less_less:              add_error("Expected LESS_LESS");              break;
    case token_type::less_less_equals:       add_error("Expected LESS_LESS_EQUALS");       break;
    case token_type::load:                   add_error("Expected LOAD");                   break;
    case token_type::lparen:                 add_error("Expected LPAREN");                 break;
    case token_type::minus:                  add_error("Expected MINUS");                  break;
    case token_type::minus_equals:           add_error("Expected MINUS_EQUALS");           break;
    case token_type::newline:                add_error("Expected NEWLINE");                break;
    case token_type::nonlocal:               add_error("Expected NONLOCAL");               break;
    case token_type::not_:                   add_error("Expected NOT");                    break;
    case token_type::not_equals:             add_error("Expected NOT_EQUALS");             break;
    case token_type::or_:                    add_error("Expected OR");                     break;
    case token_type::outdent:                add_error("Expected OUTDENT");                break;
    case token_type::pass:                   add_error("Expected PASS");                   break;
    case token_type::percent:                add_error("Expected PERCENT");                break;
    case token_type::percent_equals:         add_error("Expected PERCENT_EQUALS");         break;
    case token_type::pipe:                   add_error("Expected PIPE");                   break;
    case token_type::pipe_equals:            add_error("Expected PIPE_EQUALS");            break;
    case token_type::plus:                   add_error("Expected PLUS");                   break;
    case token_type::plus_equals:            add_error("Expected PLUS_EQUALS");            break;
    case token_type::raise:                  add_error("Expected RAISE");                  break;
    case token_type::rbrace:                 add_error("Expected RBRACE");                 break;
    case token_type::rbracket:               add_error("Expected RBRACKET");               break;
    case token_type::return_:                add_error("Expected RETURN");                 break;
    case token_type::rparen:                 add_error("Expected RPAREN");                 break;
    case token_type::semi:                   add_error("Expected SEMI");                   break;
    case token_type::slash:                  add_error("Expected SLASH");                  break;
    case token_type::slash_equals:           add_error("Expected SLASH_EQUALS");           break;
    case token_type::slash_slash:            add_error("Expected SLASH_SLASH");            break;
    case token_type::slash_slash_equals:     add_error("Expected SLASH_SLASH_EQUALS");     break;
    case token_type::star:                   add_error("Expected STAR");                   break;
    case token_type::star_equals:            add_error("Expected STAR_EQUALS");            break;
    case token_type::star_star:              add_error("Expected STAR_STAR");              break;
    case token_type::string:                 add_error("Expected STRING");                 break;
    case token_type::tilde:                  add_error("Expected TILDE");                  break;
    case token_type::try_:                   add_error("Expected TRY");                    break;
    case token_type::while_:                 add_error("Expected WHILE");                  break;
    case token_type::with:                   add_error("Expected WITH");                   break;
    case token_type::yield:                  add_error("Expected YIELD");                  break;
  }
  return false;
}

void parser::add_error(const std::string& message) {
  add_error(message, lex.current_token().start());
}

void parser::add_error(const std::string& message, position pos) {
  logging.log(log_level::ERROR, message, module, pos);
  recover = true;
}

void parser::add_warning(const std::string& message) {
  logging.log(log_level::WARNING, message, module, lex.current_token().start());
  recover = true;
}

void parser::parse_statement(RepeatedPtrField<Statement>& statements) {
  std::vector<frame> frames;
  frames.emplace_back(frame{
      .state = parser_state::parse_statement,
      .statements = &statements,
  });

  do {
    const frame top = frames.back();
    frames.pop_back();
    switch (top.state) {
      case parser_state::parse_statement:
        if (capture(token_type::def)) {
          found_non_load = true;
          if (!options.allow_function_definitions) {
            add_error("Function definitions not allowed");
          }
          DefStmt* def_statement = top.statements->Add()->mutable_def_statement();
          if (!set_identifier(*def_statement->mutable_function_name())) {
            add_error("Expected an identifier");
            break;
          }

          // If this is a top-level function definition, then check whether this is causing a redefinition
          // with a previous `load` statement.
          if (is_top_level_block()) {
            if (parser_blocks[1].identifiers.contains(def_statement->function_name().nfkc_name())) {
              add_error("`def` statement redefines previously defined `load` symbol '" + def_statement->function_name().name() + "'");
            }
          }
          bind(def_statement->function_name());
          resolve(def_statement->mutable_function_name());

          if (!expect(token_type::lparen)) {
            break;
          }
          nested_loops.push_back(0);
          frames.emplace_back(frame{
            .state = parser_state::parse_statement_def_final,
            .def_statement = def_statement,
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_statement_def_0,
            .def_statement = def_statement,
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_parameters,
            .parameters = def_statement->mutable_parameter(),
            .parse_parameters_allow_trailing_comma = true,
            .parse_parameters_first = true,
            .found_star_parameter = false,
            .found_star_star_parameter = false,
            .previous_parameter_was_bare_star = false,
          });
        } else if (capture(token_type::if_)) {
          found_non_load = true;
          if (!options.allow_top_level_if_and_for && nested_loops.size() == 1) {
            add_error("`if` statements are not allowed at the top level");
          }
          IfStmt* if_statement = top.statements->Add()->mutable_if_statement();
          // It is unclear whether the attempt to parse the `elif` and `else` blocks should be
          // defined here or in parse_statement_if_0. This difference is important when there
          // are errors in the parsing and how should we attempt to recover from these errors.
          frames.emplace_back(frame{
            .state = parser_state::parse_statement_if_else,
            .if_statement = if_statement,
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_statement_if_elif,
            .if_statement = if_statement,
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_statement_if_0,
            .statements = if_statement->mutable_statement(),
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_resolve_test,
            .test = if_statement->mutable_test(),
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_test,
            .test = if_statement->mutable_test(),
          });
        } else if (capture(token_type::for_)) {
          found_non_load = true;
          if (!options.allow_top_level_if_and_for && nested_loops.size() == 1) {
            add_error("`for` statements are not allowed at the top level");
          }
          ForStmt* for_statement = top.statements->Add()->mutable_for_statement();
          PrimaryExpr* loop_variable = for_statement->add_loop_variable();
          nested_loops.back()++;
          frames.emplace_back(frame{
            .state = parser_state::parse_statement_for_final,
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_statement_for_2,
            .statements = for_statement->mutable_statement(),
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_statement_for_1,
            .for_statement = for_statement,
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_statement_for_0,
            .for_statement = for_statement,
            .for_loop_variable = loop_variable,
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_primary,
            .primary = loop_variable,
            .primary_must_be_target = true,
          });
        } else {
          frames.emplace_back(frame{
            .state = parser_state::parse_simple_statement,
            .statements = top.statements,
          });
        }
        break;
      case parser_state::parse_statement_def_0:
        create_block(parse_parameter_identifiers.back().first, parse_parameter_identifiers.back().second, top.def_statement->mutable_function_binding());
        parse_parameter_identifiers.pop_back();
        if (!expect(token_type::rparen)) {
          break;
        }
        if (!expect(token_type::colon)) {
          break;
        }
        frames.emplace_back(frame{
          .state = parser_state::parse_suite,
          .statements = top.def_statement->mutable_statement(),
        });
        break;
      case parser_state::parse_statement_def_final:
        drop_block();
        nested_loops.pop_back();
        break;
      case parser_state::parse_statement_if_0:
        if (!expect(token_type::colon)) {
          break;
        }
        frames.emplace_back(frame{
          .state = parser_state::parse_suite,
          .statements = top.statements,
        });
        break;
      case parser_state::parse_statement_if_elif:
        if (capture(token_type::elif)) {
          auto* elif = top.if_statement->add_elif();
          frames.emplace_back(top);
          frames.emplace_back(frame{
            .state = parser_state::parse_statement_if_0,
            .statements = elif->mutable_statement(),
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_resolve_test,
            .test = elif->mutable_test(),
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_test,
            .test = elif->mutable_test(),
          });
        }
        break;
      case parser_state::parse_statement_if_else:
        if (capture(token_type::else_)) {
          frames.emplace_back(frame{
            .state = parser_state::parse_statement_if_0,
            .statements = top.if_statement->mutable_else_statement(),
          });
        }
        break;
      case parser_state::parse_statement_for_0:
        bind_and_resolve(top.for_loop_variable);
        if (capture(token_type::comma)) {
          PrimaryExpr* loop_variable = top.for_statement->add_loop_variable();
          frames.emplace_back(frame{
            .state = parser_state::parse_statement_for_0,
            .for_statement = top.for_statement,
            .for_loop_variable = loop_variable,
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_primary,
            .primary = loop_variable,
            .primary_must_be_target = true,
          });
        }
        break;
      case parser_state::parse_statement_for_1:
        if (!expect(token_type::in)) {
          break;
        }
        frames.emplace_back(frame{
          .state = parser_state::parse_resolve_expression,
          .expression = top.for_statement->mutable_expression(),
        });
        frames.emplace_back(frame{
          .state = parser_state::parse_expression,
          .expression = top.for_statement->mutable_expression(),
          .expression_allow_trailing_comma = false,
        });
        break;
      case parser_state::parse_statement_for_2:
        if (!expect(token_type::colon)) {
          break;
        }
        frames.emplace_back(frame{
          .state = parser_state::parse_suite,
          .statements = top.statements,
        });
        break;
      case parser_state::parse_statement_for_final:
        nested_loops.back()--;
        break;
      case parser_state::parse_suite:
        if (capture(token_type::newline)) {
          if (!expect(token_type::indent)) {
            break;
          }
          frames.emplace_back(frame{
            .state = parser_state::parse_suite_statement_list,
            .statements = top.statements,
          });
        } else {
          frames.emplace_back(frame{
            .state = parser_state::parse_simple_statement,
            .statements = top.statements,
          });
        }
        break;
      case parser_state::parse_suite_statement_list:
        if (lex.current_token().type() != token_type::outdent && lex.current_token().type() != token_type::eof) {
          frames.emplace_back(top);
          frames.emplace_back(frame{
            .state = parser_state::parse_statement,
            .statements = top.statements,
          });
          break;
        }
        expect(token_type::outdent);
        break;
      case parser_state::parse_simple_statement:
        frames.emplace_back(frame{
          .state = parser_state::parse_simple_statement_final,
        });
        frames.emplace_back(frame{
          .state = parser_state::parse_simple_statement_0,
          .statements = top.statements,
        });
        frames.emplace_back(frame{
          .state = parser_state::parse_small_statement,
          .statement = top.statements->Add(),
        });
        break;
      case parser_state::parse_simple_statement_0:
        if (capture(token_type::semi)) {
          if (is_current(token_type::newline)) {
            break;
          }
          frames.emplace_back(top);
          frames.emplace_back(frame{
            .state = parser_state::parse_small_statement,
             .statement = top.statements->Add(),
          });
        }
        break;
      case parser_state::parse_simple_statement_final:
        if (recover) {
          while (lex.current_token().type() != token_type::newline && lex.current_token().type() != token_type::eof) {
            lex.next_token();
          }
        }
        if (!expect(token_type::newline)) {
          break;
        }
        recover = false;
        break;
      case parser_state::parse_small_statement:
        switch (lex.current_token().type()) {
          case token_type::return_:
            found_non_load = true;
            if (nested_loops.size() == 1) {
              add_error("Unexpected RETURN");
            }
            top.statement->mutable_return_statement();
            lex.next_token();
            if (!is_current(token_type::newline)) {
              frames.emplace_back(frame{
                .state = parser_state::parse_resolve_expression,
                .expression = top.statement->mutable_return_statement()->mutable_expression(),
              });
              frames.emplace_back(frame{
                .state = parser_state::parse_expression,
                .expression = top.statement->mutable_return_statement()->mutable_expression(),
                .expression_allow_trailing_comma = false,
              });
            }
            break;
          case token_type::load: {
            std::set<std::string> symbols;
            if (found_non_load && options.require_load_statements_first) {
              add_error("`load` statements must appear before other statements");
            }
            if (nested_loops.size() != 1) {
              add_error("`load` statement not at top level");
            }
            lex.next_token();
            if (!expect(token_type::lparen)) {
              break;
            }
            if (!is_current(token_type::string)) {
              add_error("Expected STRING");
              break;
            }
            top.statement->mutable_load_statement()->set_module(lex.current_token().string_value());
            lex.next_token();
            bool loaded_symbol = false;
            while (capture(token_type::comma)) {
              if (is_current(token_type::rparen)) {
                break;
              }
              loaded_symbol = true;
              auto* load_param = top.statement->mutable_load_statement()->add_load_param();
              if (is_current(token_type::identifier)) {
                set_identifier(*load_param->mutable_local_name());
                if (!expect(token_type::equals)) {
                  break;
                }
              }
              if (!is_current(token_type::string)) {
                add_error("Expected STRING");
                break;
              }
              if (!options.allow_load_private_symbols &&
                  lex.current_token().string_value().starts_with("_")) {
                add_error(std::string{"Cannot import private symbol '"} + lex.current_token().string_value() + "'");
              }
              load_param->set_remote_name(lex.current_token().string_value());
              if (!load_param->has_local_name()) {
                load_param->mutable_local_name()->set_name(load_param->remote_name());
                load_param->mutable_local_name()->set_nfkc_name(to_nfkc(load_param->remote_name()));
              }
              resolve(load_param->mutable_local_name());
              if (!symbols.insert(load_param->local_name().nfkc_name()).second) {
                add_error("`load` statement defines '" + load_param->local_name().name() + "' more than once");
              }
              if (parser_blocks.back().identifiers.contains(load_param->local_name().nfkc_name())) {
                add_error("`load` statement redefines previously defined value '" + load_param->local_name().name() + "'");
              }
              // The spec does not specify whether it is an error to bind to the file block multiple times.
              // We are taking the possition that if `allow_top_level_rebinding` is `false`, then this is not allowed.
              if (!parser_blocks[1].identifiers.insert(load_param->local_name().nfkc_name()).second && !options.allow_top_level_rebinding) {
                add_error("Multiple bindings for the top-level load symbol '" + load_param->local_name().name() + "'");
              }
              lex.next_token();
            }
            if (!loaded_symbol) {
              add_error("Expect to load at least one symbol");
            }
            expect(token_type::rparen);
            break;
          }
          case token_type::break_:
            found_non_load = true;
            if (nested_loops.back() == 0) {
              add_error("Unexpected BREAK");
            }
            top.statement->mutable_break_statement();
            lex.next_token();
            break;
          case token_type::continue_:
            found_non_load = true;
            if (nested_loops.back() == 0) {
              add_error("Unexpected CONTINUE");
            }
            top.statement->mutable_continue_statement();
            lex.next_token();
            break;
          case token_type::pass:
            found_non_load = true;
            top.statement->mutable_pass_statement();
            lex.next_token();
            break;
          default: {
            auto* statement = top.statement;
            frames.emplace_back(frame{
              .state = parser_state::parse_statement_expression_0,
              .statement = statement,
            });
            frames.emplace_back(frame{
              .state = parser_state::parse_expression,
              .expression = statement->mutable_expression_statement(),
              .expression_allow_trailing_comma = false,
            });
            break;
          }
        }
        break;
      case parser_state::parse_statement_expression_0:
        if (auto op = assign_ops.find(lex.current_token().type()); op != assign_ops.end()) {
          found_non_load = true;
          if (!is_target(top.statement->expression_statement())) {
            // Report the error and continue to parse this as an expression
            add_error("Exprecting TARGET");
          }
          if (op->first != token_type::equals && (
              top.statement->expression_statement().has_tuple() ||
              top.statement->expression_statement().value().primary_expression().has_list_expression() ||
              top.statement->expression_statement().value().primary_expression().has_list_comprehension() ||
              top.statement->expression_statement().value().primary_expression().has_dictionary_expression() ||
              top.statement->expression_statement().value().primary_expression().has_dictionary_comprehension())) {
            add_error("target is an illegal expression for augmented assignment");
          }
          {
            AssignStmt* assign_statement = Arena::Create<AssignStmt>(top.statement->GetArena());
            assign_statement->mutable_lhs()->Swap(top.statement->mutable_expression_statement());
            assign_statement->Swap(top.statement->mutable_assign_statement());
          }
          // Assignements and augmented assignment are considered a binding.
          bind_and_resolve(top.statement->mutable_assign_statement()->mutable_lhs());
          top.statement->mutable_assign_statement()->set_op(op->second);
          lex.next_token();
          frames.emplace_back(frame{
            .state = parser_state::parse_resolve_expression,
            .expression = top.statement->mutable_assign_statement()->mutable_rhs(),
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_expression,
            .expression = top.statement->mutable_assign_statement()->mutable_rhs(),
            .expression_allow_trailing_comma = false,
          });
        } else {
          resolve(top.statement->mutable_expression_statement());
        }
        break;
      case parser_state::parse_expression:
        frames.emplace_back(frame{
          .state = parser_state::parse_expression_0,
          .expression = top.expression,
          .expression_allow_trailing_comma = top.expression_allow_trailing_comma,
        });
        frames.emplace_back(frame{
          .state = parser_state::parse_test,
          .test = top.expression->mutable_value(),
        });
        break;
      case parser_state::parse_expression_0:
        if (is_current(token_type::comma)) {
          found_non_load = true;
          {
            Test* first_test = Arena::Create<Test>(top.expression->GetArena());
            first_test->Swap(top.expression->mutable_value());
            first_test->Swap(top.expression->mutable_tuple()->add_value());
          }
          frames.emplace_back(frame{
            .state = parser_state::parse_expression_1,
            .expression = top.expression,
            .expression_allow_trailing_comma = top.expression_allow_trailing_comma,
          });
        } else {
          if (top.expression->value().primary_expression().has_expression()) {
            Test* first_test = Arena::Create<Test>(top.expression->GetArena());
            first_test->Swap(top.expression->mutable_value());
            first_test->mutable_primary_expression()->mutable_expression()->Swap(top.expression);
          }
        }
        break;
      case parser_state::parse_expression_1:
        if (capture(token_type::comma)) {
          if (is_current(token_type::newline) ||
              is_current(token_type::equals) ||
              is_current(token_type::rbrace) ||
              is_current(token_type::rbracket) ||
              is_current(token_type::rparen) ||
              is_current(token_type::semi)) {
            if (!top.expression_allow_trailing_comma) {
              add_error("Unexpected COMMA");
            }
            break;
          }
          frames.emplace_back(top);
          frames.emplace_back(frame{
            .state = parser_state::parse_test,
            .test = top.expression->mutable_tuple()->add_value(),
          });
        }
        break;
      case parser_state::parse_test:
        if (is_current(token_type::lambda)) {
          found_non_load = true;
          frames.emplace_back(frame{
            .state = parser_state::parse_lambda,
            .lambda = top.test->mutable_lambda_expression(),
          });
        } else {
          frames.emplace_back(frame{
            .state = parser_state::parse_test_0,
            .test = top.test,
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_test_p,
            .test = top.test,
            .test_p_precedence = 0,
          });
        }
        break;
      case parser_state::parse_test_0:
        if (capture(token_type::if_)) {
          found_non_load = true;
          {
            Test* new_result = Arena::Create<Test>(top.test->GetArena());
            new_result->mutable_if_expression()->mutable_if_value()->Swap(top.test);
            new_result->Swap(top.test);
          }
          frames.emplace_back(frame{
            .state = parser_state::parse_test_1,
            .test = top.test,
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_test_p,
            .test = top.test->mutable_if_expression()->mutable_if_test(),
            .test_p_precedence = 0,
          });
        }
        break;
      case parser_state::parse_test_1:
        if (!expect(token_type::else_)) {
          break;
        }
        frames.emplace_back(frame{
          .state = parser_state::parse_test,
          .test = top.test->mutable_if_expression()->mutable_else_value(),
        });
        break;
      case parser_state::parse_test_p:
        if (top.test_p_precedence >= MAX_PRECEDENCE) {
          Test* result_ref = top.test;
          for (;;) {
            if (capture(token_type::plus)) {
              result_ref->mutable_unary_expression()->set_operator_(UnaryExpr::PLUS);
            } else if (capture(token_type::minus)) {
              result_ref->mutable_unary_expression()->set_operator_(UnaryExpr::MINUS);
            } else if (capture(token_type::tilde)) {
              result_ref->mutable_unary_expression()->set_operator_(UnaryExpr::TILDE);
            } else {
              break;
            }
            found_non_load = true;
            result_ref = result_ref->mutable_unary_expression()->mutable_test();
          }
          frames.emplace_back(frame{
            .state = parser_state::parse_primary,
            .primary = result_ref->mutable_primary_expression(),
            .primary_must_be_target = false,
          });
          break;
        }
        if (top.test_p_precedence == operator_precedence.at(token_type::not_).first) {
          Test* result_ref = top.test;
          for (;;) {
            if (!capture(token_type::not_)) {
              break;
            }
            found_non_load = true;
            result_ref->mutable_unary_expression()->set_operator_(UnaryExpr::NOT);
            result_ref = result_ref->mutable_unary_expression()->mutable_test();
          }
          frames.emplace_back(frame{
            .state = parser_state::parse_test_p,
            .test = result_ref,
            .test_p_precedence = top.test_p_precedence + 1,
          });
          break;
        }
        frames.emplace_back(frame{
          .state = parser_state::parse_test_p_0,
          .test = top.test,
          .test_p_precedence = top.test_p_precedence,
          .test_p_0_first = true,
        });
        frames.emplace_back(frame{
          .state = parser_state::parse_test_p,
          .test = top.test,
          .test_p_precedence = top.test_p_precedence + 1,
        });
        break;
      case parser_state::parse_test_p_0:
        if (is_current(token_type::not_)) {
          if (top.test_p_precedence != operator_precedence.at(token_type::in).first) {
            break;
          }
          if (!top.test_p_0_first) {
            add_error("Comparison operators are not associative. Use parens.");
          }
          found_non_load = true;
          lex.next_token();
          if (!expect(token_type::in)) {
            break;
          }
          {
            Test* new_result = Arena::Create<Test>(top.test->GetArena());
            new_result->mutable_binary_expression()->mutable_lhs()->Swap(top.test);
            new_result->Swap(top.test);
          }
          top.test->mutable_binary_expression()->set_operator_(BinaryExpr::NOT_IN);
          frames.emplace_back(frame{
            .state = parser_state::parse_test_p_0,
            .test = top.test,
            .test_p_precedence = top.test_p_precedence,
            .test_p_0_first = false,
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_test_p,
            .test = top.test->mutable_binary_expression()->mutable_rhs(),
            .test_p_precedence = top.test_p_precedence + 1,
          });
        } else if (auto next_op = operator_precedence.find(lex.current_token().type()); next_op != operator_precedence.end()) {
          if (top.test_p_precedence != next_op->second.first) {
            break;
          }
          if (!top.test_p_0_first && top.test_p_precedence == operator_precedence.at(token_type::equals_equals).first) {
            add_error("Comparison operators are not associative. Use parens.");
          }
          found_non_load = true;
          lex.next_token();
          {
            Test* new_result = Arena::Create<Test>(top.test->GetArena());
            new_result->mutable_binary_expression()->mutable_lhs()->Swap(top.test);
            new_result->Swap(top.test);
          }
          top.test->mutable_binary_expression()->set_operator_(next_op->second.second);
          frames.emplace_back(frame{
            .state = parser_state::parse_test_p_0,
            .test = top.test,
            .test_p_precedence = top.test_p_precedence,
            .test_p_0_first = false,
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_test_p,
            .test = top.test->mutable_binary_expression()->mutable_rhs(),
            .test_p_precedence = top.test_p_precedence + 1,
          });
        }
        break;
      case parser_state::parse_primary:
        frames.emplace_back(frame{
          .state = parser_state::parse_primary_0,
          .primary = top.primary,
          .primary_must_be_target = top.primary_must_be_target,
        });
        frames.emplace_back(frame{
          .state = parser_state::parse_operand,
          .primary = top.primary,
        });
        break;
      case parser_state::parse_primary_0:
        if (top.primary->expression().value().has_primary_expression()) {
          PrimaryExpr* new_result = Arena::Create<PrimaryExpr>(top.primary->GetArena());
          new_result->Swap(top.primary->mutable_expression()->mutable_value()->mutable_primary_expression());
          top.primary->Swap(new_result);
        }
        if (capture(token_type::dot)) {
          found_non_load = true;
          {
            PrimaryExpr* new_result = Arena::Create<PrimaryExpr>(top.primary->GetArena());
            new_result->mutable_dot_expression()->mutable_primary_expression()->Swap(top.primary);
            new_result->Swap(top.primary);
          }
          if (!set_identifier(*top.primary->mutable_dot_expression()->mutable_identifier())) {
            add_error("Expecting IDENTIFIER");
            break;
          }
          frames.emplace_back(top);
        } else if (capture(token_type::lparen)) {
          found_non_load = true;
          {
            PrimaryExpr* new_result = Arena::Create<PrimaryExpr>(top.primary->GetArena());
            new_result->mutable_call_expression()->mutable_primary_expression()->Swap(top.primary);
            new_result->Swap(top.primary);
          }
          frames.emplace_back(top);
          if (capture(token_type::rparen)) {
            break;
          }
          auto* new_argument = top.primary->mutable_call_expression()->add_argument();
          frames.emplace_back(frame{
            .state = parser_state::parse_primary_call_0,
            .primary = top.primary,
            .primary_must_be_target = top.primary_must_be_target,
            .previous_call_argument = new_argument,
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_argument,
            .argument = new_argument,
            .previous_argument = nullptr,
          });
        } else if (capture(token_type::lbracket)) {
          found_non_load = true;
          {
            PrimaryExpr* new_result = Arena::Create<PrimaryExpr>(top.primary->GetArena());
            new_result->mutable_slice_expression()->mutable_primary_expression()->Swap(top.primary);
            new_result->Swap(top.primary);
          }
          frames.emplace_back(top);
          frames.emplace_back(frame{
            .state = parser_state::parse_primary_index_final,
          });
          if (capture(token_type::colon)) {
            top.primary->mutable_slice_expression()->mutable_slice();
            frames.emplace_back(frame{
              .state = parser_state::parse_primary_index_1,
              .primary = top.primary,
              .primary_must_be_target = top.primary_must_be_target,
            });
          } else {
            frames.emplace_back(frame{
              .state = parser_state::parse_primary_index_0,
              .primary = top.primary,
              .primary_must_be_target = top.primary_must_be_target,
            });
            frames.emplace_back(frame{
              .state = parser_state::parse_expression,
              .expression = top.primary->mutable_slice_expression()->mutable_index(),
              .expression_allow_trailing_comma = true,
            });
          }
        } else {
          if (top.primary_must_be_target && !is_target(*top.primary)) {
            add_error("Expecting a TARGET");
          }
        }
        break;
      case parser_state::parse_primary_call_0:
        if (capture(token_type::comma)) {
          if (capture(token_type::rparen)) {
            break;
          }
          auto* new_argument = top.primary->mutable_call_expression()->add_argument();
          frames.emplace_back(frame{
            .state = parser_state::parse_primary_call_0,
            .primary = top.primary,
            .primary_must_be_target = top.primary_must_be_target,
            .previous_call_argument = new_argument,
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_argument,
            .argument = new_argument,
            .previous_argument = top.previous_call_argument,
          });
        } else {
          expect(token_type::rparen);
        }
        break;
      case parser_state::parse_primary_index_0:
        if (capture(token_type::colon)) {
          if (!top.primary->slice_expression().index().has_value()) {
            add_error("Unexpected TUPLE");
          }
          {
            Expression* expression = Arena::Create<Expression>(top.primary->GetArena());
            expression->Swap(top.primary->mutable_slice_expression()->mutable_index());
            top.primary->mutable_slice_expression()->mutable_slice()->mutable_start()->Swap(expression->mutable_value());
          }
          frames.emplace_back(frame{
            .state = parser_state::parse_primary_index_1,
            .primary = top.primary,
            .primary_must_be_target = top.primary_must_be_target,
          });
        }
        break;
      case parser_state::parse_primary_index_1:
        frames.emplace_back(frame{
          .state = parser_state::parse_primary_index_2,
          .primary = top.primary,
          .primary_must_be_target = top.primary_must_be_target,
        });
        if (!is_current(token_type::colon) && !is_current(token_type::rbracket)) {
          frames.emplace_back(frame{
            .state = parser_state::parse_test,
            .test = top.primary->mutable_slice_expression()->mutable_slice()->mutable_end(),
          });
        }
        break;
      case parser_state::parse_primary_index_2:
        if (capture(token_type::colon) && !is_current(token_type::rbracket)) {
          frames.emplace_back(frame{
            .state = parser_state::parse_test,
            .test = top.primary->mutable_slice_expression()->mutable_slice()->mutable_step(),
          });
        }
        break;
      case parser_state::parse_primary_index_final:
        expect(token_type::rbracket);
        break;
      case parser_state::parse_operand:
        if (is_current(token_type::int_)) {
          found_non_load = true;
          top.primary->set_int_value(lex.current_token().int_value().to_string(10));
          lex.next_token();
        } else if (is_current(token_type::identifier)) {
          found_non_load = true;
          set_identifier(*top.primary->mutable_identifier());
        } else if (is_current(token_type::float_)) {
          found_non_load = true;
          top.primary->set_float_value(lex.current_token().double_value());
          lex.next_token();
        } else if (is_current(token_type::string)) {
          top.primary->set_string_value(lex.current_token().string_value());
          lex.next_token();
        } else if (is_current(token_type::bytes)) {
          found_non_load = true;
          top.primary->set_bytes_value(lex.current_token().string_value());
          lex.next_token();
        } else if (is_current(token_type::lbracket)) {
          found_non_load = true;
          frames.emplace_back(frame{
            .state = parser_state::parse_list,
            .primary = top.primary,
          });
        } else if (is_current(token_type::lbrace)) {
          found_non_load = true;
          frames.emplace_back(frame{
            .state = parser_state::parse_dict,
            .primary = top.primary,
          });
        } else if (capture(token_type::lparen)) {
          if (capture(token_type::rparen)) {
            found_non_load = true;
            top.primary->mutable_expression()->mutable_tuple();
          } else {
            frames.emplace_back(frame{
              .state = parser_state::parse_operand_expression_0,
            });
            frames.emplace_back(frame{
              .state = parser_state::parse_expression,
              .expression = top.primary->mutable_expression(),
              .expression_allow_trailing_comma = true,
            });
          }
        } else {
          add_error("Unexpected token");
        }
        break;
      case parser_state::parse_operand_expression_0:
        if (!expect(token_type::rparen)) {
          break;
        }
        break;
      case parser_state::parse_list:
        if (!expect(token_type::lbracket)) {
          break;
        }
        if (capture(token_type::rbracket)) {
          top.primary->mutable_list_expression();
          break;
        }

        frames.emplace_back(frame{
          .state = parser_state::parse_list_final,
        });
        frames.emplace_back(frame{
          .state = parser_state::parse_list_0,
          .primary = top.primary,
        });
        frames.emplace_back(frame{
          .state = parser_state::parse_test,
          .test = top.primary->mutable_list_expression()->add_element(),
        });
        break;
      case parser_state::parse_list_0:
        switch (lex.current_token().type()) {
          case token_type::for_:
            {
              Test* expression = Arena::Create<Test>(top.primary->GetArena());
              expression->Swap(&top.primary->mutable_list_expression()->mutable_element()->at(0));
              expression->Swap(top.primary->mutable_list_comprehension()->mutable_test());
            }
            create_block({}, {}, top.primary->mutable_list_comprehension()->mutable_comprehension_binding());
            frames.emplace_back(frame{
              .state = parser_state::parse_list_comprehension_final,
              .primary = top.primary,
            });
            frames.emplace_back(frame{
              .state = parser_state::parse_comp_clauses,
              .comp_clauses = top.primary->mutable_list_comprehension()->mutable_clause(),
            });
            break;
          case token_type::rbracket:
          case token_type::comma:
            frames.emplace_back(frame{
              .state = parser_state::parse_list_index_0,
              .primary = top.primary,
            });
            break;
          default:
            break;
        }
        break;
      case parser_state::parse_list_index_0:
        if (capture(token_type::comma)) {
          if (is_current(token_type::rbracket)) {
            break;
          }
          frames.emplace_back(top);
          frames.emplace_back(frame{
            .state = parser_state::parse_test,
            .test = top.primary->mutable_list_expression()->add_element(),
          });
        }
        break;
      case parser_state::parse_list_comprehension_final:
        drop_block();
        break;
      case parser_state::parse_list_final:
        if (!expect(token_type::rbracket)) {
          break;
        }
        break;
      case parser_state::parse_dict:
        if (!expect(token_type::lbrace)) {
          break;
        }
        if (capture(token_type::rbrace)) {
          top.primary->mutable_dictionary_expression();
          break;
        }

        frames.emplace_back(frame{
          .state = parser_state::parse_dict_final,
        });
        frames.emplace_back(frame{
          .state = parser_state::parse_dict_0,
          .primary = top.primary,
        });
        frames.emplace_back(frame{
          .state = parser_state::parse_entry,
          .entry = top.primary->mutable_dictionary_expression()->add_entry(),
        });
        break;
      case parser_state::parse_dict_0:
        switch (lex.current_token().type()) {
          case token_type::for_:
            {
              Entry* entry = Arena::Create<Entry>(top.primary->GetArena());
              entry->Swap(&top.primary->mutable_dictionary_expression()->mutable_entry()->at(0));
              entry->Swap(top.primary->mutable_dictionary_comprehension()->mutable_entry());
            }
            create_block({}, {}, top.primary->mutable_dictionary_comprehension()->mutable_comprehension_binding());
            frames.emplace_back(frame{
              .state = parser_state::parse_dict_comprehension_final,
              .primary = top.primary,
            });
            frames.emplace_back(frame{
              .state = parser_state::parse_comp_clauses,
              .comp_clauses = top.primary->mutable_dictionary_comprehension()->mutable_clause(),
            });
            break;
          case token_type::rbrace:
          case token_type::comma:
            frames.emplace_back(frame{
              .state = parser_state::parse_dict_index_0,
              .primary = top.primary,
            });
            break;
          default:
            break;
        }
        break;
      case parser_state::parse_dict_index_0:
        if (capture(token_type::comma)) {
          if (is_current(token_type::rbrace)) {
            break;
          }
          frames.emplace_back(top);
          frames.emplace_back(frame{
            .state = parser_state::parse_entry,
            .entry = top.primary->mutable_dictionary_expression()->add_entry(),
          });
        }
        break;
      case parser_state::parse_dict_comprehension_final:
        drop_block();
        break;
      case parser_state::parse_dict_final:
        expect(token_type::rbrace);
        break;
      case parser_state::parse_entry:
        frames.emplace_back(frame{
          .state = parser_state::parse_entry_0,
          .entry = top.entry,
        });
        frames.emplace_back(frame{
          .state = parser_state::parse_test,
          .test = top.entry->mutable_key(),
        });
        break;
      case parser_state::parse_entry_0:
        if (!expect(token_type::colon)) {
          break;
        }
        frames.emplace_back(frame{
          .state = parser_state::parse_test,
          .test = top.entry->mutable_value(),
        });
        break;
      case parser_state::parse_comp_clauses:
        if (capture(token_type::for_)) {
          auto* comp_clause = top.comp_clauses->Add();
          auto* comp_clause_primary = comp_clause->mutable_for_clause()->add_loop_variable();
          frames.emplace_back(top);
          frames.emplace_back(frame{
            .state = parser_state::parse_comp_clauses_0,
            .comp_clause = comp_clause,
            .comp_clause_primary = comp_clause_primary,
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_primary,
            .primary = comp_clause_primary,
            .primary_must_be_target = true,
          });
        } else if (capture(token_type::if_)) {
          auto* comp_clause = top.comp_clauses->Add();
          frames.emplace_back(top);
          frames.emplace_back(frame{
            .state = parser_state::parse_resolve_test,
            .test = comp_clause->mutable_if_clause(),
          });
          // Have to avoid parsing this as an `IfExpr`.
          // This is also not allowing a lambda to be used.
          // Context: https://github.com/bazelbuild/bazel/issues/24469
          frames.emplace_back(frame{
            .state = parser_state::parse_test_p,
            .test = comp_clause->mutable_if_clause(),
            .test_p_precedence = 0,
          });
        }
        break;
      case parser_state::parse_comp_clauses_0:
        bind_and_resolve(top.comp_clause_primary);
        if (capture(token_type::comma)) {
          auto* comp_clause_primary = top.comp_clause->mutable_for_clause()->add_loop_variable();
          frames.emplace_back(frame{
            .state = parser_state::parse_comp_clauses_0,
            .comp_clause = top.comp_clause,
            .comp_clause_primary = comp_clause_primary,
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_primary,
            .primary = comp_clause_primary,
            .primary_must_be_target = true,
          });
        } else {
          if (!expect(token_type::in)) {
            break;
          }
          frames.emplace_back(frame{
            .state = parser_state::parse_resolve_test,
            .test = top.comp_clause->mutable_for_clause()->mutable_in(),
          });
          // Do not allow `IfExpr` nor lambdas.
          frames.emplace_back(frame{
            .state = parser_state::parse_test_p,
            .test = top.comp_clause->mutable_for_clause()->mutable_in(),
            .test_p_precedence = 0,
          });
        }
        break;
      case parser_state::parse_argument:
        // Check that the order is (not all elements must be present, but the order is strict):
        // - positional arguments
        // - keyword arguments
        // - At most one *args
        // - At most one **kwargs
        if (capture(token_type::star)) {
          if (!options.allow_varadic_arguments) {
            // Report the error, but keep on parsing.
            add_error("Varadic arguments are not allowed");
          }
          if (top.previous_argument != nullptr &&
              top.previous_argument->has_star_argument()) {
            add_error("Duplicate *args");
          }
          if (top.previous_argument != nullptr &&
              top.previous_argument->has_star_star_argument()) {
            add_error("**kwargs must be the last argument");
          }
          frames.emplace_back(frame{
            .state = parser_state::parse_test,
            .test = top.argument->mutable_star_argument(),
          });
        } else if (capture(token_type::star_star)) {
          if (!options.allow_varadic_arguments) {
            // Report the error, but keep on parsing.
            add_error("Varadic arguments are not allowed");
          }
          if (top.previous_argument != nullptr &&
              top.previous_argument->has_star_star_argument()) {
            add_error("Duplicate **kwargs");
          }
          frames.emplace_back(frame{
            .state = parser_state::parse_test,
            .test = top.argument->mutable_star_star_argument(),
          });
        } else {
          if (top.previous_argument != nullptr &&
              (top.previous_argument->has_star_argument() ||
               top.previous_argument->has_star_star_argument())) {
            add_error("Non-varadic arguments must be before varadic arguments");
          }
          frames.emplace_back(frame{
            .state = parser_state::parse_argument_0,
            .argument = top.argument,
            .previous_argument = top.previous_argument,
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_test,
            .test = top.argument->mutable_value(),
          });
        }
        break;
      case parser_state::parse_argument_0:
        if (capture(token_type::equals)) {
          if (!top.argument->value().primary_expression().has_identifier()) {
            add_error("Expected identifier for named arguments");
            break;
          }
          {
            Identifier* id = Arena::Create<Identifier>(top.argument->GetArena());
            id->Swap(top.argument->mutable_value()->mutable_primary_expression()->mutable_identifier());
            id->Swap(top.argument->mutable_named_argument()->mutable_identifier());
          }
          frames.emplace_back(frame{
            .state = parser_state::parse_test,
            .test = top.argument->mutable_named_argument()->mutable_value(),
          });
        } else {
          if (top.previous_argument != nullptr &&
              top.previous_argument->has_named_argument()) {
            add_error("Positional arguments must come before named arguments");
          }
        }
        break;
      case parser_state::parse_lambda:
        if (!options.allow_function_definitions) {
          add_error("Function definitions not allowed");
        }
        expect(token_type::lambda);
        frames.emplace_back(frame{
          .state = parser_state::parse_lambda_0,
          .lambda = top.lambda,
        });
        frames.emplace_back(frame{
          .state = parser_state::parse_parameters,
          .parameters = top.lambda->mutable_parameter(),
          .parse_parameters_allow_trailing_comma = false,
          .parse_parameters_first = true,
          .found_star_parameter = false,
          .found_star_star_parameter = false,
          .previous_parameter_was_bare_star = false,
        });
        break;
      case parser_state::parse_lambda_0:
        create_block(parse_parameter_identifiers.back().first,
                     parse_parameter_identifiers.back().second,
                     top.lambda->mutable_function_binding());
        parse_parameter_identifiers.pop_back();
        frames.emplace_back(frame{
          .state = parser_state::parse_lambda_final,
          .lambda = top.lambda,
        });
        if (!expect(token_type::colon)) {
          break;
        }
        frames.emplace_back(frame{
          .state = parser_state::parse_resolve_test,
          .test = top.lambda->mutable_test(),
        });
        frames.emplace_back(frame{
          .state = parser_state::parse_test,
          .test = top.lambda->mutable_test(),
        });
        break;
      case parser_state::parse_lambda_final:
        drop_block();
        break;
      case parser_state::parse_parameters:
        if (top.parse_parameters_first || capture(token_type::comma)) {
          if (top.parse_parameters_first) {
            parse_parameter_identifiers.emplace_back(std::set<std::string>{}, std::set<Identifier*>{});
          }
          if (is_current(token_type::identifier)) {
            if (top.found_star_star_parameter) {
              add_error("arguments cannot follow var-keyword argument");
            }
            frames.emplace_back(frame{
              .state = parser_state::parse_parameters,
              .parameters = top.parameters,
              .parse_parameters_allow_trailing_comma = top.parse_parameters_allow_trailing_comma,
              .parse_parameters_first = false,
              .found_star_parameter = top.found_star_parameter,
              .found_star_star_parameter = top.found_star_star_parameter,
              .previous_parameter_was_bare_star = false,
            });
            Parameter* param = top.parameters->Add();
            set_identifier(*param->mutable_identifier());
            if (capture(token_type::equals)) {
              frames.emplace_back(frame{
                .state = parser_state::parse_resolve_test,
                .test = param->mutable_initialization(),
              });
              frames.emplace_back(frame{
                .state = parser_state::parse_test,
                .test = param->mutable_initialization(),
              });
            }
            if (!parse_parameter_identifiers.back().first.insert(param->identifier().nfkc_name()).second) {
              add_error("duplicate argument '" + param->identifier().name() + "' in function definition");
            }
            // The parameters need to be resolved. The issue is that the block does not yet exists so there
            // is a need to store the Identifiers and resolve them later.
            parse_parameter_identifiers.back().second.insert(param->mutable_identifier());
          } else if (capture(token_type::star)) {
            if (top.found_star_parameter) {
              add_error("* argument may appear only once");
            }
            if (top.found_star_star_parameter) {
              add_error("arguments cannot follow var-keyword argument");
            }
            frames.emplace_back(frame{
              .state = parser_state::parse_parameters,
              .parameters = top.parameters,
              .parse_parameters_allow_trailing_comma = top.parse_parameters_allow_trailing_comma,
              .parse_parameters_first = false,
              .found_star_parameter = true,
              .found_star_star_parameter = top.found_star_star_parameter,
              .previous_parameter_was_bare_star = !is_current(token_type::identifier),
            });
            Parameter* param = top.parameters->Add();
            param->mutable_star();
            if (is_current(token_type::identifier)) {
              set_identifier(*param->mutable_identifier());
              if (!parse_parameter_identifiers.back().first.insert(param->identifier().nfkc_name()).second) {
                add_error("duplicate argument '" + param->identifier().name() + "' in function definition");
              }
              parse_parameter_identifiers.back().second.insert(param->mutable_identifier());
            }
          } else if (capture(token_type::star_star)) {
            if (top.previous_parameter_was_bare_star) {
              add_error("named arguments must follow bare *");
            }
            if (top.found_star_star_parameter) {
              add_error("arguments cannot follow var-keyword argument");
            }
            frames.emplace_back(frame{
              .state = parser_state::parse_parameters,
              .parameters = top.parameters,
              .parse_parameters_allow_trailing_comma = top.parse_parameters_allow_trailing_comma,
              .parse_parameters_first = false,
              .found_star_parameter = top.found_star_parameter,
              .found_star_star_parameter = true,
              .previous_parameter_was_bare_star = false,
            });
            Parameter* param = top.parameters->Add();
            param->mutable_star_star();
            if (!set_identifier(*param->mutable_identifier())) {
              add_error("Expected identifier after STAR_STAR when parsing parameters");
            } else {
              if (!parse_parameter_identifiers.back().first.insert(param->identifier().nfkc_name()).second) {
                add_error("duplicate argument '" + param->identifier().name() + "' in function definition");
              }
              parse_parameter_identifiers.back().second.insert(param->mutable_identifier());
            }
          } else {
            if (top.previous_parameter_was_bare_star) {
              add_error("named arguments must follow bare *");
            }
            if (!top.parse_parameters_first && !top.parse_parameters_allow_trailing_comma) {
              add_error("Unexpected COMMA");
            }
          }
        } else {
          if (top.previous_parameter_was_bare_star) {
            add_error("named arguments must follow bare *");
          }
        }
        break;
      case parser_state::parse_resolve_test:
        resolve(top.test);
        break;
      case parser_state::parse_resolve_expression:
        resolve(top.expression);
        break;
    }
  } while (!frames.empty());
}

void parser::bind_and_resolve(starlark::PrimaryExpr* primary_expression) {
  bind_and_resolve(expression_frame{
    .primary_expression = primary_expression,
  });
}

void parser::bind_and_resolve(starlark::Expression* expression) {
  bind_and_resolve(expression_frame{
    .expression = expression,
  });
}

void parser::bind_and_resolve(expression_frame frame) {
  resolve(frame);
  std::vector<expression_frame> frames{frame};
  do {
    auto top = frames.back();
    frames.pop_back();
    if (top.primary_expression != nullptr) {
      switch (top.primary_expression->primary_expression_type_case()) {
        case PrimaryExpr::kDotExpression:
        case PrimaryExpr::kSliceExpression:
          break;
        case PrimaryExpr::kIdentifier:
          if (is_top_level_block() &&
              parser_blocks[1].identifiers.contains(top.primary_expression->identifier().nfkc_name())) {
            add_error("Variable '" + top.primary_expression->identifier().name() + "' redefines symbol previously defined by a load statement");
          }
          bind(top.primary_expression->identifier());
          break;
        case PrimaryExpr::kCallExpression:
        case PrimaryExpr::kIntValue:
        case PrimaryExpr::kFloatValue:
        case PrimaryExpr::kStringValue:
        case PrimaryExpr::kBytesValue:
        case PrimaryExpr::kListComprehension:
        case PrimaryExpr::kDictionaryExpression:
        case PrimaryExpr::kDictionaryComprehension:
        case PrimaryExpr::PRIMARY_EXPRESSION_TYPE_NOT_SET:
        default:
          add_warning("Unexpected expression to bind");
          break;
        case PrimaryExpr::kListExpression:
          for (auto& item : *top.primary_expression->mutable_list_expression()->mutable_element()) {
            frames.push_back(expression_frame{
              .test = &item,
            });
          }
          break;
        case PrimaryExpr::kExpression:
          frames.push_back(expression_frame{
            .expression = top.primary_expression->mutable_expression(),
          });
          break;
      }
    }
    if (top.expression != nullptr) {
      switch (top.expression->expression_type_case()) {
        case Expression::kValue:
          frames.push_back(expression_frame{
            .test = top.expression->mutable_value(),
          });
          break;
        case Expression::EXPRESSION_TYPE_NOT_SET:
        default:
          add_warning("Unexpected expression to bind");
          break;
        case Expression::kTuple:
          for (auto& element : *top.expression->mutable_tuple()->mutable_value()) {
            frames.push_back(expression_frame{
              .test = &element,
            });
          }
          break;
      }
    }
    if (top.test != nullptr) {
      if (!top.test->has_primary_expression()) {
        add_warning("Unexpected expression to bind");
        continue;
      }
      frames.push_back(expression_frame{
        .primary_expression = top.test->mutable_primary_expression(),
      });
    }
  } while (!frames.empty());
}

void parser::bind(const starlark::Identifier& identifier) {
  // The spec states "It is a static error to bind a global variable already explicitly bound in the file"
  // This leaves to interpretation whether a `def` can be rebound. The rule followed is that `def` and variables should
  // be consistent, follow the same rules, and be handled equally.
  if (!parser_blocks.back().identifiers.insert(identifier.nfkc_name()).second && is_top_level_block() && !options.allow_top_level_rebinding) {
    add_error("Multiple bindings for the top-level symbol '" + identifier.name() + "'");
  }
}

bool parser::set_identifier(Identifier& identifier) {
  if (!is_current(token_type::identifier)) {
    return false;
  }
  auto name = lex.current_token().string_value();
  identifier.set_name(name);
  identifier.set_nfkc_name(to_nfkc(name));
  identifier_positions[&identifier] = lex.current_token().start();
  lex.next_token();
  return true;
}

void parser::create_block(const std::set<std::string>& symbols,
                    const std::set<starlark::Identifier*>& identifiers,
                    google::protobuf::RepeatedPtrField<std::string>* binding) {
  parser_blocks.emplace_back(symbols, binding);
  for (auto* id : identifiers) {
    resolve(id);
  }
}

void parser::drop_block() {
  if (parser_blocks.back().id_store != nullptr) {
    for (const auto& binding : parser_blocks.back().identifiers) {
      *parser_blocks.back().id_store->Add() = binding;
    }
  }
  for (auto& entry : parser_blocks.back().to_resolve) {
    auto pos = parser_blocks.back().identifiers.find(entry.first->nfkc_name());
    if (pos == parser_blocks.back().identifiers.end()) {
      if (parser_blocks.size() == 1) {
        add_error("name '" + entry.first->name() + "' is not defined", identifier_positions[entry.first]);
      } else {
        parser_blocks[parser_blocks.size() - 2].to_resolve.emplace_back(entry.first, entry.second + 1);
      }
    } else {
      entry.first->set_frame(entry.second);
      entry.first->set_pos_in_frame(std::distance(parser_blocks.back().identifiers.begin(), pos));
    }
  }
  parser_blocks.pop_back();
}

void parser::resolve(starlark::Identifier* identifier) {
  parser_blocks.back().to_resolve.emplace_back(identifier, 0);
}

void parser::resolve(starlark::Test* test) {
  resolve(expression_frame{
    .test = test,
  });
}

void parser::resolve(starlark::Expression* expression) {
  resolve(expression_frame{
    .expression = expression,
  });
}

void parser::resolve(expression_frame frame) {
  std::vector<expression_frame> frames{frame};
  do {
    auto top = frames.back();
    frames.pop_back();
    if (top.primary_expression != nullptr) {
      switch (top.primary_expression->primary_expression_type_case()) {
        case PrimaryExpr::kDotExpression:
          frames.push_back(expression_frame{
            .primary_expression = top.primary_expression->mutable_dot_expression()->mutable_primary_expression(),
          });
          break;
        case PrimaryExpr::kSliceExpression:
          frames.push_back(expression_frame{
            .primary_expression = top.primary_expression->mutable_slice_expression()->mutable_primary_expression(),
          });
          if (top.primary_expression->slice_expression().has_index()) {
            frames.push_back(expression_frame{
              .expression = top.primary_expression->mutable_slice_expression()->mutable_index(),
            });
          }
          if (top.primary_expression->slice_expression().slice().has_start()) {
            frames.push_back(expression_frame{
              .test = top.primary_expression->mutable_slice_expression()->mutable_slice()->mutable_start(),
            });
          }
          if (top.primary_expression->slice_expression().slice().has_end()) {
            frames.push_back(expression_frame{
              .test = top.primary_expression->mutable_slice_expression()->mutable_slice()->mutable_end(),
            });
          }
          if (top.primary_expression->slice_expression().slice().has_step()) {
            frames.push_back(expression_frame{
              .test = top.primary_expression->mutable_slice_expression()->mutable_slice()->mutable_step(),
            });
          }
          break;
        case PrimaryExpr::kIdentifier:
          resolve(top.primary_expression->mutable_identifier());
          break;
        case PrimaryExpr::kCallExpression:
          frames.push_back(expression_frame{
            .primary_expression = top.primary_expression->mutable_call_expression()->mutable_primary_expression(),
          });
          for (auto& arg : *top.primary_expression->mutable_call_expression()->mutable_argument()) {
            if (arg.has_value()) {
              frames.push_back(expression_frame{
                .test = arg.mutable_value(),
              });
            }
            if (arg.has_named_argument()) {
              frames.push_back(expression_frame{
                .test = arg.mutable_named_argument()->mutable_value(),
              });
            }
            if (arg.has_star_argument()) {
              frames.push_back(expression_frame{
                .test = arg.mutable_star_argument(),

              });
            }
            if (arg.has_star_star_argument()) {
              frames.push_back(expression_frame{
                .test = arg.mutable_star_star_argument(),
              });
            }
          }
          break;
        case PrimaryExpr::kIntValue:
        case PrimaryExpr::kFloatValue:
        case PrimaryExpr::kStringValue:
        case PrimaryExpr::kBytesValue:
        case PrimaryExpr::kListComprehension:
        case PrimaryExpr::kDictionaryComprehension:
        case PrimaryExpr::PRIMARY_EXPRESSION_TYPE_NOT_SET:
        default:
          break;
        case PrimaryExpr::kDictionaryExpression:
          for (auto& item : *top.primary_expression->mutable_dictionary_expression()->mutable_entry()) {
            frames.push_back(expression_frame{
              .test = item.mutable_key(),
            });
            frames.push_back(expression_frame{
              .test = item.mutable_value(),
            });
          }
          break;
        case PrimaryExpr::kListExpression:
          for (auto& item : *top.primary_expression->mutable_list_expression()->mutable_element()) {
            frames.push_back(expression_frame{
              .test = &item,
            });
          }
          break;
        case PrimaryExpr::kExpression:
          frames.push_back(expression_frame{
            .expression = top.primary_expression->mutable_expression(),
          });
          break;
      }
    }
    if (top.expression != nullptr) {
      switch (top.expression->expression_type_case()) {
        case Expression::kValue:
          frames.push_back(expression_frame{
            .test = top.expression->mutable_value(),
          });
          break;
        case Expression::EXPRESSION_TYPE_NOT_SET:
        default:
          add_warning("Unexpected expression to resolve");
          break;
        case Expression::kTuple:
          for (auto& element : *top.expression->mutable_tuple()->mutable_value()) {
            frames.push_back(expression_frame{
              .test = &element,
            });
          }
          break;
      }
    }
    if (top.test != nullptr) {
      switch (top.test->test_type_case()) {
        case Test::kIfExpression:
          frames.push_back(expression_frame{
            .test = top.test->mutable_if_expression()->mutable_if_value(),
          });
          frames.push_back(expression_frame{
            .test = top.test->mutable_if_expression()->mutable_if_test(),
          });
          frames.push_back(expression_frame{
            .test = top.test->mutable_if_expression()->mutable_else_value(),
          });
          break;
        case Test::kPrimaryExpression:
          frames.push_back(expression_frame{
            .primary_expression = top.test->mutable_primary_expression(),
          });
          break;
        case Test::kUnaryExpression:
          frames.push_back(expression_frame{
            .test = top.test->mutable_unary_expression()->mutable_test(),
          });
          break;
        case Test::kBinaryExpression:
          frames.push_back(expression_frame{
            .test = top.test->mutable_binary_expression()->mutable_lhs(),
          });
          frames.push_back(expression_frame{
            .test = top.test->mutable_binary_expression()->mutable_rhs(),
          });
          break;
        case Test::kLambdaExpression:
        case Test::TEST_TYPE_NOT_SET:
          break;
      }
    }
  } while (!frames.empty());
}

bool parser::is_top_level_block() const {
  return parser_blocks.size() == 3;
}

}  // namespace grammar

