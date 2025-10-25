// Copyright 2025 Lucas Mirelmann

#include <fcntl.h>

#include <gmock/gmock.h>
#include <gtest/gtest-matchers.h>
#include <gtest/gtest.h>

#include <string>

#include "grammar/ast_listener.hpp"
#include "grammar/parser.hpp"
#include "grammar/parsing_options.hpp"
#include "grammar/quoted.hpp"
#include "proto/starlark_ast.pb.h"
#include "third-party/defer.hpp"

using starlark::ast::File;
using starlark::ast::SliceExpr;
using starlark::grammar::parser;
using starlark::logging::log_level;
using starlark::logging::logger;
using testing::IsEmpty;
using testing::SizeIs;

namespace {

std::string show_errors(const logger& logging) {
  std::string result;

  for (const auto& entry : logging) {
    result += "[" + std::to_string(entry.pos.row) + "," + std::to_string(entry.pos.column) + "] " + entry.module + ":" + entry.message + "\n";
  }
  return result;
}

class ast_listener_logger : public starlark::grammar::ast_listener {
 public:
  explicit ast_listener_logger(std::string& output) : output(output) {}

  void enter_file(const starlark::ast::File* starlark_file) override {
    output += "ENTER File\n";
  }

  void exit_file(const starlark::ast::File* starlark_file) override {
    output += "EXIT File\n";
  }

  void enter_statement(const starlark::ast::Statement* statement) override {
    output += "ENTER Statement\n";
  }

  void exit_statement(const starlark::ast::Statement* statement) override {
    output += "EXIT Statement\n";
  }

  void enter_def_statement(const starlark::ast::DefStmt* def_statement) override {
    output += "ENTER Def(" + starlark::grammar::quoted(def_statement->function_name().name()) + ")\n";
  }

  void mid_def_statement(const starlark::ast::DefStmt* def_statement) override {
    output += "MID Def\n";
  }

  void exit_def_statement(const starlark::ast::DefStmt* def_statement) override {
    output += "EXIT Def\n";
  }

  void enter_if_statement(const starlark::ast::IfStmt* if_statement) override {
    output += "ENTER IfStatement\n";
  }

  void exit_if_statement(const starlark::ast::IfStmt* if_statement) override {
    output += "EXIT IfStatement\n";
  }

  void enter_for_statement(const starlark::ast::ForStmt* for_statement) override {
    output += "ENTER ForStatement\n";
  }

  void mid_for_statement(const starlark::ast::ForStmt* for_statement) override {
    output += "MID ForStatement\n";
  }

  void exit_for_statement(const starlark::ast::ForStmt* for_statement) override {
    output += "EXIT ForStatement\n";
  }

  void enter_return_statement(const starlark::ast::ReturnStmt* return_statement) override {
    output += "ENTER ReturnStatement";
    if (return_statement->has_expression()) {
      output += " expression\n";
    } else {
      output += " None\n";
    }
  }

  void exit_return_statement(const starlark::ast::ReturnStmt* return_statement) override {
    output += "EXIT ReturnStatement\n";
  }

  void enter_break_statement(const starlark::ast::BreakStmt* break_statement) override {
    output += "ENTER BreakStatement\n";
  }

  void exit_break_statement(const starlark::ast::BreakStmt* break_statement) override {
    output += "EXIT BreakStatement\n";
  }

  void enter_continue_statement(const starlark::ast::ContinueStmt* continue_statement) override {
    output += "ENTER ContinueStatement\n";
  }

  void exit_continue_statement(const starlark::ast::ContinueStmt* continue_statement) override {
    output += "EXIT ContinueStatement\n";
  }

  void enter_pass_statement(const starlark::ast::PassStmt* pass_statement) override {
    output += "ENTER Pass\n";
  }

  void exit_pass_statement(const starlark::ast::PassStmt* pass_statement) override {
    output += "EXIT Pass\n";
  }

  void enter_assign_statement(const starlark::ast::AssignStmt* assign_statement) override {
    output += "ENTER AssignStatement\n";
  }

