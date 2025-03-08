// Copyright 2024-2025 Lucas Mirelmann

#include "compiler/compiler.hpp"
#include "grammar/ast_listener.hpp"
#include "grammar/parser.hpp"
#include "grammar/logging.hpp"
#include "grammar/options.hpp"

using starlark::ast::File;
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
  explicit bytecode_generator(program& output) : output(output) {}

 private:
  program& output;

};

}  // namespace

compiler::compiler(std::string_view starlark_program) : starlark_program(starlark_program) {}

program compiler::compile() {
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

  program result;
  bytecode_generator listener(result);
  starlark::grammar::ast_walker walker;
  walker.walk(starlark_file, listener);
  return result;
}

}  // namespace compiler
}  // namespace starlark

