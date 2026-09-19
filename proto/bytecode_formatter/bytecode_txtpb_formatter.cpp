// Copyright 2026 Lucas Mirelmann

#include <fcntl.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/text_format.h"
#include "proto/bytecode_formatter/bytecode_txtpb_printer.hpp"
#include "proto/starlark_bytecode.pb.h"
#include "third-party/defer.hpp"

using ::google::protobuf::TextFormat;
using ::google::protobuf::io::FileInputStream;
using ::starlark::bytecode::Program;
using ::starlark::proto::print_bytecode_txtpb;

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

bool write_txtpb(const Program& program, std::string_view path) {
  std::ostringstream body;
  print_bytecode_txtpb(program, body);
  std::ofstream out(std::string{path}, std::ios::trunc);
  if (!out) {
    return false;
  }
  out << body.str();
  return static_cast<bool>(out);
}

void print_usage() {
  std::cerr << "Usage:\n"
            << "  bytecode_txtpb_formatter <input.txtpb> [output.txtpb]\n";
}

}  // namespace

int main(int argc, char* argv[]) {
  if (argc < 2 || argc > 3) {
    print_usage();
    return 1;
  }

  const std::string_view input_path = argv[1];
  const std::string_view output_path = argc == 3 ? std::string_view{argv[2]} : std::string_view{};

  Program program;
  if (!parse_txtpb(input_path, program)) {
    std::cerr << "Failed to parse " << input_path << '\n';
    return 2;
  }

  if (output_path.empty()) {
    print_bytecode_txtpb(program, std::cout);
    return 0;
  }

  if (!write_txtpb(program, output_path)) {
    std::cerr << "Failed to open " << output_path << " for writing\n";
    return 3;
  }
  return 0;
}
