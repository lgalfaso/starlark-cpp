// Copyright 2026 Lucas Mirelmann

#include <iostream>

#include "google/protobuf/arena.h"
#include "logging/logging.hpp"
#include "compiler/test/stack_depth_test_support.hpp"

using ::google::protobuf::Arena;
using ::starlark::logging::logger;
using ::starlark::compiler::analysis::test::all_star_test_files;
using ::starlark::compiler::analysis::test::compilable_opcode_inventory;
using ::starlark::compiler::analysis::test::compiler_internal_opcode_inventory;
using ::starlark::compiler::analysis::test::format_opcode_coverage_report;
using ::starlark::compiler::analysis::test::map_opcodes_to_source_files;
using ::starlark::compiler::analysis::test::runtime_only_opcode_inventory;

int main() {
  Arena arena;
  logger logging;
  const auto mapping = map_opcodes_to_source_files(all_star_test_files(), arena, logging);
  std::cout << format_opcode_coverage_report(mapping);

  std::size_t emitted = 0;
  std::size_t compilable_emitted = 0;
  for (const auto case_ : compilable_opcode_inventory()) {
    const auto it = mapping.find(case_);
    if (it != mapping.end() && !it->second.empty()) {
      ++compilable_emitted;
    }
  }
  for (const auto& [case_, files] : mapping) {
    if (!files.empty()) {
      ++emitted;
    }
  }

  std::cout << "\nsummary\n";
  std::cout << "compilable_inventory=" << compilable_opcode_inventory().size() << "\n";
  std::cout << "compilable_emitted=" << compilable_emitted << "\n";
  std::cout << "runtime_only_inventory=" << runtime_only_opcode_inventory().size() << "\n";
  std::cout << "compiler_internal_inventory=" << compiler_internal_opcode_inventory().size() << "\n";
  std::cout << "distinct_emitted_opcodes=" << emitted << "\n";
  return compilable_emitted == compilable_opcode_inventory().size() ? 0 : 1;
}
