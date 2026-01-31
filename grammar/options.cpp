// Copyright 2024-2025 Lucas Mirelmann

#include "grammar/options.hpp"

#include <set>
#include <string>

namespace starlark {
namespace grammar {

const std::set<std::string>& predeclared_symbols() {
  static const std::set<std::string>* result = new std::set<std::string>{
      "None",     "True",      "False",    "abs",      "all",      "any",
      "bool",     "bytes",     "chr",      "dict",     "dir",      "enumerate",
      "float",    "fail",      "getattr",  "hasattr",  "hash",     "int",
      "len",      "list",      "max",      "min",      "ord",      "print",
      "range",    "repr",      "reversed", "set",      "sorted",   "str",
      "tuple",     "type",     "zip",
  };

  return *result;
}

}  // namespace grammar
}  // namespace starlark
