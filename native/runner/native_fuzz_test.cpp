// Copyright 2026 Lucas Mirelmann

#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <utility>

#include "native/runner/native_options.hpp"
#include "native/runner/native_runner.hpp"
#include "native/runner/native_runtime.hpp"

using ::starlark::grammar::grammar_options;
using ::starlark::logging::logger;
using ::starlark::native::native_options;
using ::starlark::native::native_runner;
using ::starlark::native::native_runtime;
using ::starlark::runtime::runtime_options;
using ::starlark::runtime::starlark_obj;
using ::starlark::vm::kv_module_loader;

namespace {

void NativeFuzzing(const char* data, size_t size) {
  std::string module_name = "main";
  std::string_view starlark_code(data, size);
  std::map<std::string, std::pair<std::string, const std::map<std::string, starlark_obj*, std::less<>>>, std::less<>> modules;
  modules.try_emplace(module_name, starlark_code, std::map<std::string, starlark_obj*, std::less<>>{});
  kv_module_loader loader{modules};

  native_runtime runtime;
  native_runner runner(runtime);
  logger logging;

  runner.run(loader, module_name, grammar_options{}, runtime_options{.log2_max_bigint = 1048576, .max_sequence_size = 1048576, .max_string_length = 10485760, }, native_options{}, logging);
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  NativeFuzzing(reinterpret_cast<const char*>(data), size);
  return 0;
}
