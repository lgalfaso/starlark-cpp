// Copyright 2024-2025 Lucas Mirelmann

#include "grammar/options.hpp"

#include <limits>
#include <set>
#include <string>

namespace starlark {
namespace grammar {

const std::set<std::string> predeclared_symbols = {
    "None",     "True",      "False",    "abs",     "any",       "all",
    "bool",     "bytes",     "dict",     "dir",     "enumerate", "float",
    "fail",     "getattr",   "hasattr",  "hash",    "int",       "len",
    "list",     "max",       "min",      "print",   "range",     "repr",
    "reversed", "set",       "sorted",   "str",     "tuple",     "type",
    "zip",
};

int64_t log2_max_bigint() {
  // TODO(lmirelmann): Make this configurable.
  return 30;  // 2**30.
}

int64_t max_sequence_size() {
  // TODO(lmirelmann): Make this configurable.
  // TODO(lmirelmann): This belongs to runtime options.
  return std::numeric_limits<int32_t>::max();
}

int64_t max_string_length() {
  // TODO(lmirelmann): Make this configurable.
  // TODO(lmirelmann): This belongs to runtime options.
  return std::numeric_limits<int32_t>::max();
}

}  // namespace grammar
}  // namespace starlark
