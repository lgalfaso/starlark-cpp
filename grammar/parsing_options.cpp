// Copyright 2025 Lucas Mirelmann

#include "grammar/parsing_options.hpp"

namespace grammar {

grammar_options get_parsing_options(std::string_view starlark_program) {
  grammar::grammar_options options = {
    .allow_top_level_if_and_for = starlark_program.contains("options.allow_top_level_if_and_for"),
    .allow_top_level_rebinding = starlark_program.contains("options.allow_top_level_rebinding"),
  };
  return options;
}

}  // namespace grammar