  void exit_assign_statement(const starlark::ast::AssignStmt* assign_statement) override {
    output += "EXIT AssignStatement\n";
  }

  void enter_expression_statement(const starlark::ast::Expression* expression_statement) override {
    output += "ENTER ExpressionStatement\n";
  }

  void exit_expression_statement(const starlark::ast::Expression* expresion_statement) override {
    output += "EXIT ExpressionStatement\n";
  }

  void enter_load_statement(const starlark::ast::LoadStmt* load_statement) override {
    output += "ENTER LoadStatement\n";
  }

  void exit_load_statement(const starlark::ast::LoadStmt* load_statement) override {
    output += "EXIT LoadStatement\n";
  }

  void enter_parameter(const starlark::ast::Parameter* parameter) override {
    output += "ENTER Parameter(" + starlark::grammar::quoted(parameter->identifier().name()) + ")\n";
  }

  void exit_parameter(const starlark::ast::Parameter* parameter) override {
    output += "EXIT Parameter\n";
  }

  void enter_argument(const starlark::ast::Argument* argument) override {
    output += "ENTER Argument\n";
  }

  void exit_argument(const starlark::ast::Argument* argument) override {
    output += "EXIT Argument\n";
  }

  void enter_then(const google::protobuf::RepeatedPtrField<starlark::ast::Statement>* then) override {
    output += "ENTER Then\n";
  }

  void exit_then(const google::protobuf::RepeatedPtrField<starlark::ast::Statement>* then) override {
    output += "EXIT Then\n";
  }

  void enter_elif(const starlark::ast::ElseIf* elif) override {
    output += "ENTER Elif\n";
  }

  void exit_elif(const starlark::ast::ElseIf* elif) override {
    output += "EXIT Elif\n";
  }

  void enter_else(const google::protobuf::RepeatedPtrField<starlark::ast::Statement>* else_) override {
    output += "ENTER Else\n";
  }

  void exit_else(const google::protobuf::RepeatedPtrField<starlark::ast::Statement>* else_) override {
    output += "EXIT Else\n";
  }

  void enter_expression(const starlark::ast::Expression* expression) override {
    output += "ENTER Expression\n";
  }

  void exit_expression(const starlark::ast::Expression* expression) override {
    output += "EXIT Expression\n";
  }

  void enter_tuple(const starlark::ast::Tuple* tuple) override {
    output += "ENTER Tuple\n";
  }

  void exit_tuple(const starlark::ast::Tuple* tuple) override {
    output += "EXIT Tuple\n";
  }

  void enter_if_expression(const starlark::ast::IfExpr* if_expression) override {
    output += "ENTER IfExpression\n";
  }

  void mid_if_expression(const starlark::ast::IfExpr* if_expression) override {
    output += "MID IfExpression\n";
  }

  void exit_if_expression(const starlark::ast::IfExpr* if_expression) override {
    output += "EXIT IfExpression\n";
  }

  void enter_unary_expression(const starlark::ast::UnaryExpr* unary_expression) override {
    output += "ENTER UnaryExpression\n";
  }

  void exit_unary_expression(const starlark::ast::UnaryExpr* unary_expression) override {
    output += "EXIT UnaryExpression\n";
  }

  void enter_binary_expression(const starlark::ast::BinaryExpr* binary_expression) override {
    output += "ENTER BinaryExpression\n";
  }

  void mid_binary_expression(const starlark::ast::BinaryExpr* binary_expression) override {
    output += "MID BinaryExpression\n";
  }

  void exit_binary_expression(const starlark::ast::BinaryExpr* binary_expression) override {
    output += "EXIT BinaryExpression\n";
  }

  void enter_lambda_expression(const starlark::ast::LambdaExpr* lambda_expression) override {
    output += "ENTER LambdaExpression\n";
  }

  void mid_lambda_expression(const starlark::ast::LambdaExpr* lambda_expression) override {
    output += "MID LambdaExpression\n";
  }

