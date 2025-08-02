// Copyright 2024-2025 Lucas Mirelmann

#include "compiler/compiler.hpp"

#include <set>
#include <string>

#include "grammar/ast_listener.hpp"
#include "grammar/logging.hpp"
#include "grammar/options.hpp"
#include "grammar/parser.hpp"

using starlark::ast::File;
using starlark::ast::Statement;
using starlark::bytecode::Program;
using starlark::grammar::ast_listener;
using starlark::grammar::ast_listener_base;
using starlark::grammar::log_level;
using starlark::grammar::logger;
using starlark::grammar::options;
using starlark::grammar::parser;

namespace starlark {
namespace compiler {

namespace {

class bytecode_generator : public ast_listener_base {
 public:
  explicit bytecode_generator(Program& output);
  void exit_statement(const Statement* statement);
  void enter_int_value(const std::string* int_value);

 private:
  Program& output;
};

bytecode_generator::bytecode_generator(Program& output) : output(output) {}

void bytecode_generator::exit_statement(const Statement* statement) {
  output.add_op_code()->mutable_drop();
}

void bytecode_generator::enter_int_value(const std::string* int_value) {
  *output.add_op_code()->mutable_const_int()->mutable_value() = *int_value;
}

}  // namespace

compiler::compiler(std::string_view starlark_program) : starlark_program(starlark_program) {}

Program compiler::compile() {
  logger logging;
  // TODO(lmirelmann): Log level should be configurable.
  logging.set_level(log_level::WARNING);
  // TODO(lmirelmann): The extra symbols should be configurable.
  std::set<std::string> extra_symbols;
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
  starlark::grammar::ast_walker walker;
  walker.walk(starlark_file, listener);
  return result;
}

}  // namespace compiler
}  // namespace starlark

