// Copyright 2025 Lucas Mirelmann

#include "grammar/parsing_options.hpp"

namespace starlark {
namespace grammar {

grammar_options get_parsing_options(std::string_view starlark_program) {
  return grammar_options{
    .allow_top_level_rebinding = starlark_program.contains("options.allow_top_level_rebinding"),
  };
}

}  // namespace grammar
}  // namespace starlark

