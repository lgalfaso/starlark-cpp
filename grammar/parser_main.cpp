// Copyright 2024 Lucas Mirelmann

#include <sys/stat.h>
#include <fcntl.h>

#include <iostream>

#include "third-party/defer.hpp"
#include "grammar/options.hpp"
#include "grammar/parser.hpp"
#include "grammar/proto/starlark.pb.h"

using grammar::log_level;
using grammar::logger;
using grammar::parser;
using grammar::grammar_options;
using starlark::File;

int main(int argc, char* argv[]) {
  for (int i = 1; i < argc; ++i) {
    std::string starlark_program;
    {
      int in_fd = open(argv[i], O_RDONLY);
      if (in_fd < 0) {
        return 2;
      }
      defer { close(in_fd); };
      struct stat sb;
      if (fstat(in_fd, &sb) != 0) {
        return 3;
      }
      starlark_program.resize(sb.st_size);
      read(in_fd, starlark_program.data(), sb.st_size);
    }

    std::string arg{argv[i]};
    logger logging;
    bool is_build_or_workspace =
        arg.ends_with("WORKSPACE") ||
        arg.ends_with("WORKSPACE.bazel") ||
        arg.ends_with("BUILD") ||
        arg.ends_with("BUILD.bazel");
    parser star_parser(starlark_program,
                       grammar_options{
                           .escaped_octal_and_hex_char_are_ascii = false,
                           .require_load_statements_first = !is_build_or_workspace,
                           .allow_varadic_arguments = !is_build_or_workspace,
                       },
                       logging);
    google::protobuf::Arena arena;
    [[maybe_unused]] File* actual_starlark_file = star_parser.parse_file(arena);
    bool print_header = true;
    for (const auto& entry : logging) {
      if (entry.level != log_level::ERROR && entry.level != log_level::FATAL) {
        continue;
      }
      if (print_header) {
        std::cout << "Unable to parse: " << argv[i] << "\n";
        print_header = false;
      }
      std::cout << "  " << entry.module << ":" << entry.message << ":" << entry.pos.row << "," << entry.pos.column << "\n";
    }
  }

  return 0;
}

