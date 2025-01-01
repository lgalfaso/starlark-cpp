// Copyright 2024 Lucas Mirelmann

#include <string_view>

#include "grammar/parser.hpp"
#include "grammar/proto/starlark.pb.h"

using grammar::logger;
using grammar::parser;
using starlark::File;

void ParserFuzzing(char* data, size_t size) {
  logger logging;
  parser star_parser(std::string_view(data, size), logging);
  google::protobuf::Arena arena;
  star_parser.parse_file(arena);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  ParserFuzzing((char*)data, size);
  return 0;
}

