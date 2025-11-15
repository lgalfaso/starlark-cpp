// Copyright 2024-2025 Lucas Mirelmann

#include "grammar/ast_listener.hpp"

#include <string>
#include <vector>

using google::protobuf::RepeatedPtrField;
using starlark::ast::Argument;
using starlark::ast::AssignStmt;
using starlark::ast::BinaryExpr;
using starlark::ast::BreakStmt;
using starlark::ast::CallExpr;
using starlark::ast::CompClause;
using starlark::ast::ContinueStmt;
using starlark::ast::DefStmt;
using starlark::ast::DictComp;
using starlark::ast::DictExpr;
using starlark::ast::DotExpr;
using starlark::ast::ElseIf;
using starlark::ast::Entry;
using starlark::ast::Expression;
using starlark::ast::File;
using starlark::ast::ForClause;
using starlark::ast::ForStmt;
using starlark::ast::Identifier;
using starlark::ast::IfExpr;
using starlark::ast::IfStmt;
using starlark::ast::LambdaExpr;
using starlark::ast::ListComp;
using starlark::ast::ListExpr;
using starlark::ast::LoadStmt;
using starlark::ast::Parameter;
using starlark::ast::PassStmt;
using starlark::ast::ReturnStmt;
using starlark::ast::SliceExpr;
using starlark::ast::Statement;
using starlark::ast::Tuple;
using starlark::ast::UnaryExpr;

