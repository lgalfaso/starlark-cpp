// Copyright 2024-2025 Lucas Mirelmann

#include <fcntl.h>

#include <gmock/gmock.h>
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest-matchers.h>
#include <gtest/gtest.h>

#include <string>

#include "compiler/compiler.hpp"
#include "proto/starlark_bytecode.pb.h"
#include "third-party/defer.hpp"

using google::protobuf::Message;
using google::protobuf::util::MessageDifferencer;
using starlark::bytecode::Program;
using starlark::compiler::compiler;
using testing::IsEmpty;
using testing::SizeIs;

namespace {

std::string describe_diff(const Message& actual, const Message& expected) {
  MessageDifferencer differencer;
  std::string diff;

  differencer.ReportDifferencesToString(&diff);
  differencer.Compare(expected, actual);
  return "with the difference:\n" + diff;
}

TEST(CompilerBytecode, TestCase) {
  const auto& argv = ::testing::internal::GetArgvs();
  ASSERT_THAT(argv, SizeIs(3));

  std::string starlark_code;
  {
    int starlark_fd = open(argv[1].c_str(), O_RDONLY);
    ASSERT_GT(starlark_fd, 0);
    defer { close(starlark_fd); };
    struct stat sb;
    ASSERT_GE(fstat(starlark_fd, &sb), 0);
    starlark_code.resize(sb.st_size);
    read(starlark_fd, starlark_code.data(), sb.st_size);
  }

  Program starlark_program;
  {
    int proto_fd = open(argv[2].c_str(), O_RDONLY);
    ASSERT_GT(proto_fd, 0);
    defer { close(proto_fd); };
    ASSERT_TRUE(starlark_program.ParseFromFileDescriptor(proto_fd));
  }

  compiler star_compiler(starlark_code);
  Program actual_starlark_program = star_compiler.compile();

  EXPECT_TRUE(MessageDifferencer::Equals(actual_starlark_program, starlark_program)) <<
    describe_diff(actual_starlark_program, starlark_program);
}

}  // namespace

