// Copyright 2026 Lucas Mirelmann

#include <fcntl.h>
#include <sys/stat.h>

#include <format>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <utility>

#include "interpreter/interpreter.hpp"
#include "third-party/defer.hpp"

using ::starlark::grammar::grammar_options;
using ::starlark::interpreter::interpreter;
using ::starlark::interpreter::kv_module_loader;
using ::starlark::logging::logger;
using ::starlark::runtime::runtime_options;
using ::starlark::runtime::starlark_obj;

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
  std::string module_name = argv[1];
  std::string starlark_code;
  {
    int starlark_fd = open(argv[1], O_RDONLY);
    if (starlark_fd == 0) {
      std::cout << "Unable to open " << module_name << "\n";
      return 1;
    }
    defer { close(starlark_fd); };
    struct stat sb;
    if (fstat(starlark_fd, &sb) < 0) {
      std::cout << "Unable to read metadata on " << module_name << "\n";
      return 1;
    }
    starlark_code.resize(sb.st_size);
    read(starlark_fd, starlark_code.data(), sb.st_size);
  }

  std::map<std::string, std::pair<std::string, const std::map<std::string, starlark_obj*, std::less<>>>, std::less<>> modules;
  modules.try_emplace(module_name, starlark_code, std::map<std::string, starlark_obj*, std::less<>>{});
  kv_module_loader loader{modules};

  interpreter runner;
  logger logging;

  if (runner.run(loader, module_name, grammar_options{}, runtime_options{}, logging) == nullptr) {
    print_logs(logging);
    return 1;
  }
  return 0;
}

