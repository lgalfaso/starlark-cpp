// Copyright 2026 Lucas Mirelmann

#include <iostream>
#include <string>
#include <string_view>

#include "compiler/tools/bytecode_golden_updater.hpp"
#include "google/protobuf/arena.h"
#include "proto/bytecode_formatter/bytecode_txtpb_printer.hpp"
#include "proto/starlark_bytecode.pb.h"

using ::google::protobuf::Arena;
using ::starlark::bytecode::Program;
using ::starlark::compiler::tools::compile_bytecode_from_star;
using ::starlark::compiler::tools::update_bytecode_goldens;
using ::starlark::compiler::tools::write_bytecode_txtpb_file;
using ::starlark::proto::print_bytecode_txtpb;

namespace {

void print_usage() {
  std::cerr << "Usage:\n"
            << "  bytecode_txtpb_tool --compile <input.star> [output.txtpb]\n"
            << "  bytecode_txtpb_tool --update-goldens <directory>\n"
            << "\n"
            << "When run via `bazel run`, <directory> may be workspace-relative\n"
            << "(for example: compiler/bytecode_generator_tests).\n";
}

}  // namespace

int main(int argc, char* argv[]) {
  if (argc < 2) {
    print_usage();
    return 1;
  }

  const std::string_view mode = argv[1];
  if (mode == "--update-goldens") {
    if (argc != 3) {
      print_usage();
      return 1;
    }
    return update_bytecode_goldens(argv[2]);
  }

  if (mode != "--compile" || argc < 3) {
    print_usage();
    return 1;
  }

  const std::string_view input_path = argv[2];
  const std::string_view output_path = argc == 4 ? std::string_view{argv[3]} : std::string_view{};

  Program* program = nullptr;
  Arena compile_arena;
  if (!compile_bytecode_from_star(input_path, compile_arena, program)) {
    std::cerr << "Failed to compile " << input_path << '\n';
    return 2;
  }

  if (output_path.empty()) {
    print_bytecode_txtpb(*program, std::cout);
    return 0;
  }

  if (!write_bytecode_txtpb_file(*program, output_path)) {
    std::cerr << "Failed to open " << output_path << " for writing\n";
    return 3;
  }
  return 0;
}