namespace starlark {
namespace grammar {

enum class message_type {
  kStarlarkFile,
  kStatement,
  kDefStatement,
  kIfStatement,
  kForStatement,
  kReturnStatement,
  kBreakStatement,
  kContinueStatement,
  kPassStatement,
  kAssignStatement,
  kExpressionStatement,
  kLoadStatement,
  kParameter,
  kArgument,
  kExpression,
  kTuple,
  kIfExpression,
  kUnaryExpression,
  kBinaryExpression,
  kLambdaExpression,
  kThen,
  kElif,
  kElse,
  kForLoopVariables,
  kForInExpression,
  kDotExpression,
  kCallExpression,
  kSliceExpression,
  kIdentifier,
  kNoneValue,
  kIntValue,
  kBigIntValue,
  kFloatValue,
  kStringValue,
  kBytesValue,
  kListExpression,
  kListComprehension,
  kDictionaryExpression,
  kDictionaryComprehension,
  kCompClause,
  kForClause,
  kIfClause,
  kMapEntry,
};

enum class message_type_op {
  kEnter,
  kMid,
  kExit,
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
    const Tuple* tuple;
    const IfExpr* if_expression;
    const UnaryExpr* unary_expression;
    const BinaryExpr* binary_expression;
    const LambdaExpr* lambda_expression;
    const RepeatedPtrField<Statement>* then;
    const RepeatedPtrField<Statement>* else_;
    const ElseIf* elif;
    const Expression* for_loop_variables;
    const Expression* for_in_expression;
    const DotExpr* dot_expression;
    const CallExpr* call_expression;
    const SliceExpr* slice_expression;
    const Identifier* identifier;
    std::int64_t int_value;
    std::string_view big_int_value;
    const double float_value;
    std::string_view string_value;
    std::string_view bytes_value;
    const ListExpr* list_expression;
    const ListComp* list_comprehension;
    const DictExpr* dictionary_expression;
    const DictComp* dictionary_comprehension;
    const CompClause* comp_clause;
    const ForClause* for_clause;
    const Expression* if_clause;
    const Entry* map_entry;
  };
  message_type type;
  message_type_op op = message_type_op::kExit;
  bool for_assignment = false;
};

void ast_listener_base::enter_file(const File* starlark_file) {}
void ast_listener_base::exit_file(const File* starlark_file) {}
void ast_listener_base::enter_statement(const Statement* statement) {}
void ast_listener_base::exit_statement(const Statement* statement) {}
void ast_listener_base::enter_def_statement(const DefStmt* def_statement) {}
void ast_listener_base::mid_def_statement(const DefStmt* def_statement) {}
void ast_listener_base::exit_def_statement(const DefStmt* def_statement) {}
void ast_listener_base::enter_if_statement(const IfStmt* if_statement) {}
void ast_listener_base::exit_if_statement(const IfStmt* if_statement) {}
void ast_listener_base::enter_for_statement(const ForStmt* for_statement) {}
void ast_listener_base::mid_for_statement(const ForStmt* for_statement) {}
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
void ast_listener_base::enter_argument(const Argument* argument) {}
void ast_listener_base::exit_argument(const Argument* argument) {}
void ast_listener_base::enter_then(const RepeatedPtrField<Statement>* then) {}
void ast_listener_base::exit_then(const RepeatedPtrField<Statement>* then) {}
void ast_listener_base::enter_elif(const ElseIf* elif) {}
void ast_listener_base::exit_elif(const ElseIf* elif) {}
void ast_listener_base::enter_else(const RepeatedPtrField<Statement>* else_) {}
void ast_listener_base::exit_else(const RepeatedPtrField<Statement>* else_) {}
void ast_listener_base::enter_expression(const Expression* expression) {}
void ast_listener_base::exit_expression(const Expression* expresion) {}
void ast_listener_base::enter_tuple(const Tuple* tuple) {}
void ast_listener_base::exit_tuple(const Tuple* tuple) {}
void ast_listener_base::enter_tuple_for_assignment(const Tuple* tuple) {}
void ast_listener_base::exit_tuple_for_assignment(const Tuple* tuple) {}
void ast_listener_base::enter_if_expression(const IfExpr* if_expression) {}
void ast_listener_base::mid_if_expression(const IfExpr* if_expression) {}
void ast_listener_base::exit_if_expression(const IfExpr* if_expression) {}
void ast_listener_base::enter_unary_expression(const UnaryExpr* unary_expression) {}
void ast_listener_base::exit_unary_expression(const UnaryExpr* unary_expression) {}
void ast_listener_base::enter_binary_expression(const BinaryExpr* binary_expression) {}
void ast_listener_base::mid_binary_expression(const BinaryExpr* binary_expression) {}
void ast_listener_base::exit_binary_expression(const BinaryExpr* binary_expression) {}
void ast_listener_base::enter_lambda_expression(const LambdaExpr* lambda_expression) {}
void ast_listener_base::mid_lambda_expression(const LambdaExpr* lambda_expression) {}
void ast_listener_base::exit_lambda_expression(const LambdaExpr* lambda_expression) {}
void ast_listener_base::enter_for_loop_variables(const Expression* loop_variables) {}
void ast_listener_base::exit_for_loop_variables(const Expression* loop_variables) {}
void ast_listener_base::enter_for_in_expression(const Expression* expression) {}
void ast_listener_base::exit_for_in_expression(const Expression* expression) {}
void ast_listener_base::enter_dot_expression(const DotExpr* dot_expression) {}
void ast_listener_base::exit_dot_expression(const DotExpr* dot_expression) {}
void ast_listener_base::enter_dot_expression_for_assignment(const DotExpr* dot_expression) {}
void ast_listener_base::exit_dot_expression_for_assignment(const DotExpr* dot_expression) {}
void ast_listener_base::enter_call_expression(const CallExpr* call_expression) {}
void ast_listener_base::exit_call_expression(const CallExpr* call_expression) {}
void ast_listener_base::enter_slice_expression(const SliceExpr* slice_expression) {}
void ast_listener_base::exit_slice_expression(const SliceExpr* slice_expression) {}
void ast_listener_base::enter_slice_expression_for_assignment(const SliceExpr* slice_expression) {}
void ast_listener_base::exit_slice_expression_for_assignment(const SliceExpr* slice_expression) {}
void ast_listener_base::enter_identifier(const Identifier* identifier) {}
void ast_listener_base::exit_identifier(const Identifier* identifier) {}
void ast_listener_base::enter_identifier_for_assignment(const Identifier* identifier) {}
void ast_listener_base::exit_identifier_for_assignment(const Identifier* identifier) {}
void ast_listener_base::enter_none_value() {}
void ast_listener_base::exit_none_value() {}
void ast_listener_base::enter_int_value(std::int64_t int_value) {}
void ast_listener_base::exit_int_value(std::int64_t int_value) {}
void ast_listener_base::enter_big_int_value(std::string_view big_int_value) {}
void ast_listener_base::exit_big_int_value(std::string_view big_int_value) {}
void ast_listener_base::enter_float_value(double float_value) {}
void ast_listener_base::exit_float_value(double float_value) {}
void ast_listener_base::enter_string_value(std::string_view string_value) {}
void ast_listener_base::exit_string_value(std::string_view string_value) {}
void ast_listener_base::enter_bytes_value(std::string_view bytes_value) {}
void ast_listener_base::exit_bytes_value(std::string_view bytes_value) {}
void ast_listener_base::enter_list_expression(const ListExpr* list_expression) {}
void ast_listener_base::exit_list_expression(const ListExpr* list_expression) {}
void ast_listener_base::mid_list_expression(const ListExpr* list_expression) {}
void ast_listener_base::enter_list_expression_for_assignment(const ListExpr* list_expression) {}
void ast_listener_base::exit_list_expression_for_assignment(const ListExpr* list_expression) {}
void ast_listener_base::mid_list_expression_for_assignment(const ListExpr* list_expression) {}
void ast_listener_base::enter_list_comprehension(const ListComp* list_comprehension) {}
void ast_listener_base::exit_list_comprehension(const ListComp* list_comprehension) {}
void ast_listener_base::enter_dictionary_expression(const DictExpr* dictionary_expression) {}
void ast_listener_base::mid_dictionary_expression(const DictExpr* dictionary_expression) {}
void ast_listener_base::exit_dictionary_expression(const DictExpr* dictionary_expression) {}
void ast_listener_base::enter_dictionary_comprehension(const DictComp* dictionary_comprehension) {}
void ast_listener_base::exit_dictionary_comprehension(const DictComp* dictionary_comprehension) {}
void ast_listener_base::enter_comp_clause(const CompClause* comp_clause) {}
void ast_listener_base::exit_comp_clause(const CompClause* comp_clause) {}
void ast_listener_base::enter_for_clause(const ForClause* for_clause) {}
void ast_listener_base::mid_for_clause(const ForClause* for_clause) {}
void ast_listener_base::exit_for_clause(const ForClause* for_clause) {}
void ast_listener_base::enter_if_clause(const Expression* if_clause) {}
void ast_listener_base::exit_if_clause(const Expression* if_clause) {}
void ast_listener_base::enter_map_entry(const Entry* map_entry) {}
void ast_listener_base::exit_map_entry(const Entry* map_entry) {}

void ast_walker::walk(const File* starlark_file, ast_listener& listener) {
  std::vector<message> to_process;

  auto add_statements = [&to_process](const RepeatedPtrField<Statement>& statements) {
    for (auto it = statements.rbegin(); it != statements.rend(); ++it) {
      to_process.push_back(message{
        .statement = &*it,
        .type = message_type::kStatement,
        .op = message_type_op::kEnter,
      });
    }
  };

  to_process.push_back(message{
    .file = starlark_file,
    .type = message_type::kStarlarkFile,
    .op = message_type_op::kEnter,
  });
  while (!to_process.empty()) {
    auto top = to_process.back();
    to_process.pop_back();
    if (top.op == message_type_op::kEnter) {
      message exit_message = top;
      exit_message.op = message_type_op::kExit;
      to_process.push_back(exit_message);
      switch (top.type) {
        case message_type::kStarlarkFile:
          listener.enter_file(top.file);
          add_statements(top.file->statement());
          break;
        case message_type::kStatement:
          listener.enter_statement(top.statement);
          switch (top.statement->statement_type_case()) {
            case Statement::kDefStatement:
              to_process.push_back(message{
                .def_statement = &top.statement->def_statement(),
                .type = message_type::kDefStatement,
                .op = message_type_op::kEnter,
              });
              break;
            case Statement::kIfStatement:
              to_process.push_back(message{
                .if_statement = &top.statement->if_statement(),
                .type = message_type::kIfStatement,
                .op = message_type_op::kEnter,
              });
              break;
            case Statement::kForStatement:
              to_process.push_back(message{
                .for_statement = &top.statement->for_statement(),
                .type = message_type::kForStatement,
                .op = message_type_op::kEnter,
              });
              break;
            case Statement::kReturnStatement:
              to_process.push_back(message{
                .return_statement = &top.statement->return_statement(),
                .type = message_type::kReturnStatement,
                .op = message_type_op::kEnter,
              });
              break;
            case Statement::kBreakStatement:
              to_process.push_back(message{
                .break_statement = &top.statement->break_statement(),
                .type = message_type::kBreakStatement,
                .op = message_type_op::kEnter,
              });
              break;
            case Statement::kContinueStatement:
              to_process.push_back(message{
                .continue_statement = &top.statement->continue_statement(),
                .type = message_type::kContinueStatement,
                .op = message_type_op::kEnter,
              });
              break;
            case Statement::kPassStatement:
              to_process.push_back(message{
                .pass_statement = &top.statement->pass_statement(),
                .type = message_type::kPassStatement,
                .op = message_type_op::kEnter,
              });
              break;
            case Statement::kAssignStatement:
              to_process.push_back(message{
                .assign_statement = &top.statement->assign_statement(),
                .type = message_type::kAssignStatement,
                .op = message_type_op::kEnter,
              });
              break;
            case Statement::kExpressionStatement:
              to_process.push_back(message{
                .expression_statement = &top.statement->expression_statement(),
                .type = message_type::kExpressionStatement,
                .op = message_type_op::kEnter,
              });
              break;
            case Statement::kLoadStatement:
              to_process.push_back(message{
                .load_statement = &top.statement->load_statement(),
                .type = message_type::kLoadStatement,
                .op = message_type_op::kEnter,
              });
              break;
            case Statement::STATEMENT_TYPE_NOT_SET:
              break;
          }
          break;
        case message_type::kDefStatement:
          listener.enter_def_statement(top.def_statement);
          add_statements(top.def_statement->statement());
          to_process.push_back(message{
            .def_statement = top.def_statement,
            .type = message_type::kDefStatement,
            .op = message_type_op::kMid,
          });
          for (auto it = top.def_statement->parameter().rbegin(); it != top.def_statement->parameter().rend(); ++it) {
            to_process.push_back(message{
              .parameter = &*it,
              .type = message_type::kParameter,
              .op = message_type_op::kEnter,
            });
          }
          break;
        case message_type::kIfStatement:
          listener.enter_if_statement(top.if_statement);
          if (!top.if_statement->else_statement().empty()) {
            to_process.push_back(message{
              .else_ = &top.if_statement->else_statement(),
              .type = message_type::kElse,
              .op = message_type_op::kEnter,
            });
          }
          for (auto it = top.if_statement->elif().rbegin(); it != top.if_statement->elif().rend(); ++it) {
            to_process.push_back(message{
              .elif = &*it,
              .type = message_type::kElif,
              .op = message_type_op::kEnter,
            });
          }
          to_process.push_back(message{
            .then = &top.if_statement->statement(),
            .type = message_type::kThen,
            .op = message_type_op::kEnter,
          });
          to_process.push_back(message{
            .expression = &top.if_statement->test(),
            .type = message_type::kExpression,
            .op = message_type_op::kEnter,
          });
          break;
        case message_type::kThen:
          listener.enter_then(top.then);
          add_statements(*top.then);
          break;
        case message_type::kElif:
          listener.enter_elif(top.elif);
          to_process.push_back(message{
            .then = &top.elif->statement(),
            .type = message_type::kThen,
            .op = message_type_op::kEnter,
          });
          to_process.push_back(message{
            .expression = &top.elif->test(),
            .type = message_type::kExpression,
            .op = message_type_op::kEnter,
          });
          break;
        case message_type::kElse:
          listener.enter_else(top.else_);
          add_statements(*top.else_);
          break;
        case message_type::kForStatement:
          listener.enter_for_statement(top.for_statement);
          add_statements(top.for_statement->statement());
          to_process.push_back(message{
            .for_statement = top.for_statement,
            .type = message_type::kForStatement,
            .op = message_type_op::kMid,
          });
          to_process.push_back(message{
            .for_loop_variables = &top.for_statement->loop_variables(),
            .type = message_type::kForLoopVariables,
            .op = message_type_op::kEnter,
          });
          to_process.push_back(message{
            .for_statement = top.for_statement,
            .type = message_type::kForStatement,
            .op = message_type_op::kMid,
          });
          to_process.push_back(message{
            .for_in_expression = &top.for_statement->expression(),
            .type = message_type::kForInExpression,
            .op = message_type_op::kEnter,
          });
          break;
        case message_type::kReturnStatement:
          listener.enter_return_statement(top.return_statement);
          if (top.return_statement->has_expression()) {
            to_process.push_back(message{
              .expression = &top.return_statement->expression(),
              .type = message_type::kExpression,
              .op = message_type_op::kEnter,
            });
          } else {
            to_process.push_back(message{
              .type = message_type::kNoneValue,
              .op = message_type_op::kEnter,
            });
          }
          break;
        case message_type::kForLoopVariables:
          listener.enter_for_loop_variables(top.for_loop_variables);
          to_process.push_back(message{
            .expression = top.for_loop_variables,
            .type = message_type::kExpression,
            .op = message_type_op::kEnter,
            .for_assignment = true,
          });
          break;
        case message_type::kForInExpression:
          listener.enter_for_in_expression(top.for_in_expression);
          to_process.push_back(message{
            .expression = top.for_in_expression,
            .type = message_type::kExpression,
            .op = message_type_op::kEnter,
          });
          break;
        case message_type::kBreakStatement:
          listener.enter_break_statement(top.break_statement);
          break;
        case message_type::kContinueStatement:
          listener.enter_continue_statement(top.continue_statement);
          break;
        case message_type::kPassStatement:
          listener.enter_pass_statement(top.pass_statement);
          break;
        case message_type::kAssignStatement:
          listener.enter_assign_statement(top.assign_statement);
          to_process.push_back(message{
            .expression = &top.assign_statement->lhs(),
            .type = message_type::kExpression,
            .op = message_type_op::kEnter,
            .for_assignment = top.assign_statement->op() == AssignStmt::EQUALS,
          });
          to_process.push_back(message{
            .expression = &top.assign_statement->rhs(),
            .type = message_type::kExpression,
            .op = message_type_op::kEnter,
          });
          break;
        case message_type::kExpressionStatement:
          listener.enter_expression_statement(top.expression_statement);
          to_process.push_back(message{
            .expression = top.expression_statement,
            .type = message_type::kExpression,
            .op = message_type_op::kEnter,
          });
          break;
        case message_type::kLoadStatement:
          listener.enter_load_statement(top.load_statement);
          break;
        case message_type::kParameter:
          listener.enter_parameter(top.parameter);
          if (top.parameter->has_initialization()) {
            to_process.push_back(message{
              .expression = &top.parameter->initialization(),
              .type = message_type::kExpression,
              .op = message_type_op::kEnter,
            });
          }
          break;
        case message_type::kArgument:
          listener.enter_argument(top.argument);
          switch (top.argument->argument_type_case()) {
            case Argument::kValue:
              to_process.push_back(message{
                .expression = &top.argument->value(),
                .type = message_type::kExpression,
                .op = message_type_op::kEnter,
              });
              break;
            case Argument::kNamedArgument:
              to_process.push_back(message{
                .expression = &top.argument->named_argument().value(),
                .type = message_type::kExpression,
                .op = message_type_op::kEnter,
              });
              break;
            case Argument::kStarArgument:
              to_process.push_back(message{
                .expression = &top.argument->star_argument(),
                .type = message_type::kExpression,
                .op = message_type_op::kEnter,
              });
              break;
            case Argument::kStarStarArgument:
              to_process.push_back(message{
                .expression = &top.argument->star_star_argument(),
                .type = message_type::kExpression,
                .op = message_type_op::kEnter,
              });
              break;
            case Argument::ARGUMENT_TYPE_NOT_SET:
              break;
          }
          break;
        case message_type::kExpression:
          listener.enter_expression(top.expression);
          switch (top.expression->expression_type_case()) {
            case Expression::kTuple:
              to_process.push_back(message{
                .tuple = &top.expression->tuple(),
                .type = message_type::kTuple,
                .op = message_type_op::kEnter,
                .for_assignment = top.for_assignment,
              });
              break;
            case Expression::kIfExpression:
              to_process.push_back(message{
                .if_expression = &top.expression->if_expression(),
                .type = message_type::kIfExpression,
                .op = message_type_op::kEnter,
              });
              break;
            case Expression::kUnaryExpression:
              to_process.push_back(message{
                .unary_expression = &top.expression->unary_expression(),
                .type = message_type::kUnaryExpression,
                .op = message_type_op::kEnter,
              });
              break;
            case Expression::kBinaryExpression:
              to_process.push_back(message{
                .binary_expression = &top.expression->binary_expression(),
                .type = message_type::kBinaryExpression,
                .op = message_type_op::kEnter,
              });
              break;
            case Expression::kLambdaExpression:
              to_process.push_back(message{
                .lambda_expression = &top.expression->lambda_expression(),
                .type = message_type::kLambdaExpression,
                .op = message_type_op::kEnter,
              });
              break;
            case Expression::kDotExpression:
              to_process.push_back(message{
                .dot_expression = &top.expression->dot_expression(),
                .type = message_type::kDotExpression,
                .op = message_type_op::kEnter,
                .for_assignment = top.for_assignment,
              });
              break;
            case Expression::kCallExpression:
              to_process.push_back(message{
                .call_expression = &top.expression->call_expression(),
                .type = message_type::kCallExpression,
                .op = message_type_op::kEnter,
              });
              break;
            case Expression::kSliceExpression:
              to_process.push_back(message{
                .slice_expression = &top.expression->slice_expression(),
                .type = message_type::kSliceExpression,
                .op = message_type_op::kEnter,
                .for_assignment = top.for_assignment,
              });
              break;
            case Expression::kIdentifier:
              to_process.push_back(message{
                .identifier = &top.expression->identifier(),
                .type = message_type::kIdentifier,
                .op = message_type_op::kEnter,
                .for_assignment = top.for_assignment,
              });
              break;
            case Expression::kIntValue:
              to_process.push_back(message{
                .int_value = top.expression->int_value(),
                .type = message_type::kIntValue,
                .op = message_type_op::kEnter,
              });
              break;
            case Expression::kBigIntValue:
              to_process.push_back(message{
                .big_int_value = top.expression->big_int_value(),
                .type = message_type::kBigIntValue,
                .op = message_type_op::kEnter,
              });
              break;
            case Expression::kFloatValue:
              to_process.push_back(message{
                .float_value = top.expression->float_value(),
                .type = message_type::kFloatValue,
                .op = message_type_op::kEnter,
              });
              break;
            case Expression::kStringValue:
              to_process.push_back(message{
                .string_value = top.expression->string_value(),
                .type = message_type::kStringValue,
                .op = message_type_op::kEnter,
              });
              break;
            case Expression::kBytesValue:
              to_process.push_back(message{
                .bytes_value = top.expression->bytes_value(),
                .type = message_type::kBytesValue,
                .op = message_type_op::kEnter,
              });
              break;
            case Expression::kListExpression:
              to_process.push_back(message{
                .list_expression = &top.expression->list_expression(),
                .type = message_type::kListExpression,
                .op = message_type_op::kEnter,
                .for_assignment = top.for_assignment,
              });
              break;
            case Expression::kListComprehension:
              to_process.push_back(message{
                .list_comprehension = &top.expression->list_comprehension(),
                .type = message_type::kListComprehension,
                .op = message_type_op::kEnter,
              });
              break;
            case Expression::kDictionaryExpression:
              to_process.push_back(message{
                .dictionary_expression = &top.expression->dictionary_expression(),
                .type = message_type::kDictionaryExpression,
                .op = message_type_op::kEnter,
              });
              break;
            case Expression::kDictionaryComprehension:
              to_process.push_back(message{
                .dictionary_comprehension = &top.expression->dictionary_comprehension(),
                .type = message_type::kDictionaryComprehension,
                .op = message_type_op::kEnter,
              });
              break;
            case Expression::EXPRESSION_TYPE_NOT_SET:
              break;
          }
          break;
        case message_type::kTuple:
          if (top.for_assignment) {
            listener.enter_tuple_for_assignment(top.tuple);
          } else {
            listener.enter_tuple(top.tuple);
          }
          for (auto it = top.tuple->value().rbegin(); it != top.tuple->value().rend(); ++it) {
            to_process.push_back(message{
              .expression = &*it,
              .type = message_type::kExpression,
              .op = message_type_op::kEnter,
              .for_assignment = top.for_assignment,
            });
          }
          break;
        case message_type::kIfExpression:
          listener.enter_if_expression(top.if_expression);
          // The events follow an `if-else` and not the order that things show up in the grammar.
          to_process.push_back(message{
            .expression = &top.if_expression->else_value(),
            .type = message_type::kExpression,
            .op = message_type_op::kEnter,
          });
          to_process.push_back(message{
            .if_expression = top.if_expression,
            .type = message_type::kIfExpression,
            .op = message_type_op::kMid,
          });
          to_process.push_back(message{
            .expression = &top.if_expression->if_value(),
            .type = message_type::kExpression,
            .op = message_type_op::kEnter,
          });
          to_process.push_back(message{
            .if_expression = top.if_expression,
            .type = message_type::kIfExpression,
            .op = message_type_op::kMid,
          });
          to_process.push_back(message{
            .expression = &top.if_expression->if_test(),
            .type = message_type::kExpression,
            .op = message_type_op::kEnter,
          });
          break;
        case message_type::kUnaryExpression:
          listener.enter_unary_expression(top.unary_expression);
          to_process.push_back(message{
            .expression = &top.unary_expression->test(),
            .type = message_type::kExpression,
            .op = message_type_op::kEnter,
          });
          break;
        case message_type::kBinaryExpression:
          listener.enter_binary_expression(top.binary_expression);
          to_process.push_back(message{
            .expression = &top.binary_expression->rhs(),
            .type = message_type::kExpression,
            .op = message_type_op::kEnter,
          });
          to_process.push_back(message{
            .binary_expression = top.binary_expression,
            .type = message_type::kBinaryExpression,
            .op = message_type_op::kMid,
          });
          to_process.push_back(message{
            .expression = &top.binary_expression->lhs(),
            .type = message_type::kExpression,
            .op = message_type_op::kEnter,
          });
          break;
        case message_type::kLambdaExpression:
          listener.enter_lambda_expression(top.lambda_expression);
          to_process.push_back(message{
            .expression = &top.lambda_expression->test(),
            .type = message_type::kExpression,
            .op = message_type_op::kEnter,
          });
          to_process.push_back(message{
            .lambda_expression = top.lambda_expression,
            .type = message_type::kLambdaExpression,
            .op = message_type_op::kMid,
          });
          for (auto it = top.lambda_expression->parameter().rbegin(); it != top.lambda_expression->parameter().rend(); ++it) {
            to_process.push_back(message{
              .parameter = &*it,
              .type = message_type::kParameter,
              .op = message_type_op::kEnter,
            });
          }
          break;
        case message_type::kDotExpression:
          if (top.for_assignment) {
            listener.enter_dot_expression_for_assignment(top.dot_expression);
          } else {
            listener.enter_dot_expression(top.dot_expression);
          }
          to_process.push_back(message{
            .expression = &top.dot_expression->primary_expression(),
            .type = message_type::kExpression,
            .op = message_type_op::kEnter,
          });
          break;
        case message_type::kCallExpression:
          listener.enter_call_expression(top.call_expression);
          for (auto it = top.call_expression->argument().rbegin(); it != top.call_expression->argument().rend(); ++it) {
            to_process.push_back(message{
              .argument = &*it,
              .type = message_type::kArgument,
              .op = message_type_op::kEnter,
            });
          }
          to_process.push_back(message{
            .expression = &top.call_expression->primary_expression(),
            .type = message_type::kExpression,
            .op = message_type_op::kEnter,
          });
          break;
        case message_type::kSliceExpression:
          if (top.for_assignment) {
            listener.enter_slice_expression_for_assignment(top.slice_expression);
          } else {
            listener.enter_slice_expression(top.slice_expression);
          }
          switch (top.slice_expression->slice_type_case()) {
            case SliceExpr::kIndex:
              to_process.push_back(message{
                .expression = &top.slice_expression->index(),
                .type = message_type::kExpression,
                .op = message_type_op::kEnter,
              });
              break;
            case SliceExpr::kSlice:
              if (top.slice_expression->slice().has_step()) {
                to_process.push_back(message{
                  .expression = &top.slice_expression->slice().step(),
                  .type = message_type::kExpression,
                  .op = message_type_op::kEnter,
                });
              } else {
                to_process.push_back(message{
                  .type = message_type::kNoneValue,
                  .op = message_type_op::kEnter,
                });
              }
              if (top.slice_expression->slice().has_end()) {
                to_process.push_back(message{
                  .expression = &top.slice_expression->slice().end(),
                  .type = message_type::kExpression,
                  .op = message_type_op::kEnter,
                });
              } else {
                to_process.push_back(message{
                  .type = message_type::kNoneValue,
                  .op = message_type_op::kEnter,
                });
              }
              if (top.slice_expression->slice().has_start()) {
                to_process.push_back(message{
                  .expression = &top.slice_expression->slice().start(),
                  .type = message_type::kExpression,
                  .op = message_type_op::kEnter,
                });
              } else {
                to_process.push_back(message{
                  .type = message_type::kNoneValue,
                  .op = message_type_op::kEnter,
                });
              }
              break;
            case SliceExpr::SLICE_TYPE_NOT_SET:
              break;
          }
          to_process.push_back(message{
            .expression = &top.slice_expression->primary_expression(),
            .type = message_type::kExpression,
            .op = message_type_op::kEnter,
          });
          break;
        case message_type::kIdentifier:
          if (top.for_assignment) {
            listener.enter_identifier_for_assignment(top.identifier);
          } else {
            listener.enter_identifier(top.identifier);
          }
          break;
        case message_type::kNoneValue:
          listener.enter_none_value();
          break;
        case message_type::kIntValue:
          listener.enter_int_value(top.int_value);
          break;
        case message_type::kBigIntValue:
          listener.enter_big_int_value(top.big_int_value);
          break;
        case message_type::kFloatValue:
          listener.enter_float_value(top.float_value);
          break;
        case message_type::kStringValue:
          listener.enter_string_value(top.string_value);
          break;
        case message_type::kBytesValue:
          listener.enter_bytes_value(top.bytes_value);
          break;
        case message_type::kListExpression:
          if (top.for_assignment) {
            listener.enter_list_expression_for_assignment(top.list_expression);
          } else {
            listener.enter_list_expression(top.list_expression);
          }
          for (auto it = top.list_expression->element().rbegin(); it != top.list_expression->element().rend(); ++it) {
            if (it != top.list_expression->element().rbegin()) {
              to_process.push_back(message{
                .list_expression = top.list_expression,
                .type = message_type::kListExpression,
                .op = message_type_op::kMid,
                .for_assignment = top.for_assignment,
              });
            }
            to_process.push_back(message{
              .expression = &*it,
              .type = message_type::kExpression,
              .op = message_type_op::kEnter,
              .for_assignment = top.for_assignment,
            });
          }
          break;
        case message_type::kListComprehension:
          listener.enter_list_comprehension(top.list_comprehension);
          to_process.push_back(message{
            .expression = &top.list_comprehension->test(),
            .type = message_type::kExpression,
            .op = message_type_op::kEnter,
          });
          for (auto it = top.list_comprehension->clause().rbegin(); it != top.list_comprehension->clause().rend(); ++it) {
            to_process.push_back(message{
              .comp_clause = &*it,
              .type = message_type::kCompClause,
              .op = message_type_op::kEnter,
            });
          }
          break;
        case message_type::kDictionaryExpression:
          listener.enter_dictionary_expression(top.dictionary_expression);
          for (auto it = top.dictionary_expression->entry().rbegin(); it != top.dictionary_expression->entry().rend(); ++it) {
            if (it != top.dictionary_expression->entry().rbegin()) {
              to_process.push_back(message{
                .dictionary_expression = top.dictionary_expression,
                .type = message_type::kDictionaryExpression,
                .op = message_type_op::kMid,
              });
            }
            to_process.push_back(message{
              .map_entry = &*it,
              .type = message_type::kMapEntry,
              .op = message_type_op::kEnter,
            });
          }
          break;
        case message_type::kDictionaryComprehension:
          listener.enter_dictionary_comprehension(top.dictionary_comprehension);
          to_process.push_back(message{
            .map_entry = &top.dictionary_comprehension->entry(),
            .type = message_type::kMapEntry,
            .op = message_type_op::kEnter,
          });
          for (auto it = top.dictionary_comprehension->clause().rbegin(); it != top.dictionary_comprehension->clause().rend(); ++it) {
            to_process.push_back(message{
              .comp_clause = &*it,
              .type = message_type::kCompClause,
              .op = message_type_op::kEnter,
            });
          }
          break;
        case message_type::kCompClause:
          listener.enter_comp_clause(top.comp_clause);
          switch (top.comp_clause->comp_clause_type_case()) {
            case CompClause::kForClause:
              to_process.push_back(message{
                .for_clause = &top.comp_clause->for_clause(),
                .type = message_type::kForClause,
                .op = message_type_op::kEnter,
              });
              break;
            case CompClause::kIfClause:
              to_process.push_back(message{
                .if_clause = &top.comp_clause->if_clause(),
                .type = message_type::kIfClause,
                .op = message_type_op::kEnter,
              });
              break;
            case CompClause::COMP_CLAUSE_TYPE_NOT_SET:
              break;
          }
          break;
        case message_type::kForClause:
          listener.enter_for_clause(top.for_clause);
          to_process.push_back(message{
            .for_loop_variables = &top.for_clause->loop_variables(),
            .type = message_type::kForLoopVariables,
            .op = message_type_op::kEnter,
          });
          to_process.push_back(message{
            .for_clause = top.for_clause,
            .type = message_type::kForClause,
            .op = message_type_op::kMid,
          });
          to_process.push_back(message{
            .for_in_expression = &top.for_clause->in(),
            .type = message_type::kForInExpression,
            .op = message_type_op::kEnter,
          });
          break;
        case message_type::kIfClause:
          listener.enter_if_clause(top.if_clause);
          to_process.push_back(message{
            .expression = top.if_clause,
            .type = message_type::kExpression,
            .op = message_type_op::kEnter,
          });
          break;
        case message_type::kMapEntry:
          listener.enter_map_entry(top.map_entry);
          to_process.push_back(message{
            .expression = &top.map_entry->value(),
            .type = message_type::kExpression,
            .op = message_type_op::kEnter,
          });
          to_process.push_back(message{
            .expression = &top.map_entry->key(),
            .type = message_type::kExpression,
            .op = message_type_op::kEnter,
          });
          break;
      }
    } else if (top.op == message_type_op::kExit) {
      switch (top.type) {
        case message_type::kStarlarkFile:
          listener.exit_file(top.file);
          break;
        case message_type::kStatement:
          listener.exit_statement(top.statement);
          break;
        case message_type::kDefStatement:
          listener.exit_def_statement(top.def_statement);
          break;
        case message_type::kIfStatement:
          listener.exit_if_statement(top.if_statement);
          break;
        case message_type::kThen:
          listener.exit_then(top.then);
          break;
        case message_type::kElif:
          listener.exit_elif(top.elif);
          break;
        case message_type::kElse:
          listener.exit_else(top.else_);
          break;
        case message_type::kForStatement:
          listener.exit_for_statement(top.for_statement);
          break;
        case message_type::kForLoopVariables:
          listener.exit_for_loop_variables(top.for_loop_variables);
          break;
        case message_type::kForInExpression:
          listener.exit_for_in_expression(top.for_in_expression);
          break;
        case message_type::kReturnStatement:
          listener.exit_return_statement(top.return_statement);
          break;
        case message_type::kBreakStatement:
          listener.exit_break_statement(top.break_statement);
          break;
        case message_type::kContinueStatement:
          listener.exit_continue_statement(top.continue_statement);
          break;
        case message_type::kPassStatement:
          listener.exit_pass_statement(top.pass_statement);
          break;
        case message_type::kAssignStatement:
          listener.exit_assign_statement(top.assign_statement);
          break;
        case message_type::kExpressionStatement:
          listener.exit_expression_statement(top.expression_statement);
          break;
        case message_type::kLoadStatement:
          listener.exit_load_statement(top.load_statement);
          break;
        case message_type::kParameter:
          listener.exit_parameter(top.parameter);
          break;
        case message_type::kArgument:
          listener.exit_argument(top.argument);
          break;
        case message_type::kExpression:
          listener.exit_expression(top.expression);
          break;
        case message_type::kTuple:
          if (top.for_assignment) {
            listener.exit_tuple_for_assignment(top.tuple);
          } else {
            listener.exit_tuple(top.tuple);
          }
          break;
        case message_type::kIfExpression:
          listener.exit_if_expression(top.if_expression);
          break;
        case message_type::kUnaryExpression:
          listener.exit_unary_expression(top.unary_expression);
          break;
        case message_type::kBinaryExpression:
          listener.exit_binary_expression(top.binary_expression);
          break;
        case message_type::kLambdaExpression:
          listener.exit_lambda_expression(top.lambda_expression);
          break;
        case message_type::kDotExpression:
          if (top.for_assignment) {
            listener.exit_dot_expression_for_assignment(top.dot_expression);
          } else {
            listener.exit_dot_expression(top.dot_expression);
          }
          break;
        case message_type::kCallExpression:
          listener.exit_call_expression(top.call_expression);
          break;
        case message_type::kSliceExpression:
          if (top.for_assignment) {
            listener.exit_slice_expression_for_assignment(top.slice_expression);
          } else {
            listener.exit_slice_expression(top.slice_expression);
          }
          break;
        case message_type::kIdentifier:
          if (top.for_assignment) {
            listener.exit_identifier_for_assignment(top.identifier);
          } else {
            listener.exit_identifier(top.identifier);
          }
          break;
        case message_type::kNoneValue:
          listener.exit_none_value();
          break;
        case message_type::kIntValue:
          listener.exit_int_value(top.int_value);
          break;
        case message_type::kBigIntValue:
          listener.exit_big_int_value(top.big_int_value);
          break;
        case message_type::kFloatValue:
          listener.exit_float_value(top.float_value);
          break;
        case message_type::kStringValue:
          listener.exit_string_value(top.string_value);
          break;
        case message_type::kBytesValue:
          listener.exit_bytes_value(top.bytes_value);
          break;
        case message_type::kListExpression:
          if (top.for_assignment) {
            listener.exit_list_expression_for_assignment(top.list_expression);
          } else {
            listener.exit_list_expression(top.list_expression);
          }
          break;
        case message_type::kListComprehension:
          listener.exit_list_comprehension(top.list_comprehension);
          break;
        case message_type::kDictionaryExpression:
          listener.exit_dictionary_expression(top.dictionary_expression);
          break;
        case message_type::kDictionaryComprehension:
          listener.exit_dictionary_comprehension(top.dictionary_comprehension);
          break;
        case message_type::kCompClause:
          listener.exit_comp_clause(top.comp_clause);
          break;
        case message_type::kForClause:
          listener.exit_for_clause(top.for_clause);
          break;
        case message_type::kIfClause:
          listener.exit_if_clause(top.if_clause);
          break;
        case message_type::kMapEntry:
          listener.exit_map_entry(top.map_entry);
          break;
      }
    } else {
      switch (top.type) {
        case message_type::kBinaryExpression:
          listener.mid_binary_expression(top.binary_expression);
          break;
        case message_type::kListExpression:
          if (top.for_assignment) {
            listener.mid_list_expression_for_assignment(top.list_expression);
          } else {
            listener.mid_list_expression(top.list_expression);
          }
          break;
        case message_type::kDictionaryExpression:
          listener.mid_dictionary_expression(top.dictionary_expression);
          break;
        case message_type::kIfExpression:
          listener.mid_if_expression(top.if_expression);
          break;
        case message_type::kForStatement:
          listener.mid_for_statement(top.for_statement);
          break;
        case message_type::kForClause:
          listener.mid_for_clause(top.for_clause);
          break;
        case message_type::kDefStatement:
          listener.mid_def_statement(top.def_statement);
          break;
        case message_type::kLambdaExpression:
          listener.mid_lambda_expression(top.lambda_expression);
          break;
        default:
          break;
      }
    }
  }
}

}  // namespace grammar
}  // namespace starlark

