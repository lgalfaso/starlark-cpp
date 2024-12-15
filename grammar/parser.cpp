// Copyright 2024 Lucas Mirelmann

#include "grammar/parser.hpp"

#include <map>
#include <utility>

#include "unicode/normalization.hpp"
#include "third-party/defer.hpp"

using google::protobuf::RepeatedPtrField;
using starlark::AssignStmt;
using starlark::Expression;
using starlark::File;
using starlark::Identifier;
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

bool is_target(const PrimaryExpr& primary_expression) {
  switch (primary_expression.primary_expression_type_case()) {
    case PrimaryExpr::kDotExpression:
    case PrimaryExpr::kSliceExpression:
      return true;
    case PrimaryExpr::kCallExpression:
    case PrimaryExpr::PRIMARY_EXPRESSION_TYPE_NOT_SET:
      return false;
    case PrimaryExpr::kOperand:
      switch (primary_expression.operand().operand_type_case()) {
        case PrimaryExpr::Operand::kIdentifier:
          return true;
        case PrimaryExpr::Operand::kIntValue:
        case PrimaryExpr::Operand::kFloatValue:
        case PrimaryExpr::Operand::kStringValue:
        case PrimaryExpr::Operand::kBytesValue:
        case PrimaryExpr::Operand::kListExpression:
        case PrimaryExpr::Operand::kListComprehension:
        case PrimaryExpr::Operand::kDictionaryExpression:
        case PrimaryExpr::Operand::kDictionaryComprehension:
        case PrimaryExpr::Operand::OPERAND_TYPE_NOT_SET:
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
      return expression.value().has_primary_expression() &&
        is_target(expression.value().primary_expression());
      break;
    case Expression::kTuple:
      for (const auto& element : expression.tuple().value()) {
        if (!element.has_primary_expression() ||
            !is_target(element.primary_expression())) {
          return false;
        }
      }
      return true;
  }
}

void set_identifier(Identifier* identifier, std::string_view name) {
  identifier->set_name(name);
  identifier->set_nfkc_name(to_nfkc(name));
}

}

parser::parser(std::string_view input) : lex(input) {
  lex.next_token();
  nested_loops.push_back(0);
}

