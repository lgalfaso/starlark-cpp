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
using ::starlark::ast::ForClause;
using ::starlark::ast::ForStmt;
using ::starlark::ast::Identifier;
using ::starlark::ast::IfExpr;
using ::starlark::ast::IfStmt;
using ::starlark::ast::LambdaExpr;
using ::starlark::ast::ListComp;
using ::starlark::ast::ListExpr;
using ::starlark::ast::LoadStmt;
using ::starlark::ast::Parameter;
using ::starlark::ast::ReturnStmt;
using ::starlark::ast::SliceExpr;
using ::starlark::ast::Statement;
using ::starlark::ast::Tuple;
using ::starlark::ast::UnaryExpr;
using ::starlark::bytecode::Block;
using ::starlark::bytecode::BlockType;
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
  void enter_int_value(std::int64_t int_value) override;
  void enter_big_int_value(std::string_view big_int_value) override;
  void enter_float_value(double float_value) override;
  void enter_string_value(std::string_view string_value) override;
  void enter_bytes_value(std::string_view bytes_value) override;
  void enter_identifier(const Identifier* identifier) override;
  void enter_identifier_for_assignment(const Identifier* identifier, AssignStmt::AssignOperator op) override;
  void exit_unary_expression(const UnaryExpr* unary_expression) override;
  void mid_binary_expression(const BinaryExpr* binary_expression) override;
  void exit_binary_expression(const BinaryExpr* binary_expression) override;
  void exit_dot_expression(const DotExpr* dot_expression) override;
  void exit_dot_expression_for_assignment(const DotExpr* dot_expression, AssignStmt::AssignOperator op) override;
  void exit_slice_expression(const SliceExpr* slice_expression) override;
  void exit_slice_expression_for_assignment(const SliceExpr* slice_expression, AssignStmt::AssignOperator op) override;

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

void bytecode_generator::enter_int_value(std::int64_t int_value) {
  mutable_block()->add_op_code()->mutable_const_int()->set_value(int_value);
}

void bytecode_generator::enter_big_int_value(std::string_view big_int_value) {
  mutable_block()->add_op_code()->mutable_const_big_int()->set_value(big_int_value);
}

void bytecode_generator::enter_float_value(double float_value) {
  mutable_block()->add_op_code()->mutable_const_float()->set_value(float_value);
}

void bytecode_generator::enter_string_value(std::string_view string_value) {
  mutable_block()->add_op_code()->mutable_const_string()->set_value(string_value);
}

void bytecode_generator::enter_bytes_value(std::string_view bytes_value) {
  mutable_block()->add_op_code()->mutable_const_bytes()->set_value(bytes_value);
}

void bytecode_generator::enter_identifier(const Identifier* identifier) {
  auto* id_op = mutable_block()->add_op_code()->mutable_load();
  id_op->set_frame(identifier->frame());
  id_op->set_pos_in_frame(identifier->pos_in_frame());
}

