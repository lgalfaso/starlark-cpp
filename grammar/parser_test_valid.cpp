// Copyright 2024-2025 Lucas Mirelmann

#include <gmock/gmock.h>
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest-matchers.h>
#include <gtest/gtest.h>

#include <string>

#include "grammar/parser.hpp"
#include "grammar/parsing_options.hpp"
#include "io/read_file.hpp"

using google::protobuf::Arena;
using google::protobuf::Message;
using starlark::ast::File;
using starlark::grammar::parser;
using starlark::logging::LogLevel;
using starlark::logging::logger;
using testing::IsEmpty;
using testing::SizeIs;

namespace {

std::string show_errors(const logger& logging) {
  std::string result;

  for (const auto& entry : logging) {
    result += "[" + std::to_string(entry.pos().row()) + "," + std::to_string(entry.pos().column()) + "] " + std::string(entry.module()) + ":" + std::string(entry.message()) + "\n";
  }
  return result;
}

TEST(Parser, TestCase) {
  const auto& argv = ::testing::internal::GetArgvs();
  ASSERT_THAT(argv, SizeIs(2));

  auto starlark_program = starlark::io::read_file(argv[1]);
  ASSERT_TRUE(starlark_program.has_value());

  logger logging;
  logging.set_level(LogLevel::LOG_LEVEL_ERROR);
  starlark::grammar::grammar_options options = starlark::grammar::get_parsing_options(*starlark_program);
  parser star_parser(argv[1], *starlark_program, options, {}, logging);
  Arena arena;
  star_parser.parse_file(arena);
  EXPECT_THAT(logging, IsEmpty()) << show_errors(logging);
}

}  // namespace