File parser::parse_file() {
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

const std::vector<std::pair<std::string, std::size_t>>& parser::parser_errors() const {
  return errors;
}

const std::vector<std::pair<std::string, std::size_t>>& parser::lexer_errors() const {
  return lex.errors();
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
  errors.emplace_back(error_message, lex.current_token().start());
  recover = true;
}

void parser::parse_statement(RepeatedPtrField<Statement>& statements) {
  if (capture(token_type::def)) {
    nested_loops.push_back(0);
    defer { nested_loops.pop_back(); };
    Statement* def_statement = statements.Add();
    if (!is_current(token_type::identifier)) {
      add_error("Expected an identifier");
      nested_loops.pop_back();
      return;
    }
    set_identifier(def_statement->mutable_def_statement()->mutable_function_name(), lex.current_token().string_value());
    lex.next_token();
    if (!expect(token_type::lparen)) {
      return;
    }
    parse_parameters(*def_statement->mutable_def_statement()->mutable_parameter(), true);
    if (!expect(token_type::rparen)) {
      return;
    }
    if (!expect(token_type::colon)) {
      return;
    }
    parse_suite(*def_statement->mutable_def_statement()->mutable_statement());
  } else if (capture(token_type::if_)) {
    Statement* if_statement = statements.Add();
    *if_statement->mutable_if_statement()->mutable_test() = parse_test();
    if (!expect(token_type::colon)) {
      return;
    }
    parse_suite(*if_statement->mutable_if_statement()->mutable_statement());
    while (capture(token_type::elif)) {
      auto* elif = if_statement->mutable_if_statement()->add_elif();
      *elif->mutable_test() = parse_test();
      if (!expect(token_type::colon)) {
        return;
      }
      parse_suite(*elif->mutable_statement());
    }
    if (capture(token_type::else_)) {
      if (!expect(token_type::colon)) {
        return;
      }
      parse_suite(*if_statement->mutable_if_statement()->mutable_else_statement());
    }
  } else if (capture(token_type::for_)) {
    nested_loops.back()++;
    defer { nested_loops.back()--; };
    Statement* for_statement = statements.Add();
    do {
      *for_statement->mutable_for_statement()->add_loop_variable() = parse_primary();
      if (!is_target(*for_statement->for_statement().loop_variable().rbegin())) {
        add_error("Expecting a TARGET");
        return;
      }
    } while (capture(token_type::comma));
    if (!expect(token_type::in)) {
      return;
    }
    *for_statement->mutable_for_statement()->mutable_expression() = parse_expression(false);
    if (!expect(token_type::colon)) {
      return;
    }
    parse_suite(*for_statement->mutable_for_statement()->mutable_statement());
  } else {
    parse_simple_statement(statements);
  }
}

void parser::parse_suite(RepeatedPtrField<Statement>& statements) {
  if (capture(token_type::newline)) {
    if (!expect(token_type::indent)) {
      return;
    }
    while (!capture(token_type::outdent)) {
      parse_statement(statements);
    }
  } else {
    parse_simple_statement(statements);
  }
}

void parser::parse_simple_statement(RepeatedPtrField<Statement>& statements) {
  *statements.Add() = parse_small_statement();
  while (capture(token_type::semi)) {
    if (is_current(token_type::newline)) {
      break;
    }
    *statements.Add() = parse_small_statement();
  }
  if (recover) {
    while (lex.current_token().type() != token_type::newline) {
      lex.next_token();
    }
    recover = false;
  }
  if (!expect(token_type::newline)) {
    return;
  }
}

Statement parser::parse_small_statement() {
  Statement result;
  switch (lex.current_token().type()) {
    case token_type::return_:
      if (nested_loops.size() == 1) {
        add_error("Unexpected RETURN");
        return result;
      }
      result.mutable_return_statement();
      lex.next_token();
      if (!is_current(token_type::newline)) {
        *result.mutable_return_statement()->mutable_expression() = parse_expression(false);
      }
      break;
    case token_type::load:
      lex.next_token();
      if (!expect(token_type::lparen)) {
        return result;
      }
      if (!is_current(token_type::string)) {
        add_error("Expected STRING");
        return result;
      }
      result.mutable_load_statement()->set_module(lex.current_token().string_value());
      lex.next_token();
      while (capture(token_type::comma)) {
        if (is_current(token_type::rparen)) {
          break;
        }
        auto* load_params = result.mutable_load_statement()->add_load_params();
        if (is_current(token_type::identifier)) {
          set_identifier(load_params->mutable_local_name(), lex.current_token().string_value());
          lex.next_token();
          if (!expect(token_type::equals)) {
            return result;
          }
        }
        if (!is_current(token_type::string)) {
          add_error("Expected STRING");
          return result;
        }
        load_params->set_remote_name(lex.current_token().string_value());
        lex.next_token();
      }
      if (!expect(token_type::rparen)) {
        return result;
      }
      break;
    case token_type::break_:
      if (nested_loops.back() == 0) {
        add_error("Unexpected BREAK");
        return result;
      }
      result.mutable_break_statement();
      lex.next_token();
      break;
    case token_type::continue_: {
      if (nested_loops.back() == 0) {
        add_error("Unexpected CONTINUE");
        return result;
      }
      result.mutable_continue_statement();
      lex.next_token();
      break;
    }
    case token_type::pass: {
      result.mutable_pass_statement();
      lex.next_token();
      break;
    }
    default: {
      Expression expression = parse_expression(false);
      if (auto op = assign_ops.find(lex.current_token().type()); op != assign_ops.end()) {
        if (!is_target(expression)) {
          add_error("Exprecting TARGET");
          return result;
        }
        result.mutable_assign_statement()->mutable_lhs()->Swap(&expression);
        result.mutable_assign_statement()->set_op(op->second);
        lex.next_token();
        *result.mutable_assign_statement()->mutable_rhs() = parse_expression(false);
      } else {
        *result.mutable_expression_statement()->mutable_expression() = expression;
      }
      break;
    }
  }
  return result;
}

Expression parser::parse_expression(bool allow_trailing_comma) {
  Expression result;
  if (is_current(token_type::rparen)) {
    if (!allow_trailing_comma) {
      add_error("Unexpected TUPLE");
      return result;
    }
    result.mutable_tuple();
    return result;
  }

  auto first_test = parse_test();
  if (is_current(token_type::comma)) {
    first_test.Swap(result.mutable_tuple()->add_value());
    while (capture(token_type::comma)) {
      if (is_current(token_type::rparen)) {
        if (!allow_trailing_comma) {
          add_error("Unexpected COMMA");
          return result;
        }
        return result;
      }
      *result.mutable_tuple()->add_value() = parse_test();
    }
  } else {
    if (first_test.primary_expression().operand().has_expression()) {
      first_test.mutable_primary_expression()->mutable_operand()->mutable_expression()->Swap(&result);
    } else {
      first_test.Swap(result.mutable_value());
    }
  }

  return result;
}

Test parser::parse_test() {
  Test result;
  if (is_current(token_type::lambda)) {
    *result.mutable_lambda_expression() = parse_lambda();
  } else {
    result = parse_test(0);
    if (capture(token_type::if_)) {
      Test new_result;
      new_result.mutable_if_expression()->mutable_if_value()->Swap(&result);
      new_result.Swap(&result);
      *result.mutable_if_expression()->mutable_if_test() = parse_test(0);
      if (!expect(token_type::else_)) {
        return result;
      }
      *result.mutable_if_expression()->mutable_else_value() = parse_test();
    }
  }
  return result;
}

Test parser::parse_test(int precedence) {
  Test result;
  Test* result_ref = &result;
  if (precedence >= MAX_PRECEDENCE) {
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
    *result_ref->mutable_primary_expression() = parse_primary();
    return result;
  }
  if (is_current(token_type::not_) &&
      precedence == operator_precedence.at(token_type::not_).first) {
    for (;;) {
      if (!capture(token_type::not_)) {
        break;
      }
      result_ref->mutable_unary_expression()->set_operator_(Test::UnaryExpr::NOT);
      result_ref = result_ref->mutable_unary_expression()->mutable_test();
    }
    *result_ref = parse_test(precedence + 1);
    return result;
  }
  result = parse_test(precedence + 1);
  for (int loop_count = 0;;++loop_count) {
    if (is_current(token_type::not_)) {
      if (precedence != operator_precedence.at(token_type::in).first) {
        return result;
      }
      if (loop_count > 0) {
        add_error("Comparison operators are not associative. Use parens.");
        return result;
      }
      lex.next_token();
      if (!expect(token_type::in)) {
        return result;
      }
      Test new_result;
      new_result.mutable_binary_expression()->mutable_lhs()->Swap(&result);
      new_result.Swap(&result);
      result.mutable_binary_expression()->set_operator_(Test::BinaryExpr::NOT_IN);
      *result.mutable_binary_expression()->mutable_rhs() = parse_test(precedence + 1);
    } else if (auto next_op = operator_precedence.find(lex.current_token().type()); next_op != operator_precedence.end()) {
      if (loop_count > 0 && precedence == operator_precedence.at(token_type::equals_equals).first) {
        add_error("Comparison operators are not associative. Use parens.");
        return result;
      }
      if (precedence != next_op->second.first) {
        return result;
      }
      lex.next_token();
      Test new_result;
      new_result.mutable_binary_expression()->mutable_lhs()->Swap(&result);
      new_result.Swap(&result);
      result.mutable_binary_expression()->set_operator_(next_op->second.second);
      *result.mutable_binary_expression()->mutable_rhs() = parse_test(precedence + 1);
    } else {
      return result;
    }
  }
}

PrimaryExpr parser::parse_primary() {
  PrimaryExpr result;
  *result.mutable_operand() = parse_operand();
  if (result.operand().expression().value().has_primary_expression()) {
    PrimaryExpr new_result = result.operand().expression().value().primary_expression();
    result.Swap(&new_result);
  }
  for (;;) {
    if (capture(token_type::dot)) {
      PrimaryExpr new_result;
      new_result.mutable_dot_expression()->mutable_primary_expression()->Swap(&result);
      new_result.Swap(&result);
      if (!is_current(token_type::identifier)) {
        add_error("Expecting IDENTIFIER");
        return PrimaryExpr::default_instance();
      }
      set_identifier(result.mutable_dot_expression()->mutable_identifier(), lex.current_token().string_value());
      lex.next_token();
    } else if (capture(token_type::lparen)) {
      PrimaryExpr new_result;
      new_result.mutable_call_expression()->mutable_primary_expression()->Swap(&result);
      new_result.Swap(&result);
      if (capture(token_type::rparen)) {
        continue;
      }
      *result.mutable_call_expression()->add_argument() = parse_argument();
      while (capture(token_type::comma)) {
        if (is_current(token_type::rparen)) {
          break;
        }
        *result.mutable_call_expression()->add_argument() = parse_argument();
      }
      if (!expect(token_type::rparen)) {
        return result;
      }
    } else if (capture(token_type::lbracket)) {
      PrimaryExpr new_result;
      new_result.mutable_slice_expression()->mutable_primary_expression()->Swap(&result);
      new_result.Swap(&result);
      Expression expression = parse_expression(true);
      if (capture(token_type::colon)) {
        if (!expression.has_value()) {
          add_error("Unexpected TUPLE");
          return result;
        }
        *result.mutable_slice_expression()->mutable_slice()->mutable_start() = expression.value();
        if (!is_current(token_type::colon) && !is_current(token_type::rbracket)) {
          *result.mutable_slice_expression()->mutable_slice()->mutable_end() = parse_test();
        }
        if (capture(token_type::colon) && !is_current(token_type::rbracket)) {
          *result.mutable_slice_expression()->mutable_slice()->mutable_step() = parse_test();
        }
      } else {
        *result.mutable_slice_expression()->mutable_index() = expression;
      }
      if (!expect(token_type::rbracket)) {
        return result;
      }
    } else {
      break;
    }
  }
  return result;
}

PrimaryExpr::Operand parser::parse_operand() {
  PrimaryExpr::Operand result;
  if (is_current(token_type::int_)) {
    result.set_int_value(lex.current_token().int_value().to_string(10));
    lex.next_token();
  } else if (is_current(token_type::identifier)) {
    set_identifier(result.mutable_identifier(), lex.current_token().string_value());
    lex.next_token();
  } else if (is_current(token_type::float_)) {
    result.set_float_value(lex.current_token().double_value());
    lex.next_token();
  } else if (is_current(token_type::string)) {
    result.set_string_value(lex.current_token().string_value());
    lex.next_token();
  } else if (is_current(token_type::bytes)) {
    result.set_bytes_value(lex.current_token().string_value());
    lex.next_token();
  } else if (is_current(token_type::lbracket)) {
    result = parse_list();
  } else if (is_current(token_type::lbrace)) {
    result = parse_dict();
  } else if (capture(token_type::lparen)) {
    *result.mutable_expression() = parse_expression(true);
    if (!expect(token_type::rparen)) {
      return result;
    }
  } else {
    add_error("Unexpected token");
    return result;
  }
  return result;
}

PrimaryExpr::Operand parser::parse_list() {
  PrimaryExpr::Operand result;
  if (!expect(token_type::lbracket)) {
    return result;
  }
  if (capture(token_type::rbracket)) {
    result.mutable_list_expression();
    return result;
  }

  Test expression = parse_test();
  switch (lex.current_token().type()) {
    case token_type::for_:
      *result.mutable_list_comprehension()->mutable_test() = expression;
      while (is_current(token_type::for_) || is_current(token_type::if_)) {
        *result.mutable_list_comprehension()->add_clause() = parse_comp_clause();
      }
      break;
    case token_type::rbracket:
    case token_type::comma:
      *result.mutable_list_expression()->add_element() = expression;
      while (capture(token_type::comma)) {
        if (is_current(token_type::rbracket)) {
          break;
        }
        *result.mutable_list_expression()->add_element() = parse_test();
      }
      break;
    default:
      break;
  }
  if (!expect(token_type::rbracket)) {
    return result;
  }
  return result;
}

PrimaryExpr::Operand parser::parse_dict() {
  PrimaryExpr::Operand result;
  if (!expect(token_type::lbrace)) {
    return result;
  }
  if (capture(token_type::rbrace)) {
    result.mutable_dictionary_expression();
    return result;
  }

  PrimaryExpr::Operand::Entry entry = parse_entry();
  switch (lex.current_token().type()) {
    case token_type::for_:
      *result.mutable_dictionary_comprehension()->mutable_entry() = entry;
      while (is_current(token_type::for_) || is_current(token_type::if_)) {
        *result.mutable_dictionary_comprehension()->add_clause() = parse_comp_clause();
      }
      break;
    case token_type::rbrace:
    case token_type::comma:
      *result.mutable_dictionary_expression()->add_entry() = entry;
      while (capture(token_type::comma)) {
        if (is_current(token_type::rbrace)) {
          break;
        }
        *result.mutable_dictionary_expression()->add_entry() = parse_entry();
      }
      break;
    default:
      break;
  }
  if (!expect(token_type::rbrace)) {
    return result;
  }
  return result;
}

PrimaryExpr::Operand::Entry parser::parse_entry() {
  PrimaryExpr::Operand::Entry result;
  *result.mutable_key() = parse_test();
  if (!expect(token_type::colon)) {
    return result;
  }
  *result.mutable_value() = parse_test();
  return result;
}

PrimaryExpr::Operand::CompClause parser::parse_comp_clause() {
  PrimaryExpr::Operand::CompClause result;
  if (capture(token_type::for_)) {
    do {
      *result.mutable_for_clause()->add_loop_variable() = parse_primary();
      if (!is_target(*result.for_clause().loop_variable().rbegin())) {
        add_error("Expecting TARGET");
        return result;
      }
    } while (capture(token_type::comma));
    if (!expect(token_type::in)) {
      return result;
    }
    // Do not allow `IfExpr` nor lambdas.
    *result.mutable_for_clause()->mutable_in() = parse_test(0);
  } else if (capture(token_type::if_)) {
    // Have to avoid parsing this as an `IfExpr`.
    // This is also not allowing a lambda to be used.
    // Context: https://github.com/bazelbuild/bazel/issues/24469
    *result.mutable_if_clause() = parse_test(0);
  } else {
    add_error("Expected `for` or `if`.");
    return result;
  }
  return result;
}

PrimaryExpr::CallExpr::Argument parser::parse_argument() {
  PrimaryExpr::CallExpr::Argument result;
  if (capture(token_type::star)) {
    *result.mutable_star_argument() = parse_test();
  } else if (capture(token_type::star_star)) {
    *result.mutable_star_star_argument() = parse_test();
  } else {
    auto argument = parse_test();
    if (capture(token_type::equals)) {
      if (!argument.primary_expression().operand().has_identifier()) {
        add_error("Expected identifier for named arguments");
        return result;
      }
      *result.mutable_named_argument()->mutable_identifier() = argument.primary_expression().operand().identifier();
      *result.mutable_named_argument()->mutable_value() = parse_test();
    } else {
      argument.Swap(result.mutable_value());
    }
  }
  return result;
}

Test::LambdaExpr parser::parse_lambda() {
  Test::LambdaExpr result;
  Test::LambdaExpr* result_ref = &result;
  while (capture(token_type::lambda)) {
    parse_parameters(*result_ref->mutable_parameter(), false);
    if (!expect(token_type::colon)) {
      return result;
    }
    if (is_current(token_type::lambda)) {
      result_ref = result_ref->mutable_test()->mutable_lambda_expression();
    } else {
      *result_ref->mutable_test() = parse_test();
    }
  }
  return result;
}

void parser::parse_parameters(google::protobuf::RepeatedPtrField<starlark::Parameter>& parameters,
                              bool allow_trailing_comma) {
  bool found_parameter = false;
  for (;;) {
    if (is_current(token_type::identifier)) {
      Parameter* param = parameters.Add();
      set_identifier(param->mutable_identifier(), lex.current_token().string_value());
      lex.next_token();
      if (capture(token_type::equals)) {
        *param->mutable_initialization() =  parse_test();
      }
      found_parameter = true;
    } else if (capture(token_type::star)) {
      Parameter* param = parameters.Add();
      param->mutable_star();
      if (is_current(token_type::identifier)) {
        set_identifier(param->mutable_identifier(), lex.current_token().string_value());
        lex.next_token();
      }
      found_parameter = true;
    } else if (capture(token_type::star_star)) {
      Parameter* param = parameters.Add();
      param->mutable_star_star();
      if (is_current(token_type::identifier)) {
        set_identifier(param->mutable_identifier(), lex.current_token().string_value());
        lex.next_token();
      } else {
        add_error("Expected identifier after STAR_STAR when parsing parameters");
        return;
      }
      found_parameter = true;
    } else {
      if (found_parameter && !allow_trailing_comma) {
        add_error("Unexpected COMMA");
        return;
      }
      break;
    }
    if (!capture(token_type::comma)) {
      break;
    }
  }
}

}  // namespace grammar

