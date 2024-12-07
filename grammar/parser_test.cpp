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
using grammar::parser;
using starlark::File;
using testing::IsEmpty;

TEST(Parser, TestCase) {
  const auto& argv = ::testing::internal::GetArgvs();
  ASSERT_EQ(argv.size(), 3);

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

  parser star_parser(starlark_program);
  File actual_starlark_file = star_parser.parse_file();
  EXPECT_TRUE(MessageDifferencer::Equals(actual_starlark_file, starlark_file)) <<
      "Expected: " << starlark_file.DebugString() << "\n" <<
      "Actual:   " << actual_starlark_file.DebugString() << "\n";
  EXPECT_THAT(star_parser.parser_errors(), IsEmpty());
  EXPECT_THAT(star_parser.lexer_errors(), IsEmpty());
}

