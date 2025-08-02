// Copyright 2024-2025 Lucas Mirelmann

#include <string_view>

#include "grammar/parser.hpp"
#include "proto/starlark_ast.pb.h"

using starlark::ast::File;
using starlark::grammar::logger;
using starlark::grammar::parser;

namespace {

void ParserFuzzing(const char* data, size_t size) {
  logger logging;
  parser star_parser(std::string_view(data, size), logging);
  google::protobuf::Arena arena;
  star_parser.parse_file(arena);
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  ParserFuzzing(reinterpret_cast<const char*>(data), size);
  return 0;
}

