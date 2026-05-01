// Copyright 2026 Lucas Mirelmann

#include <fcntl.h>
#include <sys/stat.h>

#include <format>
#include <iostream>
#include <map>
#include <string>

#include "interpreter/interpreter.hpp"
#include "third-party/defer.hpp"

using ::google::protobuf::Arena;
using ::starlark::grammar::grammar_options;
using ::starlark::runtime::runtime_options;
using ::starlark::interpreter::interpreter;
using ::starlark::logging::logger;

std::string print_logs(logger& logging) {
  std::string result;
  for (const auto& entry : logging) {
    std::cout << std::format("Error at {}\n{}\n", entry.pos().ShortDebugString(), entry.message());
  }
  return result;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << argv[0] << " input-file\n";
    return 1;
  }
  std::string starlark_code;
  {
    int starlark_fd = open(argv[1], O_RDONLY);
    if (starlark_fd == 0) {
      std::cout << "Unable to open " << argv[1] << "\n";
      return 1;
    }
    defer { close(starlark_fd); };
    struct stat sb;
    if (fstat(starlark_fd, &sb) < 0) {
      std::cout << "Unable to read metadata on " << argv[1] << "\n";
      return 1;
    }
    starlark_code.resize(sb.st_size);
    read(starlark_fd, starlark_code.data(), sb.st_size);
  }

  interpreter runner;
  logger logging;
  Arena arena;

  if (runner.run(starlark_code, grammar_options{}, runtime_options{}, {}, arena, logging) == nullptr) {
    print_logs(logging);
  }
}

