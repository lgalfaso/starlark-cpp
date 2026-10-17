// Copyright 2026 Lucas Mirelmann

#include "differential/spec_support.hpp"

#include "google/protobuf/arena.h"
#include "vm/test_assertions.hpp"

using ::google::protobuf::Arena;
using ::starlark::runtime::runtime_options;
using ::starlark::vm::test::make_common_test_bindings;

namespace starlark {
namespace differential {

compare_result check_source_like_spec_tests(std::string_view source,
    const runtime_options& options,
    const starlark::native::native_options& native_options) {
  Arena arena;
  auto bindings = make_common_test_bindings(arena);
  return check_source(source, options, bindings, native_options);
}

}  // namespace differential
}  // namespace starlark
