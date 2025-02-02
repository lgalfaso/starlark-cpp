// Copyright 2024-2025 Lucas Mirelmann

#include "grammar/ast_listener.hpp"

#include <vector>

using google::protobuf::RepeatedPtrField;
using starlark::Argument;
using starlark::AssignStmt;
using starlark::BinaryExpr;
using starlark::BreakStmt;
using starlark::CallExpr;
using starlark::CompClause;
using starlark::ContinueStmt;
using starlark::DefStmt;
using starlark::DictComp;
using starlark::DictExpr;
using starlark::DotExpr;
using starlark::ElseIf;
using starlark::Entry;
using starlark::Expression;
using starlark::File;
using starlark::ForClause;
using starlark::ForStmt;
using starlark::Identifier;
using starlark::IfExpr;
using starlark::IfStmt;
using starlark::LambdaExpr;
using starlark::ListComp;
using starlark::ListExpr;
using starlark::LoadStmt;
using starlark::Parameter;
using starlark::PassStmt;
using starlark::PrimaryExpr;
using starlark::ReturnStmt;
using starlark::SliceExpr;
using starlark::Statement;
using starlark::Test;
using starlark::Tuple;
using starlark::UnaryExpr;

namespace grammar {

enum class message_type {
  FILE,
  STATEMENT,
  DEF_STATEMENT,
  IF_STATEMENT,
  FOR_STATEMENT,
  RETURN_STATEMENT,
  BREAK_STATEMENT,
  CONTINUE_STATEMENT,
  PASS_STATEMENT,
  ASSIGN_STATEMENT,
  EXPRESSION_STATEMENT,
  LOAD_STATEMENT,
  PARAMETER,
  ARGUMENT,
  EXPRESSION,
  TEST,
  TUPLE,
  IF_EXPRESSION,
  PRIMARY_EXPRESSION,
  UNARY_EXPRESSION,
  BINARY_EXPRESSION,
  LAMBDA_EXPRESSION,
  THEN,
  ELIF,
  ELSE,
  FOR_LOOP_VARIABLES,
  FOR_LOOP_VARIABLE,
  FOR_IN_EXPRESSION,
  DOT_EXPRESSION,
  CALL_EXPRESSION,
  SLICE_EXPRESSION,
  IDENTIFIER,
  INT_VALUE,
  FLOAT_VALUE,
  STRING_VALUE,
  BYTES_VALUE,
  LIST_EXPRESSION,
  LIST_COMPREHENSION,
  DICTIONARY_EXPRESSION,
  DICTIONARY_COMPREHENSION,
  COMP_CLAUSE,
  FOR_CLAUSE,
  FOR_IN_TEST,
  IF_CLAUSE,
  MAP_ENTRY,
};

struct message {
  union {
    const File* file;
    const Statement* statement;
    const DefStmt* def_statement;
    const IfStmt* if_statement;
    const ForStmt* for_statement;
    const ReturnStmt* return_statement;
    const BreakStmt* break_statement;
    const ContinueStmt* continue_statement;
    const PassStmt* pass_statement;
    const AssignStmt* assign_statement;
    const Expression* expression_statement;
    const LoadStmt* load_statement;
    const Parameter* parameter;
    const Argument* argument;
    const Expression* expression;
    const Test* test;
    const Tuple* tuple;
    const IfExpr* if_expression;
    const PrimaryExpr* primary_expression;
    const UnaryExpr* unary_expression;
    const BinaryExpr* binary_expression;
    const LambdaExpr* lambda_expression;
    const RepeatedPtrField<Statement>* then;
    const RepeatedPtrField<Statement>* else_;
    const ElseIf* elif;
    const RepeatedPtrField<PrimaryExpr>* for_loop_variables;
    const PrimaryExpr* for_loop_variable;
    const Expression* for_in_expression;
    const DotExpr* dot_expression;
    const CallExpr* call_expression;
    const SliceExpr* slice_expression;
    const Identifier* identifier;
    const std::string* int_value;
    const double float_value;
    const std::string* string_value;
    const std::string* bytes_value;
    const ListExpr* list_expression;
    const ListComp* list_comprehension;
    const DictExpr* dictionary_expression;
    const DictComp* dictionary_comprehension;
    const CompClause* comp_clause;
    const ForClause* for_clause;
    const Test* for_in_test;
    const Test* if_clause;
    const Entry* map_entry;
  };
  message_type type;
  bool enter = false;
};

void ast_listener_base::enter_file(const File* starlark_file) {}
void ast_listener_base::exit_file(const File* starlark_file) {}
void ast_listener_base::enter_statement(const Statement* statement) {}
void ast_listener_base::exit_statement(const Statement* statement) {}
void ast_listener_base::enter_def_statement(const DefStmt* def_statement) {}
void ast_listener_base::exit_def_statement(const DefStmt* def_statement) {}
void ast_listener_base::enter_if_statement(const IfStmt* if_statement) {}
void ast_listener_base::exit_if_statement(const IfStmt* if_statement) {}
void ast_listener_base::enter_for_statement(const ForStmt* for_statement) {}
void ast_listener_base::exit_for_statement(const ForStmt* for_statement) {}
void ast_listener_base::enter_return_statement(const ReturnStmt* return_statement) {}
void ast_listener_base::exit_return_statement(const ReturnStmt* return_statement) {}
void ast_listener_base::enter_break_statement(const BreakStmt* break_statement) {}
void ast_listener_base::exit_break_statement(const BreakStmt* break_statement) {}
void ast_listener_base::enter_continue_statement(const ContinueStmt* continue_statement) {}
void ast_listener_base::exit_continue_statement(const ContinueStmt* continue_statement) {}
void ast_listener_base::enter_pass_statement(const PassStmt* pass_statement) {}
void ast_listener_base::exit_pass_statement(const PassStmt* pass_statement) {}
void ast_listener_base::enter_assign_statement(const AssignStmt* assign_statement) {}
void ast_listener_base::exit_assign_statement(const AssignStmt* assign_statement) {}
void ast_listener_base::enter_expression_statement(const Expression* expression_statement) {}
void ast_listener_base::exit_expression_statement(const Expression* expresion_statement) {}
void ast_listener_base::enter_load_statement(const LoadStmt* load_statement) {}
void ast_listener_base::exit_load_statement(const LoadStmt* load_statement) {}
void ast_listener_base::enter_parameter(const Parameter* parameter) {}
void ast_listener_base::exit_parameter(const Parameter* parameter) {}
void ast_listener_base::enter_argument(const starlark::Argument* argument) {}
void ast_listener_base::exit_argument(const starlark::Argument* argument) {}
void ast_listener_base::enter_then(const RepeatedPtrField<Statement>* then) {}
void ast_listener_base::exit_then(const RepeatedPtrField<Statement>* then) {}
void ast_listener_base::enter_elif(const ElseIf* elif) {}
void ast_listener_base::exit_elif(const ElseIf* elif) {}
void ast_listener_base::enter_else(const RepeatedPtrField<Statement>* else_) {}
void ast_listener_base::exit_else(const RepeatedPtrField<Statement>* else_) {}
void ast_listener_base::enter_expression(const Expression* expression) {}
void ast_listener_base::exit_expression(const Expression* expresion) {}
void ast_listener_base::enter_test(const Test* test) {}
void ast_listener_base::exit_test(const Test* test) {}
void ast_listener_base::enter_tuple(const Tuple* tuple) {}
void ast_listener_base::exit_tuple(const Tuple* tuple) {}
void ast_listener_base::enter_if_expression(const starlark::IfExpr* if_expression) {}
void ast_listener_base::exit_if_expression(const starlark::IfExpr* if_expression) {}
void ast_listener_base::enter_primary_expression(const starlark::PrimaryExpr* primary_expression) {}
void ast_listener_base::exit_primary_expression(const starlark::PrimaryExpr* primary_expression) {}
void ast_listener_base::enter_unary_expression(const starlark::UnaryExpr* unary_expression) {}
void ast_listener_base::exit_unary_expression(const starlark::UnaryExpr* unary_expression) {}
void ast_listener_base::enter_binary_expression(const starlark::BinaryExpr* binary_expression) {}
void ast_listener_base::exit_binary_expression(const starlark::BinaryExpr* binary_expression) {}
void ast_listener_base::enter_lambda_expression(const starlark::LambdaExpr* lambda_expression) {}
void ast_listener_base::exit_lambda_expression(const starlark::LambdaExpr* lambda_expression) {}
void ast_listener_base::enter_for_loop_variables(const google::protobuf::RepeatedPtrField<starlark::PrimaryExpr>* loop_variables) {}
void ast_listener_base::exit_for_loop_variables(const google::protobuf::RepeatedPtrField<starlark::PrimaryExpr>* loop_variables) {}
void ast_listener_base::enter_for_loop_variable(const starlark::PrimaryExpr* loop_variable) {}
void ast_listener_base::exit_for_loop_variable(const starlark::PrimaryExpr* loop_variable) {}
void ast_listener_base::enter_for_in_expression(const starlark::Expression* expression) {}
void ast_listener_base::exit_for_in_expression(const starlark::Expression* expression) {}
void ast_listener_base::enter_for_in_test(const starlark::Test* test) {}
void ast_listener_base::exit_for_in_test(const starlark::Test* test) {}
void ast_listener_base::enter_dot_expression(const starlark::DotExpr* dot_expression) {}
void ast_listener_base::exit_dot_expression(const starlark::DotExpr* dot_expression) {}
void ast_listener_base::enter_call_expression(const starlark::CallExpr* call_expression) {}
void ast_listener_base::exit_call_expression(const starlark::CallExpr* call_expression) {}
void ast_listener_base::enter_slice_expression(const starlark::SliceExpr* slice_expression) {}
void ast_listener_base::exit_slice_expression(const starlark::SliceExpr* slice_expression) {}
void ast_listener_base::enter_identifier(const starlark::Identifier* identifier) {}
void ast_listener_base::exit_identifier(const starlark::Identifier* identifier) {}
void ast_listener_base::enter_int_value(const std::string* int_value) {}
void ast_listener_base::exit_int_value(const std::string* int_value) {}
void ast_listener_base::enter_float_value(double float_value) {}
void ast_listener_base::exit_float_value(double float_value) {}
void ast_listener_base::enter_string_value(const std::string* string_value) {}
void ast_listener_base::exit_string_value(const std::string* string_value) {}
void ast_listener_base::enter_bytes_value(const std::string* bytes_value) {}
void ast_listener_base::exit_bytes_value(const std::string* bytes_value) {}
void ast_listener_base::enter_list_expression(const starlark::ListExpr* list_expression) {}
void ast_listener_base::exit_list_expression(const starlark::ListExpr* list_expression) {}
void ast_listener_base::enter_list_comprehension(const starlark::ListComp* list_comprehension) {}
void ast_listener_base::exit_list_comprehension(const starlark::ListComp* list_comprehension) {}
void ast_listener_base::enter_dictionary_expression(const starlark::DictExpr* dictionary_expression) {}
void ast_listener_base::exit_dictionary_expression(const starlark::DictExpr* dictionary_expression) {}
void ast_listener_base::enter_dictionary_comprehension(const starlark::DictComp* dictionary_comprehension) {}
void ast_listener_base::exit_dictionary_comprehension(const starlark::DictComp* dictionary_comprehension) {}
void ast_listener_base::enter_comp_clause(const starlark::CompClause* comp_clause) {}
void ast_listener_base::exit_comp_clause(const starlark::CompClause* comp_clause) {}
void ast_listener_base::enter_for_clause(const starlark::ForClause* for_clause) {}
void ast_listener_base::exit_for_clause(const starlark::ForClause* for_clause) {}
void ast_listener_base::enter_if_clause(const starlark::Test* if_clause) {}
void ast_listener_base::exit_if_clause(const starlark::Test* if_clause) {}
void ast_listener_base::enter_map_entry(const starlark::Entry* map_entry) {}
void ast_listener_base::exit_map_entry(const starlark::Entry* map_entry) {}

void ast_walker::walk(const starlark::File* starlark_file, ast_listener& listener) {
  std::vector<message> to_process;

  auto add_statements = [&to_process](const RepeatedPtrField<Statement>& statements) {
    for (auto it = statements.rbegin(); it != statements.rend(); ++it) {
      to_process.push_back(message{
        .statement = &*it,
        .type = message_type::STATEMENT,
        .enter = true,
      });
    }
  };

  to_process.push_back(message{
    .file = starlark_file,
    .type = message_type::FILE,
    .enter = true,
  });
  while (!to_process.empty()) {
    auto top = to_process.back();
    to_process.pop_back();
    if (top.enter) {
      message exit_message = top;
      exit_message.enter = false;
      to_process.push_back(exit_message);
      switch (top.type) {
        case message_type::FILE:
          listener.enter_file(top.file);
          add_statements(top.file->statement());
          break;
        case message_type::STATEMENT:
          listener.enter_statement(top.statement);
          switch (top.statement->statement_type_case()) {
            case Statement::kDefStatement:
              to_process.push_back(message{
                .def_statement = &top.statement->def_statement(),
                .type = message_type::DEF_STATEMENT,
                .enter = true,
              });
              break;
            case Statement::kIfStatement:
              to_process.push_back(message{
                .if_statement = &top.statement->if_statement(),
                .type = message_type::IF_STATEMENT,
                .enter = true,
              });
              break;
            case Statement::kForStatement:
              to_process.push_back(message{
                .for_statement = &top.statement->for_statement(),
                .type = message_type::FOR_STATEMENT,
                .enter = true,
              });
              break;
            case Statement::kReturnStatement:
              to_process.push_back(message{
                .return_statement = &top.statement->return_statement(),
                .type = message_type::RETURN_STATEMENT,
                .enter = true,
              });
              break;
            case Statement::kBreakStatement:
              to_process.push_back(message{
                .break_statement = &top.statement->break_statement(),
                .type = message_type::BREAK_STATEMENT,
                .enter = true,
              });
              break;
            case Statement::kContinueStatement:
              to_process.push_back(message{
                .continue_statement = &top.statement->continue_statement(),
                .type = message_type::CONTINUE_STATEMENT,
                .enter = true,
              });
              break;
            case Statement::kPassStatement:
              to_process.push_back(message{
                .pass_statement = &top.statement->pass_statement(),
                .type = message_type::PASS_STATEMENT,
                .enter = true,
              });
              break;
            case Statement::kAssignStatement:
              to_process.push_back(message{
                .assign_statement = &top.statement->assign_statement(),
                .type = message_type::ASSIGN_STATEMENT,
                .enter = true,
              });
              break;
            case Statement::kExpressionStatement:
              to_process.push_back(message{
                .expression_statement = &top.statement->expression_statement(),
                .type = message_type::EXPRESSION_STATEMENT,
                .enter = true,
              });
              break;
            case Statement::kLoadStatement:
              to_process.push_back(message{
                .load_statement = &top.statement->load_statement(),
                .type = message_type::LOAD_STATEMENT,
                .enter = true,
              });
              break;
            case Statement::STATEMENT_TYPE_NOT_SET:
              break;
          }
          break;
        case message_type::DEF_STATEMENT:
          listener.enter_def_statement(top.def_statement);
          add_statements(top.def_statement->statement());
          for (auto it = top.def_statement->parameter().rbegin(); it != top.def_statement->parameter().rend(); ++it) {
            to_process.push_back(message{
              .parameter = &*it,
              .type = message_type::PARAMETER,
              .enter = true,
            });
          }
          break;
        case message_type::IF_STATEMENT:
          listener.enter_if_statement(top.if_statement);
          if (!top.if_statement->else_statement().empty()) {
            to_process.push_back(message{
              .else_ = &top.if_statement->else_statement(),
              .type = message_type::ELSE,
              .enter = true,
            });
          }
          for (auto it = top.if_statement->elif().rbegin(); it != top.if_statement->elif().rend(); ++it) {
            to_process.push_back(message{
              .elif = &*it,
              .type = message_type::ELIF,
              .enter = true,
            });
          }
          to_process.push_back(message{
            .then = &top.if_statement->statement(),
            .type = message_type::THEN,
            .enter = true,
          });
          to_process.push_back(message{
            .test = &top.if_statement->test(),
            .type = message_type::TEST,
            .enter = true,
          });
          break;
        case message_type::THEN:
          listener.enter_then(top.then);
          add_statements(*top.then);
          break;
        case message_type::ELIF:
          listener.enter_elif(top.elif);
          to_process.push_back(message{
            .then = &top.elif->statement(),
            .type = message_type::THEN,
            .enter = true,
          });
          to_process.push_back(message{
            .test = &top.elif->test(),
            .type = message_type::TEST,
            .enter = true,
          });
          break;
        case message_type::ELSE:
          listener.enter_else(top.else_);
          add_statements(*top.else_);
          break;
        case message_type::FOR_STATEMENT:
          listener.enter_for_statement(top.for_statement);
          add_statements(top.for_statement->statement());
          to_process.push_back(message{
            .for_in_expression = &top.for_statement->expression(),
            .type = message_type::FOR_IN_EXPRESSION,
            .enter = true,
          });
          to_process.push_back(message{
            .for_loop_variables = &top.for_statement->loop_variable(),
            .type = message_type::FOR_LOOP_VARIABLES,
            .enter = true,
          });
          break;
        case message_type::RETURN_STATEMENT:
          listener.enter_return_statement(top.return_statement);
          if (top.return_statement->has_expression()) {
            to_process.push_back(message{
              .expression = &top.return_statement->expression(),
              .type = message_type::EXPRESSION,
              .enter = true,
            });
          }
          break;
        case message_type::FOR_LOOP_VARIABLES:
          listener.enter_for_loop_variables(top.for_loop_variables);
          for (auto it = top.for_loop_variables->rbegin(); it != top.for_loop_variables->rend(); ++it) {
            to_process.push_back(message{
              .for_loop_variable = &*it,
              .type = message_type::FOR_LOOP_VARIABLE,
              .enter = true,
            });
          }
          break;
        case message_type::FOR_LOOP_VARIABLE:
          listener.enter_for_loop_variable(top.for_loop_variable);
          to_process.push_back(message{
            .primary_expression = top.for_loop_variable,
            .type = message_type::PRIMARY_EXPRESSION,
            .enter = true,
          });
          break;
        case message_type::FOR_IN_EXPRESSION:
          listener.enter_for_in_expression(top.for_in_expression);
          to_process.push_back(message{
            .expression = top.for_in_expression,
            .type = message_type::EXPRESSION,
            .enter = true,
          });
          break;
        case message_type::FOR_IN_TEST:
          listener.enter_for_in_test(top.for_in_test);
          to_process.push_back(message{
            .test = top.for_in_test,
            .type = message_type::TEST,
            .enter = true,
          });
          break;
        case message_type::BREAK_STATEMENT:
          listener.enter_break_statement(top.break_statement);
          break;
        case message_type::CONTINUE_STATEMENT:
          listener.enter_continue_statement(top.continue_statement);
          break;
        case message_type::PASS_STATEMENT:
          listener.enter_pass_statement(top.pass_statement);
          break;
        case message_type::ASSIGN_STATEMENT:
          listener.enter_assign_statement(top.assign_statement);
          to_process.push_back(message{
            .expression = &top.assign_statement->rhs(),
            .type = message_type::EXPRESSION,
            .enter = true,
          });
          to_process.push_back(message{
            .expression = &top.assign_statement->lhs(),
            .type = message_type::EXPRESSION,
            .enter = true,
          });
          break;
        case message_type::EXPRESSION_STATEMENT:
          listener.enter_expression_statement(top.expression_statement);
          to_process.push_back(message{
            .expression = top.expression_statement,
            .type = message_type::EXPRESSION,
            .enter = true,
          });
          break;
        case message_type::LOAD_STATEMENT:
          listener.enter_load_statement(top.load_statement);
          break;
        case message_type::PARAMETER:
          listener.enter_parameter(top.parameter);
          if (top.parameter->has_initialization()) {
            to_process.push_back(message{
              .test = &top.parameter->initialization(),
              .type = message_type::TEST,
              .enter = true,
            });
          }
          break;
        case message_type::ARGUMENT:
          listener.enter_argument(top.argument);
          switch (top.argument->argument_type_case()) {
            case Argument::kValue:
              to_process.push_back(message{
                .test = &top.argument->value(),
                .type = message_type::TEST,
                .enter = true,
              });
              break;
            case Argument::kNamedArgument:
              to_process.push_back(message{
                .test = &top.argument->named_argument().value(),
                .type = message_type::TEST,
                .enter = true,
              });
              break;
            case Argument::kStarArgument:
              to_process.push_back(message{
                .test = &top.argument->star_argument(),
                .type = message_type::TEST,
                .enter = true,
              });
              break;
            case Argument::kStarStarArgument:
              to_process.push_back(message{
                .test = &top.argument->star_star_argument(),
                .type = message_type::TEST,
                .enter = true,
              });
              break;
            case Argument::ARGUMENT_TYPE_NOT_SET:
              break;
          }
          break;
        case message_type::EXPRESSION:
          listener.enter_expression(top.expression);
          switch (top.expression->expression_type_case()) {
            case Expression::kValue:
            to_process.push_back(message{
              .test = &top.expression->value(),
              .type = message_type::TEST,
              .enter = true,
            });
            break;
            case Expression::kTuple:
            to_process.push_back(message{
              .tuple = &top.expression->tuple(),
              .type = message_type::TUPLE,
              .enter = true,
            });
            break;
            case Expression::EXPRESSION_TYPE_NOT_SET:
            break;
          }
          break;
        case message_type::TEST:
          listener.enter_test(top.test);
          switch (top.test->test_type_case()) {
            case Test::kIfExpression:
              to_process.push_back(message{
                .if_expression = &top.test->if_expression(),
                .type = message_type::IF_EXPRESSION,
                .enter = true,
              });
              break;
            case Test::kPrimaryExpression:
              to_process.push_back(message{
                .primary_expression = &top.test->primary_expression(),
                .type = message_type::PRIMARY_EXPRESSION,
                .enter = true,
              });
              break;
            case Test::kUnaryExpression:
              to_process.push_back(message{
                .unary_expression = &top.test->unary_expression(),
                .type = message_type::UNARY_EXPRESSION,
                .enter = true,
              });
              break;
            case Test::kBinaryExpression:
              to_process.push_back(message{
                .binary_expression = &top.test->binary_expression(),
                .type = message_type::BINARY_EXPRESSION,
                .enter = true,
              });
              break;
            case Test::kLambdaExpression:
              to_process.push_back(message{
                .lambda_expression = &top.test->lambda_expression(),
                .type = message_type::LAMBDA_EXPRESSION,
                .enter = true,
              });
              break;
            case Test::TEST_TYPE_NOT_SET:
              break;
          }
          break;
        case message_type::TUPLE:
          listener.enter_tuple(top.tuple);
          for (auto it = top.tuple->value().rbegin(); it != top.tuple->value().rend(); ++it) {
            to_process.push_back(message{
              .test = &*it,
              .type = message_type::TEST,
              .enter = true,
            });
          }
          break;
        case message_type::IF_EXPRESSION:
          listener.enter_if_expression(top.if_expression);
          // The events follow an `if-else` and not the order that things show up in the grammar.
          to_process.push_back(message{
            .test = &top.if_expression->else_value(),
            .type = message_type::TEST,
            .enter = true,
          });
          to_process.push_back(message{
            .test = &top.if_expression->if_value(),
            .type = message_type::TEST,
            .enter = true,
          });
          to_process.push_back(message{
            .test = &top.if_expression->if_test(),
            .type = message_type::TEST,
            .enter = true,
          });
          break;
        case message_type::PRIMARY_EXPRESSION:
          listener.enter_primary_expression(top.primary_expression);
          switch (top.primary_expression->primary_expression_type_case()) {
            case PrimaryExpr::kDotExpression:
              to_process.push_back(message{
                .dot_expression = &top.primary_expression->dot_expression(),
                .type = message_type::DOT_EXPRESSION,
                .enter = true,
              });
              break;
            case PrimaryExpr::kCallExpression:
              to_process.push_back(message{
                .call_expression = &top.primary_expression->call_expression(),
                .type = message_type::CALL_EXPRESSION,
                .enter = true,
              });
              break;
            case PrimaryExpr::kSliceExpression:
              to_process.push_back(message{
                .slice_expression = &top.primary_expression->slice_expression(),
                .type = message_type::SLICE_EXPRESSION,
                .enter = true,
              });
              break;
            case PrimaryExpr::kIdentifier:
              to_process.push_back(message{
                .identifier = &top.primary_expression->identifier(),
                .type = message_type::IDENTIFIER,
                .enter = true,
              });
              break;
            case PrimaryExpr::kIntValue:
              to_process.push_back(message{
                .int_value = &top.primary_expression->int_value(),
                .type = message_type::INT_VALUE,
                .enter = true,
              });
              break;
            case PrimaryExpr::kFloatValue:
              to_process.push_back(message{
                .float_value = top.primary_expression->float_value(),
                .type = message_type::FLOAT_VALUE,
                .enter = true,
              });
              break;
            case PrimaryExpr::kStringValue:
              to_process.push_back(message{
                .string_value = &top.primary_expression->string_value(),
                .type = message_type::STRING_VALUE,
                .enter = true,
              });
              break;
            case PrimaryExpr::kBytesValue:
              to_process.push_back(message{
                .bytes_value = &top.primary_expression->bytes_value(),
                .type = message_type::BYTES_VALUE,
                .enter = true,
              });
              break;
            case PrimaryExpr::kListExpression:
              to_process.push_back(message{
                .list_expression = &top.primary_expression->list_expression(),
                .type = message_type::LIST_EXPRESSION,
                .enter = true,
              });
              break;
            case PrimaryExpr::kListComprehension:
              to_process.push_back(message{
                .list_comprehension = &top.primary_expression->list_comprehension(),
                .type = message_type::LIST_COMPREHENSION,
                .enter = true,
              });
              break;
            case PrimaryExpr::kDictionaryExpression:
              to_process.push_back(message{
                .dictionary_expression = &top.primary_expression->dictionary_expression(),
                .type = message_type::DICTIONARY_EXPRESSION,
                .enter = true,
              });
              break;
            case PrimaryExpr::kDictionaryComprehension:
              to_process.push_back(message{
                .dictionary_comprehension = &top.primary_expression->dictionary_comprehension(),
                .type = message_type::DICTIONARY_COMPREHENSION,
                .enter = true,
              });
              break;
            case PrimaryExpr::kExpression:
              to_process.push_back(message{
                .expression = &top.primary_expression->expression(),
                .type = message_type::EXPRESSION,
                .enter = true,
              });
              break;
            case PrimaryExpr::PRIMARY_EXPRESSION_TYPE_NOT_SET:
              break;
          }
          break;
        case message_type::UNARY_EXPRESSION:
          listener.enter_unary_expression(top.unary_expression);
          to_process.push_back(message{
            .test = &top.unary_expression->test(),
            .type = message_type::TEST,
            .enter = true,
          });
          break;
        case message_type::BINARY_EXPRESSION:
          listener.enter_binary_expression(top.binary_expression);
          to_process.push_back(message{
            .test = &top.binary_expression->rhs(),
            .type = message_type::TEST,
            .enter = true,
          });
          to_process.push_back(message{
            .test = &top.binary_expression->lhs(),
            .type = message_type::TEST,
            .enter = true,
          });
          break;
        case message_type::LAMBDA_EXPRESSION:
          listener.enter_lambda_expression(top.lambda_expression);
          to_process.push_back(message{
            .test = &top.lambda_expression->test(),
            .type = message_type::TEST,
            .enter = true,
          });
          for (auto it = top.lambda_expression->parameter().rbegin(); it != top.lambda_expression->parameter().rend(); ++it) {
            to_process.push_back(message{
              .parameter = &*it,
              .type = message_type::PARAMETER,
              .enter = true,
            });
          }
          break;
        case message_type::DOT_EXPRESSION:
          listener.enter_dot_expression(top.dot_expression);
          to_process.push_back(message{
            .primary_expression = &top.dot_expression->primary_expression(),
            .type = message_type::PRIMARY_EXPRESSION,
            .enter = true,
          });
          break;
        case message_type::CALL_EXPRESSION:
          listener.enter_call_expression(top.call_expression);
          for (auto it = top.call_expression->argument().rbegin(); it != top.call_expression->argument().rend(); ++it) {
            to_process.push_back(message{
              .argument = &*it,
              .type = message_type::ARGUMENT,
              .enter = true,
            });
          }
          to_process.push_back(message{
            .primary_expression = &top.call_expression->primary_expression(),
            .type = message_type::PRIMARY_EXPRESSION,
            .enter = true,
          });
          break;
        case message_type::SLICE_EXPRESSION:
          listener.enter_slice_expression(top.slice_expression);
          switch (top.slice_expression->slice_type_case()) {
            case SliceExpr::kIndex:
              to_process.push_back(message{
                .expression = &top.slice_expression->index(),
                .type = message_type::EXPRESSION,
                .enter = true,
              });
              break;
            case SliceExpr::kSlice:
              if (top.slice_expression->slice().has_step()) {
                to_process.push_back(message{
                  .test = &top.slice_expression->slice().step(),
                  .type = message_type::TEST,
                  .enter = true,
                });
              }
              if (top.slice_expression->slice().has_end()) {
                to_process.push_back(message{
                  .test = &top.slice_expression->slice().end(),
                  .type = message_type::TEST,
                  .enter = true,
                });
              }
              if (top.slice_expression->slice().has_start()) {
                to_process.push_back(message{
                  .test = &top.slice_expression->slice().start(),
                  .type = message_type::TEST,
                  .enter = true,
                });
              }
              break;
            case SliceExpr::SLICE_TYPE_NOT_SET:
              break;
          }
          to_process.push_back(message{
            .primary_expression = &top.slice_expression->primary_expression(),
            .type = message_type::PRIMARY_EXPRESSION,
            .enter = true,
          });
          break;
        case message_type::IDENTIFIER:
          listener.enter_identifier(top.identifier);
          break;
        case message_type::INT_VALUE:
          listener.enter_int_value(top.int_value);
          break;
        case message_type::FLOAT_VALUE:
          listener.enter_float_value(top.float_value);
          break;
        case message_type::STRING_VALUE:
          listener.enter_string_value(top.string_value);
          break;
        case message_type::BYTES_VALUE:
          listener.enter_bytes_value(top.bytes_value);
          break;
        case message_type::LIST_EXPRESSION:
          listener.enter_list_expression(top.list_expression);
          for (auto it = top.list_expression->element().rbegin(); it != top.list_expression->element().rend(); ++it) {
            to_process.push_back(message{
              .test = &*it,
              .type = message_type::TEST,
              .enter = true,
            });
          }
          break;
        case message_type::LIST_COMPREHENSION:
          listener.enter_list_comprehension(top.list_comprehension);
          to_process.push_back(message{
            .test = &top.list_comprehension->test(),
            .type = message_type::TEST,
            .enter = true,
          });
          for (auto it = top.list_comprehension->clause().rbegin(); it != top.list_comprehension->clause().rend(); ++it) {
            to_process.push_back(message{
              .comp_clause = &*it,
              .type = message_type::COMP_CLAUSE,
              .enter = true,
            });
          }
          break;
        case message_type::DICTIONARY_EXPRESSION:
          listener.enter_dictionary_expression(top.dictionary_expression);
          for (auto it = top.dictionary_expression->entry().rbegin(); it != top.dictionary_expression->entry().rend(); ++it) {
            to_process.push_back(message{
              .map_entry = &*it,
              .type = message_type::MAP_ENTRY,
              .enter = true,
            });
          }
          break;
        case message_type::DICTIONARY_COMPREHENSION:
          listener.enter_dictionary_comprehension(top.dictionary_comprehension);
          to_process.push_back(message{
            .map_entry = &top.dictionary_comprehension->entry(),
            .type = message_type::MAP_ENTRY,
            .enter = true,
          });
          for (auto it = top.dictionary_comprehension->clause().rbegin(); it != top.dictionary_comprehension->clause().rend(); ++it) {
            to_process.push_back(message{
              .comp_clause = &*it,
              .type = message_type::COMP_CLAUSE,
              .enter = true,
            });
          }
          break;
        case message_type::COMP_CLAUSE:
          listener.enter_comp_clause(top.comp_clause);
          switch (top.comp_clause->comp_clause_type_case()) {
            case CompClause::kForClause:
              to_process.push_back(message{
                .for_clause = &top.comp_clause->for_clause(),
                .type = message_type::FOR_CLAUSE,
                .enter = true,
              });
              break;
            case CompClause::kIfClause:
              to_process.push_back(message{
                .if_clause = &top.comp_clause->if_clause(),
                .type = message_type::IF_CLAUSE,
                .enter = true,
              });
              break;
            case CompClause::COMP_CLAUSE_TYPE_NOT_SET:
              break;
          }
          break;
        case message_type::FOR_CLAUSE:
          listener.enter_for_clause(top.for_clause);
          to_process.push_back(message{
            .for_in_test = &top.for_clause->in(),
            .type = message_type::FOR_IN_TEST,
            .enter = true,
          });
          to_process.push_back(message{
            .for_loop_variables = &top.for_clause->loop_variable(),
            .type = message_type::FOR_LOOP_VARIABLES,
            .enter = true,
          });
          break;
        case message_type::IF_CLAUSE:
          listener.enter_if_clause(top.if_clause);
          to_process.push_back(message{
            .test = top.if_clause,
            .type = message_type::TEST,
            .enter = true,
          });
          break;
        case message_type::MAP_ENTRY:
          listener.enter_map_entry(top.map_entry);
          to_process.push_back(message{
            .test = &top.map_entry->value(),
            .type = message_type::TEST,
            .enter = true,
          });
          to_process.push_back(message{
            .test = &top.map_entry->key(),
            .type = message_type::TEST,
            .enter = true,
          });
          break;
      }
    } else {
      switch (top.type) {
        case message_type::FILE:
          listener.exit_file(top.file);
          break;
        case message_type::STATEMENT:
          listener.exit_statement(top.statement);
          break;
        case message_type::DEF_STATEMENT:
          listener.exit_def_statement(top.def_statement);
          break;
        case message_type::IF_STATEMENT:
          listener.exit_if_statement(top.if_statement);
          break;
        case message_type::THEN:
          listener.exit_then(top.then);
          break;
        case message_type::ELIF:
          listener.exit_elif(top.elif);
          break;
        case message_type::ELSE:
          listener.exit_else(top.else_);
          break;
        case message_type::FOR_STATEMENT:
          listener.exit_for_statement(top.for_statement);
          break;
        case message_type::FOR_LOOP_VARIABLES:
          listener.exit_for_loop_variables(top.for_loop_variables);
          break;
        case message_type::FOR_LOOP_VARIABLE:
          listener.exit_for_loop_variable(top.for_loop_variable);
          break;
        case message_type::FOR_IN_EXPRESSION:
          listener.exit_for_in_expression(top.for_in_expression);
          break;
        case message_type::FOR_IN_TEST:
          listener.exit_for_in_test(top.for_in_test);
          break;
        case message_type::RETURN_STATEMENT:
          listener.exit_return_statement(top.return_statement);
          break;
        case message_type::BREAK_STATEMENT:
          listener.exit_break_statement(top.break_statement);
          break;
        case message_type::CONTINUE_STATEMENT:
          listener.exit_continue_statement(top.continue_statement);
          break;
        case message_type::PASS_STATEMENT:
          listener.exit_pass_statement(top.pass_statement);
          break;
        case message_type::ASSIGN_STATEMENT:
          listener.exit_assign_statement(top.assign_statement);
          break;
        case message_type::EXPRESSION_STATEMENT:
          listener.exit_expression_statement(top.expression_statement);
          break;
        case message_type::LOAD_STATEMENT:
          listener.exit_load_statement(top.load_statement);
          break;
        case message_type::PARAMETER:
          listener.exit_parameter(top.parameter);
          break;
        case message_type::ARGUMENT:
          listener.exit_argument(top.argument);
          break;
        case message_type::EXPRESSION:
          listener.exit_expression(top.expression);
          break;
        case message_type::TEST:
          listener.exit_test(top.test);
          break;
        case message_type::TUPLE:
          listener.exit_tuple(top.tuple);
          break;
        case message_type::IF_EXPRESSION:
          listener.exit_if_expression(top.if_expression);
          break;
        case message_type::PRIMARY_EXPRESSION:
          listener.exit_primary_expression(top.primary_expression);
          break;
        case message_type::UNARY_EXPRESSION:
          listener.exit_unary_expression(top.unary_expression);
          break;
        case message_type::BINARY_EXPRESSION:
          listener.exit_binary_expression(top.binary_expression);
          break;
        case message_type::LAMBDA_EXPRESSION:
          listener.exit_lambda_expression(top.lambda_expression);
          break;
        case message_type::DOT_EXPRESSION:
          listener.exit_dot_expression(top.dot_expression);
          break;
        case message_type::CALL_EXPRESSION:
          listener.exit_call_expression(top.call_expression);
          break;
        case message_type::SLICE_EXPRESSION:
          listener.exit_slice_expression(top.slice_expression);
          break;
        case message_type::IDENTIFIER:
          listener.exit_identifier(top.identifier);
          break;
        case message_type::INT_VALUE:
          listener.exit_int_value(top.int_value);
          break;
        case message_type::FLOAT_VALUE:
          listener.exit_float_value(top.float_value);
          break;
        case message_type::STRING_VALUE:
          listener.exit_string_value(top.string_value);
          break;
        case message_type::BYTES_VALUE:
          listener.exit_bytes_value(top.bytes_value);
          break;
        case message_type::LIST_EXPRESSION:
          listener.exit_list_expression(top.list_expression);
          break;
        case message_type::LIST_COMPREHENSION:
          listener.exit_list_comprehension(top.list_comprehension);
          break;
        case message_type::DICTIONARY_EXPRESSION:
          listener.exit_dictionary_expression(top.dictionary_expression);
          break;
        case message_type::DICTIONARY_COMPREHENSION:
          listener.exit_dictionary_comprehension(top.dictionary_comprehension);
          break;
        case message_type::COMP_CLAUSE:
          listener.exit_comp_clause(top.comp_clause);
          break;
        case message_type::FOR_CLAUSE:
          listener.exit_for_clause(top.for_clause);
          break;
        case message_type::IF_CLAUSE:
          listener.exit_if_clause(top.if_clause);
          break;
        case message_type::MAP_ENTRY:
          listener.exit_map_entry(top.map_entry);
          break;
      }
    }
  }
}

}  // namespace grammar

