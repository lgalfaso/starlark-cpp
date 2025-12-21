// Copyright 2024-2025 Lucas Mirelmann

#include <fcntl.h>

#include <gmock/gmock.h>
#include <gtest/gtest-matchers.h>
#include <gtest/gtest.h>

#include <string>

#include "grammar/options.hpp"
#include "grammar/parser.hpp"
#include "proto/starlark_ast.pb.h"

using google::protobuf::Arena;
using starlark::ast::File;
using starlark::grammar::options;
using starlark::grammar::parser;
using starlark::logging::LogLevel;
using starlark::logging::logger;
using testing::IsEmpty;
using testing::Not;
using testing::SizeIs;

namespace {

void checkInvalid(std::string_view program, const options& opts) {
  logger logging;
  logging.set_level(LogLevel::LOG_LEVEL_ERROR);
  parser star_parser(program, opts, {}, logging);
  Arena arena;
  star_parser.parse_file(arena);
  EXPECT_THAT(logging, Not(IsEmpty()));
}

TEST(Parser, NoFunctionDefinition) {
  checkInvalid(R"starlark(
def foo():
  pass
)starlark", options{ .allow_function_definitions = false, });
  checkInvalid(R"starlark(
foo = lambda: True
)starlark", options{ .allow_function_definitions = false, });
}

TEST(Parser, VaradicArguments) {
  checkInvalid(R"starlark(
foo(*[1,2,3])
)starlark", options{ .allow_variadic_arguments = false, });
  checkInvalid(R"starlark(
foo(**{'a': 1, 'b': 2, 'c': 3})
)starlark", options{ .allow_variadic_arguments = false, });
}

}  // namespace

