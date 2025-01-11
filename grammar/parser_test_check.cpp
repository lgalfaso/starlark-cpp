// Copyright 2024 Lucas Mirelmann

#include <fcntl.h>

#include <gmock/gmock.h>
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest-matchers.h>
#include <gtest/gtest.h>

#include "grammar/parser.hpp"
#include "grammar/proto/starlark.pb.h"
#include "third-party/defer.hpp"

using google::protobuf::Message;
using google::protobuf::util::MessageDifferencer;
using grammar::log_level;
using grammar::logger;
using grammar::parser;
using starlark::File;
using testing::IsEmpty;
using testing::SizeIs;

std::string describe_diff(const Message& actual, const Message& expected) {
  MessageDifferencer differencer;
  std::string diff;

  differencer.ReportDifferencesToString(&diff);
  differencer.Compare(expected, actual);
  return "with the difference:\n" + diff;
}

std::string show_errors(const logger& logging) {
  std::string result;

  for (const auto& entry : logging) {
    if (entry.level == log_level::FATAL || entry.level == log_level::ERROR) {
      result += "[" + std::to_string(entry.pos.row) + "," + std::to_string(entry.pos.column) + "] " + entry.module + ":" + entry.message + "\n";
    }
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
  logging.set_level(log_level::ERROR);
  grammar::grammar_options options = {
    .allow_top_level_if_and_for = starlark_program.contains("options.allow_top_level_if_and_for"),
  };
  parser star_parser(starlark_program, options, logging);
  google::protobuf::Arena arena;
  File* actual_starlark_file = star_parser.parse_file(arena);

  EXPECT_TRUE(MessageDifferencer::Equals(*actual_starlark_file, starlark_file)) <<
      describe_diff(*actual_starlark_file, starlark_file);
  EXPECT_THAT(logging, IsEmpty()) << show_errors(logging);
}