void bytecode_generator::enter_identifier_for_assignment(const Identifier* identifier, AssignStmt::AssignOperator op) {
  switch (op) {
    case AssignStmt::EQUALS: {
      auto* id_op = mutable_block()->add_op_code()->mutable_store();
      id_op->set_frame(identifier->frame());
      id_op->set_pos_in_frame(identifier->pos_in_frame());
      break;
    }
    case AssignStmt::PLUS_EQUALS: {
      auto* id_op = mutable_block()->add_op_code()->mutable_assign_plus_equals();
      id_op->set_frame(identifier->frame());
      id_op->set_pos_in_frame(identifier->pos_in_frame());
      break;
    }
    case AssignStmt::MINUS_EQUALS: {
      auto* id_op = mutable_block()->add_op_code()->mutable_assign_minus_equals();
      id_op->set_frame(identifier->frame());
      id_op->set_pos_in_frame(identifier->pos_in_frame());
      break;
    }
    case AssignStmt::STAR_EQUALS: {
      auto* id_op = mutable_block()->add_op_code()->mutable_assign_star_equals();
      id_op->set_frame(identifier->frame());
      id_op->set_pos_in_frame(identifier->pos_in_frame());
      break;
    }
    case AssignStmt::SLASH_EQUALS: {
      auto* id_op = mutable_block()->add_op_code()->mutable_assign_slash_equals();
      id_op->set_frame(identifier->frame());
      id_op->set_pos_in_frame(identifier->pos_in_frame());
      break;
    }
    case AssignStmt::SLASH_SLASH_EQUALS: {
      auto* id_op = mutable_block()->add_op_code()->mutable_assign_slash_slash_equals();
      id_op->set_frame(identifier->frame());
      id_op->set_pos_in_frame(identifier->pos_in_frame());
      break;
    }
    case AssignStmt::PERCENT_EQUALS: {
      auto* id_op = mutable_block()->add_op_code()->mutable_assign_percent_equals();
      id_op->set_frame(identifier->frame());
      id_op->set_pos_in_frame(identifier->pos_in_frame());
      break;
    }
    case AssignStmt::AMPERSAND_EQUALS: {
      auto* id_op = mutable_block()->add_op_code()->mutable_assign_ampersand_equals();
      id_op->set_frame(identifier->frame());
      id_op->set_pos_in_frame(identifier->pos_in_frame());
      break;
    }
    case AssignStmt::PIPE_EQUALS: {
      auto* id_op = mutable_block()->add_op_code()->mutable_assign_pipe_equals();
      id_op->set_frame(identifier->frame());
      id_op->set_pos_in_frame(identifier->pos_in_frame());
      break;
    }
    case AssignStmt::HAT_EQUALS: {
      auto* id_op = mutable_block()->add_op_code()->mutable_assign_hat_equals();
      id_op->set_frame(identifier->frame());
      id_op->set_pos_in_frame(identifier->pos_in_frame());
      break;
    }
    case AssignStmt::LESS_LESS_EQUALS: {
      auto* id_op = mutable_block()->add_op_code()->mutable_assign_less_less_equals();
      id_op->set_frame(identifier->frame());
      id_op->set_pos_in_frame(identifier->pos_in_frame());
      break;
    }
    case AssignStmt::GREATER_GREATER_EQUALS: {
      auto* id_op = mutable_block()->add_op_code()->mutable_assign_greater_greater_equals();
      id_op->set_frame(identifier->frame());
      id_op->set_pos_in_frame(identifier->pos_in_frame());
      break;
    }
    default:
      assert(false);
  }
}

