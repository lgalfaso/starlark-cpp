// Copyright 2024-2025 Lucas Mirelmann

#include <gmock/gmock.h>
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest-matchers.h>
#include <gtest/gtest.h>

#include <fcntl.h>
#include <unistd.h>

#include <string>

#include "grammar/parser.hpp"
#include "grammar/parsing_options.hpp"
#include "io/read_file.hpp"
#include "proto/starlark_ast.pb.h"
#include "protobuf-matchers/protocol-buffer-matchers.h"
#include "third-party/defer.hpp"

using ::google::protobuf::Arena;
using ::google::protobuf::Message;
using ::protobuf_matchers::EqualsProto;
using ::starlark::ast::File;
using ::starlark::grammar::parser;
using ::starlark::logging::LogLevel;
using ::starlark::logging::logger;
using ::testing::IsEmpty;
using ::testing::SizeIs;

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
  ASSERT_THAT(argv, SizeIs(3));

  File starlark_file;
  {
    int proto_fd = open(argv[2].c_str(), O_RDONLY);
    ASSERT_GT(proto_fd, 0);
    defer { close(proto_fd); };
    ASSERT_TRUE(starlark_file.ParseFromFileDescriptor(proto_fd));
  }

  auto starlark_program = starlark::io::read_file(argv[1]);
  ASSERT_TRUE(starlark_program.has_value());

  logger logging;
  logging.set_level(LogLevel::LOG_LEVEL_ERROR);
  starlark::grammar::grammar_options options = starlark::grammar::get_parsing_options(*starlark_program);
  parser star_parser(argv[1], *starlark_program, options, {}, logging);
  Arena arena;
  File* actual_starlark_file = star_parser.parse_file(arena);

  EXPECT_THAT(*actual_starlark_file, EqualsProto(starlark_file));
  EXPECT_THAT(logging, IsEmpty()) << show_errors(logging);
}

}  // namespace
