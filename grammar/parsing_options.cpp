// Copyright 2025 Lucas Mirelmann

#include "grammar/parsing_options.hpp"

namespace starlark {
namespace grammar {

grammar_options get_parsing_options(std::string_view starlark_program) {
  grammar_options result = grammar_options{};
#define PARAM(name) \
  if (starlark_program.contains("options.no_" #name)) {       \
    result.name = false;                                      \
  } else if (starlark_program.contains("options." #name)) {   \
    result.name = true;                                       \
  }

  PARAM(escaped_octal_and_hex_char_are_ascii);
  PARAM(allow_load_private_symbols);
  PARAM(allow_function_definitions);
  PARAM(require_load_statements_first);
  PARAM(allow_variadic_arguments);
  PARAM(allow_top_level_rebinding);
  PARAM(allow_top_level_for);
  PARAM(allow_top_level_if);
  PARAM(allow_binary_integer_literals);
#undef PARAM
  return result;
}

}  // namespace grammar
}  // namespace starlark

