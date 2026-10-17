// Copyright 2026 Lucas Mirelmann

#include <iostream>
#include <string>

#include "differential/spec_support.hpp"
#include "io/read_file.hpp"
#include "runtime/options.hpp"

using ::starlark::differential::check_source_like_spec_tests;
using ::starlark::runtime::runtime_options;

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "usage: differential_check <path.star>\n";
    return 2;
  }
  auto source = starlark::io::read_file(argv[1]);
  if (!source.has_value()) {
    std::cerr << "failed to read " << argv[1] << '\n';
    return 1;
  }
  runtime_options options{
      .log2_max_bigint = 1048576,
      .max_sequence_size = 1048576,
      .max_string_length = 10485760,
  };
  auto result = check_source_like_spec_tests(*source, options);
  if (!result.match) {
    std::cerr << "mismatch: " << result.reason << '\n';
    return 1;
  }
  std::cout << "ok\n";
  return 0;
}
