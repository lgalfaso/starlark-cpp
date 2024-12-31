// Copyright 2024 Lucas Mirelmann

#include <fcntl.h>

#include <gmock/gmock.h>
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest-matchers.h>
#include <gtest/gtest.h>

#include "grammar/parser.hpp"
#include "grammar/proto/starlark.pb.h"
#include "third-party/defer.hpp"

using google::protobuf::util::MessageDifferencer;
using grammar::log_level;
using grammar::logger;
using grammar::parser;
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

TEST(Parser, TestCase) {
  const auto& argv = ::testing::internal::GetArgvs();
  ASSERT_THAT(argv, SizeIs(2));

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
  parser star_parser(starlark_program, logging);
  google::protobuf::Arena arena;
  [[maybe_unused]] File* actual_starlark_file = star_parser.parse_file(arena);
  EXPECT_TRUE(has_error(logging));
}

