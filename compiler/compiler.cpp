// Copyright 2024-2026 Lucas Mirelmann

#include "compiler/compiler.hpp"

#include <cassert>

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "grammar/ast_listener.hpp"
#include "grammar/options.hpp"
#include "grammar/parser.hpp"
#include "logging/logging.hpp"

using ::google::protobuf::Arena;
using ::google::protobuf::RepeatedPtrField;
using ::starlark::ast::Argument;
using ::starlark::ast::AssignStmt;
using ::starlark::ast::BinaryExpr;
using ::starlark::ast::BreakStmt;
using ::starlark::ast::CallExpr;
using ::starlark::ast::CompClause;
using ::starlark::ast::ContinueStmt;
using ::starlark::ast::DefStmt;
using ::starlark::ast::DictComp;
using ::starlark::ast::DictExpr;
using ::starlark::ast::DotExpr;
using ::starlark::ast::Expression;
using ::starlark::ast::File;
using ::starlark::ast::FloatValue;
using ::starlark::ast::ForClause;
using ::starlark::ast::ForStmt;
using ::starlark::ast::Identifier;
using ::starlark::ast::IfClause;
using ::starlark::ast::IfExpr;
using ::starlark::ast::IfStmt;
using ::starlark::ast::IntValue;
using ::starlark::ast::LambdaExpr;
using ::starlark::ast::ListComp;
using ::starlark::ast::ListExpr;
using ::starlark::ast::LoadStmt;
using ::starlark::ast::Parameter;
using ::starlark::ast::ReturnStmt;
using ::starlark::ast::SliceExpr;
using ::starlark::ast::Statement;
using ::starlark::ast::StringValue;
using ::starlark::ast::Tuple;
using ::starlark::ast::UnaryExpr;
using ::starlark::bytecode::Block;
using ::starlark::bytecode::BlockType;
using ::starlark::bytecode::OpCode;
using ::starlark::bytecode::Program;
using ::starlark::grammar::ast_listener;
using ::starlark::grammar::ast_listener_base;
using ::starlark::grammar::grammar_options;
using ::starlark::grammar::parser;
using ::starlark::logging::LogLevel;
using ::starlark::logging::logger;
using ::starlark::logging::logger_wrap;

namespace starlark {
namespace compiler {

namespace {

class bytecode_generator : public ast_listener_base {
 public:
  explicit bytecode_generator(Program& output);
  void enter_file(const File* starlark_file) override;
  void exit_file(const File* starlark_file) override;
  void enter_load_statement(const LoadStmt* load_statement) override;
  void exit_expression_statement(const Expression* statement) override;
  void enter_none_value() override;
  void enter_int_value(const IntValue* int_value) override;
  void enter_big_int_value(const StringValue* big_int_value) override;
  void enter_float_value(const FloatValue* float_value) override;
  void enter_string_value(const StringValue* string_value) override;
  void enter_bytes_value(const StringValue* bytes_value) override;
  void enter_identifier(const Identifier* identifier) override;
  void exit_identifier_for_assignment(const Identifier* identifier, AssignStmt::AssignOperator op) override;
  void exit_unary_expression(const UnaryExpr* unary_expression) override;
  void mid_binary_expression(const BinaryExpr* binary_expression) override;
  void exit_binary_expression(const BinaryExpr* binary_expression) override;
  void exit_dot_expression(const DotExpr* dot_expression) override;
  void exit_dot_expression_for_assignment(const DotExpr* dot_expression, AssignStmt::AssignOperator op) override;
  void exit_slice_expression(const SliceExpr* slice_expression) override;
  void exit_slice_expression_for_assignment(const SliceExpr* slice_expression, AssignStmt::AssignOperator op) override;
  void exit_assign_statement(const AssignStmt* assign_statement) override;

  void exit_tuple(const Tuple* tuple) override;
  void enter_tuple_for_assignment(const Tuple* tuple) override;
  void enter_list_expression(const ListExpr* list_expression) override;
  void exit_list_expression(const ListExpr* list_expression) override;
  void enter_list_expression_for_assignment(const ListExpr* list_expression) override;
  void enter_dictionary_expression(const DictExpr* dictionary_expression) override;
  void exit_dictionary_expression(const DictExpr* dictionary_expression) override;

  void enter_list_comprehension(const ListComp* list_comprehension) override;
  void exit_list_comprehension(const ListComp* list_comprehension) override;
  void enter_dictionary_comprehension(const DictComp* dictionary_comprehension) override;
  void exit_dictionary_comprehension(const DictComp* dictionary_comprehension) override;
  void mid_for_clause(const ForClause* for_clause) override;
  void exit_if_clause(const IfClause* if_clause) override;

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

  void exit_return_statement(const ReturnStmt* return_statement) override;

  void mid_lambda_expression(const LambdaExpr* lambda_expression) override;
  void exit_lambda_expression(const LambdaExpr* lambda_expression) override;
  void mid_def_statement(const DefStmt* def_statement) override;
  void exit_def_statement(const DefStmt* def_statement) override;

 private:
  Program& output;
  std::map<const BinaryExpr*, uint64_t> binary_op_mid_pos;
  std::map<const IfExpr*, uint64_t> if_expression_op_mid_pos;
  std::map<const RepeatedPtrField<Statement>*, uint64_t> if_statement_then;
  std::vector<OpCode*> for_unpack;
  std::vector<std::vector<uint64_t>> if_statement_to_fix_to_the_end;
  std::vector<int> blocks;

  std::map<const ForStmt*, uint64_t> for_statement_op_mid_pos;
  std::vector<std::vector<uint64_t>> for_statement_op_continue;
  std::vector<std::vector<uint64_t>> for_statement_op_break;
  std::vector<std::vector<uint64_t>> comprehension_comp_clause;

