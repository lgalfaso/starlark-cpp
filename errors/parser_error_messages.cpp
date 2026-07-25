// Copyright 2026 Lucas Mirelmann

#include "errors/parser_error_messages.hpp"

#include <format>
#include <string>
#include <string_view>

namespace starlark {
namespace error_messages {

std::string_view error_function_definition_not_allowed() {
  return "Function definitions not allowed";
}

std::string_view error_if_not_allowed_at_top_level() {
  return "`if` statements are not allowed at the top level";
}

std::string_view error_for_not_allowed_at_top_level() {
  return "`for` statements are not allowed at the top level";
}



std::string_view error_load_first() {
  return "`load` statements must appear before other statements";
}

std::string_view error_load_not_at_top_level() {
  return "`load` statement not at top level";
}

std::string_view error_load_at_least_one_symbol() {
  return "Expect to load at least one symbol";
}


std::string_view error_params_star_parameter_may_appear_only_once() {
  return "* parameter may appear only once";
}

std::string_view error_params_non_optional_after_optional() {
  return "SyntaxError: parameter without a default follows parameter with a default";
}

std::string_view error_params_variadic_keyword_argument_must_be_last() {
  return "Parameters cannot follow variadic-keyword parameter";
}

std::string_view error_params_named_argument_must_follow_bare_star() {
  return "named arguments must follow bare *";
}

std::string_view error_params_expected_identifier_after_star_star_token() {
  return "Expected identifier after STAR_STAR when parsing parameters";
}


std::string_view error_illegal_target_for_augmented_assignment() {
  return "target is an illegal expression for augmented assignment";
}

std::string_view error_comparison_operators_are_not_associative() {
  return "Comparison operators are not associative. Use parens.";
}


std::string_view error_arguments_duplicate_star_args() {
  return "Duplicate *args";
}

std::string_view error_arguments_duplicate_star_star_kvargs() {
  return "Duplicate **kwargs";
}

std::string_view error_arguments_varadic_arguments_not_allowed() {
  return "Varadic arguments are not allowed";
}

std::string_view error_arguments_star_star_argument_must_be_last() {
  return "**kwargs must be the last argument";
}

std::string_view error_arguments_non_variadic_before_variadic() {
  return "Non-variadic arguments must be before variadic arguments";
}

std::string_view error_arguments_expected_identifier_for_named_arguments() {
  return "Expected identifier for named arguments";
}

std::string_view error_arguments_positional_before_named_arguments() {
  return "Positional arguments must come before named arguments";
}



std::string_view error_expected_identifier() {
  return "Expecting IDENTIFIER";
}

std::string_view error_expected_string() {
  return "Expecting STRING";
}

std::string_view error_expected_target() {
  return "Expecting TARGET";
}

std::string error_expected_symbol(std::string_view symbol) {
  return std::format("Expecting {}", symbol);
}

std::string_view error_unexpected_return() {
  return "Unexpected RETURN";
}

std::string_view error_unexpected_break() {
  return "Unexpected BREAK";
}

std::string_view error_unexpected_continue() {
  return "Unexpected CONTINUE";
}

std::string_view error_unexpected_comma() {
  return "Unexpected COMMA";
}

std::string_view error_unexpected_token() {
  return "Unexpected token";
}


std::string error_duplicate_binding_previous_load(std::string_view symbol) {
  return std::format("`def` statement redefines previously defined `load` symbol '{}'", symbol);
}

std::string error_cannot_load_private_symbol(std::string_view symbol) {
  return std::format("Cannot import private symbol '{}'", symbol);
}

std::string error_duplicate_load_binding(std::string_view symbol) {
  return std::format("`load` statement defines '{}' more than once", symbol);
}

std::string error_duplicate_binding_by_load(std::string_view symbol) {
  return std::format("`load` statement redefines previously defined value '{}'", symbol);
}

std::string error_duplicate_load_binding_by_load(std::string_view symbol) {
  return std::format("Multiple bindings for the top-level load symbol '{}'", symbol);
}

std::string error_params_duplicate_params(std::string_view name) {
  return std::format("Duplicate argument '{}' in function definition", name);
}

std::string error_duplicate_binding_from_load(std::string_view symbol) {
  return std::format("Variable '{}' redefines symbol previously defined by a load statement", symbol);
}

std::string error_duplicate_binding(std::string_view symbol) {
  return std::format("Multiple bindings for the top-level symbol '{}'", symbol);
}

std::string error_undefined_name(std::string_view name) {
  return std::format("name '{}' is not defined", name);
}

}  // namespace error_messages
}  // namespace starlark


