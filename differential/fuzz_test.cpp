// Copyright 2026 Lucas Mirelmann

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <string_view>
#include <vector>

#include "differential/spec_support.hpp"
#include "native/runner/native_options.hpp"
#include "runtime/options.hpp"

using ::starlark::differential::check_source_like_spec_tests;
using ::starlark::differential::compare_result;
using ::starlark::native::native_options;
using ::starlark::runtime::runtime_options;

namespace {

constexpr std::size_t kMaxInputSize = 64 * 1024;

bool argv_has_prefix(int argc, char** argv, std::string_view prefix) {
  for (int i = 0; i < argc; ++i) {
    if (std::string_view{argv[i]}.starts_with(prefix)) {
      return true;
    }
  }
  return false;
}

runtime_options fuzz_runtime_options() {
  // Match //differential:engine_test limits; large values multiply ASAN peak RSS.
  return runtime_options{
      .log2_max_bigint = 65536,
      .max_sequence_size = 65536,
      .max_string_length = 65536,
  };
}

native_options fuzz_native_options() {
  return native_options{
      .fast_jit = true,
  };
}

}  // namespace

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv) {
  // Bazel's default replay engine replays every argv[1..] as a corpus path; do not inject flags.
  const char* regression = std::getenv("FUZZER_IS_REGRESSION");
  if (regression != nullptr && regression[0] == '1' && regression[1] == '\0') {
    return 0;
  }

  static std::once_flag once;
  static std::vector<char*> new_argv;

  std::call_once(once, [argc, argv] {
    new_argv.reserve(static_cast<std::size_t>(*argc) + 2);
    new_argv.push_back((*argv)[0]);

    // Insert before corpus paths: libFuzzer treats trailing argv entries as inputs to replay.
    // String literals: pointers must stay valid for the process lifetime (FuzzerDriver copies argv later).
    // Value profiling stores a large feature set per edge; this target already has huge coverage (LLVM JIT).
    if (!argv_has_prefix(*argc, *argv, "-use_value_profile=")) {
      new_argv.push_back(const_cast<char*>("-use_value_profile=0"));
    }
    // libFuzzer defaults to 2048 MiB; differential+ASAN+seed corpora needs more headroom.
    if (!argv_has_prefix(*argc, *argv, "-rss_limit_mb=")) {
      new_argv.push_back(const_cast<char*>("-rss_limit_mb=8192"));
    }

    for (int i = 1; i < *argc; ++i) {
      new_argv.push_back((*argv)[i]);
    }

    *argc = static_cast<int>(new_argv.size());
    *argv = new_argv.data();
  });
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size == 0 || size > kMaxInputSize) {
    return 0;
  }
  std::string_view source(reinterpret_cast<const char*>(data), size);
  compare_result result = check_source_like_spec_tests(source, fuzz_runtime_options(), fuzz_native_options());
  if (!result.match) {
    std::fprintf(stderr, "Interpreter/native differential mismatch: %s\n", result.reason.c_str());
    std::fwrite(source.data(), 1, source.size(), stderr);
    std::fputc('\n', stderr);
    __builtin_trap();
  }
  return 0;
}