void bytecode_generator::exit_unary_expression(const UnaryExpr* unary_expression) {
  switch (unary_expression->operator_()) {
    case UnaryExpr::PLUS:
      mutable_block()->add_op_code()->mutable_unary_plus();
      break;
    case UnaryExpr::MINUS:
      mutable_block()->add_op_code()->mutable_unary_minus();
      break;
    case UnaryExpr::TILDE:
      mutable_block()->add_op_code()->mutable_unary_tilde();
      break;
    case UnaryExpr::NOT:
      mutable_block()->add_op_code()->mutable_unary_not();
      break;
    default:
      assert(false);
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
  switch (binary_expression->operator_()) {
    case BinaryExpr::OR: {
      auto op_pos = binary_op_mid_pos[binary_expression];
      mutable_block()->mutable_op_code(op_pos)->mutable_jump_if_true_or_pop()->set_address_delta(block().op_code_size() - op_pos);
      break;
    }
    case BinaryExpr::AND: {
      auto op_pos = binary_op_mid_pos[binary_expression];
      mutable_block()->mutable_op_code(op_pos)->mutable_jump_if_false_or_pop()->set_address_delta(block().op_code_size() - op_pos);
      break;
    }
    case BinaryExpr::EQUALS_EQUALS:
      mutable_block()->add_op_code()->mutable_binary_equals_equals();
      break;
    case BinaryExpr::BANG_EQUALS:
      mutable_block()->add_op_code()->mutable_binary_bang_equals();
      break;
    case BinaryExpr::LESS_THAN:
      mutable_block()->add_op_code()->mutable_binary_less_than();
      break;
    case BinaryExpr::GREATER_THAN:
      mutable_block()->add_op_code()->mutable_binary_greater_than();
      break;
    case BinaryExpr::LESS_THAN_EQUALS:
      mutable_block()->add_op_code()->mutable_binary_less_than_equals();
      break;
    case BinaryExpr::GREATER_THAN_EQUALS:
      mutable_block()->add_op_code()->mutable_binary_greater_than_equals();
      break;
    case BinaryExpr::IN:
      mutable_block()->add_op_code()->mutable_binary_in();
      break;
    case BinaryExpr::NOT_IN:
      mutable_block()->add_op_code()->mutable_binary_not_in();
      break;
    case BinaryExpr::PIPE:
      mutable_block()->add_op_code()->mutable_binary_pipe();
      break;
    case BinaryExpr::HAT:
      mutable_block()->add_op_code()->mutable_binary_hat();
      break;
    case BinaryExpr::AMPERSAND:
      mutable_block()->add_op_code()->mutable_binary_ampersand();
      break;
    case BinaryExpr::LESS_THAN_LESS_THAN:
      mutable_block()->add_op_code()->mutable_binary_less_than_less_than();
      break;
    case BinaryExpr::GREATER_THAN_GREATER_THAN:
      mutable_block()->add_op_code()->mutable_binary_greater_than_greater_than();
      break;
    case BinaryExpr::MINUS:
      mutable_block()->add_op_code()->mutable_binary_minus();
      break;
    case BinaryExpr::PLUS:
      mutable_block()->add_op_code()->mutable_binary_plus();
      break;
    case BinaryExpr::STAR:
      mutable_block()->add_op_code()->mutable_binary_star();
      break;
    case BinaryExpr::PERCENT:
      mutable_block()->add_op_code()->mutable_binary_percent();
      break;
    case BinaryExpr::SLASH:
      mutable_block()->add_op_code()->mutable_binary_slash();
      break;
    case BinaryExpr::SLASH_SLASH:
      mutable_block()->add_op_code()->mutable_binary_slash_slash();
      break;
    default:
      assert(false);
  }
  binary_op_mid_pos.erase(binary_expression);
}

void bytecode_generator::exit_dot_expression(const DotExpr* dot_expression) {
  mutable_block()->add_op_code()->mutable_dot_member()->set_member(dot_expression->identifier().nfkc_name());
}

void bytecode_generator::exit_dot_expression_for_assignment(const DotExpr* dot_expression, AssignStmt::AssignOperator op) {
  switch (op) {
    case AssignStmt::EQUALS: {
      mutable_block()->add_op_code()->mutable_assign_dot_member()->set_member(dot_expression->identifier().nfkc_name());
      break;
    }
    case AssignStmt::PLUS_EQUALS: {
      mutable_block()->add_op_code()->mutable_assign_dot_member_plus_equals()->set_member(dot_expression->identifier().nfkc_name());
      break;
    }
    case AssignStmt::MINUS_EQUALS: {
      mutable_block()->add_op_code()->mutable_assign_dot_member_minus_equals()->set_member(dot_expression->identifier().nfkc_name());
      break;
    }
    case AssignStmt::STAR_EQUALS: {
      mutable_block()->add_op_code()->mutable_assign_dot_member_star_equals()->set_member(dot_expression->identifier().nfkc_name());
      break;
    }
    case AssignStmt::SLASH_EQUALS: {
      mutable_block()->add_op_code()->mutable_assign_dot_member_slash_equals()->set_member(dot_expression->identifier().nfkc_name());
      break;
    }
    case AssignStmt::SLASH_SLASH_EQUALS: {
      mutable_block()->add_op_code()->mutable_assign_dot_member_slash_slash_equals()->set_member(dot_expression->identifier().nfkc_name());
      break;
    }
    case AssignStmt::PERCENT_EQUALS: {
      mutable_block()->add_op_code()->mutable_assign_dot_member_percent_equals()->set_member(dot_expression->identifier().nfkc_name());
      break;
    }
    case AssignStmt::AMPERSAND_EQUALS: {
      mutable_block()->add_op_code()->mutable_assign_dot_member_ampersand_equals()->set_member(dot_expression->identifier().nfkc_name());
      break;
    }
    case AssignStmt::PIPE_EQUALS: {
      mutable_block()->add_op_code()->mutable_assign_dot_member_pipe_equals()->set_member(dot_expression->identifier().nfkc_name());
      break;
    }
    case AssignStmt::HAT_EQUALS: {
      mutable_block()->add_op_code()->mutable_assign_dot_member_hat_equals()->set_member(dot_expression->identifier().nfkc_name());
      break;
    }
    case AssignStmt::LESS_LESS_EQUALS: {
      mutable_block()->add_op_code()->mutable_assign_dot_member_less_less_equals()->set_member(dot_expression->identifier().nfkc_name());
      break;
    }
    case AssignStmt::GREATER_GREATER_EQUALS: {
      mutable_block()->add_op_code()->mutable_assign_dot_member_greater_greater_equals()->set_member(dot_expression->identifier().nfkc_name());
      break;
    }
    default:
      assert(false);
  }
}

void bytecode_generator::exit_slice_expression(const SliceExpr* slice_expression) {
  switch (slice_expression->slice_type_case()) {
    case SliceExpr::kIndex:
      mutable_block()->add_op_code()->mutable_index_member();
      break;
    case SliceExpr::kSlice:
      mutable_block()->add_op_code()->mutable_slice_range();
      break;
    default:
      assert(false);
  }
}

void bytecode_generator::exit_slice_expression_for_assignment(const SliceExpr* slice_expression, AssignStmt::AssignOperator op) {
  switch (slice_expression->slice_type_case()) {
    case SliceExpr::kIndex:
      switch (op) {
        case AssignStmt::EQUALS: {
          mutable_block()->add_op_code()->mutable_assign_index_member();
          break;
        }
        case AssignStmt::PLUS_EQUALS: {
          mutable_block()->add_op_code()->mutable_assign_index_member_plus_equals();
          break;
        }
        case AssignStmt::MINUS_EQUALS: {
          mutable_block()->add_op_code()->mutable_assign_index_member_minus_equals();
          break;
        }
        case AssignStmt::STAR_EQUALS: {
          mutable_block()->add_op_code()->mutable_assign_index_member_star_equals();
          break;
        }
        case AssignStmt::SLASH_EQUALS: {
          mutable_block()->add_op_code()->mutable_assign_index_member_slash_equals();
          break;
        }
        case AssignStmt::SLASH_SLASH_EQUALS: {
          mutable_block()->add_op_code()->mutable_assign_index_member_slash_slash_equals();
          break;
        }
        case AssignStmt::PERCENT_EQUALS: {
          mutable_block()->add_op_code()->mutable_assign_index_member_percent_equals();
          break;
        }
        case AssignStmt::AMPERSAND_EQUALS: {
          mutable_block()->add_op_code()->mutable_assign_index_member_ampersand_equals();
          break;
        }
        case AssignStmt::PIPE_EQUALS: {
          mutable_block()->add_op_code()->mutable_assign_index_member_pipe_equals();
          break;
        }
        case AssignStmt::HAT_EQUALS: {
          mutable_block()->add_op_code()->mutable_assign_index_member_hat_equals();
          break;
        }
        case AssignStmt::LESS_LESS_EQUALS: {
          mutable_block()->add_op_code()->mutable_assign_index_member_less_less_equals();
          break;
        }
        case AssignStmt::GREATER_GREATER_EQUALS: {
          mutable_block()->add_op_code()->mutable_assign_index_member_greater_greater_equals();
          break;
        }
        default:
          assert(false);
      }
      break;
    case SliceExpr::kSlice:
      switch (op) {
        case AssignStmt::EQUALS: {
          mutable_block()->add_op_code()->mutable_assign_slice_range();
          break;
        }
        case AssignStmt::PLUS_EQUALS: {
          mutable_block()->add_op_code()->mutable_assign_slice_range_plus_equals();
          break;
        }
        case AssignStmt::MINUS_EQUALS: {
          mutable_block()->add_op_code()->mutable_assign_slice_range_minus_equals();
          break;
        }
        case AssignStmt::STAR_EQUALS: {
          mutable_block()->add_op_code()->mutable_assign_slice_range_star_equals();
          break;
        }
        case AssignStmt::SLASH_EQUALS: {
          mutable_block()->add_op_code()->mutable_assign_slice_range_slash_equals();
          break;
        }
        case AssignStmt::SLASH_SLASH_EQUALS: {
          mutable_block()->add_op_code()->mutable_assign_slice_range_slash_slash_equals();
          break;
        }
        case AssignStmt::PERCENT_EQUALS: {
          mutable_block()->add_op_code()->mutable_assign_slice_range_percent_equals();
          break;
        }
        case AssignStmt::AMPERSAND_EQUALS: {
          mutable_block()->add_op_code()->mutable_assign_slice_range_ampersand_equals();
          break;
        }
        case AssignStmt::PIPE_EQUALS: {
          mutable_block()->add_op_code()->mutable_assign_slice_range_pipe_equals();
          break;
        }
        case AssignStmt::HAT_EQUALS: {
          mutable_block()->add_op_code()->mutable_assign_slice_range_hat_equals();
          break;
        }
        case AssignStmt::LESS_LESS_EQUALS: {
          mutable_block()->add_op_code()->mutable_assign_slice_range_less_less_equals();
          break;
        }
        case AssignStmt::GREATER_GREATER_EQUALS: {
          mutable_block()->add_op_code()->mutable_assign_slice_range_greater_greater_equals();
          break;
        }
        default:
          assert(false);
      }
      break;
    default:
      assert(false);
  }
}

void bytecode_generator::exit_tuple(const Tuple* tuple) {
  mutable_block()->add_op_code()->mutable_make_tuple()->set_number_of_elements(tuple->value_size());
}

void bytecode_generator::enter_tuple_for_assignment(const Tuple* tuple) {
  mutable_block()->add_op_code()->mutable_unpack()->set_number_of_elements(tuple->value_size());
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
  mutable_block()->add_op_code()->mutable_unpack()->set_number_of_elements(list_expression->element_size());
}

void bytecode_generator::enter_dictionary_expression(const DictExpr* dictionary_expression) {
  mutable_block()->add_op_code()->mutable_make_dictionary()->set_reserve_size(dictionary_expression->entry_size());
}

void bytecode_generator::exit_dictionary_expression(const DictExpr* dictionary_expression) {
  if (dictionary_expression->entry_size() != 0) {
    auto* add_to_dict = mutable_block()->add_op_code()->mutable_add_to_dictionary();
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
  mutable_block()->add_op_code()->mutable_get_iterator();
  comprehension_comp_clause.back().push_back(block().op_code_size());
  mutable_block()->add_op_code()->mutable_for_iterator();
}

void bytecode_generator::exit_if_clause(const Expression* if_clause) {
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
    mutable_block()->add_op_code()->mutable_get_iterator();
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
  auto* call = mutable_block()->add_op_code()->mutable_call();
  call->set_positional_arguments_count(pos_arguments);
  call->set_named_arguments_count(named_arguments);
  call->set_has_variadic_positional_argument(variadic_pos_arg);
  call->set_has_variadic_named_argument(variadic_named_arg);
}

void bytecode_generator::enter_argument(const Argument* argument) {
  if (argument->argument_type_case() == Argument::kNamedArgument) {
    mutable_block()->add_op_code()->mutable_const_string_view()->set_value(argument->named_argument().identifier().nfkc_name());
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
        if (param.identifier().name().empty()) {
          keyword_only_mode = true;
        } else {
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

}  // namespace

compiler::compiler(std::set<std::string, std::less<>>& binding) : binding(binding) {}

Program* compiler::compile(std::string_view starlark_program, grammar_options opt, logger& logging, Arena& arena) {
  logger_wrap logging_wrap(logging);
  parser star_parser(starlark_program,
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
  return result;
}

}  // namespace compiler
}  // namespace starlark

