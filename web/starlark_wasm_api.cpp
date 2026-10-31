// Copyright 2026 Lucas Mirelmann

#include <cstdlib>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define STARLARK_WASM_EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define STARLARK_WASM_EXPORT
#endif

#include "interpreter/interpreter.hpp"
#include "logging/logging.hpp"
#include "runtime/options.hpp"
#include "vm/module_loader.hpp"

namespace {

using ::starlark::grammar::grammar_options;
using ::starlark::interpreter::interpreter;
using ::starlark::logging::logger;
using ::starlark::logging::pretty_log;
using ::starlark::runtime::runtime_options;
using ::starlark::runtime::starlark_obj;
using ::starlark::vm::kv_module_loader;

using module_map_t = std::map<std::string, std::pair<std::string, const std::map<std::string, starlark_obj*, std::less<>>>, std::less<>>;

const std::map<std::string, starlark_obj*, std::less<>> g_empty_bindings{};
module_map_t g_modules;

std::string json_escape(std::string_view text) {
  std::string escaped;
  escaped.reserve(text.size() + 8);
  for (const char ch : text) {
    switch (ch) {
      case '\\':
        escaped += "\\\\";
        break;
      case '"':
        escaped += "\\\"";
        break;
      case '\n':
        escaped += "\\n";
        break;
      case '\r':
        escaped += "\\r";
        break;
      case '\t':
        escaped += "\\t";
        break;
      default:
        if (static_cast<unsigned char>(ch) < 0x20) {
          escaped += "\\u00";
          escaped.push_back(static_cast<char>('0' + (static_cast<unsigned char>(ch) >> 4)));
          const int nybble = static_cast<unsigned char>(ch) & 0xf;
          escaped.push_back(static_cast<char>(nybble < 10 ? '0' + nybble : 'a' + nybble - 10));
        } else {
          escaped.push_back(ch);
        }
        break;
    }
  }
  return escaped;
}

char* allocate_json(std::string json) {
  char* buffer = static_cast<char*>(std::malloc(json.size() + 1));
  if (buffer == nullptr) {
    return nullptr;
  }
  json.copy(buffer, json.size());
  buffer[json.size()] = '\0';
  return buffer;
}

std::string make_success_json(std::string_view output) {
  return std::string{"{\"ok\":true,\"output\":\""} + json_escape(output) + "\"}";
}

std::string make_failure_json(const logger& logging) {
  std::string json = "{\"ok\":false,\"errors\":[";
  bool first = true;
  for (const auto& entry : logging) {
    if (!first) {
      json += ',';
    }
    first = false;
    json += '"';
    json += json_escape(pretty_log(entry));
    json += '"';
  }
  json += "]}";
  return json;
}

char* run_entry_module(std::string_view entry_module) {
  if (entry_module.empty()) {
    return allocate_json("{\"ok\":false,\"errors\":[\"missing entry module\"]}");
  }
  if (g_modules.find(std::string{entry_module}) == g_modules.end()) {
    return allocate_json("{\"ok\":false,\"errors\":[\"unknown entry module\"]}");
  }

  kv_module_loader loader{g_modules};

  std::ostringstream output;
  runtime_options r_options{.out = output};

  interpreter runner;
  logger logging;

  if (!runner.run(loader, entry_module, grammar_options{}, r_options, logging).ok()) {
    return allocate_json(make_failure_json(logging));
  }
  return allocate_json(make_success_json(output.str()));
}

}  // namespace

extern "C" {

STARLARK_WASM_EXPORT
void starlark_clear_modules() {
  g_modules.clear();
}

STARLARK_WASM_EXPORT
void starlark_define_module(const char* name, const char* source) {
  if (name == nullptr || source == nullptr) {
    return;
  }
  const std::string module_name{name};
  auto it = g_modules.find(module_name);
  if (it != g_modules.end()) {
    g_modules.erase(it);
  }
  g_modules.emplace(module_name, std::pair{std::string{source}, g_empty_bindings});
}

STARLARK_WASM_EXPORT
char* starlark_run(const char* entry_module) {
  if (entry_module == nullptr) {
    return allocate_json("{\"ok\":false,\"errors\":[\"missing entry module\"]}");
  }
  return run_entry_module(entry_module);
}

}  // extern "C"
