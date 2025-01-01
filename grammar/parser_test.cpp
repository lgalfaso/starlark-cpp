// Copyright 2024 Lucas Mirelmann

#include <fcntl.h>

#include <gmock/gmock.h>
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest-matchers.h>
#include <gtest/gtest.h>

#include "grammar/options.hpp"
#include "grammar/parser.hpp"
#include "grammar/proto/starlark.pb.h"

using google::protobuf::Arena;
using grammar::log_level;
using grammar::logger;
using grammar::parser;
using grammar::grammar_options;
using starlark::File;
using testing::IsEmpty;
using testing::Not;
using testing::SizeIs;

void checkInvalid(std::string_view program, const grammar_options& opts) {
  logger logging;
  logging.set_level(log_level::ERROR);
  parser star_parser(program, opts, logging);
  Arena arena;
  star_parser.parse_file(arena);
  EXPECT_THAT(logging, Not(IsEmpty()));
}

TEST(Parser, NoFunctionDefinition) {
  checkInvalid(R"starlark(
def foo():
  pass
)starlark", grammar_options{ .allow_function_definitions = false, });
  checkInvalid(R"starlark(
foo = lambda: True
)starlark", grammar_options{ .allow_function_definitions = false, });
}

TEST(Parser, VaradicArguments) {
  checkInvalid(R"starlark(
foo(*[1,2,3])
)starlark", grammar_options{ .allow_varadic_arguments = false, });
  checkInvalid(R"starlark(
foo(**{'a': 1, 'b': 2, 'c': 3})
)starlark", grammar_options{ .allow_varadic_arguments = false, });
}

