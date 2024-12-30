// Copyright 2024 Lucas Mirelmann

#ifndef GRAMMAR_OPTIONS_HPP_
#define GRAMMAR_OPTIONS_HPP_

#pragma GCC visibility push(default)

namespace grammar {

struct grammar_options {
  // Whether when parsing a string, to allow octal or hex escape sequences
  // for characters in the 128-255 rangei (ASCII characters are always
  // allowed). Enabling this option has the undesirable side-effect that
  // octal and hex escaped sequences will be added literally to the string
  // and not as UTF-8 encoding of the character.
  // This is one of the options that would have been best not to 
  // have but there are libraries(1) that make use of these sequences even
  // when the spec states: 
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

  // Whether to allow `if`s and `for`s at the top level.
  bool allow_top_level_if_and_for = false;

  // Whether all `load` statements must be before other statements.
  bool require_load_statements_first = true;

  // Whether to allow variadic arguments `*args` and `**kwargs` is calls.
  bool allow_varadic_arguments = true;
};

}  // namespace grammar

#pragma GCC visibility pop

#endif  // GRAMMAR_OPTIONS_HPP_


