// Copyright 2024-2025 Lucas Mirelmann

#include "grammar/options.hpp"

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

}  // namespace grammar
}  // namespace starlark
