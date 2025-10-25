// Copyright 2024-2025 Lucas Mirelmann

#include "compiler/compiler.hpp"

#include <map>
#include <set>
#include <string>
#include <vector>

#include "grammar/ast_listener.hpp"
#include "grammar/options.hpp"
#include "grammar/parser.hpp"
#include "logging/logging.hpp"

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
using starlark::ast::ReturnStmt;
using starlark::ast::SliceExpr;
using starlark::ast::Statement;
using starlark::ast::Tuple;
using starlark::ast::UnaryExpr;
using starlark::bytecode::Program;
using starlark::grammar::ast_listener;
using starlark::grammar::ast_listener_base;
using starlark::grammar::options;
using starlark::grammar::parser;
using starlark::logging::log_level;
using starlark::logging::logger;

namespace starlark {
namespace compiler {

namespace {

class bytecode_generator : public ast_listener_base {
 public:
  explicit bytecode_generator(Program& output);
  void enter_load_statement(const LoadStmt* load_statement) override;
  void exit_expression_statement(const Expression* statement) override;
  void enter_none_value() override;
  void enter_int_value(std::string_view int_value) override;
  void enter_float_value(double float_value) override;
  void enter_string_value(std::string_view string_value) override;
  void enter_bytes_value(std::string_view bytes_value) override;
  void enter_identifier(const Identifier* identifier) override;
  void exit_unary_expression(const UnaryExpr* unary_expression) override;
  void mid_binary_expression(const BinaryExpr* binary_expression) override;
  void exit_binary_expression(const BinaryExpr* binary_expression) override;
  void exit_dot_expression(const DotExpr* dot_expression) override;
  void exit_slice_expression(const SliceExpr* slice_expression) override;

  void exit_tuple(const Tuple* tuple) override;
  void enter_list_expression(const ListExpr* list_expression) override;
  void mid_list_expression(const ListExpr* list_expression) override;
  void exit_list_expression(const ListExpr* list_expression) override;
  void enter_dictionary_expression(const DictExpr* dictionary_expression) override;
  void mid_dictionary_expression(const DictExpr* dictionary_expression) override;
  void exit_dictionary_expression(const DictExpr* dictionary_expression) override;

  void enter_list_comprehension(const ListComp* list_comprehension) override;
  void exit_list_comprehension(const ListComp* list_comprehension) override;
  void enter_dictionary_comprehension(const DictComp* dictionary_comprehension) override;
  void exit_dictionary_comprehension(const DictComp* dictionary_comprehension) override;
  void mid_for_clause(const ForClause* for_clause) override;
  void exit_for_clause(const ForClause* for_clause) override;
  void exit_if_clause(const Expression* if_clause) override;

  void mid_if_expression(const IfExpr* if_expression) override;
  void exit_if_expression(const IfExpr* if_expression) override;

  void enter_for_statement(const ForStmt* for_statement) override;
  void mid_for_statement(const ForStmt* for_statement) override;
  void exit_for_statement(const ForStmt* for_statement) override;
  void exit_break_statement(const BreakStmt* break_statement) override;
  void exit_continue_statement(const ContinueStmt* continue_statement) override;

  void enter_if_statement(const IfStmt* if_statement) override;
  void exit_if_statement(const IfStmt* if_statement) override;
  void enter_then(const RepeatedPtrField<Statement>* then) override;
  void exit_then(const RepeatedPtrField<Statement>* then) override;

  void exit_call_expression(const CallExpr* call_expression) override;
  void enter_argument(const Argument* argument) override;
  void exit_argument(const Argument* argument) override;

  void exit_assign_statement(const AssignStmt* assign_statement) override;

  void exit_return_statement(const ReturnStmt* return_statement) override;

  void mid_lambda_expression(const LambdaExpr* lambda_expression) override;
  void exit_lambda_expression(const LambdaExpr* lambda_expression) override;
  void mid_def_statement(const DefStmt* def_statement) override;
  void exit_def_statement(const DefStmt* def_statement) override;