  void exit_lambda_expression(const starlark::ast::LambdaExpr* lambda_expression) override {
    output += "EXIT LambdaExpression\n";
  }

  void enter_for_loop_variables(const starlark::ast::Expression* loop_variables) override {
    output += "ENTER LoopVariables\n";
  }

  void exit_for_loop_variables(const starlark::ast::Expression* loop_variables) override {
    output += "EXIT LoopVariables\n";
  }

  void enter_for_in_expression(const starlark::ast::Expression* expression) override {
    output += "ENTER ForInExpression\n";
  }

  void exit_for_in_expression(const starlark::ast::Expression* expression) override {
    output += "EXIT ForInExpression\n";
  }

  void enter_dot_expression(const starlark::ast::DotExpr* dot_expression) override {
    output += "ENTER DotExpression\n";
  }

  void exit_dot_expression(const starlark::ast::DotExpr* dot_expression) override {
    output += "EXIT DotExpression\n";
  }

  void enter_call_expression(const starlark::ast::CallExpr* call_expression) override {
    output += "ENTER CallExpression\n";
  }

  void exit_call_expression(const starlark::ast::CallExpr* call_expression) override {
    output += "EXIT CallExpression\n";
  }

  void enter_slice_expression(const starlark::ast::SliceExpr* slice_expression) override {
    switch (slice_expression->slice_type_case()) {
      case SliceExpr::kIndex:
        output += "ENTER SliceIndexExpression\n";
        break;
      case SliceExpr::kSlice:
        output += "ENTER SliceRangeExpression\n";
        break;
      default:
        break;
    }
  }

  void exit_slice_expression(const starlark::ast::SliceExpr* slice_expression) override {
    switch (slice_expression->slice_type_case()) {
      case SliceExpr::kIndex:
        output += "EXIT SliceIndexExpression\n";
        break;
      case SliceExpr::kSlice:
        output += "EXIT SliceRangeExpression\n";
        break;
      default:
        break;
    }
  }

  void enter_identifier(const starlark::ast::Identifier* identifier) override {
    output += "ENTER Identifier(" + starlark::grammar::quoted(identifier->name()) + ")\n";
  }

  void exit_identifier(const starlark::ast::Identifier* identifier) override {
    output += "EXIT Identifier\n";
  }

  void enter_none_value() override {
    output += "ENTER NoneValue\n";
  }

  void exit_none_value() override {
    output += "EXIT NoneValue\n";
  }

  void enter_int_value(std::string_view int_value) override {
    output += "ENTER IntValue(" + starlark::grammar::quoted(int_value) + ")\n";
  }

  void exit_int_value(std::string_view int_value) override {
    output += "EXIT IntValue\n";
  }

  void enter_float_value(double float_value) override {
    output += "ENTER FloatValue(" + std::to_string(float_value) + ")\n";
  }

  void exit_float_value(double float_value) override {
    output += "EXIT FloatValue\n";
  }

  void enter_string_value(std::string_view string_value) override {
    output += "ENTER StringValue(" + starlark::grammar::quoted(string_value) + ")\n";
  }

  void exit_string_value(std::string_view string_value) override {
    output += "EXIT StringValue\n";
  }

  void enter_bytes_value(std::string_view bytes_value) override {
    output += "ENTER BytesValue(" + starlark::grammar::quoted(bytes_value) + ")\n";
  }

  void exit_bytes_value(std::string_view bytes_value) override {
    output += "EXIT BytesValue\n";
  }

  void enter_list_expression(const starlark::ast::ListExpr* list_expression) override {
    output += "ENTER ListExpression\n";
  }

  void mid_list_expression(const starlark::ast::ListExpr* list_expression) override {
    output += "MID ListExpression\n";
  }

  void exit_list_expression(const starlark::ast::ListExpr* list_expression) override {
    output += "EXIT ListExpression\n";
  }

  void enter_list_comprehension(const starlark::ast::ListComp* list_comprehension) override {
    output += "ENTER ListComprehension\n";
  }