  void fix_comp_clause(const RepeatedPtrField<CompClause>& clauses);
  void mid_def_or_lambda_expression(std::string_view fn_name, const RepeatedPtrField<Parameter>* params);
  void exit_def_or_lambda_expression(const RepeatedPtrField<Parameter>* params);
  Block* mutable_block();
  const Block& block() const;
  std::map<std::string, int64_t> const_string;
};

bytecode_generator::bytecode_generator(Program& output) : output(output) {}

void bytecode_generator::enter_file(const File* starlark_file) {
  blocks.push_back(0);
  output.add_block();
  auto* predeclared_block = mutable_block()->add_op_code()->mutable_create_frame();
  predeclared_block->set_block_type(BlockType::PREDECLARED_BLOCK);
  for (const auto& symbol : starlark_file->global_binding()) {
    predeclared_block->add_symbol(symbol);
  }
  auto* module_block = mutable_block()->add_op_code()->mutable_create_frame();
  module_block->set_block_type(BlockType::MODULE_BLOCK);
  for (const auto& symbol : starlark_file->module_binding()) {
    module_block->add_symbol(symbol);
  }
  auto* file_block = mutable_block()->add_op_code()->mutable_create_frame();
  file_block->set_block_type(BlockType::FILE_BLOCK);
  for (const auto& symbol : starlark_file->file_binding()) {
    file_block->add_symbol(symbol);
  }
}

void bytecode_generator::exit_file(const File* starlark_file) {
  assert(blocks.size() == 1);
  assert(blocks.back() == 0);
  mutable_block()->add_op_code()->mutable_pop_frame();
  mutable_block()->add_op_code()->mutable_pop_frame();
  mutable_block()->add_op_code()->mutable_pop_frame();
  mutable_block()->add_op_code()->mutable_end();
}

void bytecode_generator::enter_load_statement(const LoadStmt* load_statement) {
  auto* load_op = mutable_block()->add_op_code()->mutable_load_module();
  load_op->set_module(load_statement->module());
  for (const auto& symbol : load_statement->load_param()) {
    auto* load_param = load_op->add_value();
    load_param->set_remote_symbol(symbol.remote_name());
    load_param->mutable_pos()->set_frame(symbol.local_name().frame());
    load_param->mutable_pos()->set_pos_in_frame(symbol.local_name().pos_in_frame());
  }
}

void bytecode_generator::exit_expression_statement(const Expression* statement) {
  mutable_block()->add_op_code()->mutable_pop();
}

void bytecode_generator::enter_none_value() {
  mutable_block()->add_op_code()->mutable_const_none();
}

void bytecode_generator::enter_int_value(const IntValue* int_value) {
  mutable_block()->add_op_code()->mutable_const_int()->set_value(int_value->value());
}

void bytecode_generator::enter_big_int_value(const StringValue* big_int_value) {
  mutable_block()->add_op_code()->mutable_const_big_int()->set_value(big_int_value->value());
}

void bytecode_generator::enter_float_value(const FloatValue* float_value) {
  mutable_block()->add_op_code()->mutable_const_float()->set_value(float_value->value());
}

void bytecode_generator::enter_string_value(const StringValue* string_value) {
  auto* const_string_op = mutable_block()->add_op_code();
  auto it = const_string.insert({std::string{string_value->value()}, const_string.size()});
  const_string_op->mutable_const_string()->set_const_string_pos(it.first->second);
  if (it.second) {
    output.add_const_string(string_value->value());
  }
  *const_string_op->mutable_sh()->mutable_start() = string_value->pif().start();
  *const_string_op->mutable_sh()->mutable_highlight_start() = string_value->pif().start();
  *const_string_op->mutable_sh()->mutable_highlight_end() = string_value->pif().end();
  *const_string_op->mutable_sh()->mutable_end() = string_value->pif().end();
}

void bytecode_generator::enter_bytes_value(const StringValue* bytes_value) {
  auto* const_bytes_op = mutable_block()->add_op_code();
  const_bytes_op->mutable_const_bytes()->set_value(bytes_value->value());
  *const_bytes_op->mutable_sh()->mutable_start() = bytes_value->pif().start();
  *const_bytes_op->mutable_sh()->mutable_highlight_start() = bytes_value->pif().start();
  *const_bytes_op->mutable_sh()->mutable_highlight_end() = bytes_value->pif().end();
  *const_bytes_op->mutable_sh()->mutable_end() = bytes_value->pif().end();
}

void bytecode_generator::enter_identifier(const Identifier* identifier) {
  auto* op_code = mutable_block()->add_op_code();
  auto* id_op = op_code->mutable_load();
  id_op->set_frame(identifier->frame());
  id_op->set_pos_in_frame(identifier->pos_in_frame());
  *op_code->mutable_sh()->mutable_start() = identifier->pif().start();
  *op_code->mutable_sh()->mutable_highlight_start() = identifier->pif().start();
  *op_code->mutable_sh()->mutable_highlight_end() = identifier->pif().end();
  *op_code->mutable_sh()->mutable_end() = identifier->pif().end();
}

void bytecode_generator::exit_identifier_for_assignment(const Identifier* identifier, AssignStmt::AssignOperator op) {
  if (op != AssignStmt::EQUALS) {
    return;
  }
  auto* id_op = mutable_block()->add_op_code()->mutable_store();
  id_op->set_frame(identifier->frame());
  id_op->set_pos_in_frame(identifier->pos_in_frame());
}

void bytecode_generator::exit_unary_expression(const UnaryExpr* unary_expression) {
  OpCode* op_code = nullptr;
  switch (unary_expression->operator_()) {
    case UnaryExpr::PLUS: {
      op_code = mutable_block()->add_op_code();
      op_code->mutable_unary_plus();
      break;
    }
    case UnaryExpr::MINUS: {
      op_code = mutable_block()->add_op_code();
      op_code->mutable_unary_minus();
      break;
    }
    case UnaryExpr::TILDE: {
      op_code = mutable_block()->add_op_code();
      op_code->mutable_unary_tilde();
      break;
    }
    case UnaryExpr::NOT: {
      op_code = mutable_block()->add_op_code();
      op_code->mutable_unary_not();
      break;
    }
    default:
      assert(false);
  }
  if (op_code != nullptr) {
    *op_code->mutable_sh()->mutable_start() = unary_expression->op_pif().start();
    *op_code->mutable_sh()->mutable_highlight_start() = unary_expression->op_pif().end();
    *op_code->mutable_sh()->mutable_highlight_end() = unary_expression->pif().end();
    *op_code->mutable_sh()->mutable_end() = unary_expression->pif().end();
  }
}

void bytecode_generator::mid_binary_expression(const BinaryExpr* binary_expression) {
  binary_op_mid_pos[binary_expression] = block().op_code_size();
  switch (binary_expression->operator_()) {
    case BinaryExpr::OR:
      mutable_block()->add_op_code()->mutable_jump_if_true_or_pop();
      break;
    case BinaryExpr::AND:
      mutable_block()->add_op_code()->mutable_jump_if_false_or_pop();
      break;
    case BinaryExpr::EQUALS_EQUALS:
    case BinaryExpr::BANG_EQUALS:
    case BinaryExpr::LESS_THAN:
    case BinaryExpr::GREATER_THAN:
    case BinaryExpr::LESS_THAN_EQUALS:
    case BinaryExpr::GREATER_THAN_EQUALS:
    case BinaryExpr::IN:
    case BinaryExpr::NOT_IN:
    case BinaryExpr::PIPE:
    case BinaryExpr::HAT:
    case BinaryExpr::AMPERSAND:
    case BinaryExpr::LESS_THAN_LESS_THAN:
    case BinaryExpr::GREATER_THAN_GREATER_THAN:
    case BinaryExpr::MINUS:
    case BinaryExpr::PLUS:
    case BinaryExpr::STAR:
    case BinaryExpr::PERCENT:
    case BinaryExpr::SLASH:
    case BinaryExpr::SLASH_SLASH:
      break;
    default:
      assert(false);
  }
}

void bytecode_generator::exit_binary_expression(const BinaryExpr* binary_expression) {
  OpCode* op_code = nullptr;
  switch (binary_expression->operator_()) {
    case BinaryExpr::OR: {
      auto op_pos = binary_op_mid_pos[binary_expression];
      op_code = mutable_block()->mutable_op_code(op_pos);
      op_code->mutable_jump_if_true_or_pop()->set_address_delta(block().op_code_size() - op_pos);
      break;
    }
    case BinaryExpr::AND: {
      auto op_pos = binary_op_mid_pos[binary_expression];
      op_code = mutable_block()->mutable_op_code(op_pos);
      op_code->mutable_jump_if_false_or_pop()->set_address_delta(block().op_code_size() - op_pos);
      break;
    }
    case BinaryExpr::EQUALS_EQUALS: {
      op_code = mutable_block()->add_op_code();
      op_code->mutable_binary_equals_equals();
      break;
    }
    case BinaryExpr::BANG_EQUALS: {
      op_code = mutable_block()->add_op_code();
      op_code->mutable_binary_bang_equals();
      break;
    }
    case BinaryExpr::LESS_THAN: {
      op_code = mutable_block()->add_op_code();
      op_code->mutable_binary_less_than();
      break;
    }
    case BinaryExpr::GREATER_THAN: {
      op_code = mutable_block()->add_op_code();
      op_code->mutable_binary_greater_than();
      break;
    }
    case BinaryExpr::LESS_THAN_EQUALS: {
      op_code = mutable_block()->add_op_code();
      op_code->mutable_binary_less_than_equals();
      break;
    }
    case BinaryExpr::GREATER_THAN_EQUALS: {
      op_code = mutable_block()->add_op_code();
      op_code->mutable_binary_greater_than_equals();
      break;
    }
    case BinaryExpr::IN: {
      op_code = mutable_block()->add_op_code();
      op_code->mutable_binary_in();
      break;
    }
    case BinaryExpr::NOT_IN: {
      op_code = mutable_block()->add_op_code();
      op_code->mutable_binary_not_in();
      break;
    }
    case BinaryExpr::PIPE: {
      op_code = mutable_block()->add_op_code();
      op_code->mutable_binary_pipe();
      break;
    }
    case BinaryExpr::HAT: {
      op_code = mutable_block()->add_op_code();
      op_code->mutable_binary_hat();
      break;
    }
    case BinaryExpr::AMPERSAND: {
      op_code = mutable_block()->add_op_code();
      op_code->mutable_binary_ampersand();
      break;
    }
    case BinaryExpr::LESS_THAN_LESS_THAN: {
      op_code = mutable_block()->add_op_code();
      op_code->mutable_binary_less_than_less_than();
      break;
    }
    case BinaryExpr::GREATER_THAN_GREATER_THAN: {
      op_code = mutable_block()->add_op_code();
      op_code->mutable_binary_greater_than_greater_than();
      break;
    }
    case BinaryExpr::MINUS: {
      op_code = mutable_block()->add_op_code();
      op_code->mutable_binary_minus();
      break;
    }
    case BinaryExpr::PLUS: {
      op_code = mutable_block()->add_op_code();
      op_code->mutable_binary_plus();
      break;
    }
    case BinaryExpr::STAR: {
      op_code = mutable_block()->add_op_code();
      op_code->mutable_binary_star();
      break;
    }
    case BinaryExpr::PERCENT: {
      op_code = mutable_block()->add_op_code();
      op_code->mutable_binary_percent();
      break;
    }
    case BinaryExpr::SLASH: {
      op_code = mutable_block()->add_op_code();
      op_code->mutable_binary_slash();
      break;
    }
    case BinaryExpr::SLASH_SLASH: {
      op_code = mutable_block()->add_op_code();
      op_code->mutable_binary_slash_slash();
      break;
    }
    default:
      assert(false);
  }
  // TODO(lmirelmann): It should be possible to improve the highlight if there were 4 points instead of 3.
  if (op_code != nullptr) {
    *op_code->mutable_sh()->mutable_start() = binary_expression->op_pif().start();
    *op_code->mutable_sh()->mutable_highlight_start() = binary_expression->op_pif().start();
    *op_code->mutable_sh()->mutable_highlight_end() = binary_expression->op_pif().end();
    *op_code->mutable_sh()->mutable_end() = binary_expression->op_pif().end();
  }
  binary_op_mid_pos.erase(binary_expression);
}

void bytecode_generator::exit_dot_expression(const DotExpr* dot_expression) {
  auto op_code = mutable_block()->add_op_code();
  op_code->mutable_dot_member()->set_member(dot_expression->identifier().nfkc_name());
  *op_code->mutable_sh()->mutable_start() = dot_expression->pif().start();
  *op_code->mutable_sh()->mutable_highlight_start() = dot_expression->primary_expression().pif().end();
  *op_code->mutable_sh()->mutable_highlight_end() = dot_expression->pif().end();
  *op_code->mutable_sh()->mutable_end() = dot_expression->pif().end();
}

void bytecode_generator::exit_dot_expression_for_assignment(const DotExpr* dot_expression, AssignStmt::AssignOperator op) {
  if (op != AssignStmt::EQUALS) {
    return;
  }
  auto op_code = mutable_block()->add_op_code();
  op_code->mutable_assign_dot_member()->set_member(dot_expression->identifier().nfkc_name());
  *op_code->mutable_sh()->mutable_start() = dot_expression->pif().start();
  *op_code->mutable_sh()->mutable_highlight_start() = dot_expression->primary_expression().pif().end();
  *op_code->mutable_sh()->mutable_highlight_end() = dot_expression->pif().end();
  *op_code->mutable_sh()->mutable_end() = dot_expression->pif().end();
}


void bytecode_generator::exit_slice_expression(const SliceExpr* slice_expression) {
  switch (slice_expression->slice_type_case()) {
    case SliceExpr::kIndex: {
      auto op_code = mutable_block()->add_op_code();
      op_code->mutable_index_member();
      *op_code->mutable_sh()->mutable_start() = slice_expression->pif().start();
      *op_code->mutable_sh()->mutable_highlight_start() = slice_expression->primary_expression().pif().end();
      *op_code->mutable_sh()->mutable_highlight_end() = slice_expression->pif().end();
      *op_code->mutable_sh()->mutable_end() = slice_expression->pif().end();
      break;
    }
    case SliceExpr::kSlice: {
      auto op_code = mutable_block()->add_op_code();
      op_code->mutable_slice_range();
      *op_code->mutable_sh()->mutable_start() = slice_expression->pif().start();
      *op_code->mutable_sh()->mutable_highlight_start() = slice_expression->primary_expression().pif().end();
      *op_code->mutable_sh()->mutable_highlight_end() = slice_expression->pif().end();
      *op_code->mutable_sh()->mutable_end() = slice_expression->pif().end();
      break;
    }
    default:
      assert(false);
  }
}

void bytecode_generator::exit_slice_expression_for_assignment(const SliceExpr* slice_expression, AssignStmt::AssignOperator op) {
  if (op != AssignStmt::EQUALS) {
    return;
  }
  switch (slice_expression->slice_type_case()) {
    case SliceExpr::kIndex: {
      auto op_code = mutable_block()->add_op_code();
      op_code->mutable_assign_index_member();
      *op_code->mutable_sh()->mutable_start() = slice_expression->pif().start();
      *op_code->mutable_sh()->mutable_highlight_start() = slice_expression->primary_expression().pif().end();
      *op_code->mutable_sh()->mutable_highlight_end() = slice_expression->pif().end();
      *op_code->mutable_sh()->mutable_end() = slice_expression->pif().end();
      break;
    }
    case SliceExpr::kSlice: {
      auto op_code = mutable_block()->add_op_code();
      op_code->mutable_assign_slice_range();
      *op_code->mutable_sh()->mutable_start() = slice_expression->pif().start();
      *op_code->mutable_sh()->mutable_highlight_start() = slice_expression->primary_expression().pif().end();
      *op_code->mutable_sh()->mutable_highlight_end() = slice_expression->pif().end();
      *op_code->mutable_sh()->mutable_end() = slice_expression->pif().end();
      break;
    }
    default:
      assert(false);
  }
}

void bytecode_generator::exit_assign_statement(const AssignStmt* assign_statement) {
  const Expression& expression = assign_statement->lhs();
  switch (expression.expression_type_case()) {
    case Expression::kIdentifier: {
      const auto& identifier = expression.identifier();
      OpCode* op_code = nullptr;
      switch (assign_statement->op()) {
        case AssignStmt::EQUALS:
          break;
        case AssignStmt::PLUS_EQUALS: {
          op_code = mutable_block()->add_op_code();
          auto* id_op = op_code->mutable_assign_plus_equals();
          id_op->set_frame(identifier.frame());
          id_op->set_pos_in_frame(identifier.pos_in_frame());
          break;
        }
        case AssignStmt::MINUS_EQUALS: {
          op_code = mutable_block()->add_op_code();
          auto* id_op = op_code->mutable_assign_minus_equals();
          id_op->set_frame(identifier.frame());
          id_op->set_pos_in_frame(identifier.pos_in_frame());
          break;
        }
        case AssignStmt::STAR_EQUALS: {
          op_code = mutable_block()->add_op_code();
          auto* id_op = op_code->mutable_assign_star_equals();
          id_op->set_frame(identifier.frame());
          id_op->set_pos_in_frame(identifier.pos_in_frame());
          break;
        }
        case AssignStmt::SLASH_EQUALS: {
          op_code = mutable_block()->add_op_code();
          auto* id_op = op_code->mutable_assign_slash_equals();
          id_op->set_frame(identifier.frame());
          id_op->set_pos_in_frame(identifier.pos_in_frame());
          break;
        }
        case AssignStmt::SLASH_SLASH_EQUALS: {
          op_code = mutable_block()->add_op_code();
          auto* id_op = op_code->mutable_assign_slash_slash_equals();
          id_op->set_frame(identifier.frame());
          id_op->set_pos_in_frame(identifier.pos_in_frame());
          break;
        }
        case AssignStmt::PERCENT_EQUALS: {
          op_code = mutable_block()->add_op_code();
          auto* id_op = op_code->mutable_assign_percent_equals();
          id_op->set_frame(identifier.frame());
          id_op->set_pos_in_frame(identifier.pos_in_frame());
          break;
        }
        case AssignStmt::AMPERSAND_EQUALS: {
          op_code = mutable_block()->add_op_code();
          auto* id_op = op_code->mutable_assign_ampersand_equals();
          id_op->set_frame(identifier.frame());
          id_op->set_pos_in_frame(identifier.pos_in_frame());
          break;
        }
        case AssignStmt::PIPE_EQUALS: {
          op_code = mutable_block()->add_op_code();
          auto* id_op = op_code->mutable_assign_pipe_equals();
          id_op->set_frame(identifier.frame());
          id_op->set_pos_in_frame(identifier.pos_in_frame());
          break;
        }
        case AssignStmt::HAT_EQUALS: {
          op_code = mutable_block()->add_op_code();
          auto* id_op = op_code->mutable_assign_hat_equals();
          id_op->set_frame(identifier.frame());
          id_op->set_pos_in_frame(identifier.pos_in_frame());
          break;
        }
        case AssignStmt::LESS_LESS_EQUALS: {
          op_code = mutable_block()->add_op_code();
          auto* id_op = op_code->mutable_assign_less_less_equals();
          id_op->set_frame(identifier.frame());
          id_op->set_pos_in_frame(identifier.pos_in_frame());
          break;
        }
        case AssignStmt::GREATER_GREATER_EQUALS: {
          op_code = mutable_block()->add_op_code();
          auto* id_op = op_code->mutable_assign_greater_greater_equals();
          id_op->set_frame(identifier.frame());
          id_op->set_pos_in_frame(identifier.pos_in_frame());
          break;
        }
        default:
          assert(false);
      }
      if (op_code != nullptr) {
        *op_code->mutable_sh()->mutable_start() = assign_statement->pif().start();
        *op_code->mutable_sh()->mutable_highlight_start() = assign_statement->op_pif().start();
        *op_code->mutable_sh()->mutable_highlight_end() = assign_statement->op_pif().end();
        *op_code->mutable_sh()->mutable_end() = assign_statement->op_pif().end();
      }
      break;
    }
    case Expression::kSliceExpression: {
      const auto& slice_expression = expression.slice_expression();
      OpCode* op_code = nullptr;
      switch (slice_expression.slice_type_case()) {
        case SliceExpr::kIndex: {
          switch (assign_statement->op()) {
            case AssignStmt::EQUALS:
              break;
            case AssignStmt::PLUS_EQUALS: {
              op_code = mutable_block()->add_op_code();
              op_code->mutable_assign_index_member_plus_equals();
              break;
            }
            case AssignStmt::MINUS_EQUALS: {
              op_code = mutable_block()->add_op_code();
              op_code->mutable_assign_index_member_minus_equals();
              break;
            }
            case AssignStmt::STAR_EQUALS: {
              op_code = mutable_block()->add_op_code();
              op_code->mutable_assign_index_member_star_equals();
              break;
            }
            case AssignStmt::SLASH_EQUALS: {
              op_code = mutable_block()->add_op_code();
              op_code->mutable_assign_index_member_slash_equals();
              break;
            }
            case AssignStmt::SLASH_SLASH_EQUALS: {
              op_code = mutable_block()->add_op_code();
              op_code->mutable_assign_index_member_slash_slash_equals();
              break;
            }
            case AssignStmt::PERCENT_EQUALS: {
              op_code = mutable_block()->add_op_code();
              op_code->mutable_assign_index_member_percent_equals();
              break;
            }
            case AssignStmt::AMPERSAND_EQUALS: {
              op_code = mutable_block()->add_op_code();
              op_code->mutable_assign_index_member_ampersand_equals();
              break;
            }
            case AssignStmt::PIPE_EQUALS: {
              op_code = mutable_block()->add_op_code();
              op_code->mutable_assign_index_member_pipe_equals();
              break;
            }
            case AssignStmt::HAT_EQUALS: {
              op_code = mutable_block()->add_op_code();
              op_code->mutable_assign_index_member_hat_equals();
              break;
            }
            case AssignStmt::LESS_LESS_EQUALS: {
              op_code = mutable_block()->add_op_code();
              op_code->mutable_assign_index_member_less_less_equals();
              break;
            }
            case AssignStmt::GREATER_GREATER_EQUALS: {
              op_code = mutable_block()->add_op_code();
              op_code->mutable_assign_index_member_greater_greater_equals();
              break;
            }
            default:
              assert(false);
          }
          break;
        }
        case SliceExpr::kSlice: {
          switch (assign_statement->op()) {
            case AssignStmt::EQUALS:
              break;
            case AssignStmt::PLUS_EQUALS: {
              op_code = mutable_block()->add_op_code();
              op_code->mutable_assign_slice_range_plus_equals();
              break;
            }
            case AssignStmt::MINUS_EQUALS: {
              op_code = mutable_block()->add_op_code();
              op_code->mutable_assign_slice_range_minus_equals();
              break;
            }
            case AssignStmt::STAR_EQUALS: {
              op_code = mutable_block()->add_op_code();
              op_code->mutable_assign_slice_range_star_equals();
              break;
            }
            case AssignStmt::SLASH_EQUALS: {
              op_code = mutable_block()->add_op_code();
              op_code->mutable_assign_slice_range_slash_equals();
              break;
            }
            case AssignStmt::SLASH_SLASH_EQUALS: {
              op_code = mutable_block()->add_op_code();
              op_code->mutable_assign_slice_range_slash_slash_equals();
              break;
            }
            case AssignStmt::PERCENT_EQUALS: {
              op_code = mutable_block()->add_op_code();
              op_code->mutable_assign_slice_range_percent_equals();
              break;
            }
            case AssignStmt::AMPERSAND_EQUALS: {
              op_code = mutable_block()->add_op_code();
              op_code->mutable_assign_slice_range_ampersand_equals();
              break;
            }
            case AssignStmt::PIPE_EQUALS: {
              op_code = mutable_block()->add_op_code();
              op_code->mutable_assign_slice_range_pipe_equals();
              break;
            }
            case AssignStmt::HAT_EQUALS: {
              op_code = mutable_block()->add_op_code();
              op_code->mutable_assign_slice_range_hat_equals();
              break;
            }
            case AssignStmt::LESS_LESS_EQUALS: {
              op_code = mutable_block()->add_op_code();
              op_code->mutable_assign_slice_range_less_less_equals();
              break;
            }
            case AssignStmt::GREATER_GREATER_EQUALS: {
              op_code = mutable_block()->add_op_code();
              op_code->mutable_assign_slice_range_greater_greater_equals();
              break;
            }
            default:
              assert(false);
          }
          break;
        }
        default:
          assert(false);
      }
      if (op_code != nullptr) {
        *op_code->mutable_sh()->mutable_start() = slice_expression.pif().start();
        *op_code->mutable_sh()->mutable_highlight_start() = slice_expression.primary_expression().pif().end();
        *op_code->mutable_sh()->mutable_highlight_end() = slice_expression.pif().end();
        *op_code->mutable_sh()->mutable_end() = slice_expression.pif().end();
      }
      break;
    }
    case Expression::kDotExpression: {
      const auto& dot_expression = expression.dot_expression();
      OpCode* op_code = nullptr;
      switch (assign_statement->op()) {
        case AssignStmt::EQUALS:
          break;
        case AssignStmt::PLUS_EQUALS: {
          op_code = mutable_block()->add_op_code();
          op_code->mutable_assign_dot_member_plus_equals()->set_member(dot_expression.identifier().nfkc_name());
          break;
        }
        case AssignStmt::MINUS_EQUALS: {
          op_code = mutable_block()->add_op_code();
          op_code->mutable_assign_dot_member_minus_equals()->set_member(dot_expression.identifier().nfkc_name());
          break;
        }
        case AssignStmt::STAR_EQUALS: {
          op_code = mutable_block()->add_op_code();
          op_code->mutable_assign_dot_member_star_equals()->set_member(dot_expression.identifier().nfkc_name());
          break;
        }
        case AssignStmt::SLASH_EQUALS: {
          op_code = mutable_block()->add_op_code();
          op_code->mutable_assign_dot_member_slash_equals()->set_member(dot_expression.identifier().nfkc_name());
          break;
        }
        case AssignStmt::SLASH_SLASH_EQUALS: {
          op_code = mutable_block()->add_op_code();
          op_code->mutable_assign_dot_member_slash_slash_equals()->set_member(dot_expression.identifier().nfkc_name());
          break;
        }
        case AssignStmt::PERCENT_EQUALS: {
          op_code = mutable_block()->add_op_code();
          op_code->mutable_assign_dot_member_percent_equals()->set_member(dot_expression.identifier().nfkc_name());
          break;
        }
        case AssignStmt::AMPERSAND_EQUALS: {
          op_code = mutable_block()->add_op_code();
          op_code->mutable_assign_dot_member_ampersand_equals()->set_member(dot_expression.identifier().nfkc_name());
          break;
        }
        case AssignStmt::PIPE_EQUALS: {
          op_code = mutable_block()->add_op_code();
          op_code->mutable_assign_dot_member_pipe_equals()->set_member(dot_expression.identifier().nfkc_name());
          break;
        }
        case AssignStmt::HAT_EQUALS: {
          op_code = mutable_block()->add_op_code();
          op_code->mutable_assign_dot_member_hat_equals()->set_member(dot_expression.identifier().nfkc_name());
          break;
        }
        case AssignStmt::LESS_LESS_EQUALS: {
          op_code = mutable_block()->add_op_code();
          op_code->mutable_assign_dot_member_less_less_equals()->set_member(dot_expression.identifier().nfkc_name());
          break;
        }
        case AssignStmt::GREATER_GREATER_EQUALS: {
          op_code = mutable_block()->add_op_code();
          op_code->mutable_assign_dot_member_greater_greater_equals()->set_member(dot_expression.identifier().nfkc_name());
          break;
        }
        default:
          assert(false);
      }
      if (op_code != nullptr) {
        *op_code->mutable_sh()->mutable_start() = assign_statement->pif().start();
        *op_code->mutable_sh()->mutable_highlight_start() = assign_statement->op_pif().start();
        *op_code->mutable_sh()->mutable_highlight_end() = assign_statement->op_pif().end();
        *op_code->mutable_sh()->mutable_end() = assign_statement->op_pif().end();
      }
      break;
    }
    default:
      break;
  }

  // Set the uppack highlight points.
  for (auto* op_code : for_unpack) {
    *op_code->mutable_sh()->mutable_start() = assign_statement->pif().start();
    *op_code->mutable_sh()->mutable_highlight_start() = assign_statement->rhs().pif().start();
    *op_code->mutable_sh()->mutable_highlight_end() = assign_statement->pif().end();
    *op_code->mutable_sh()->mutable_end() = assign_statement->pif().end();
  }
  for_unpack.clear();
}

void bytecode_generator::exit_tuple(const Tuple* tuple) {
  auto* tuple_op = mutable_block()->add_op_code();
  tuple_op->mutable_make_tuple()->set_number_of_elements(tuple->value_size());
  *tuple_op->mutable_sh()->mutable_start() = tuple->pif().start();
  *tuple_op->mutable_sh()->mutable_highlight_start() = tuple->pif().start();
  *tuple_op->mutable_sh()->mutable_highlight_end() = tuple->pif().end();
  *tuple_op->mutable_sh()->mutable_end() = tuple->pif().end();
}

void bytecode_generator::enter_tuple_for_assignment(const Tuple* tuple) {
  auto* op_code = mutable_block()->add_op_code();
  op_code->mutable_unpack()->set_number_of_elements(tuple->value_size());
  for_unpack.push_back(op_code);
}

void bytecode_generator::enter_list_expression(const ListExpr* list_expression) {
  mutable_block()->add_op_code()->mutable_make_list()->set_reserve_size(list_expression->element_size());
}

void bytecode_generator::exit_list_expression(const ListExpr* list_expression) {
  if (list_expression->element_size() != 0) {
    auto* add_to_list = mutable_block()->add_op_code()->mutable_add_to_list();
    add_to_list->set_number_of_elements(list_expression->element_size());
  }
}

void bytecode_generator::enter_list_expression_for_assignment(const ListExpr* list_expression) {
  auto* op_code = mutable_block()->add_op_code();
  op_code->mutable_unpack()->set_number_of_elements(list_expression->element_size());
  for_unpack.push_back(op_code);
}

void bytecode_generator::enter_dictionary_expression(const DictExpr* dictionary_expression) {
  mutable_block()->add_op_code()->mutable_make_dictionary()->set_reserve_size(dictionary_expression->entry_size());
}

void bytecode_generator::exit_dictionary_expression(const DictExpr* dictionary_expression) {
  if (dictionary_expression->entry_size() != 0) {
    auto* op_code = mutable_block()->add_op_code();
    auto* add_to_dict = op_code->mutable_add_to_dictionary();
    *op_code->mutable_sh()->mutable_start() = dictionary_expression->pif().start();
    *op_code->mutable_sh()->mutable_highlight_start() = dictionary_expression->pif().start();
    *op_code->mutable_sh()->mutable_highlight_end() = dictionary_expression->pif().end();
    *op_code->mutable_sh()->mutable_end() = dictionary_expression->pif().end();
    add_to_dict->set_number_of_elements(dictionary_expression->entry_size());
  }
}

void bytecode_generator::enter_list_comprehension(const ListComp* list_comprehension) {
  comprehension_comp_clause.push_back({});
  mutable_block()->add_op_code()->mutable_make_list()->set_reserve_size(0);
  auto* comprehension_block = mutable_block()->add_op_code()->mutable_create_frame();
  comprehension_block->set_block_type(BlockType::COMPREHENSION_BLOCK);
  for (const auto&  symbol : list_comprehension->comprehension_binding()) {
    comprehension_block->add_symbol(symbol);
  }
}

void bytecode_generator::exit_list_comprehension(const ListComp* list_comprehension) {
  auto* add_to_list = mutable_block()->add_op_code()->mutable_add_to_list();
  add_to_list->set_number_of_elements(1);
  fix_comp_clause(list_comprehension->clause());
  comprehension_comp_clause.pop_back();
  mutable_block()->add_op_code()->mutable_pop_frame();
}

void bytecode_generator::enter_dictionary_comprehension(const DictComp* dictionary_comprehension) {
  comprehension_comp_clause.push_back({});
  mutable_block()->add_op_code()->mutable_make_dictionary()->set_reserve_size(0);
  auto* comprehension_block = mutable_block()->add_op_code()->mutable_create_frame();
  comprehension_block->set_block_type(BlockType::COMPREHENSION_BLOCK);
  for (const auto& symbol : dictionary_comprehension->comprehension_binding()) {
    comprehension_block->add_symbol(symbol);
  }
}

void bytecode_generator::exit_dictionary_comprehension(const DictComp* dictionary_comprehension) {
  auto* add_to_dict = mutable_block()->add_op_code()->mutable_add_to_dictionary();
  add_to_dict->set_number_of_elements(1);
  fix_comp_clause(dictionary_comprehension->clause());
  comprehension_comp_clause.pop_back();
  mutable_block()->add_op_code()->mutable_pop_frame();
}

void bytecode_generator::mid_for_clause(const ForClause* for_clause) {
  auto* get_it_op_code = mutable_block()->add_op_code();
  get_it_op_code->mutable_get_iterator();
  *get_it_op_code->mutable_sh()->mutable_start() = for_clause->pif().start();
  *get_it_op_code->mutable_sh()->mutable_highlight_start() = for_clause->in().pif().start();
  *get_it_op_code->mutable_sh()->mutable_highlight_end() = for_clause->in().pif().end();
  *get_it_op_code->mutable_sh()->mutable_end() = for_clause->in().pif().end();
  comprehension_comp_clause.back().push_back(block().op_code_size());
  mutable_block()->add_op_code()->mutable_for_iterator();
}

void bytecode_generator::exit_if_clause(const IfClause* if_clause) {
  comprehension_comp_clause.back().push_back(block().op_code_size());
  mutable_block()->add_op_code()->mutable_jump_if_false();
}

void bytecode_generator::mid_if_expression(const IfExpr* if_expression) {
  auto op_code_size = block().op_code_size();
  if (!if_expression_op_mid_pos.contains(if_expression)) {
    // This is the first time this is called for this `if expression`.
    if_expression_op_mid_pos[if_expression] = op_code_size;
    mutable_block()->add_op_code()->mutable_jump_if_false();
  } else {
    auto op_pos = if_expression_op_mid_pos[if_expression];
    mutable_block()->mutable_op_code(op_pos)->mutable_jump_if_false()->set_address_delta(op_code_size + 1 - op_pos);
    if_expression_op_mid_pos[if_expression] = op_code_size;
    mutable_block()->add_op_code()->mutable_goto_();
  }
}

void bytecode_generator::exit_if_expression(const IfExpr* if_expression) {
  auto op_code_size = block().op_code_size();
  auto op_pos = if_expression_op_mid_pos[if_expression];
  mutable_block()->mutable_op_code(op_pos)->mutable_goto_()->set_address_delta(op_code_size - op_pos);
  if_expression_op_mid_pos.erase(if_expression);
}

void bytecode_generator::enter_for_statement(const ForStmt* for_statement) {
  for_statement_op_continue.push_back({});
  for_statement_op_break.push_back({});
}

void bytecode_generator::mid_for_statement(const ForStmt* for_statement) {
  auto op_code_size = block().op_code_size();
  if (!for_statement_op_mid_pos.contains(for_statement)) {
    // This is the first time this is called for this `for statement`.
    for_statement_op_mid_pos[for_statement] = op_code_size;
    auto* get_it_op_code = mutable_block()->add_op_code();
    get_it_op_code->mutable_get_iterator();
    *get_it_op_code->mutable_sh()->mutable_start() = for_statement->pif().start();
    *get_it_op_code->mutable_sh()->mutable_highlight_start() = for_statement->expression().pif().start();
    *get_it_op_code->mutable_sh()->mutable_highlight_end() = for_statement->expression().pif().end();
    *get_it_op_code->mutable_sh()->mutable_end() = for_statement->expression().pif().end();
    mutable_block()->add_op_code()->mutable_for_iterator();
  }
}

void bytecode_generator::exit_for_statement(const ForStmt* for_statement) {
  auto op_code_size = block().op_code_size();
  auto begin_address = for_statement_op_mid_pos[for_statement] + 1;
  mutable_block()->add_op_code()->mutable_goto_()->set_address_delta(begin_address - op_code_size);
  mutable_block()->mutable_op_code(begin_address)->mutable_for_iterator()->set_address_delta(op_code_size + 1 - begin_address);
  mutable_block()->add_op_code()->mutable_end_iterator();

  // Fix `break` and `continue` statements.
  for (auto i : for_statement_op_break.back()) {
    mutable_block()->mutable_op_code(i)->mutable_goto_()->set_address_delta(op_code_size + 1 - i);
  }
  for (auto i : for_statement_op_continue.back()) {
    mutable_block()->mutable_op_code(i)->mutable_goto_()->set_address_delta(begin_address - i);
  }

  // Cleanup.
  for_statement_op_continue.pop_back();
  for_statement_op_break.pop_back();
  for_statement_op_mid_pos.erase(for_statement);
}

void bytecode_generator::exit_break_statement(const BreakStmt* break_statement) {
  for_statement_op_break.back().push_back(block().op_code_size());
  mutable_block()->add_op_code()->mutable_goto_();
}

void bytecode_generator::exit_continue_statement(const ContinueStmt* continue_statement) {
  for_statement_op_continue.back().push_back(block().op_code_size());
  mutable_block()->add_op_code()->mutable_goto_();
}

void bytecode_generator::enter_if_statement(const IfStmt* if_statement) {
  if_statement_to_fix_to_the_end.push_back({});
}

void bytecode_generator::exit_if_statement(const IfStmt* if_statement) {
  auto op_code_size = block().op_code_size();
  for (auto pos : if_statement_to_fix_to_the_end.back()) {
    mutable_block()->mutable_op_code(pos)->mutable_goto_()->set_address_delta(op_code_size - pos);
  }
  if_statement_to_fix_to_the_end.pop_back();
}

void bytecode_generator::enter_then(const RepeatedPtrField<Statement>* then) {
  if_statement_then[then] = block().op_code_size();
  mutable_block()->add_op_code()->mutable_jump_if_false();
}

void bytecode_generator::exit_then(const RepeatedPtrField<Statement>* then) {
  auto op_code_size = block().op_code_size();
  auto op_pos = if_statement_then[then];
  mutable_block()->mutable_op_code(op_pos)->mutable_jump_if_false()->set_address_delta(op_code_size + 1 - op_pos);
  mutable_block()->add_op_code()->mutable_goto_();
  if_statement_to_fix_to_the_end.back().push_back(op_code_size);

  if_statement_then.erase(then);
}

void bytecode_generator::exit_call_expression(const CallExpr* call_expression) {
  int pos_arguments = 0;
  int named_arguments = 0;
  bool variadic_pos_arg = false;
  bool variadic_named_arg = false;

  for (const auto& arg : call_expression->argument()) {
    switch (arg.argument_type_case()) {
      case Argument::kValue:
        pos_arguments++;
        break;
      case Argument::kNamedArgument:
        named_arguments++;
        break;
      case Argument::kStarArgument:
        variadic_pos_arg = true;
        break;
      case Argument::kStarStarArgument:
        variadic_named_arg = true;
        break;
      case Argument::ARGUMENT_TYPE_NOT_SET:
        assert(false);
    }
  }
  auto* new_op = mutable_block()->add_op_code();
  if (!variadic_pos_arg && !variadic_named_arg) {
    if (named_arguments == 0) {
      switch (pos_arguments) {
        case 0:
          new_op->mutable_call_pos0();
          break;
        case 1:
          new_op->mutable_call_pos1();
          break;
        case 2:
          new_op->mutable_call_pos2();
          break;
        case 3:
          new_op->mutable_call_pos3();
          break;
        default:
          new_op->mutable_call_pos()->set_positional_count(pos_arguments);
          break;
      }
    } else {
      auto* call_named = new_op->mutable_call_named();
      call_named->set_positional_arguments_count(pos_arguments);
      call_named->set_named_arguments_count(named_arguments);
    }
  } else if (variadic_pos_arg && !variadic_named_arg && named_arguments == 0) {
    new_op->mutable_call_pos_star()->set_positional_arguments_count(pos_arguments);
  } else {
    auto* call = new_op->mutable_call();
    call->set_positional_arguments_count(pos_arguments);
    call->set_named_arguments_count(named_arguments);
    call->set_has_variadic_positional_argument(variadic_pos_arg);
    call->set_has_variadic_named_argument(variadic_named_arg);
  }
  *new_op->mutable_sh()->mutable_start() = call_expression->pif().start();
  *new_op->mutable_sh()->mutable_highlight_start() = call_expression->primary_expression().pif().end();
  *new_op->mutable_sh()->mutable_highlight_end() = call_expression->pif().end();
  *new_op->mutable_sh()->mutable_end() = call_expression->pif().end();
}

void bytecode_generator::enter_argument(const Argument* argument) {
  if (argument->argument_type_case() == Argument::kNamedArgument) {
    auto* const_string_view = mutable_block()->add_op_code();
    auto it = const_string.insert({std::string{argument->named_argument().identifier().nfkc_name()}, const_string.size()});
    const_string_view->mutable_const_string_view()->set_const_string_pos(it.first->second);
    if (it.second) {
      output.add_const_string(argument->named_argument().identifier().nfkc_name());
    }
  }
}

void bytecode_generator::exit_return_statement(const ReturnStmt* return_statement) {
  mutable_block()->add_op_code()->mutable_return_();
}

void bytecode_generator::mid_lambda_expression(const LambdaExpr* lambda_expression) {
  mid_def_or_lambda_expression("<lambda>", &lambda_expression->parameter());
  auto* function_block = mutable_block()->add_op_code()->mutable_create_frame();
  function_block->set_block_type(BlockType::FUNCTION_BLOCK);
  for (const auto& symbol : lambda_expression->function_binding()) {
    function_block->add_symbol(symbol);
  }
}

void bytecode_generator::exit_lambda_expression(const LambdaExpr* lambda_expression) {
  mutable_block()->add_op_code()->mutable_return_();
  exit_def_or_lambda_expression(&lambda_expression->parameter());
}

void bytecode_generator::mid_def_statement(const DefStmt* def_statement) {
  mid_def_or_lambda_expression(def_statement->function_name().nfkc_name(), &def_statement->parameter());
  auto* function_block = mutable_block()->add_op_code()->mutable_create_frame();
  function_block->set_block_type(BlockType::FUNCTION_BLOCK);
  for (const auto& symbol : def_statement->function_binding()) {
    function_block->add_symbol(symbol);
  }
}

void bytecode_generator::exit_def_statement(const DefStmt* def_statement) {
  if (block().op_code().empty() ||
      !block().op_code().rbegin()->has_return_()) {
    mutable_block()->add_op_code()->mutable_const_none();
    mutable_block()->add_op_code()->mutable_return_();
  }
  exit_def_or_lambda_expression(&def_statement->parameter());
  auto* id_op = mutable_block()->add_op_code()->mutable_store();
  id_op->set_frame(def_statement->function_name().frame());
  id_op->set_pos_in_frame(def_statement->function_name().pos_in_frame());
}

void bytecode_generator::mid_def_or_lambda_expression(std::string_view fn_name, const RepeatedPtrField<Parameter>* params) {
  // Capture the function signature.
  int arguments_with_defaults_count = 0;
  bool has_star_argument = false;
  bool has_star_star_argument = false;
  int keyword_only_parameter_count = 0;
  bool keyword_only_mode = false;
  for (const auto& param : *params) {
    switch (param.parameter_type_case()) {
      case Parameter::kInitialization:
        ++arguments_with_defaults_count;
        if (keyword_only_mode) {
          keyword_only_parameter_count++;
        }
        break;
      case Parameter::kStar:
        keyword_only_mode = true;
        if (!param.identifier().name().empty()) {
          has_star_argument = true;
        }
        break;
      case Parameter::kStarStar:
        has_star_star_argument = true;
        break;
      default:
        if (keyword_only_mode) {
          keyword_only_parameter_count++;
        }
        break;
    }
  }

  auto* make_function = mutable_block()->add_op_code()->mutable_make_function();
  make_function->set_default_values_count(arguments_with_defaults_count);

  int block_for_function = output.block_size();
  make_function->set_entrypoint(block_for_function);
  blocks.push_back(block_for_function);
  output.add_block();
  auto* function_signature = mutable_block()->mutable_function_signature();
  function_signature->set_fn_name(fn_name);
  function_signature->set_has_star_argument(has_star_argument);
  function_signature->set_has_star_star_argument(has_star_star_argument);
  function_signature->set_keyword_only_parameter_count(keyword_only_parameter_count);
  for (const auto& param : *params) {
    if (param.identifier().name().empty()) {
      continue;
    }
    auto fn_param = function_signature->add_param();
    fn_param->set_name(param.identifier().nfkc_name());
    fn_param->mutable_pos()->set_frame(param.identifier().frame());
    fn_param->mutable_pos()->set_pos_in_frame(param.identifier().pos_in_frame());
    fn_param->set_default_initialization(param.parameter_type_case() == Parameter::kInitialization);
  }
}

void bytecode_generator::exit_def_or_lambda_expression(const RepeatedPtrField<Parameter>* params) {
  blocks.pop_back();
}

void bytecode_generator::fix_comp_clause(const RepeatedPtrField<CompClause>& clauses) {
  int clause_pos = 0;
  auto clauses_count = clauses.size();
  for (auto it = clauses.rbegin(); it != clauses.rend(); ++it) {
    const auto& clause = *it;
    if (clause.comp_clause_type_case() == CompClause::kForClause) {
      auto begin_address = comprehension_comp_clause.back()[clauses_count - clause_pos - 1];
      auto op_pos = block().op_code_size();
      mutable_block()->add_op_code()->mutable_goto_()->set_address_delta(begin_address - op_pos);
      mutable_block()->mutable_op_code(begin_address)->mutable_for_iterator()->set_address_delta(block().op_code_size() - begin_address);
      mutable_block()->add_op_code()->mutable_end_iterator();
    } else {
      auto op_pos = comprehension_comp_clause.back()[clauses_count - clause_pos - 1];
      mutable_block()->mutable_op_code(op_pos)
          ->mutable_jump_if_false()->set_address_delta(block().op_code_size() - op_pos);
    }
    clause_pos++;
  }
}

Block* bytecode_generator::mutable_block() {
  return output.mutable_block(blocks.back());
}

const Block& bytecode_generator::block() const {
  return output.block(blocks.back());
}


void remove_extra_store(Program* program) {
  struct store_info {
    std::vector<OpCode*> store;
    bool found_load = false;
  };
  struct ip_info {
    int64_t block;
    int64_t pos;
  };
  std::vector<std::vector<store_info>> frames;
  std::vector<ip_info> traverse;
  traverse.emplace_back(ip_info{.block = 0, .pos = 0 });
  auto pop_frame = [&frames]() {
    if (frames.empty()) {
      return;
    }
    // The top 3 frames are not candidates for store removal.
    if (frames.size() <= 3) {
      frames.pop_back();
      return;
    }
    for (auto& stores : frames.back()) {
      if (!stores.found_load) {
        for (auto* store : stores.store) {
          store->mutable_pop();
        }
      }
    }
    frames.pop_back();
  };

  while (!traverse.empty()) {
    if (program->block(traverse.back().block).op_code().size() == traverse.back().pos) {
      pop_frame();
      traverse.pop_back();
      continue;
    }
    const auto& op_code = program->block(traverse.back().block).op_code(traverse.back().pos);
    traverse.back().pos++;
    switch(op_code.op_code_case()) {
      case OpCode::kLoad:
        frames[frames.size() - 1 - op_code.load().frame()][op_code.load().pos_in_frame()].found_load = true;
        break;
      case OpCode::kAssignPlusEquals:
        frames[frames.size() - 1 - op_code.assign_plus_equals().frame()][op_code.assign_plus_equals().pos_in_frame()].found_load = true;
        break;
      case OpCode::kAssignMinusEquals:
        frames[frames.size() - 1 - op_code.assign_minus_equals().frame()][op_code.assign_minus_equals().pos_in_frame()].found_load = true;
        break;
      case OpCode::kAssignStarEquals:
        frames[frames.size() - 1 - op_code.assign_star_equals().frame()][op_code.assign_star_equals().pos_in_frame()].found_load = true;
        break;
      case OpCode::kAssignSlashEquals:
        frames[frames.size() - 1 - op_code.assign_slash_equals().frame()][op_code.assign_slash_equals().pos_in_frame()].found_load = true;
        break;
      case OpCode::kAssignSlashSlashEquals:
        frames[frames.size() - 1 - op_code.assign_slash_slash_equals().frame()][op_code.assign_slash_slash_equals().pos_in_frame()].found_load = true;
        break;
      case OpCode::kAssignPercentEquals:
        frames[frames.size() - 1 - op_code.assign_percent_equals().frame()][op_code.assign_percent_equals().pos_in_frame()].found_load = true;
        break;
      case OpCode::kAssignAmpersandEquals:
        frames[frames.size() - 1 - op_code.assign_ampersand_equals().frame()][op_code.assign_ampersand_equals().pos_in_frame()].found_load = true;
        break;
      case OpCode::kAssignPipeEquals:
        frames[frames.size() - 1 - op_code.assign_pipe_equals().frame()][op_code.assign_pipe_equals().pos_in_frame()].found_load = true;
        break;
      case OpCode::kAssignHatEquals:
        frames[frames.size() - 1 - op_code.assign_hat_equals().frame()][op_code.assign_hat_equals().pos_in_frame()].found_load = true;
        break;
      case OpCode::kAssignLessLessEquals:
        frames[frames.size() - 1 - op_code.assign_less_less_equals().frame()][op_code.assign_less_less_equals().pos_in_frame()].found_load = true;
        break;
      case OpCode::kAssignGreaterGreaterEquals:
        frames[frames.size() - 1 - op_code.assign_greater_greater_equals().frame()][op_code.assign_greater_greater_equals().pos_in_frame()].found_load = true;
        break;
      case OpCode::kMakeFunction:
        traverse.emplace_back(ip_info{.block = op_code.make_function().entrypoint(), .pos = 0});
        break;
      case OpCode::kCreateFrame:
        frames.emplace_back(op_code.create_frame().symbol().size());
        for (int i = 0; i < op_code.create_frame().symbol().size(); ++i) {
          frames.back().push_back(store_info{});
        }
        break;
      case OpCode::kPopFrame:
        pop_frame();
        break;
      case OpCode::kStore:
        frames[frames.size() - 1 - op_code.store().frame()][op_code.store().pos_in_frame()].store.push_back(
            program->mutable_block(traverse.back().block)->mutable_op_code(traverse.back().pos - 1));
        break;
      default:
        break;
    }
  }
}

void simplify_for_loop(Program* program) {
  for (int i = 0; i < program->block().size(); ++i) {
    for (int j = 0; j < program->block(i).op_code().size(); ++j) {
      if (program->block(i).op_code(j).op_code_case() == OpCode::kForIterator &&
          program->block(i).op_code(j + 1).op_code_case() == OpCode::kPop) {
        auto address_delta = program->block(i).op_code(j).for_iterator().address_delta();
        program->mutable_block(i)->mutable_op_code(j)->mutable_for_iterator_ext()->set_address_delta(address_delta);
        program->mutable_block(i)->mutable_op_code(j + 1)->mutable_nop();
      }
    }
  }
}

}  // namespace

compiler::compiler(std::set<std::string, std::less<>>& binding) : binding(binding) {}

Program* compiler::compile(std::string_view program_name, std::string_view starlark_program, grammar_options opt, logger& logging, Arena& arena) {
  logger_wrap logging_wrap(logging);
  parser star_parser(program_name,
                     starlark_program,
                     opt,
                     binding,
                     logging_wrap);
  Arena parser_arena;
  File* starlark_file = star_parser.parse_file(parser_arena);

  auto report = logging_wrap.report();
  // If there are errors, then return early.
  if (report.error > 0 || report.fatal > 0) {
    return nullptr;
  }

  Program* result = Arena::Create<Program>(&arena);
  bytecode_generator listener(*result);
  grammar::ast_walker walker;
  walker.walk(starlark_file, listener);
  remove_extra_store(result);
  simplify_for_loop(result);
  return result;
}

}  // namespace compiler
}  // namespace starlark

