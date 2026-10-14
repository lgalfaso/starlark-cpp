// Copyright 2026 Lucas Mirelmann

#include <gtest/gtest.h>

#include "proto/starlark_bytecode.pb.h"
#include "runtime/options.hpp"
#include "vm/program_limits.hpp"

using ::starlark::bytecode::Program;
using ::starlark::runtime::runtime_options;
using ::starlark::vm::check_program_limits;

namespace {

Program program_with_const_string(std::string_view literal) {
  Program program;
  program.add_const_string(std::string(literal));
  auto* op = program.add_block()->add_op_code();
  op->mutable_const_string()->set_const_string_pos(0);
  return program;
}

TEST(ProgramLimitsTest, ConstStringLimitUsesCodePointCount) {
  // Two emoji: 8 UTF-8 bytes, 2 elements.
  const Program program = program_with_const_string("\xf0\x9f\x98\x80\xf0\x9f\x98\x81");
  runtime_options options{.max_string_length = 5};

  EXPECT_FALSE(check_program_limits(program, "", options).has_value());

  options.max_string_length = 1;
  EXPECT_TRUE(check_program_limits(program, "", options).has_value());
}

}  // namespace
