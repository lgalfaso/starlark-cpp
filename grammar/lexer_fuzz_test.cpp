// Copyright 2024-2025 Lucas Mirelmann

#include <string_view>

#include "grammar/lexer.hpp"

using starlark::grammar::lexer;
using starlark::grammar::logger;
using starlark::grammar::token_type;

namespace {

void LexerFuzzing(const char* data, size_t size) {
  logger logging;
  lexer l(std::string_view(data, size), logging);
  do {
    l.next_token();
  } while (l.current_token().type() != token_type::kEof);
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  LexerFuzzing(reinterpret_cast<const char*>(data), size);
  return 0;
}

