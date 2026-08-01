// Copyright 2024-2025 Lucas Mirelmann

#include <string_view>

#include "grammar/parser.hpp"
#include "proto/starlark_ast.pb.h"

using ::google::protobuf::Arena;
using ::starlark::ast::File;
using ::starlark::grammar::parser;
using ::starlark::logging::logger;

namespace {

void ParserFuzzing(const char* data, size_t size) {
  logger logging;
  parser star_parser("main", std::string_view(data, size), logging);
  Arena arena;
  star_parser.parse_file(arena);
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  ParserFuzzing(reinterpret_cast<const char*>(data), size);
  return 0;
}

