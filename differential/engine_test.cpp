// Copyright 2026 Lucas Mirelmann

#include <gtest/gtest.h>

#include <string>

#include "differential/engine.hpp"
#include "runtime/options.hpp"

using ::starlark::differential::check_source;
using ::starlark::differential::programs_from_input;
using ::starlark::runtime::runtime_options;

namespace {

runtime_options test_runtime_options() {
  return runtime_options{
      .log2_max_bigint = 65536,
      .max_sequence_size = 65536,
      .max_string_length = 65536,
  };
}

}  // namespace

TEST(DifferentialEngine, SimpleProgramMatches) {
  const std::string source = "result = 1 + 2 * 3\n";
  const starlark::vm::module_info::bindings_t no_extra_bindings;
  auto comparison = check_source(source, test_runtime_options(), no_extra_bindings);
  EXPECT_TRUE(comparison.match) << comparison.reason;
}

TEST(DifferentialEngine, StaticErrorMatches) {
  const std::string source = "def broken(:\n  pass\n";
  const starlark::vm::module_info::bindings_t no_extra_bindings;
  auto comparison = check_source(source, test_runtime_options(), no_extra_bindings);
  EXPECT_TRUE(comparison.match) << comparison.reason;
}

TEST(DifferentialEngine, ProgramsFromInputSingleModule) {
  auto programs = programs_from_input("x = 1\n");
  ASSERT_EQ(programs.size(), 1u);
  EXPECT_EQ(programs["main"], "x = 1\n");
}

TEST(DifferentialEngine, LoadOrderMatchesWhenEarlierDependencyMissing) {
  const std::string source = R"(## Begin module: "//:test1.bzl"
a = [].missing_attr
## End module
## Begin module: "//:test2.bzl"
load("//:test1.bzl", "a")
b = a
## End module
## Main
load("//:test0.bzl", "a")
load("//:test2.bzl", "b")

assert_eq(a, b)
)";
  const starlark::vm::module_info::bindings_t no_extra_bindings;
  auto comparison = check_source(source, test_runtime_options(), no_extra_bindings);
  EXPECT_TRUE(comparison.match) << comparison.reason;
}

TEST(DifferentialEngine, CrossModuleComprehensionMatches) {
  const std::string source = R"(## Begin module "test1"
def outer():
    return [x for x in range(2)]
## End module

load("test1", "outer")
result = outer()
)";
  const starlark::vm::module_info::bindings_t no_extra_bindings;
  auto comparison = check_source(source, test_runtime_options(), no_extra_bindings);
  EXPECT_TRUE(comparison.match) << comparison.reason;
}