  void exit_list_comprehension(const starlark::ast::ListComp* list_comprehension) override {
    output += "EXIT ListComprehension\n";
  }

  void enter_dictionary_expression(const starlark::ast::DictExpr* dictionary_expression) override {
    output += "ENTER DictionaryExpression\n";
  }

  void mid_dictionary_expression(const starlark::ast::DictExpr* dictionary_expression) override {
    output += "MID DictionaryExpression\n";
  }

  void exit_dictionary_expression(const starlark::ast::DictExpr* dictionary_expression) override {
    output += "EXIT DictionaryExpression\n";
  }

  void enter_dictionary_comprehension(const starlark::ast::DictComp* dictionary_comprehension) override {
    output += "ENTER DictionaryComprehension\n";
  }

  void exit_dictionary_comprehension(const starlark::ast::DictComp* dictionary_comprehension) override {
    output += "EXIT DictionaryComprehension\n";
  }

  void enter_comp_clause(const starlark::ast::CompClause* comp_clause) override {
    output += "ENTER CompClause\n";
  }

  void exit_comp_clause(const starlark::ast::CompClause* comp_clause) override {
    output += "EXIT CompClause\n";
  }

  void enter_for_clause(const starlark::ast::ForClause* for_clause) override {
    output += "ENTER ForClause\n";
  }

  void mid_for_clause(const starlark::ast::ForClause* for_clause) override {
    output += "MID ForClause\n";
  }

  void exit_for_clause(const starlark::ast::ForClause* for_clause) override {
    output += "EXIT ForClause\n";
  }

  void enter_if_clause(const starlark::ast::Expression* if_clause) override {
    output += "ENTER IfClause\n";
  }

  void exit_if_clause(const starlark::ast::Expression* if_clause) override {
    output += "EXIT IfClause\n";
  }

  void enter_map_entry(const starlark::ast::Entry* map_entry) override {
    output += "ENTER MapEntry\n";
  }

  void exit_map_entry(const starlark::ast::Entry* map_entry) override {
    output += "EXIT MapEntry\n";
  }

 private:
  std::string& output;
};


TEST(Parser, TestCase) {
  const auto& argv = ::testing::internal::GetArgvs();
  ASSERT_THAT(argv, SizeIs(3));

  std::string starlark_program;
  {
    int starlark_fd = open(argv[1].c_str(), O_RDONLY);
    ASSERT_GT(starlark_fd, 0);
    defer { close(starlark_fd); };
    struct stat sb;
    ASSERT_GE(fstat(starlark_fd, &sb), 0);
    starlark_program.resize(sb.st_size);
    read(starlark_fd, starlark_program.data(), sb.st_size);
  }
  std::string expeted_ast_walking_output;
  {
    int ast_walk_output_fd = open(argv[2].c_str(), O_RDONLY);
    ASSERT_GT(ast_walk_output_fd, 0);
    defer { close(ast_walk_output_fd); };
    struct stat sb;
    ASSERT_GE(fstat(ast_walk_output_fd, &sb), 0);
    expeted_ast_walking_output.resize(sb.st_size);
    read(ast_walk_output_fd, expeted_ast_walking_output.data(), sb.st_size);
  }

  logger logging;
  logging.set_level(log_level::kError);
  starlark::grammar::options opts = starlark::grammar::get_parsing_options(starlark_program);
  parser star_parser(starlark_program, opts, {}, logging);
  google::protobuf::Arena arena;
  File* starlark_file = star_parser.parse_file(arena);
  EXPECT_THAT(logging, IsEmpty()) << show_errors(logging);

  std::string ast_walk_output;
  ast_listener_logger listener(ast_walk_output);
  starlark::grammar::ast_walker walker;
  walker.walk(starlark_file, listener);
  EXPECT_EQ(ast_walk_output, expeted_ast_walking_output) << starlark_file->DebugString();

  starlark::grammar::ast_listener_base base;
  walker.walk(starlark_file, base);
}

}  // namespace

