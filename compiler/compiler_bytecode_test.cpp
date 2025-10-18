// Copyright 2024-2025 Lucas Mirelmann

#include <fcntl.h>

#include <gmock/gmock.h>
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest-matchers.h>
#include <gtest/gtest.h>

#include <set>
#include <string>

#include "compiler/compiler.hpp"
#include "proto/starlark_bytecode.pb.h"
#include "protobuf-matchers/protocol-buffer-matchers.h"
#include "third-party/defer.hpp"

using ::google::protobuf::Message;
using ::protobuf_matchers::EqualsProto;
using ::starlark::bytecode::Program;
using ::starlark::compiler::compiler;
using ::testing::IsEmpty;
using ::testing::SizeIs;

namespace {

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

  Program expected_starlark_program;
  {
    int proto_fd = open(argv[2].c_str(), O_RDONLY);
    ASSERT_GT(proto_fd, 0);
    defer { close(proto_fd); };
    ASSERT_TRUE(expected_starlark_program.ParseFromFileDescriptor(proto_fd));
  }

  std::set<std::string, std::less<>> binding{"None, True, False, len"};
  compiler star_compiler(binding);
  Program actual_starlark_program = star_compiler.compile(starlark_code);

  EXPECT_THAT(actual_starlark_program, EqualsProto(expected_starlark_program));
}

}  // namespace

