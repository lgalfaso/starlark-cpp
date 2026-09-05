// Copyright 2026 Lucas Mirelmann

#include <format>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <utility>

#include "interpreter/interpreter.hpp"
#include "io/read_file.hpp"

using ::starlark::grammar::grammar_options;
using ::starlark::interpreter::interpreter;
using ::starlark::logging::logger;
using ::starlark::runtime::runtime_options;
using ::starlark::runtime::starlark_obj;
using ::starlark::vm::kv_module_loader;

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
  auto starlark_code = starlark::io::read_file(module_name);
  if (!starlark_code) {
    std::cout << "Unable to read " << module_name << "\n";
    return 1;
  }

  std::map<std::string, std::pair<std::string, const std::map<std::string, starlark_obj*, std::less<>>>, std::less<>> modules;
  modules.try_emplace(module_name, *starlark_code, std::map<std::string, starlark_obj*, std::less<>>{});
  kv_module_loader loader{modules};

  interpreter runner;
  logger logging;

  if (!runner.run(loader, module_name, grammar_options{}, runtime_options{}, logging).ok()) {
    print_logs(logging);
    return 1;
  }
  return 0;
}

