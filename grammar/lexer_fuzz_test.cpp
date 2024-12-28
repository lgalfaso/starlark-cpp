// Copyright 2024 Lucas Mirelmann

#include <string_view>

#include "grammar/lexer.hpp"

using grammar::lexer;
using grammar::logger;
using grammar::token_type;

void LexerFuzzing(char* data, size_t size) {
  logger logging;
  lexer l(std::string_view(data, size), logging);
  do {
    l.next_token();
  } while (l.current_token().type() != token_type::eof);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  LexerFuzzing((char*)data, size);
  return 0;
}

