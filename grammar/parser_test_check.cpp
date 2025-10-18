// Copyright 2024-2025 Lucas Mirelmann

#include <fcntl.h>

#include <gmock/gmock.h>
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest-matchers.h>
#include <gtest/gtest.h>

#include <string>

#include "grammar/parser.hpp"
#include "grammar/parsing_options.hpp"
#include "proto/starlark_ast.pb.h"
#include "protobuf-matchers/protocol-buffer-matchers.h"
#include "third-party/defer.hpp"

using ::google::protobuf::Message;
using ::protobuf_matchers::EqualsProto;
using ::starlark::ast::File;
using ::starlark::grammar::log_level;
using ::starlark::grammar::logger;
using ::starlark::grammar::parser;
using ::testing::IsEmpty;
using ::testing::SizeIs;

namespace {

std::string show_errors(const logger& logging) {
  std::string result;

  for (const auto& entry : logging) {
    result += "[" + std::to_string(entry.pos.row) + "," + std::to_string(entry.pos.column) + "] " + entry.module + ":" + entry.message + "\n";
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

  std::string starlark_program;
  {
    int starlark_fd = open(argv[1].c_str(), O_RDONLY);
    ASSERT_GT(starlark_fd, 0);
    defer { close(starlark_fd); };
    struct stat sb;
    ASSERT_GE(fstat(starlark_fd, &sb), 0);
    starlark_program.resize(sb.st_size);
    read(starlark_fd, starlark_program.data(), sb.st_size);
  }

  logger logging;
  logging.set_level(log_level::kError);
  starlark::grammar::options options = starlark::grammar::get_parsing_options(starlark_program);
  parser star_parser(starlark_program, options, {}, logging);
  google::protobuf::Arena arena;
  File* actual_starlark_file = star_parser.parse_file(arena);

  EXPECT_THAT(*actual_starlark_file, EqualsProto(starlark_file));
  EXPECT_THAT(logging, IsEmpty()) << show_errors(logging);
}

}  // namespace
