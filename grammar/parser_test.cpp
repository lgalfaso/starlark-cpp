// Copyright 2024 Lucas Mirelmann

#include <fcntl.h>

#include <gmock/gmock.h>
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest-matchers.h>
#include <gtest/gtest.h>

#include "grammar/options.hpp"
#include "grammar/parser.hpp"
#include "grammar/proto/starlark.pb.h"

using grammar::log_level;
using grammar::logger;
using grammar::parser;
using grammar::grammar_options;
using starlark::File;
using testing::SizeIs;

bool has_error(const logger& logging) {
  for (const auto& entry : logging) {
    if (entry.level == log_level::FATAL || entry.level == log_level::ERROR) {
      return true;
    }
  }
  return false;
}

void checkInvalid(std::string_view program, const grammar_options& opts) {
  logger logging;
  parser star_parser(program, opts, logging);
  google::protobuf::Arena arena;
  [[maybe_unused]] File* actual_starlark_file = star_parser.parse_file(arena);
  EXPECT_TRUE(has_error(logging));
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

