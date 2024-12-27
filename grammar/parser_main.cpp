// Copyright 2024 Lucas Mirelmann

#include <sys/stat.h>
#include <fcntl.h>

#include <iostream>

#include "third-party/defer.hpp"
#include "grammar/options.hpp"
#include "grammar/parser.hpp"
#include "grammar/proto/starlark.pb.h"

using grammar::parser;
using grammar::grammar_options;
using starlark::File;

int main(int argc, char* argv[]) {
  if (argc != 2) {
    return 1;
  }

  std::string starlark_program;
  {
    int in_fd = open(argv[1], O_RDONLY);
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

  parser star_parser(starlark_program, grammar_options{ .escaped_octal_and_hex_char_are_ascii = false, });
  File actual_starlark_file = star_parser.parse_file();
  bool found_errors = !star_parser.lexer_errors().empty() ||
     !star_parser.parser_errors().empty();
  if (found_errors) {
    std::cout << "Unable to parse: " << argv[1] << "\n";
    std::cout << "Lexer errors\n";
    for (const auto& entry : star_parser.lexer_errors()) {
      std::cout << "  " << entry.first << ":" << entry.second.row << "," << entry.second.column << "   '" << starlark_program[entry.second.pos] << "'\n";
    }
    std::cout << "Grammar errors\n";
    for (const auto& entry : star_parser.parser_errors()) {
      std::cout << "  " << entry.first << ":" << entry.second.row << "," << entry.second.column << "\n";
    }
  }

  return 0;
}

