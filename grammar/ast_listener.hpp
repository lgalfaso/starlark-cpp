// Copyright 2025 Lucas Mirelmann

#ifndef GRAMMAR_AST_LISTENER_HPP_
#define GRAMMAR_AST_LISTENER_HPP_

#include <string>

#include "grammar/proto/starlark.pb.h"

#pragma GCC visibility push(default)

namespace grammar {

class ast_listener {
 public:
  virtual void enter_file(const starlark::File* starlark_file) = 0;
  virtual void exit_file(const starlark::File* starlark_file) = 0;
  virtual void enter_statement(const starlark::Statement* statement) = 0;
  virtual void exit_statement(const starlark::Statement* statement) = 0;
  virtual void enter_def_statement(const starlark::DefStmt* def_statement) = 0;
  virtual void exit_def_statement(const starlark::DefStmt* def_statement) = 0;
  virtual void enter_if_statement(const starlark::IfStmt* if_statement) = 0;
  virtual void exit_if_statement(const starlark::IfStmt* if_statement) = 0;
  virtual void enter_for_statement(const starlark::ForStmt* for_statement) = 0;
  virtual void exit_for_statement(const starlark::ForStmt* for_statement) = 0;
  virtual void enter_return_statement(const starlark::ReturnStmt* return_statement) = 0;
  virtual void exit_return_statement(const starlark::ReturnStmt* return_statement) = 0;
  virtual void enter_break_statement(const starlark::BreakStmt* break_statement) = 0;
  virtual void exit_break_statement(const starlark::BreakStmt* break_statement) = 0;
  virtual void enter_continue_statement(const starlark::ContinueStmt* continue_statement) = 0;
  virtual void exit_continue_statement(const starlark::ContinueStmt* continue_statement) = 0;
  virtual void enter_pass_statement(const starlark::PassStmt* pass_statement) = 0;
  virtual void exit_pass_statement(const starlark::PassStmt* pass_statement) = 0;
  virtual void enter_assign_statement(const starlark::AssignStmt* assign_statement) = 0;
  virtual void exit_assign_statement(const starlark::AssignStmt* assign_statement) = 0;
  virtual void enter_expression_statement(const starlark::Expression* expression_statement) = 0;
  virtual void exit_expression_statement(const starlark::Expression* expresion_statement) = 0;
  virtual void enter_load_statement(const starlark::LoadStmt* load_statement) = 0;
  virtual void exit_load_statement(const starlark::LoadStmt* load_statement) = 0;
  virtual void enter_parameter(const starlark::Parameter* parameter) = 0;
  virtual void exit_parameter(const starlark::Parameter* parameter) = 0;
  virtual void enter_argument(const starlark::Argument* argument) = 0;
  virtual void exit_argument(const starlark::Argument* argument) = 0;
  virtual void enter_then(const google::protobuf::RepeatedPtrField<starlark::Statement>* then) = 0;
  virtual void exit_then(const google::protobuf::RepeatedPtrField<starlark::Statement>* then) = 0;
  virtual void enter_elif(const starlark::ElseIf* elif) = 0;
  virtual void exit_elif(const starlark::ElseIf* elif) = 0;
  virtual void enter_else(const google::protobuf::RepeatedPtrField<starlark::Statement>* else_) = 0;
  virtual void exit_else(const google::protobuf::RepeatedPtrField<starlark::Statement>* else_) = 0;
  virtual void enter_expression(const starlark::Expression* expression) = 0;
  virtual void exit_expression(const starlark::Expression* expression) = 0;
  virtual void enter_test(const starlark::Test* test) = 0;
  virtual void exit_test(const starlark::Test* test) = 0;
  virtual void enter_tuple(const starlark::Tuple* tuple) = 0;
  virtual void exit_tuple(const starlark::Tuple* tuple) = 0;
  virtual void enter_if_expression(const starlark::IfExpr* if_expression) = 0;
  virtual void exit_if_expression(const starlark::IfExpr* if_expression) = 0;
  virtual void enter_primary_expression(const starlark::PrimaryExpr* primary_expression) = 0;
  virtual void exit_primary_expression(const starlark::PrimaryExpr* primary_expression) = 0;
  virtual void enter_unary_expression(const starlark::UnaryExpr* unary_expression) = 0;
  virtual void exit_unary_expression(const starlark::UnaryExpr* unary_expression) = 0;
  virtual void enter_binary_expression(const starlark::BinaryExpr* binary_expression) = 0;
  virtual void exit_binary_expression(const starlark::BinaryExpr* binary_expression) = 0;
  virtual void enter_lambda_expression(const starlark::LambdaExpr* lambda_expression) = 0;
  virtual void exit_lambda_expression(const starlark::LambdaExpr* lambda_expression) = 0;
  virtual void enter_for_loop_variables(const google::protobuf::RepeatedPtrField<starlark::PrimaryExpr>* loop_variables) = 0;
  virtual void exit_for_loop_variables(const google::protobuf::RepeatedPtrField<starlark::PrimaryExpr>* loop_variables) = 0;
  virtual void enter_for_loop_variable(const starlark::PrimaryExpr* loop_variable) = 0;
  virtual void exit_for_loop_variable(const starlark::PrimaryExpr* loop_variable) = 0;
  virtual void enter_for_in_expression(const starlark::Expression* expression) = 0;
  virtual void exit_for_in_expression(const starlark::Expression* expression) = 0;
  virtual void enter_for_in_test(const starlark::Test* test) = 0;
  virtual void exit_for_in_test(const starlark::Test* test) = 0;
  virtual void enter_dot_expression(const starlark::DotExpr* dot_expression) = 0;
  virtual void exit_dot_expression(const starlark::DotExpr* dot_expression) = 0;
  virtual void enter_call_expression(const starlark::CallExpr* call_expression) = 0;
  virtual void exit_call_expression(const starlark::CallExpr* call_expression) = 0;
  virtual void enter_slice_expression(const starlark::SliceExpr* slice_expression) = 0;
  virtual void exit_slice_expression(const starlark::SliceExpr* slice_expression) = 0;
  virtual void enter_identifier(const starlark::Identifier* identifier) = 0;
  virtual void exit_identifier(const starlark::Identifier* identifier) = 0;
  virtual void enter_int_value(const std::string* int_value) = 0;
  virtual void exit_int_value(const std::string* int_value) = 0;
  virtual void enter_float_value(double float_value) = 0;
  virtual void exit_float_value(double float_value) = 0;
  virtual void enter_string_value(const std::string* string_value) = 0;
  virtual void exit_string_value(const std::string* string_value) = 0;
  virtual void enter_bytes_value(const std::string* bytes_value) = 0;
  virtual void exit_bytes_value(const std::string* bytes_value) = 0;
  virtual void enter_list_expression(const starlark::ListExpr* list_expression) = 0;
  virtual void exit_list_expression(const starlark::ListExpr* list_expression) = 0;
  virtual void enter_list_comprehension(const starlark::ListComp* list_comprehension) = 0;
  virtual void exit_list_comprehension(const starlark::ListComp* list_comprehension) = 0;
  virtual void enter_dictionary_expression(const starlark::DictExpr* dictionary_expression) = 0;
  virtual void exit_dictionary_expression(const starlark::DictExpr* dictionary_expression) = 0;
  virtual void enter_dictionary_comprehension(const starlark::DictComp* dictionary_comprehension) = 0;
  virtual void exit_dictionary_comprehension(const starlark::DictComp* dictionary_comprehension) = 0;
  virtual void enter_comp_clause(const starlark::CompClause* comp_clause) = 0;
  virtual void exit_comp_clause(const starlark::CompClause* comp_clause) = 0;
  virtual void enter_for_clause(const starlark::ForClause* for_clause) = 0;
  virtual void exit_for_clause(const starlark::ForClause* for_clause) = 0;
  virtual void enter_if_clause(const starlark::Test* if_clause) = 0;
  virtual void exit_if_clause(const starlark::Test* if_clause) = 0;
  virtual void enter_map_entry(const starlark::Entry* map_entry) = 0;
  virtual void exit_map_entry(const starlark::Entry* map_entry) = 0;
};

class ast_listener_base : public ast_listener {
  void enter_file(const starlark::File* starlark_file) override;
  void exit_file(const starlark::File* starlark_file) override;
  void enter_statement(const starlark::Statement* statement) override;
  void exit_statement(const starlark::Statement* statement) override;
  void enter_def_statement(const starlark::DefStmt* def_statement) override;
  void exit_def_statement(const starlark::DefStmt* def_statement) override;
  void enter_if_statement(const starlark::IfStmt* if_statement) override;
  void exit_if_statement(const starlark::IfStmt* if_statement) override;
  void enter_for_statement(const starlark::ForStmt* for_statement) override;
  void exit_for_statement(const starlark::ForStmt* for_statement) override;
  void enter_return_statement(const starlark::ReturnStmt* return_statement) override;
  void exit_return_statement(const starlark::ReturnStmt* return_statement) override;
  void enter_break_statement(const starlark::BreakStmt* break_statement) override;
  void exit_break_statement(const starlark::BreakStmt* break_statement) override;
  void enter_continue_statement(const starlark::ContinueStmt* continue_statement) override;
  void exit_continue_statement(const starlark::ContinueStmt* continue_statement) override;
  void enter_pass_statement(const starlark::PassStmt* pass_statement) override;
  void exit_pass_statement(const starlark::PassStmt* pass_statement) override;
  void enter_assign_statement(const starlark::AssignStmt* assign_statement) override;
  void exit_assign_statement(const starlark::AssignStmt* assign_statement) override;
  void enter_expression_statement(const starlark::Expression* expression_statement) override;
  void exit_expression_statement(const starlark::Expression* expresion_statement) override;
  void enter_load_statement(const starlark::LoadStmt* load_statement) override;
  void exit_load_statement(const starlark::LoadStmt* load_statement) override;
  void enter_parameter(const starlark::Parameter* parameter) override;
  void exit_parameter(const starlark::Parameter* parameter) override;
  void enter_argument(const starlark::Argument* argument) override;
  void exit_argument(const starlark::Argument* argument) override;
  void enter_then(const google::protobuf::RepeatedPtrField<starlark::Statement>* then) override;
  void exit_then(const google::protobuf::RepeatedPtrField<starlark::Statement>* then) override;
  void enter_elif(const starlark::ElseIf* elif) override;
  void exit_elif(const starlark::ElseIf* elif) override;
  void enter_else(const google::protobuf::RepeatedPtrField<starlark::Statement>* else_) override;
  void exit_else(const google::protobuf::RepeatedPtrField<starlark::Statement>* else_) override;
  void enter_expression(const starlark::Expression* expression) override;
  void exit_expression(const starlark::Expression* expression) override;
  void enter_test(const starlark::Test* test) override;
  void exit_test(const starlark::Test* test) override;
  void enter_tuple(const starlark::Tuple* tuple) override;
  void exit_tuple(const starlark::Tuple* tuple) override;
  void enter_if_expression(const starlark::IfExpr* if_expression) override;
  void exit_if_expression(const starlark::IfExpr* if_expression) override;
  void enter_primary_expression(const starlark::PrimaryExpr* primary_expression) override;
  void exit_primary_expression(const starlark::PrimaryExpr* primary_expression) override;
  void enter_unary_expression(const starlark::UnaryExpr* unary_expression) override;
  void exit_unary_expression(const starlark::UnaryExpr* unary_expression) override;
  void enter_binary_expression(const starlark::BinaryExpr* binary_expression) override;
  void exit_binary_expression(const starlark::BinaryExpr* binary_expression) override;
  void enter_lambda_expression(const starlark::LambdaExpr* lambda_expression) override;
  void exit_lambda_expression(const starlark::LambdaExpr* lambda_expression) override;
  void enter_for_loop_variables(const google::protobuf::RepeatedPtrField<starlark::PrimaryExpr>* loop_variables) override;
  void exit_for_loop_variables(const google::protobuf::RepeatedPtrField<starlark::PrimaryExpr>* loop_variables) override;
  void enter_for_loop_variable(const starlark::PrimaryExpr* loop_variable) override;
  void exit_for_loop_variable(const starlark::PrimaryExpr* loop_variable) override;
  void enter_for_in_expression(const starlark::Expression* expression) override;
  void exit_for_in_expression(const starlark::Expression* expression) override;
  void enter_for_in_test(const starlark::Test* test) override;
  void exit_for_in_test(const starlark::Test* test) override;
  void enter_dot_expression(const starlark::DotExpr* dot_expression) override;
  void exit_dot_expression(const starlark::DotExpr* dot_expression) override;
  void enter_call_expression(const starlark::CallExpr* call_expression) override;
  void exit_call_expression(const starlark::CallExpr* call_expression) override;
  void enter_slice_expression(const starlark::SliceExpr* slice_expression) override;
  void exit_slice_expression(const starlark::SliceExpr* slice_expression) override;
  void enter_identifier(const starlark::Identifier* identifier) override;
  void exit_identifier(const starlark::Identifier* identifier) override;
  void enter_int_value(const std::string* int_value) override;
  void exit_int_value(const std::string* int_value) override;
  void enter_float_value(double float_value) override;
  void exit_float_value(double float_value) override;
  void enter_string_value(const std::string* string_value) override;
  void exit_string_value(const std::string* string_value) override;
  void enter_bytes_value(const std::string* bytes_value) override;
  void exit_bytes_value(const std::string* bytes_value) override;
  void enter_list_expression(const starlark::ListExpr* list_expression) override;
  void exit_list_expression(const starlark::ListExpr* list_expression) override;
  void enter_list_comprehension(const starlark::ListComp* list_comprehension) override;
  void exit_list_comprehension(const starlark::ListComp* list_comprehension) override;
  void enter_dictionary_expression(const starlark::DictExpr* dictionary_expression) override;
  void exit_dictionary_expression(const starlark::DictExpr* dictionary_expression) override;
  void enter_dictionary_comprehension(const starlark::DictComp* dictionary_comprehension) override;
  void exit_dictionary_comprehension(const starlark::DictComp* dictionary_comprehension) override;
  void enter_comp_clause(const starlark::CompClause* comp_clause) override;
  void exit_comp_clause(const starlark::CompClause* comp_clause) override;
  void enter_for_clause(const starlark::ForClause* for_clause) override;
  void exit_for_clause(const starlark::ForClause* for_clause) override;
  void enter_if_clause(const starlark::Test* if_clause) override;
  void exit_if_clause(const starlark::Test* if_clause) override;
  void enter_map_entry(const starlark::Entry* map_entry) override;
  void exit_map_entry(const starlark::Entry* map_entry) override;
};

class ast_walker {
 public:
  void walk(const starlark::File* starlark_file, ast_listener& listener);
};

}  // namespace grammar

#pragma GCC visibility pop

#endif  // GRAMMAR_AST_LISTENER_HPP_

