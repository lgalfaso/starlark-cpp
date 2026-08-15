// Copyright 2026 Lucas Mirelmann

#include "errors/parser_error_messages.hpp"

#include <format>
#include <string>
#include <string_view>

#include "errors/source_highlight.hpp"
#include "proto/starlark_logging.pb.h"

using ::starlark::logging::Position;

namespace starlark {
namespace error_messages {

std::string error_v2_function_definition_not_allowed(std::string_view program, const Position& start, const Position& end) {
  return std::format("function definitions not allowed\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_if_not_allowed_at_top_level(std::string_view program, const Position& start, const Position& end) {
  return std::format("`if` statements are not allowed at the top level\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_for_not_allowed_at_top_level(std::string_view program, const Position& start, const Position& end) {
  return std::format("`for` statements are not allowed at the top level\n{}", get_line_and_underline(program, start, end));
}



std::string error_v2_load_first(std::string_view program, const Position& start, const Position& end) {
  return std::format("`load` statements must appear before other statements\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_load_not_at_top_level(std::string_view program, const Position& start, const Position& end) {
  return std::format("`load` statement not at top level\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_load_at_least_one_symbol(std::string_view program, const Position& start, const Position& end) {
  return std::format("expect to load at least one symbol\n{}", get_line_and_underline(program, start, previous_position(program, end), end, end, ""));
}



std::string error_v2_params_star_parameter_may_appear_only_once(std::string_view program, const Position& start, const Position& end) {
  return std::format("variadic parameter may appear only once\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_params_non_optional_after_optional(std::string_view program, const Position& start, const Position& end) {
  return std::format("parameter without a default follows parameter with a default\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_params_keyword_variadic_param_must_be_last(std::string_view program, const Position& start, const Position& end) {
  return std::format("parameters cannot follow the keyword-variadic parameter\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_params_duplicate_keyword_variadic_paramter(std::string_view program, const Position& start, const Position& end) {
  return std::format("duplicate keyword-variadic parameter\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_params_named_param_must_follow_bare_star(std::string_view program, const Position& start, const Position& end) {
  return std::format("named parameter must follow bare * parameter\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_params_expected_identifier_after_star_star_token(std::string_view program, const Position& start, const Position& end) {
  return std::format("expected identifier after `**` in a variadic-keyword parameter\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_params_duplicate_params(std::string_view name, std::string_view program, const Position& start, const Position& end) {
  return std::format("duplicate parameter '{}' in function definition\n{}", name, get_line_and_underline(program, start, end));
}


std::string error_v2_illegal_target_for_augmented_assignment(std::string_view program, const Position& start, const Position& end) {
  return std::format("target is an illegal expression for augmented assignment\n{}", get_line_and_underline(program, start, end));
}


std::string error_v2_comparison_operators_are_not_associative(std::string_view program, const Position& start, const Position& end) {
  return std::format("comparison operators are not associative. Use parens\n{}", get_line_and_underline(program, start, end));
}


std::string error_v2_arguments_duplicate_star_args(std::string_view program, const Position& start, const Position& end) {
  return std::format("multiple variadic arguments\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_arguments_duplicate_star_star_kvargs(std::string_view program, const Position& start, const Position& end) {
  return std::format("multiple keyword-variadic arguments\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_arguments_varadic_arguments_not_allowed(std::string_view program, const Position& start, const Position& end) {
  return std::format("varadic arguments are not allowed\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_arguments_star_star_argument_must_be_last(std::string_view program, const Position& start, const Position& end) {
  return std::format("keyword-variadic argument `**kwargs` must be the last argument\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_arguments_non_variadic_before_variadic(std::string_view program, const Position& start, const Position& end) {
  return std::format("non-variadic arguments must be before variadic arguments\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_arguments_expected_identifier_for_named_arguments(std::string_view program, const Position& start, const Position& end) {
  return std::format("expected identifier for named arguments\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_arguments_positional_before_named_arguments(std::string_view program, const Position& start, const Position& end) {
  return std::format("positional arguments must come before named arguments\n{}", get_line_and_underline(program, start, end));
}


std::string error_v2_duplicate_binding_previous_load(std::string_view symbol, std::string_view program, const Position& start, const Position& end) {
  return std::format("`def` statement redefines previously defined `load` symbol '{}'\n{}", symbol, get_line_and_underline(program, start, end));
}

std::string error_v2_cannot_load_private_symbol(std::string_view symbol, std::string_view program, const Position& start, const Position& end) {
  return std::format("cannot import private symbol '{}'\n{}", symbol, get_line_and_underline(program, start, end));
}

std::string error_v2_duplicate_load_binding(std::string_view symbol, std::string_view program, const Position& start, const Position& end) {
  return std::format("`load` statement defines '{}' more than once\n{}", symbol, get_line_and_underline(program, start, end));
}

std::string error_v2_duplicate_binding_by_load(std::string_view symbol, std::string_view program, const Position& start, const Position& end) {
  return std::format("`load` statement redefines previously defined value '{}'\n{}", symbol, get_line_and_underline(program, start, end));
}

std::string error_v2_duplicate_load_binding_by_load(std::string_view symbol, std::string_view program, const Position& start, const Position& end) {
  return std::format("multiple bindings for the top-level load symbol '{}'\n{}", symbol, get_line_and_underline(program, start, end));
}

std::string error_v2_duplicate_binding_from_load(std::string_view symbol, std::string_view program, const Position& start, const Position& end) {
  return std::format("variable '{}' redefines symbol previously defined by a load statement\n{}", symbol, get_line_and_underline(program, start, end));
}

std::string error_v2_duplicate_binding(std::string_view symbol, std::string_view program, const Position& start, const Position& end) {
  return std::format("multiple bindings for the top-level symbol '{}'\n{}", symbol, get_line_and_underline(program, start, end));
}

std::string error_v2_undefined_name(std::string_view name, std::string_view best_candidate, std::string_view program, const Position& start, const Position& end) {
  if (best_candidate.empty()) {
    return std::format("name '{}' is not defined\n{}", name, get_line_and_underline(program, start, end));
  } else {
    return std::format("name '{}' is not defined; did you mean '{}'?\n{}", name, best_candidate, get_line_and_underline(program, start, end, best_candidate));
  }
}


std::string error_v2_expected_identifier(std::string_view program, const Position& start, const Position& end) {
  return std::format("expecting an identifier\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_expected_string(std::string_view program, const Position& start, const Position& end) {
  return std::format("expecting a string literal\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_expected_lparentheses(std::string_view program, const Position& start, const Position& end) {
  return std::format("expecting open parentheses '('\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_expected_rparentheses(std::string_view program, const Position& start, const Position& end) {
  return std::format("expecting close parentheses ')'\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_expected_lbracket(std::string_view program, const Position& start, const Position& end) {
  return std::format("expecting open bracket '['\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_expected_rbracket(std::string_view program, const Position& start, const Position& end) {
  return std::format("expecting close bracket ']'\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_expected_lbrace(std::string_view program, const Position& start, const Position& end) {
  return std::format("expecting open brace '{{'\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_expected_rbrace(std::string_view program, const Position& start, const Position& end) {
  return std::format("expecting close brace '}}'\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_expected_colon(std::string_view program, const Position& start, const Position& end) {
  return std::format("expecting colon ':'\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_expected_in(std::string_view program, const Position& start, const Position& end) {
  return std::format("expecting IN(`in`) ':'\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_expected_indent(std::string_view program, const Position& start, const Position& end) {
  return std::format("expecting an indent\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_expected_outdent(std::string_view program, const Position& start, const Position& end) {
  return std::format("expecting an outdent\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_expected_newline(std::string_view program, const Position& start, const Position& end) {
  return std::format("expecting a NEWLINE(`\\n`)\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_expected_equals(std::string_view program, const Position& start, const Position& end) {
  return std::format("expecting a EQUALS(`=`)\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_expected_else(std::string_view program, const Position& start, const Position& end) {
  return std::format("expecting a `else`\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_expected_lambda(std::string_view program, const Position& start, const Position& end) {
  return std::format("expecting a `lambda`\n{}", get_line_and_underline(program, start, end));
}


std::string error_v2_unexpected_return(std::string_view program, const Position& start, const Position& end) {
  return std::format("unexpected `return`\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_unexpected_break(std::string_view program, const Position& start, const Position& end) {
  return std::format("unexpected `break`\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_unexpected_continue(std::string_view program, const Position& start, const Position& end) {
  return std::format("unexpected `continue`\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_unexpected_comma(std::string_view program, const Position& start, const Position& end) {
  return std::format("unexpected COMMA(`,`)\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_unexpected_token(std::string_view program, const Position& start, const Position& end) {
  return std::format("unexpected token\n{}", get_line_and_underline(program, start, end));
}

std::string error_v2_expected_target(std::string_view program, const Position& start, const Position& end) {
  return std::format("expression is not a valid target\n{}", get_line_and_underline(program, start, end));
}

}  // namespace error_messages
}  // namespace starlark


