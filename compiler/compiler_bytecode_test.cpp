// Copyright 2024-2025 Lucas Mirelmann

#include <gmock/gmock.h>
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest-matchers.h>
#include <gtest/gtest.h>

#include <fstream>
#include <functional>
#include <set>
#include <string>

#include "compiler/compiler.hpp"
#include "io/read_file.hpp"
#include "proto/starlark_bytecode.pb.h"
#include "protobuf-matchers/protocol-buffer-matchers.h"

using ::google::protobuf::Arena;
using ::google::protobuf::Message;
using ::protobuf_matchers::EqualsProto;
using ::starlark::bytecode::Program;
using ::starlark::compiler::compiler;
using ::starlark::grammar::grammar_options;
using ::starlark::logging::logger;
using ::testing::IsEmpty;
using ::testing::SizeIs;

namespace {

std::string get_errors(logger& logging) {
  std::string result;
  for (const auto& log : logging) {
    result += log.DebugString();
  }
  return result;
}

TEST(CompilerBytecode, TestCase) {
  const auto& argv = ::testing::internal::GetArgvs();
  ASSERT_THAT(argv, SizeIs(3));

  auto starlark_code = starlark::io::read_file(argv[1]);
  ASSERT_TRUE(starlark_code.has_value());

  Program expected_starlark_program;
  std::ifstream expected_proto(argv[2], std::ios::binary);
  ASSERT_TRUE(expected_proto.is_open());
  ASSERT_TRUE(expected_starlark_program.ParseFromIstream(&expected_proto));

  // We are only using the predeclared symbols.
  std::set<std::string, std::less<>> binding{};
  compiler star_compiler(binding);
  grammar_options opt {
    .allow_top_level_rebinding = true,
    .allow_top_level_for = true,
    .allow_top_level_if = true,
  };
  logger logging;
  Arena arena;
  Program* actual_starlark_program = star_compiler.compile(argv[1], *starlark_code, opt, logging, arena);

  ASSERT_NE(actual_starlark_program, nullptr) << get_errors(logging);
  EXPECT_THAT(*actual_starlark_program, EqualsProto(expected_starlark_program));
}

}  // namespace