  void exit_file(const File* starlark_file) override;

 private:
  Program& output;
  std::map<const BinaryExpr*, uint64_t> binary_op_mid_pos;
  std::map<const IfExpr*, uint64_t> if_expression_op_mid_pos;
  std::map<const RepeatedPtrField<Statement>*, uint64_t> if_statement_then;
  std::map<const RepeatedPtrField<Parameter>*, uint64_t> def_or_lambda_expression_goto;
  std::vector<std::vector<uint64_t>> if_statement_to_fix_to_the_end;

  std::map<const ForStmt*, uint64_t> for_statement_op_mid_pos;
  std::vector<std::vector<uint64_t>> for_statement_op_continue;
  std::vector<std::vector<uint64_t>> for_statement_op_break;
  std::vector<std::vector<uint64_t>> comprehension_comp_clause;

  void fix_comp_clause(const RepeatedPtrField<CompClause>& clauses);
  void mid_def_or_lambda_expression(const RepeatedPtrField<Parameter>* params);
  void exit_def_or_lambda_expression(const RepeatedPtrField<Parameter>* params);
};

bytecode_generator::bytecode_generator(Program& output) : output(output) {}

void bytecode_generator::enter_load_statement(const LoadStmt* load_statement) {
  auto* load_op = output.add_op_code()->mutable_load_module();
  load_op->set_module(load_statement->module());
  for (const auto& symbol : load_statement->load_param()) {
    auto* load_param = load_op->add_value();
    load_param->set_identifier(symbol.local_name().nfkc_name());
    load_param->set_remote_symbol(symbol.remote_name());
  }
}

void bytecode_generator::exit_expression_statement(const Expression* statement) {
  output.add_op_code()->mutable_drop();
}

void bytecode_generator::enter_none_value() {
  output.add_op_code()->mutable_const_none();
}

void bytecode_generator::enter_int_value(std::string_view int_value) {
  output.add_op_code()->mutable_const_int()->set_value(int_value);
}

void bytecode_generator::enter_float_value(double float_value) {
  output.add_op_code()->mutable_const_float()->set_value(float_value);
}

void bytecode_generator::enter_string_value(std::string_view string_value) {
  output.add_op_code()->mutable_const_string()->set_value(string_value);
}

void bytecode_generator::enter_bytes_value(std::string_view bytes_value) {
  output.add_op_code()->mutable_const_bytes()->set_value(bytes_value);
}

void bytecode_generator::enter_identifier(const Identifier* identifier) {
  auto* id_op = output.add_op_code()->mutable_load();
  id_op->set_frame(identifier->frame());
  id_op->set_pos_in_frame(identifier->pos_in_frame());
}

void bytecode_generator::exit_unary_expression(const UnaryExpr* unary_expression) {
  switch (unary_expression->operator_()) {
    case UnaryExpr::PLUS:
      output.add_op_code()->mutable_unary_plus();
      break;
    case UnaryExpr::MINUS:
      output.add_op_code()->mutable_unary_minus();
      break;
    case UnaryExpr::TILDE:
      output.add_op_code()->mutable_unary_tilde();
      break;
    case UnaryExpr::NOT:
      output.add_op_code()->mutable_unary_not();
      break;
    default:
      break;
  }
}

void bytecode_generator::mid_binary_expression(const BinaryExpr* binary_expression) {
  binary_op_mid_pos[binary_expression] = output.op_code_size();
  switch (binary_expression->operator_()) {
    case BinaryExpr::OR:
      output.add_op_code()->mutable_dup();
      output.add_op_code()->mutable_if_true();
      output.add_op_code()->mutable_drop();
      break;
    case BinaryExpr::AND:
      output.add_op_code()->mutable_dup();
      output.add_op_code()->mutable_if_false();
      output.add_op_code()->mutable_drop();
      break;
    default:
      break;
  }
}

void bytecode_generator::exit_binary_expression(const BinaryExpr* binary_expression) {
  switch (binary_expression->operator_()) {
    case BinaryExpr::OR:
      output.mutable_op_code(binary_op_mid_pos[binary_expression] + 1)->mutable_if_true()->set_address(output.op_code_size());
      break;
    case BinaryExpr::AND:
      output.mutable_op_code(binary_op_mid_pos[binary_expression] + 1)->mutable_if_false()->set_address(output.op_code_size());
      break;
    case BinaryExpr::EQUALS_EQUALS:
      output.add_op_code()->mutable_binary_equals_equals();
      break;
    case BinaryExpr::BANG_EQUALS:
      output.add_op_code()->mutable_binary_bang_equals();
      break;
    case BinaryExpr::LESS_THAN:
      output.add_op_code()->mutable_binary_less_than();
      break;
    case BinaryExpr::GREATER_THAN:
      output.add_op_code()->mutable_binary_greater_than();
      break;
    case BinaryExpr::LESS_THAN_EQUALS:
      output.add_op_code()->mutable_binary_less_than_equals();
      break;
    case BinaryExpr::GREATER_THAN_EQUALS:
      output.add_op_code()->mutable_binary_greater_than_equals();
      break;
    case BinaryExpr::IN:
      output.add_op_code()->mutable_binary_in();
      break;
    case BinaryExpr::NOT_IN:
      output.add_op_code()->mutable_binary_not_in();
      break;
    case BinaryExpr::PIPE:
      output.add_op_code()->mutable_binary_pipe();
      break;
    case BinaryExpr::HAT:
      output.add_op_code()->mutable_binary_hat();
      break;
    case BinaryExpr::AMPERSAND:
      output.add_op_code()->mutable_binary_ampersand();
      break;
    case BinaryExpr::LESS_THAN_LESS_THAN:
      output.add_op_code()->mutable_binary_less_than_less_than();
      break;
    case BinaryExpr::GREATER_THAN_GREATER_THAN:
      output.add_op_code()->mutable_binary_greater_than_greater_than();
      break;
    case BinaryExpr::MINUS:
      output.add_op_code()->mutable_binary_minus();
      break;
    case BinaryExpr::PLUS:
      output.add_op_code()->mutable_binary_plus();
      break;
    case BinaryExpr::STAR:
      output.add_op_code()->mutable_binary_star();
      break;
    case BinaryExpr::PERCENT:
      output.add_op_code()->mutable_binary_percent();
      break;
    case BinaryExpr::SLASH:
      output.add_op_code()->mutable_binary_slash();
      break;
    case BinaryExpr::SLASH_SLASH:
      output.add_op_code()->mutable_binary_slash_slash();
      break;
    default:
      break;
  }
  binary_op_mid_pos.erase(binary_expression);
}

void bytecode_generator::exit_dot_expression(const DotExpr* dot_expression) {
  output.add_op_code()->mutable_dot_member()->set_member(dot_expression->identifier().nfkc_name());
}

void bytecode_generator::exit_slice_expression(const SliceExpr* slice_expression) {
  switch (slice_expression->slice_type_case()) {
    case SliceExpr::kIndex:
      output.add_op_code()->mutable_index_member();
      break;
    case SliceExpr::kSlice:
      output.add_op_code()->mutable_slice_range();
      break;
    default:
      break;
  }
}

void bytecode_generator::exit_tuple(const Tuple* tuple) {
  output.add_op_code()->mutable_make_tuple()->set_number_of_elements(tuple->value_size());
}

void bytecode_generator::enter_list_expression(const ListExpr* list_expression) {
  output.add_op_code()->mutable_make_list()->set_reserve_size(list_expression->element_size());
}

void bytecode_generator::mid_list_expression(const ListExpr* list_expression) {
  output.add_op_code()->mutable_add_to_list()->set_pos(1);
}

void bytecode_generator::exit_list_expression(const ListExpr* list_expression) {
  if (list_expression->element_size() != 0) {
    output.add_op_code()->mutable_add_to_list()->set_pos(1);
  }
}

void bytecode_generator::enter_dictionary_expression(const DictExpr* dictionary_expression) {
  output.add_op_code()->mutable_make_dictionary()->set_reserve_size(dictionary_expression->entry_size());
}

void bytecode_generator::mid_dictionary_expression(const DictExpr* dictionary_expression) {
  // TODO(lmirelmann): I think it would be better not to generate the `make_tuple` entry
  //     and change `add_to_dictionary` to take two elements from the stack.
  //     The underlying issue is that dictionary comprehension expressions still takes a tuple.
  output.add_op_code()->mutable_make_tuple()->set_number_of_elements(2);
  output.add_op_code()->mutable_add_to_dictionary()->set_pos(1);
}

void bytecode_generator::exit_dictionary_expression(const DictExpr* dictionary_expression) {
  if (dictionary_expression->entry_size() != 0) {
    output.add_op_code()->mutable_make_tuple()->set_number_of_elements(2);
    output.add_op_code()->mutable_add_to_dictionary()->set_pos(1);
  }
}

void bytecode_generator::enter_list_comprehension(const ListComp* list_comprehension) {
  comprehension_comp_clause.push_back({});
  output.add_op_code()->mutable_make_list()->set_reserve_size(0);
}

void bytecode_generator::exit_list_comprehension(const ListComp* list_comprehension) {
  int number_for_clauses = 0;
  for (const auto& clause : list_comprehension->clause()) {
    if (clause.comp_clause_type_case() == CompClause::kForClause) {
      number_for_clauses++;
    }
  }
  output.add_op_code()->mutable_add_to_list()->set_pos(number_for_clauses + 1);
  fix_comp_clause(list_comprehension->clause());
  comprehension_comp_clause.pop_back();
}

void bytecode_generator::enter_dictionary_comprehension(const DictComp* dictionary_comprehension) {
  comprehension_comp_clause.push_back({});
  output.add_op_code()->mutable_make_dictionary()->set_reserve_size(0);
}

void bytecode_generator::exit_dictionary_comprehension(const DictComp* dictionary_comprehension) {
  int number_for_clauses = 0;
  for (const auto& clause : dictionary_comprehension->clause()) {
    if (clause.comp_clause_type_case() == CompClause::kForClause) {
      number_for_clauses++;
    }
  }
  output.add_op_code()->mutable_add_to_dictionary()->set_pos(number_for_clauses + 1);
  fix_comp_clause(dictionary_comprehension->clause());
  comprehension_comp_clause.pop_back();
}

void bytecode_generator::mid_for_clause(const ForClause* for_clause) {
  output.add_op_code()->mutable_get_iterator();
  comprehension_comp_clause.back().push_back(output.op_code_size());
  output.add_op_code()->mutable_for_iterator();
}

void bytecode_generator::exit_for_clause(const ForClause* for_clause) {
  output.add_op_code()->mutable_assign();
}

void bytecode_generator::exit_if_clause(const Expression* if_clause) {
  comprehension_comp_clause.back().push_back(output.op_code_size());
  output.add_op_code()->mutable_if_false();
}

void bytecode_generator::mid_if_expression(const IfExpr* if_expression) {
  auto op_code_size = output.op_code_size();
  if (!if_expression_op_mid_pos.contains(if_expression)) {
    // This is the first time this is called for this `if expression`.
    if_expression_op_mid_pos[if_expression] = op_code_size;
    output.add_op_code()->mutable_if_false();
  } else {
    output.mutable_op_code(if_expression_op_mid_pos[if_expression])->mutable_if_false()->set_address(op_code_size + 1);
    if_expression_op_mid_pos[if_expression] = op_code_size;
    output.add_op_code()->mutable_goto_();
  }
}

void bytecode_generator::exit_if_expression(const IfExpr* if_expression) {
  auto op_code_size = output.op_code_size();
  output.mutable_op_code(if_expression_op_mid_pos[if_expression])->mutable_goto_()->set_address(op_code_size);
  if_expression_op_mid_pos.erase(if_expression);
}

void bytecode_generator::enter_for_statement(const ForStmt* for_statement) {
  for_statement_op_continue.push_back({});
  for_statement_op_break.push_back({});
}

void bytecode_generator::mid_for_statement(const ForStmt* for_statement) {
  auto op_code_size = output.op_code_size();
  if (!for_statement_op_mid_pos.contains(for_statement)) {
    // This is the first time this is called for this `for statement`.
    for_statement_op_mid_pos[for_statement] = op_code_size;
    output.add_op_code()->mutable_get_iterator();
    output.add_op_code()->mutable_for_iterator();
  } else {
    output.add_op_code()->mutable_assign();
  }
}

void bytecode_generator::exit_for_statement(const ForStmt* for_statement) {
  auto op_code_size = output.op_code_size();
  auto begin_address = for_statement_op_mid_pos[for_statement] + 1;
  output.add_op_code()->mutable_goto_()->set_address(begin_address);
  output.mutable_op_code(for_statement_op_mid_pos[for_statement] + 1)->mutable_for_iterator()->set_address(op_code_size + 1);
  output.add_op_code()->mutable_end_iterator();

  // Fix `break` and `continue` statements.
  for (auto i : for_statement_op_break.back()) {
    output.mutable_op_code(i)->mutable_goto_()->set_address(op_code_size + 1);
  }
  for (auto i : for_statement_op_continue.back()) {
    output.mutable_op_code(i)->mutable_goto_()->set_address(begin_address);
  }

  // Cleanup.
  for_statement_op_continue.pop_back();
  for_statement_op_break.pop_back();
  for_statement_op_mid_pos.erase(for_statement);
}

void bytecode_generator::exit_break_statement(const BreakStmt* break_statement) {
  for_statement_op_break.back().push_back(output.op_code_size());
  output.add_op_code()->mutable_goto_();
}

void bytecode_generator::exit_continue_statement(const ContinueStmt* continue_statement) {
  for_statement_op_continue.back().push_back(output.op_code_size());
  output.add_op_code()->mutable_goto_();
}

void bytecode_generator::enter_if_statement(const IfStmt* if_statement) {
  if_statement_to_fix_to_the_end.push_back({});
}

void bytecode_generator::exit_if_statement(const IfStmt* if_statement) {
  auto op_code_size = output.op_code_size();
  for (auto pos : if_statement_to_fix_to_the_end.back()) {
    output.mutable_op_code(pos)->mutable_goto_()->set_address(op_code_size);
  }
  if_statement_to_fix_to_the_end.pop_back();
}

void bytecode_generator::enter_then(const RepeatedPtrField<Statement>* then) {
  if_statement_then[then] = output.op_code_size();
  output.add_op_code()->mutable_if_false();
}

void bytecode_generator::exit_then(const RepeatedPtrField<Statement>* then) {
  auto op_code_size = output.op_code_size();
  output.mutable_op_code(if_statement_then[then])->mutable_if_false()->set_address(op_code_size + 1);
  output.add_op_code()->mutable_goto_();
  if_statement_to_fix_to_the_end.back().push_back(op_code_size);

  if_statement_then.erase(then);
}

void bytecode_generator::exit_call_expression(const CallExpr* call_expression) {
  int pos_arguments = 0;
  int named_arguments = 0;
  bool varadic_pos_arg = false;
  bool varadic_named_arg = false;

  for (const auto& arg : call_expression->argument()) {
    switch (arg.argument_type_case()) {
      case Argument::kValue:
        pos_arguments++;
        break;
      case Argument::kNamedArgument:
        named_arguments++;
        break;
      case Argument::kStarArgument:
        varadic_pos_arg = true;
        break;
      case Argument::kStarStarArgument:
        varadic_named_arg = true;
        break;
      case Argument::ARGUMENT_TYPE_NOT_SET:
        break;
    }
  }
  auto* call = output.add_op_code()->mutable_call();
  call->set_positional_arguments_count(pos_arguments);
  call->set_named_arguments_count(named_arguments);
  call->set_has_varadic_positional_argument(varadic_pos_arg);
  call->set_has_varadic_named_argument(varadic_named_arg);
}

void bytecode_generator::enter_argument(const Argument* argument) {
  if (argument->argument_type_case() == Argument::kNamedArgument) {
    output.add_op_code()->mutable_const_string()->set_value(argument->named_argument().identifier().nfkc_name());
  }
}

void bytecode_generator::exit_argument(const Argument* argument) {
  if (argument->argument_type_case() == Argument::kNamedArgument) {
    output.add_op_code()->mutable_make_tuple()->set_number_of_elements(2);
  }
}

void bytecode_generator::exit_assign_statement(const AssignStmt* assign_statement) {
  switch (assign_statement->op()) {
    case AssignStmt::EQUALS:
      output.add_op_code()->mutable_assign();
      break;
    case AssignStmt::PLUS_EQUALS:
      output.add_op_code()->mutable_plus_assign();
      break;
    case AssignStmt::MINUS_EQUALS:
      output.add_op_code()->mutable_minus_assign();
      break;
    case AssignStmt::STAR_EQUALS:
      output.add_op_code()->mutable_star_assign();
      break;
    case AssignStmt::SLASH_EQUALS:
      output.add_op_code()->mutable_slash_assign();
      break;
    case AssignStmt::SLASH_SLASH_EQUALS:
      output.add_op_code()->mutable_slash_slash_assign();
      break;
    case AssignStmt::PERCENT_EQUALS:
      output.add_op_code()->mutable_percent_assign();
      break;
    case AssignStmt::AMPERSAND_EQUALS:
      output.add_op_code()->mutable_ampersand_assign();
      break;
    case AssignStmt::PIPE_EQUALS:
      output.add_op_code()->mutable_pipe_assign();
      break;
    case AssignStmt::HAT_EQUALS:
      output.add_op_code()->mutable_hat_assign();
      break;
    case AssignStmt::LESS_LESS_EQUALS:
      output.add_op_code()->mutable_less_less_assign();
      break;
    case AssignStmt::GREATER_GREATER_EQUALS:
      output.add_op_code()->mutable_greater_greater_assign();
      break;
    default:
      break;
  }
}

void bytecode_generator::exit_return_statement(const ReturnStmt* return_statement) {
  output.add_op_code()->mutable_return_();
}

void bytecode_generator::mid_lambda_expression(const LambdaExpr* lambda_expression) {
  mid_def_or_lambda_expression(&lambda_expression->parameter());
}

void bytecode_generator::exit_lambda_expression(const LambdaExpr* lambda_expression) {
  output.add_op_code()->mutable_return_();
  exit_def_or_lambda_expression(&lambda_expression->parameter());
}

void bytecode_generator::mid_def_statement(const DefStmt* def_statement) {
  mid_def_or_lambda_expression(&def_statement->parameter());
}
void bytecode_generator::exit_def_statement(const DefStmt* def_statement) {
  if (!output.op_code().rbegin()->has_return_()) {
    output.add_op_code()->mutable_const_none();
    output.add_op_code()->mutable_return_();
  }
  exit_def_or_lambda_expression(&def_statement->parameter());
  auto* id_op = output.add_op_code()->mutable_load();
  id_op->set_frame(def_statement->function_name().frame());
  id_op->set_pos_in_frame(def_statement->function_name().pos_in_frame());
  output.add_op_code()->mutable_assign();
}

void bytecode_generator::mid_def_or_lambda_expression(const RepeatedPtrField<Parameter>* params) {
  // Capture the function signature.
  auto signature_pos = output.function_signature_size();
  auto* function_signature = output.add_function_signature();
  int arguments_with_defaults_count = 0;
  function_signature->set_has_star_argument(false);
  function_signature->set_has_star_star_argument(false);
  for (const auto& param : *params) {
    function_signature->add_name(param.identifier().nfkc_name());
    switch (param.parameter_type_case()) {
      case Parameter::kInitialization:
        ++arguments_with_defaults_count;
        break;
      case Parameter::kStar:
        function_signature->set_has_star_argument(true);
        break;
      case Parameter::kStarStar:
        function_signature->set_has_star_star_argument(true);
        break;
      default:
        break;
    }
  }
  function_signature->set_default_arguments_count(arguments_with_defaults_count);

  // Maybe prepare the default arguments.
  if (arguments_with_defaults_count > 0) {
    output.add_op_code()->mutable_make_tuple()->set_number_of_elements(arguments_with_defaults_count);
  }
  // Call make_function with the signature.
  auto* make_function = output.add_op_code()->mutable_make_function();
  make_function->set_signature(signature_pos);

  // Maybe store the default arguments.
  if (arguments_with_defaults_count > 0) {
    output.add_op_code()->mutable_set_default_values();
  }
  def_or_lambda_expression_goto[params] = output.op_code_size();
  make_function->set_entrypoint(output.op_code_size() + 1);
  // Create the goto to skip the bytecodes.
  output.add_op_code();
}

void bytecode_generator::exit_def_or_lambda_expression(const RepeatedPtrField<Parameter>* params) {
  // Fix the goto and clean up the cache.
  output.mutable_op_code(def_or_lambda_expression_goto[params])->mutable_goto_()->set_address(output.op_code_size());
  def_or_lambda_expression_goto.erase(params);
}

void bytecode_generator::exit_file(const File* starlark_file) {
  output.add_op_code()->mutable_end();
}

void bytecode_generator::fix_comp_clause(const RepeatedPtrField<CompClause>& clauses) {
  int clause_pos = 0;
  auto clauses_count = clauses.size();
  for (auto it = clauses.rbegin(); it != clauses.rend(); ++it) {
    const auto& clause = *it;
    if (clause.comp_clause_type_case() == CompClause::kForClause) {
      output.mutable_op_code(comprehension_comp_clause.back()[clauses_count - clause_pos - 1])
          ->mutable_for_iterator()->set_address(output.op_code_size());
      output.add_op_code()->mutable_end_iterator();
    } else {
      output.mutable_op_code(comprehension_comp_clause.back()[clauses_count - clause_pos - 1])
          ->mutable_if_false()->set_address(output.op_code_size());
    }
    clause_pos++;
  }
}

}  // namespace

compiler::compiler(std::set<std::string, std::less<>>& binding) : binding(binding) {}

Program compiler::compile(std::string_view starlark_program) {
  // TODO(lmirelmann): The logger should be a parameter, but there should be a way to know
  // whether the parser generated an error.
  logger logging;
  logging.set_level(log_level::kWarning);
  // TODO(lmirelmann): The extra symbols should be configurable.
  std::set<std::string, std::less<>> extra_symbols;
  parser star_parser(starlark_program,
                     // TODO(lmirelmann): Grammar options should be configurable.
                     options{},
                     extra_symbols,
                     logging);
  google::protobuf::Arena arena;
  File* starlark_file = star_parser.parse_file(arena);

  // TODO(lmirelmann): If there are errors, then return early.

  Program result;
  bytecode_generator listener(result);
  grammar::ast_walker walker;
  walker.walk(starlark_file, listener);
  return result;
}

}  // namespace compiler
}  // namespace starlark

