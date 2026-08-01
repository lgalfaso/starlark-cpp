// Copyright 2026 Lucas Mirelmann

#ifndef ERRORS_PARSER_ERROR_MESSAGES_HPP_
#define ERRORS_PARSER_ERROR_MESSAGES_HPP_

#include <string>
#include <string_view>

#include "proto/starlark_logging.pb.h"

#pragma GCC visibility push(default)

namespace starlark {
namespace error_messages {

std::string_view error_expected_identifier();
std::string_view error_expected_string();
std::string_view error_expected_target();
std::string error_expected_symbol(std::string_view symbol);
std::string_view error_unexpected_return();
std::string_view error_unexpected_break();
std::string_view error_unexpected_continue();
std::string_view error_unexpected_comma();
std::string_view error_unexpected_token();

std::string error_duplicate_binding_previous_load(std::string_view symbol);
std::string error_cannot_load_private_symbol(std::string_view symbol);
std::string error_duplicate_load_binding(std::string_view symbol);
std::string error_duplicate_binding_by_load(std::string_view symbol);
std::string error_duplicate_load_binding_by_load(std::string_view symbol);
std::string error_duplicate_binding_from_load(std::string_view symbol);
std::string error_duplicate_binding(std::string_view symbol);
std::string error_undefined_name(std::string_view name);

std::string error_v2_function_definition_not_allowed(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_if_not_allowed_at_top_level(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_for_not_allowed_at_top_level(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);

std::string error_v2_load_first(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_load_not_at_top_level(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_load_at_least_one_symbol(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);

std::string error_v2_params_star_parameter_may_appear_only_once(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_params_non_optional_after_optional(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_params_keyword_variadic_param_must_be_last(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_params_duplicate_keyword_variadic_paramter(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_params_named_param_must_follow_bare_star(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_params_expected_identifier_after_star_star_token(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_params_duplicate_params(std::string_view name, std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);

std::string error_v2_illegal_target_for_augmented_assignment(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);

std::string error_v2_comparison_operators_are_not_associative(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);

std::string error_v2_arguments_duplicate_star_args(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_arguments_duplicate_star_star_kvargs(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_arguments_varadic_arguments_not_allowed(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_arguments_star_star_argument_must_be_last(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_arguments_non_variadic_before_variadic(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_arguments_expected_identifier_for_named_arguments(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_arguments_positional_before_named_arguments(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);

}  // namespace error_messages
}  // namespace starlark

#pragma GCC visibility pop

#endif  // ERRORS_PARSER_ERROR_MESSAGES_HPP_

