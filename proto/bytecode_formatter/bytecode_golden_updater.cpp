// Copyright 2026 Lucas Mirelmann

#include "proto/bytecode_formatter/bytecode_golden_updater.hpp"

#include <cstdlib>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <string_view>

#include "compiler/compiler.hpp"
#include "google/protobuf/arena.h"
#include "grammar/options.hpp"
#include "io/read_file.hpp"
#include "logging/logging.hpp"
#include "proto/bytecode_formatter/bytecode_txtpb_printer.hpp"
#include "proto/starlark_bytecode.pb.h"

using ::google::protobuf::Arena;
using ::starlark::bytecode::Program;
using ::starlark::compiler::compiler;
using ::starlark::grammar::grammar_options;
using ::starlark::logging::logger;
using ::starlark::proto::print_bytecode_txtpb;

namespace starlark {
namespace proto {
namespace {

std::string resolve_path(std::string_view path) {
  if (path.empty()) {
    return {};
  }
  if (path.front() == '/') {
    return std::string{path};
  }
  if (const char* workspace = std::getenv("BUILD_WORKSPACE_DIRECTORY")) {
    return std::string{workspace} + "/" + std::string{path};
  }
  return std::string{path};
}

bool compile_star(std::string_view path, Arena& arena, Program*& program) {
  auto source = starlark::io::read_file(path);
  if (!source) {
    return false;
  }
  std::set<std::string, std::less<>> binding;
  class compiler star_compiler(binding);
  grammar_options options{
      .allow_top_level_rebinding = true,
      .allow_top_level_for = true,
      .allow_top_level_if = true,
  };
  logger logging;
  program = star_compiler.compile(path, *source, options, logging, arena);
  return program != nullptr;
}

bool write_txtpb(const Program& program, const std::string& path) {
  std::ostringstream body;
  print_bytecode_txtpb(program, body);
  std::ofstream out(path, std::ios::trunc);
  if (!out) {
    return false;
  }
  out << body.str();
  return static_cast<bool>(out);
}

}  // namespace

bool compile_bytecode_from_star(std::string_view path, Arena& arena, Program*& program) {
  return compile_star(path, arena, program);
}

bool write_bytecode_txtpb_file(const Program& program, std::string_view path) {
  return write_txtpb(program, std::string{path});
}

int update_bytecode_goldens(std::string_view directory) {
  const std::string dir = resolve_path(directory);
  if (!std::filesystem::is_directory(dir)) {
    std::cerr << "Not a directory: " << dir << '\n';
    return 1;
  }

  int updated = 0;
  for (const auto& entry : std::filesystem::directory_iterator(dir)) {
    if (!entry.is_regular_file() || entry.path().extension() != ".star") {
      continue;
    }
    const std::filesystem::path star_file = entry.path();
    const std::string star_path = star_file.string();
    std::filesystem::path txtpb_file = star_file;
    txtpb_file.replace_extension(".txtpb");
    const std::string txtpb_path = txtpb_file.string();

    Arena arena;
    Program* program = nullptr;
    if (!compile_star(star_path, arena, program)) {
      std::cerr << "Failed to compile " << star_path << '\n';
      return 2;
    }
    if (!write_txtpb(*program, txtpb_path)) {
      std::cerr << "Failed to write " << txtpb_path << '\n';
      return 3;
    }
    std::cout << "Updated " << txtpb_path << '\n';
    ++updated;
  }

  std::cout << "Updated " << updated << " golden files in " << dir << '\n';
  return 0;
}

}  // namespace proto
}  // namespace starlark
