// Copyright 2026 Lucas Mirelmann

#include <fcntl.h>

#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

#include "google/protobuf/arena.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/text_format.h"
#include "proto/bytecode_formatter/bytecode_golden_updater.hpp"
#include "proto/bytecode_formatter/bytecode_txtpb_printer.hpp"
#include "proto/starlark_bytecode.pb.h"
#include "third-party/defer.hpp"

using ::google::protobuf::Arena;
using ::google::protobuf::TextFormat;
using ::google::protobuf::io::FileInputStream;
using ::starlark::bytecode::Program;
using ::starlark::proto::compile_bytecode_from_star;
using ::starlark::proto::print_bytecode_txtpb;
using ::starlark::proto::update_bytecode_goldens;
using ::starlark::proto::write_bytecode_txtpb_file;

namespace {

bool parse_txtpb(std::string_view path, Program& program) {
  const int fd = open(std::string{path}.c_str(), O_RDONLY);
  if (fd < 0) {
    return false;
  }
  defer { close(fd); };
  FileInputStream input(fd);
  return TextFormat::Parse(&input, &program);
}

void print_usage() {
  std::cerr << "Usage:\n"
            << "  bytecode_txtpb_formatter <input.txtpb> [output.txtpb]\n"
            << "  bytecode_txtpb_formatter --compile <input.star> [output.txtpb]\n"
            << "  bytecode_txtpb_formatter --update-goldens <directory>\n"
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

  if (argc < 2 || argc > 4) {
    print_usage();
    return 1;
  }

  const bool compile_mode = mode == "--compile";
  if (compile_mode && argc < 3) {
    print_usage();
    return 1;
  }

  const std::string_view input_path = compile_mode ? std::string_view{argv[2]} : std::string_view{argv[1]};
  const std::string_view output_path = compile_mode ? (argc == 4 ? std::string_view{argv[3]} : std::string_view{})
                                                  : (argc == 3 ? std::string_view{argv[2]} : std::string_view{});

  Program parsed_program;
  Program* program = nullptr;
  Arena compile_arena;
  if (compile_mode) {
    if (!compile_bytecode_from_star(input_path, compile_arena, program)) {
      std::cerr << "Failed to compile " << input_path << '\n';
      return 2;
    }
  } else if (!parse_txtpb(input_path, parsed_program)) {
    std::cerr << "Failed to parse " << input_path << '\n';
    return 3;
  } else {
    program = &parsed_program;
  }

  if (output_path.empty()) {
    print_bytecode_txtpb(*program, std::cout);
    return 0;
  }

  if (!write_bytecode_txtpb_file(*program, output_path)) {
    std::cerr << "Failed to open " << output_path << " for writing\n";
    return 4;
  }
  return 0;
}
