// Copyright 2024-2025 Lucas Mirelmann

#include <fcntl.h>

#include <gmock/gmock.h>
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest-matchers.h>
#include <gtest/gtest.h>

#include <string>

#include "grammar/parser.hpp"
#include "grammar/parsing_options.hpp"
#include "grammar/proto/starlark.pb.h"
#include "third-party/defer.hpp"

using google::protobuf::util::MessageDifferencer;
using starlark::grammar::log_level;
using starlark::grammar::logger;
using starlark::grammar::parser;
using starlark::ast::File;
using testing::IsEmpty;
using testing::Not;
using testing::SizeIs;

namespace {

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
  logging.set_level(log_level::ERROR);
  starlark::grammar::grammar_options options = starlark::grammar::get_parsing_options(starlark_program);
  parser star_parser(starlark_program, options, {}, logging);
  google::protobuf::Arena arena;
  star_parser.parse_file(arena);
  EXPECT_THAT(logging, Not(IsEmpty()));
}

}  // namespace
