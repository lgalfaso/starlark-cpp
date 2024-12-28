// Copyright 2024 Lucas Mirelmann

#include "grammar/parser.hpp"

#include <map>
#include <utility>
#include <vector>

#include "unicode/normalization.hpp"
#include "third-party/defer.hpp"

using google::protobuf::RepeatedPtrField;
using starlark::AssignStmt;
using starlark::DefStmt;
using starlark::Expression;
using starlark::File;
using starlark::ForStmt;
using starlark::Identifier;
using starlark::IfStmt;
using starlark::Parameter;
using starlark::PrimaryExpr;
using starlark::Statement;
using starlark::Test;
using unicode::to_nfkc;

namespace grammar {

namespace {

const std::map<token_type, std::pair<int, Test::BinaryExpr::BinaryOperator>> operator_precedence = {
  {token_type::or_, {1, Test::BinaryExpr::OR}},
  {token_type::and_, {2, Test::BinaryExpr::AND}},
  {token_type::not_, {3, Test::BinaryExpr::UNKNOWN}},  // As a prefix.
  {token_type::equals_equals, {4, Test::BinaryExpr::EQUALS_EQUALS}},
  {token_type::not_equals, {4, Test::BinaryExpr::BANG_EQUALS}},
  {token_type::less, {4, Test::BinaryExpr::LESS_THAN}},
  {token_type::greater, {4, Test::BinaryExpr::GREATER_THAN}},
  {token_type::less_equals, {4, Test::BinaryExpr::LESS_THAN_EQUALS}},
  {token_type::greater_equals, {4, Test::BinaryExpr::GREATER_THAN_EQUALS}},
  {token_type::in, {4, Test::BinaryExpr::IN}},
  {token_type::pipe, {5, Test::BinaryExpr::PIPE}},
  {token_type::caret, {6, Test::BinaryExpr::HAT}},
  {token_type::ampersand, {7, Test::BinaryExpr::AMPERSAND}},
  {token_type::less_less, {8, Test::BinaryExpr::LESS_THAN_LESS_THAN}},
  {token_type::greater_greater, {8, Test::BinaryExpr::GREATER_THAN_GREATER_THAN}},
  {token_type::minus, {9, Test::BinaryExpr::MINUS}},
  {token_type::plus, {9, Test::BinaryExpr::PLUS}},
  {token_type::star, {10, Test::BinaryExpr::STAR}},
  {token_type::percent, {10, Test::BinaryExpr::PERCENT}},
  {token_type::slash, {10, Test::BinaryExpr::SLASH}},
  {token_type::slash_slash, {10, Test::BinaryExpr::SLASH_SLASH}},
};

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

constexpr int MAX_PRECEDENCE = 11;

bool is_target(const Expression& expression);
bool is_target(const Test& test);

bool is_target(const PrimaryExpr& primary_expression) {
  switch (primary_expression.primary_expression_type_case()) {
    case PrimaryExpr::kDotExpression:
    case PrimaryExpr::kSliceExpression:
      return true;
    case PrimaryExpr::kCallExpression:
    case PrimaryExpr::PRIMARY_EXPRESSION_TYPE_NOT_SET:
    default:
      return false;
    case PrimaryExpr::kOperand:
      switch (primary_expression.operand().operand_type_case()) {
        case PrimaryExpr::Operand::kIdentifier:
          return true;
        case PrimaryExpr::Operand::kListExpression:
          for (const auto& item : primary_expression.operand().list_expression().element()) {
            if (!is_target(item)) {
              return false;
            }
          }
          return true;
        case PrimaryExpr::Operand::kIntValue:
        case PrimaryExpr::Operand::kFloatValue:
        case PrimaryExpr::Operand::kStringValue:
        case PrimaryExpr::Operand::kBytesValue:
        case PrimaryExpr::Operand::kListComprehension:
        case PrimaryExpr::Operand::kDictionaryExpression:
        case PrimaryExpr::Operand::kDictionaryComprehension:
        case PrimaryExpr::Operand::OPERAND_TYPE_NOT_SET:
        default:
          return false;
        case PrimaryExpr::Operand::kExpression:
          return is_target(primary_expression.operand().expression());
      }
  }
}

bool is_target(const Expression& expression) {
  switch (expression.expression_type_case()) {
    case Expression::kValue:
    case Expression::EXPRESSION_TYPE_NOT_SET:
    default:
      return is_target(expression.value());
      break;
    case Expression::kTuple:
      for (const auto& element : expression.tuple().value()) {
        if (!is_target(element)) {
          return false;
        }
      }
      return true;
  }
}

bool is_target(const Test& test) {
  return test.has_primary_expression() &&
      is_target(test.primary_expression());
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
  parse_simple_statement_1,
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
  parse_list_final,
  parse_dict,
  parse_dict_0,
  parse_dict_index_0,
  parse_dict_final,
  parse_entry,
  parse_entry_0,
  parse_comp_clauses,
  parse_comp_clauses_0,
  parse_argument,
  parse_argument_0,
};

struct frame {
  parser_state state;
  union {
    RepeatedPtrField<Statement>* statements;
    Statement* statement;
    DefStmt* def_statement;
    IfStmt* if_statement;
    ForStmt* for_statement;
    Test::LambdaExpr* lambda;
    PrimaryExpr::Operand* operand;
    PrimaryExpr::CallExpr::Argument* argument;
    PrimaryExpr::Operand::Entry* entry;
    RepeatedPtrField<PrimaryExpr::Operand::CompClause>* comp_clauses;
    PrimaryExpr::Operand::CompClause* comp_clause;
    struct {
      RepeatedPtrField<Parameter>* parameters;
      bool parse_parameters_allow_trailing_comma;
      bool parse_parameters_first;
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
      bool primary_must_be_target = false;
    };
  };
};

}  // namespace

parser::parser(std::string_view input, logger& logging) : parser(input, grammar_options{}, logging) {
}

parser::parser(std::string_view input, const grammar_options& options, logger& logging)
    : options(options), lex(input, options, logging), nested_loops(1), logging(logging) {
  lex.next_token();
}

File parser::parse_file() {
  // TODO(lmirelmann): Put the binding on the identifiers
  // TODO(lmirelmann): Add validation on identifier use
  File result;
  while (lex.current_token().type() != token_type::eof) {
    if (lex.current_token().type() == token_type::newline) {
      lex.next_token();
    } else {
      parse_statement(*result.mutable_statement());
    }
  }
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

void parser::add_error(const std::string& error_message) {
  logging.log(log_level::ERROR, error_message, module, lex.current_token().start());
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
          DefStmt* def_statement = top.statements->Add()->mutable_def_statement();
          nested_loops.push_back(0);
          frames.emplace_back(frame{
            .state = parser_state::parse_statement_def_final,
          });
          if (!set_identifier(*def_statement->mutable_function_name())) {
            add_error("Expected an identifier");
            break;
          }
          if (!expect(token_type::lparen)) {
            break;
          }
          frames.emplace_back(frame{
            .state = parser_state::parse_statement_def_0,
            .def_statement = def_statement,
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_parameters,
            .parameters = def_statement->mutable_parameter(),
            .parse_parameters_allow_trailing_comma = true,
            .parse_parameters_first = true,
          });
        } else if (capture(token_type::if_)) {
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
            .state = parser_state::parse_test,
            .test = if_statement->mutable_test(),
          });
        } else if (capture(token_type::for_)) {
          ForStmt* for_statement = top.statements->Add()->mutable_for_statement();
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
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_primary,
            .primary = for_statement->add_loop_variable(),
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
        if (capture(token_type::comma)) {
          frames.emplace_back(top);
          frames.emplace_back(frame{
            .state = parser_state::parse_primary,
            .primary = top.for_statement->add_loop_variable(),
            .primary_must_be_target = true,
          });
        }
        break;
      case parser_state::parse_statement_for_1:
        if (!expect(token_type::in)) {
          break;
        }
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
          .state = parser_state::parse_simple_statement_1,
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
      case parser_state::parse_simple_statement_1:
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
            if (nested_loops.size() == 1) {
              add_error("Unexpected RETURN");
            }
            top.statement->mutable_return_statement();
            lex.next_token();
            if (!is_current(token_type::newline)) {
              frames.emplace_back(frame{
                .state = parser_state::parse_expression,
                .expression = top.statement->mutable_return_statement()->mutable_expression(),
                .expression_allow_trailing_comma = false,
              });
            }
            break;
          case token_type::load:
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
            while (capture(token_type::comma)) {
              if (is_current(token_type::rparen)) {
                break;
              }
              auto* load_params = top.statement->mutable_load_statement()->add_load_params();
              if (is_current(token_type::identifier)) {
                set_identifier(*load_params->mutable_local_name());
                if (!expect(token_type::equals)) {
                  break;
                }
              }
              if (!is_current(token_type::string)) {
                add_error("Expected STRING");
                break;
              }
              load_params->set_remote_name(lex.current_token().string_value());
              lex.next_token();
            }
            expect(token_type::rparen);
            break;
          case token_type::break_:
            if (nested_loops.back() == 0) {
              add_error("Unexpected BREAK");
            }
            top.statement->mutable_break_statement();
            lex.next_token();
            break;
          case token_type::continue_:
            if (nested_loops.back() == 0) {
              add_error("Unexpected CONTINUE");
            }
            top.statement->mutable_continue_statement();
            lex.next_token();
            break;
          case token_type::pass:
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
          if (!is_target(top.statement->expression_statement())) {
            add_error("Exprecting TARGET");
            break;
          }
          {
            AssignStmt assign_statement;
            assign_statement.mutable_lhs()->Swap(top.statement->mutable_expression_statement());
            assign_statement.Swap(top.statement->mutable_assign_statement());
          }
          top.statement->mutable_assign_statement()->set_op(op->second);
          lex.next_token();
          frames.emplace_back(frame{
            .state = parser_state::parse_expression,
            .expression = top.statement->mutable_assign_statement()->mutable_rhs(),
            .expression_allow_trailing_comma = false,
          });
        }
        break;
      case parser_state::parse_expression:
        if (is_current(token_type::rparen)) {
          if (!top.expression_allow_trailing_comma) {
            add_error("Unexpected TUPLE");
          }
          top.expression->mutable_tuple();
          break;
        }

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
          {
            Test first_test;
            first_test.Swap(top.expression->mutable_value());
            first_test.Swap(top.expression->mutable_tuple()->add_value());
          }
          frames.emplace_back(frame{
            .state = parser_state::parse_expression_1,
            .expression = top.expression,
            .expression_allow_trailing_comma = top.expression_allow_trailing_comma,
          });
        } else {
          if (top.expression->value().primary_expression().operand().has_expression()) {
            Test first_test;
            first_test.Swap(top.expression->mutable_value());
            first_test.mutable_primary_expression()->mutable_operand()->mutable_expression()->Swap(top.expression);
          }
        }
        break;
      case parser_state::parse_expression_1:
        if (capture(token_type::comma)) {
          if (is_current(token_type::rparen)) {
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
          {
            Test new_result;
            new_result.mutable_if_expression()->mutable_if_value()->Swap(top.test);
            new_result.Swap(top.test);
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
          .state = parser_state::parse_test_p,
          .test = top.test->mutable_if_expression()->mutable_else_value(),
          .test_p_precedence = 0,
        });
        break;
      case parser_state::parse_test_p:
        if (top.test_p_precedence >= MAX_PRECEDENCE) {
          Test* result_ref = top.test;
          for (;;) {
            if (capture(token_type::plus)) {
              result_ref->mutable_unary_expression()->set_operator_(Test::UnaryExpr::PLUS);
            } else if (capture(token_type::minus)) {
              result_ref->mutable_unary_expression()->set_operator_(Test::UnaryExpr::MINUS);
            } else if (capture(token_type::tilde)) {
              result_ref->mutable_unary_expression()->set_operator_(Test::UnaryExpr::TILDE);
            } else {
              break;
            }
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
            result_ref->mutable_unary_expression()->set_operator_(Test::UnaryExpr::NOT);
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
          lex.next_token();
          if (!expect(token_type::in)) {
            break;
          }
          {
            Test new_result;
            new_result.mutable_binary_expression()->mutable_lhs()->Swap(top.test);
            new_result.Swap(top.test);
          }
          top.test->mutable_binary_expression()->set_operator_(Test::BinaryExpr::NOT_IN);
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
          lex.next_token();
          {
            Test new_result;
            new_result.mutable_binary_expression()->mutable_lhs()->Swap(top.test);
            new_result.Swap(top.test);
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
          .operand = top.primary->mutable_operand(),
        });
        break;
      case parser_state::parse_primary_0:
        if (top.primary->operand().expression().value().has_primary_expression()) {
          PrimaryExpr new_result;
          new_result.Swap(top.primary->mutable_operand()->mutable_expression()->mutable_value()->mutable_primary_expression());
          top.primary->Swap(&new_result);
        }
        if (capture(token_type::dot)) {
          {
            PrimaryExpr new_result;
            new_result.mutable_dot_expression()->mutable_primary_expression()->Swap(top.primary);
            new_result.Swap(top.primary);
          }
          if (!set_identifier(*top.primary->mutable_dot_expression()->mutable_identifier())) {
            add_error("Expecting IDENTIFIER");
            break;
          }
          frames.emplace_back(top);
        } else if (capture(token_type::lparen)) {
          {
            PrimaryExpr new_result;
            new_result.mutable_call_expression()->mutable_primary_expression()->Swap(top.primary);
            new_result.Swap(top.primary);
          }
          frames.emplace_back(top);
          if (capture(token_type::rparen)) {
            break;
          }
          frames.emplace_back(frame{
            .state = parser_state::parse_primary_call_0,
            .primary = top.primary,
            .primary_must_be_target = top.primary_must_be_target,
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_argument,
            .argument = top.primary->mutable_call_expression()->add_argument(),
          });
        } else if (capture(token_type::lbracket)) {
          {
            PrimaryExpr new_result;
            new_result.mutable_slice_expression()->mutable_primary_expression()->Swap(top.primary);
            new_result.Swap(top.primary);
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
          frames.emplace_back(top);
          frames.emplace_back(frame{
            .state = parser_state::parse_argument,
            .argument = top.primary->mutable_call_expression()->add_argument(),
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
            Expression expression;
            expression.Swap(top.primary->mutable_slice_expression()->mutable_index());
            top.primary->mutable_slice_expression()->mutable_slice()->mutable_start()->Swap(expression.mutable_value());
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
          top.operand->set_int_value(lex.current_token().int_value().to_string(10));
          lex.next_token();
        } else if (is_current(token_type::identifier)) {
          set_identifier(*top.operand->mutable_identifier());
        } else if (is_current(token_type::float_)) {
          top.operand->set_float_value(lex.current_token().double_value());
          lex.next_token();
        } else if (is_current(token_type::string)) {
          top.operand->set_string_value(lex.current_token().string_value());
          lex.next_token();
        } else if (is_current(token_type::bytes)) {
          top.operand->set_bytes_value(lex.current_token().string_value());
          lex.next_token();
        } else if (is_current(token_type::lbracket)) {
          frames.emplace_back(frame{
            .state = parser_state::parse_list,
            .operand = top.operand,
          });
        } else if (is_current(token_type::lbrace)) {
          frames.emplace_back(frame{
            .state = parser_state::parse_dict,
            .operand = top.operand,
          });
        } else if (capture(token_type::lparen)) {
          frames.emplace_back(frame{
            .state = parser_state::parse_operand_expression_0,
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_expression,
            .expression = top.operand->mutable_expression(),
            .expression_allow_trailing_comma = true,
          });
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
          top.operand->mutable_list_expression();
          break;
        }

        frames.emplace_back(frame{
          .state = parser_state::parse_list_final,
        });
        frames.emplace_back(frame{
          .state = parser_state::parse_list_0,
          .operand = top.operand,
        });
        frames.emplace_back(frame{
          .state = parser_state::parse_test,
          .test = top.operand->mutable_list_expression()->add_element(),
        });
        break;
      case parser_state::parse_list_0:
        switch (lex.current_token().type()) {
          case token_type::for_:
            {
              Test expression;
              expression.Swap(&top.operand->mutable_list_expression()->mutable_element()->at(0));
              expression.Swap(top.operand->mutable_list_comprehension()->mutable_test());
            }
            frames.emplace_back(frame{
              .state = parser_state::parse_comp_clauses,
              .comp_clauses = top.operand->mutable_list_comprehension()->mutable_clause(),
            });
            break;
          case token_type::rbracket:
          case token_type::comma:
            frames.emplace_back(frame{
              .state = parser_state::parse_list_index_0,
              .operand = top.operand,
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
            .test = top.operand->mutable_list_expression()->add_element(),
          });
        }
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
          top.operand->mutable_dictionary_expression();
          break;
        }
 
        frames.emplace_back(frame{
          .state = parser_state::parse_dict_final,
        });
        frames.emplace_back(frame{
          .state = parser_state::parse_dict_0,
          .operand = top.operand,
        });
        frames.emplace_back(frame{
          .state = parser_state::parse_entry,
          .entry = top.operand->mutable_dictionary_expression()->add_entry(),
        });
        break;
      case parser_state::parse_dict_0:
        switch (lex.current_token().type()) {
          case token_type::for_:
            {
              PrimaryExpr::Operand::Entry entry;
              entry.Swap(&top.operand->mutable_dictionary_expression()->mutable_entry()->at(0));
              entry.Swap(top.operand->mutable_dictionary_comprehension()->mutable_entry());
            }
            frames.emplace_back(frame{
              .state = parser_state::parse_comp_clauses,
              .comp_clauses = top.operand->mutable_dictionary_comprehension()->mutable_clause(),
            });
            break;
          case token_type::rbrace:
          case token_type::comma:
            frames.emplace_back(frame{
              .state = parser_state::parse_dict_index_0,
              .operand = top.operand,
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
            .entry = top.operand->mutable_dictionary_expression()->add_entry(),
          });
        }
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
          frames.emplace_back(top);
          frames.emplace_back(frame{
            .state = parser_state::parse_comp_clauses_0,
            .comp_clause = comp_clause,
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_primary,
            .primary = comp_clause->mutable_for_clause()->add_loop_variable(),
            .primary_must_be_target = true,
          });
        } else if (capture(token_type::if_)) {
          frames.emplace_back(top);
          // Have to avoid parsing this as an `IfExpr`.
          // This is also not allowing a lambda to be used.
          // Context: https://github.com/bazelbuild/bazel/issues/24469
          frames.emplace_back(frame{
            .state = parser_state::parse_test_p,
            .test = top.comp_clauses->Add()->mutable_if_clause(),
            .test_p_precedence = 0,
          });
        }
        break;
      case parser_state::parse_comp_clauses_0:
        if (capture(token_type::comma)) {
          frames.emplace_back(top);
          frames.emplace_back(frame{
            .state = parser_state::parse_primary,
            .primary = top.comp_clause->mutable_for_clause()->add_loop_variable(),
            .primary_must_be_target = true,
          });
        } else {
          if (!expect(token_type::in)) {
            break;
          }
          // Do not allow `IfExpr` nor lambdas.
          frames.emplace_back(frame{
            .state = parser_state::parse_test_p,
            .test = top.comp_clause->mutable_for_clause()->mutable_in(),
            .test_p_precedence = 0,
          });
        }
        break;
      case parser_state::parse_argument:
        if (capture(token_type::star)) {
          frames.emplace_back(frame{
            .state = parser_state::parse_test,
            .test = top.argument->mutable_star_argument(),
          });
        } else if (capture(token_type::star_star)) {
          frames.emplace_back(frame{
            .state = parser_state::parse_test,
            .test = top.argument->mutable_star_star_argument(),
          });
        } else {
          frames.emplace_back(frame{
            .state = parser_state::parse_argument_0,
            .argument = top.argument,
          });
          frames.emplace_back(frame{
            .state = parser_state::parse_test,
            .test = top.argument->mutable_value(),
          });
        }
        break;
      case parser_state::parse_argument_0:
        if (capture(token_type::equals)) {
          if (!top.argument->value().primary_expression().operand().has_identifier()) {
            add_error("Expected identifier for named arguments");
            break;
          }
          {
            Identifier id;
            id.Swap(top.argument->mutable_value()->mutable_primary_expression()->mutable_operand()->mutable_identifier());
            id.Swap(top.argument->mutable_named_argument()->mutable_identifier());
          }
          frames.emplace_back(frame{
            .state = parser_state::parse_test,
            .test = top.argument->mutable_named_argument()->mutable_value(),
          });
        }
        break;
      case parser_state::parse_lambda:
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
        });
        break;
      case parser_state::parse_lambda_0:
        if (!expect(token_type::colon)) {
          break;
        }
        frames.emplace_back(frame{
          .state = parser_state::parse_test,
          .test = top.lambda->mutable_test(),
        });
        break;
      case parser_state::parse_parameters:
        if (top.parse_parameters_first || capture(token_type::comma)) {
          if (is_current(token_type::identifier)) {
            frames.emplace_back(frame{
              .state = parser_state::parse_parameters,
              .parameters = top.parameters,
              .parse_parameters_allow_trailing_comma = top.parse_parameters_allow_trailing_comma,
              .parse_parameters_first = false,
            });
            Parameter* param = top.parameters->Add();
            set_identifier(*param->mutable_identifier());
            if (capture(token_type::equals)) {
              frames.emplace_back(frame{
                .state = parser_state::parse_test,
                .test = param->mutable_initialization(),
              });
            }
          } else if (capture(token_type::star)) {
            frames.emplace_back(frame{
              .state = parser_state::parse_parameters,
              .parameters = top.parameters,
              .parse_parameters_allow_trailing_comma = top.parse_parameters_allow_trailing_comma,
              .parse_parameters_first = false,
            });
            Parameter* param = top.parameters->Add();
            param->mutable_star();
            if (is_current(token_type::identifier)) {
              set_identifier(*param->mutable_identifier());
            }
          } else if (capture(token_type::star_star)) {
            frames.emplace_back(frame{
              .state = parser_state::parse_parameters,
              .parameters = top.parameters,
              .parse_parameters_allow_trailing_comma = top.parse_parameters_allow_trailing_comma,
              .parse_parameters_first = false,
            });
            Parameter* param = top.parameters->Add();
            param->mutable_star_star();
            if (!set_identifier(*param->mutable_identifier())) {
              add_error("Expected identifier after STAR_STAR when parsing parameters");
            }
          } else {
            if (!top.parse_parameters_first && !top.parse_parameters_allow_trailing_comma) {
              add_error("Unexpected COMMA");
            }
          }
        }
        break;
    }
  } while (!frames.empty());
}

bool parser::set_identifier(Identifier& identifier) {
  if (!is_current(token_type::identifier)) {
    return false;
  }
  auto name = lex.current_token().string_value();
  identifier.set_name(name);
  identifier.set_nfkc_name(to_nfkc(name));
  lex.next_token();
  return true;
}

}  // namespace grammar

