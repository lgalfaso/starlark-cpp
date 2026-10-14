// Copyright 2024-2025 Lucas Mirelmann

#ifndef GRAMMAR_OPTIONS_HPP_
#define GRAMMAR_OPTIONS_HPP_

#include <set>
#include <string>

#pragma GCC visibility push(default)

namespace starlark {
namespace grammar {

struct grammar_options {
  // Whether when parsing a text string, to allow octal or hex escape sequences
  // for code points in the 128-255 range (values 0-127 are always allowed).
  // Octal/hex values are always UTF-8-encoded in text strings; bytes literals
  // still treat escapes as raw bytes. This is one of the options that would
  // have been best not to have but there are libraries(1) that use these
  // sequences even when the spec states:
  //
  //     It is an error if the value of an octal or hexadecimal escape is
  //     greater than decimal 127.
  //
  // External references:
  // (1) https://github.com/bazel-contrib/bazel-lib/blob/main/lib/private/strings.bzl
  //
  bool escaped_octal_and_hex_char_are_ascii = true;

  // Whether to allow this module to load private symbols from other modules.
  bool allow_load_private_symbols = false;

  // Whether to allow function and lambdas to be defined.
  bool allow_function_definitions = true;

  // Whether all `load` statements must be before other statements.
  bool require_load_statements_first = true;

  // Whether to allow variadic arguments `*args` and `**kwargs` is calls.
  bool allow_variadic_arguments = true;

  // Whether to allow a top-level rebindings.
  bool allow_top_level_rebinding = false;

  // Whether to allow a top-level `for` statements.
  bool allow_top_level_for = false;

  // Whether to allow a top-level `if` statements.
  bool allow_top_level_if = false;

  // Whether to allow binary number literals.
  bool allow_binary_integer_literals = false;
};

const std::set<std::string>& predeclared_symbols();

}  // namespace grammar
}  // namespace starlark

#pragma GCC visibility pop

#endif  // GRAMMAR_OPTIONS_HPP_

