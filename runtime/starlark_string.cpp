// Copyright 2025-2026 Lucas Mirelmann

#include "runtime/starlark_string.hpp"

#include <cassert>

#include <algorithm>
#include <functional>
#include <limits>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "bigint/number.hpp"
#include "runtime/error_messages.hpp"
#include "runtime/hex_encoder.hpp"
#include "runtime/options.hpp"
#include "runtime/siphash.hpp"
#include "runtime/starlark_list.hpp"
#include "runtime/starlark_numeric.hpp"
#include "runtime/starlark_tuple.hpp"
#include "runtime/starlark_types.hpp"
#include "unicode/encode.hpp"
#include "unicode/ucd_code_points.hpp"
#include "unicode/utf8_reader.hpp"
#include "unicode/utf8_reverse_reader.hpp"
#include "unicode/word_break.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;
using ::starlark::result::status_code;
using ::starlark::result::status_or;
using ::starlark::ucd::is_Case_Ignorable;
using ::starlark::ucd::is_Cased;
using ::starlark::ucd::is_Lowercase;
using ::starlark::ucd::is_Uppercase;
using ::starlark::ucd::is_alpha;
using ::starlark::ucd::is_digit;
using ::starlark::ucd::is_numeric;
using ::starlark::ucd::is_space;
using ::starlark::ucd::to_lower;
using ::starlark::ucd::to_title;
using ::starlark::ucd::to_upper;
using ::starlark::unicode::utf8_encode_code_point;
using ::starlark::unicode::utf8_reader;
using ::starlark::unicode::utf8_reverse_reader;
using ::starlark::unicode::word_break;

namespace starlark {
namespace runtime {

starlark_obj* starlark_string_fn_capitalize(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_count(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_elem_ords(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_elems(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_endswith(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_find(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_format(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_index(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_isalnum(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_isalpha(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_isdigit(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_islower(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_isspace(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_istitle(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_isupper(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_join(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_lower(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_lstrip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_partition(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_replace(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_removeprefix(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_removesuffix(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_rfind(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_rindex(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_rpartition(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_rsplit(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_rstrip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_split(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_splitlines(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_startswith(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_strip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_title(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_upper(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);

const std::map<std::string, starlark_obj::fn*, std::less<>>& starlark_string::method_refs() {
  static const std::map<std::string, starlark_obj::fn*, std::less<>>* result =
    new std::map<std::string, starlark_obj::fn*, std::less<>>{
      {"capitalize", starlark_string_fn_capitalize},
      {"count", starlark_string_fn_count},
      {"elem_ords", starlark_string_fn_elem_ords},
      {"elems", starlark_string_fn_elems},
      {"endswith", starlark_string_fn_endswith},
      {"find", starlark_string_fn_find},
      {"format", starlark_string_fn_format},
      {"index", starlark_string_fn_index},
      {"isalnum", starlark_string_fn_isalnum},
      {"isalpha", starlark_string_fn_isalpha},
      {"isdigit", starlark_string_fn_isdigit},
      {"islower", starlark_string_fn_islower},
      {"isspace", starlark_string_fn_isspace},
      {"istitle", starlark_string_fn_istitle},
      {"isupper", starlark_string_fn_isupper},
      {"join", starlark_string_fn_join},
      {"lower", starlark_string_fn_lower},
      {"lstrip", starlark_string_fn_lstrip},
      {"partition", starlark_string_fn_partition},
      {"replace", starlark_string_fn_replace},
      {"removeprefix", starlark_string_fn_removeprefix},
      {"removesuffix", starlark_string_fn_removesuffix},
      {"rfind", starlark_string_fn_rfind},
      {"rindex", starlark_string_fn_rindex},
      {"rpartition", starlark_string_fn_rpartition},
      {"rsplit", starlark_string_fn_rsplit},
      {"rstrip", starlark_string_fn_rstrip},
      {"split", starlark_string_fn_split},
      {"splitlines", starlark_string_fn_splitlines},
      {"startswith", starlark_string_fn_startswith},
      {"strip", starlark_string_fn_strip},
      {"title", starlark_string_fn_title},
      {"upper", starlark_string_fn_upper},
  };

  return *result;
}

const std::vector<std::string>& starlark_string::attributes() {
  static const std::vector<std::string>* result =
    new std::vector<std::string>(([]() {
    std::vector<std::string> result;
    result.reserve(method_refs().size());
    for (const auto& [k, v] : method_refs()) {
      result.push_back(k);
    }
    return result;
  })());

  return *result;
}

starlark_string::starlark_string(std::string&& value) : value(value) {}
starlark_string::starlark_string(std::string_view value) : value(value) {}

std::string_view starlark_string::type() const {
  return starlark_types::string_t;
}

bool starlark_string::primitive() const {
  return true;
}

const std::vector<std::string>& starlark_string::dir() const {
  return attributes();
}

const std::map<std::string, starlark_obj::fn*, std::less<>>& starlark_string::methods_meta() const {
  return method_refs();
}

std::string starlark_string::str() const {
  return value;
}

int64_t starlark_string::len(bool produce_error, error_fn& error_callback) const {
  return value.size();
}

bool starlark_string::inner_repr(printer& print, printer_action action) const {
  assert(action == printer_action::kPrintTop);
  // TODO(lmirelmann): If this function were to be executed a lot, then there are a
  // few things that can we can try:
  // - Check whether the original value can be used just adding quotes
  // - Keep the value of `result` in a mutable field
  std::string result = "\"";
  utf8_reader reader(value, false, false);
  while (reader.pending()) {
    // TODO(lmirelmann): Would be nice to avoid calling the read twice just to be able to
    // handle the error case.
    auto code_point = reader.peek_code_point();
    if (code_point != utf8_reader::kReplacementCharacter) {
      write_printable(reader.peek_code_point(), /*allow_non_ascii_printable=*/ true, result);
      reader.read_code_point();
    } else {
      result += value[reader.pos()];
      reader.skip();
    }
  }
  result += "\"";
  print.append(result);
  return false;
}

bool starlark_string::truthy() const {
  return !value.empty();
}

bool starlark_string::binary_in(const starlark_obj& other, error_fn& error_callback) const {
  if (other.type() != type()) {
    error_callback.add_error(error_in_element(type(), other.type(), type()));
    return false;
  }

  const starlark_string& s_other = static_cast<const starlark_string&>(other);
  return value.contains(s_other.value);
}

namespace {

starlark_obj* plus_op(const starlark_string& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  if (other.type() != this_obj.type()) {
    error_callback.add_error(error_no_concat(this_obj.type(), other.type()));
    return nullptr;
  }
  // TODO(lmirelmann): Check that the value length would not go over the limit.
  std::string result{this_obj.as_string()};
  result += other.as_string();
  return Arena::Create<starlark_string>(&ctx.arena(), std::move(result));
}

starlark_obj* star_op(const starlark_string& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64: {
      if (this_obj.as_string().empty()) {
        return Arena::Create<starlark_string>(&ctx.arena(), std::string_view{});
      }
      // TODO(lmirelmann): Check whether the size will be over the maximum allowed.
      auto multiplier = other.as_int64();
      std::string result;
      for (int64_t i = 0; i < multiplier; ++i) {
        result += this_obj.as_string();
      }
      return Arena::Create<starlark_string>(&ctx.arena(), std::move(result));
    }
    case starlark_numeric_type::kBigInt: {
      if (this_obj.as_string().empty()) {
        return Arena::Create<starlark_string>(&ctx.arena(), std::string_view{});
      }
      const auto& multiplier = other.as_bigint();
      if (multiplier <= number::zero()) {
        return Arena::Create<starlark_string>(&ctx.arena(), std::string_view{});
      }
      if (multiplier.bit_size() >= 63) {
        error_callback.add_error(error_max_sequence_length(max_string_length()));
        return nullptr;
      }
      int64_t int_value = multiplier.at(0);
      // TODO(lmirelmann): Check whether the size will be over the maximum allowed.
      std::string result;
      for (int64_t i = 0; i < int_value; ++i) {
        result += this_obj.as_string();
      }
      return Arena::Create<starlark_string>(&ctx.arena(), std::move(result));
    }
    default:
      error_callback.add_error(error_no_multiply_sequence(other.type()));
      return nullptr;
  }
}

bool parse_interpolation(std::string_view format, std::vector<std::string>& parts, std::vector<std::pair<char, std::size_t>>& convertions, error_fn& error_callback) {
  bool last_is_percent = false;
  parts.emplace_back();
  std::size_t pos = 0;
  for (const auto& c : format) {
    if (last_is_percent) {
      if (c == '%') {
        parts.back() += c;
      } else {
        convertions.push_back(std::make_pair(c, pos));
        parts.emplace_back();
      }
      last_is_percent = false;
    } else if (c == '%') {
      last_is_percent = true;
    } else {
      parts.back() += c;
    }
    ++pos;
  }
  if (last_is_percent) {
    error_callback.add_error(error_incomplete_format());
  }
  return !last_is_percent;
}

bool interpolation_convertion(std::string& result, const starlark_obj& element, char format, std::size_t index, context& ctx, error_fn& error_callback) {
  // TODO(lmirelmann): Resolve inline the cases that need a float to int conversion without the creation of a new object.
  // TODO(lmirelmann): There is a lot of duplication, there are many things that can be simplified.
  switch (format) {
    case 's':
      result += element.str();
      break;
    case 'r':
      result += element.repr();
      break;
    case 'd':
      switch (element.numeric_type()) {
        case starlark_numeric_type::kInt64:
          result += std::format("{:d}", element.as_int64());
          break;
        case starlark_numeric_type::kBigInt:
          result += element.as_bigint().to_string(10, false);
          break;
        case starlark_numeric_type::kFloat: {
          auto* as_integer = create_integer_from_float(element.as_float(), ctx, error_callback);
          if (as_integer == nullptr) {
            return false;
          }
          return interpolation_convertion(result, *create_integer_from_float(element.as_float(), ctx, error_callback), format, index, ctx, error_callback);
        }
        default:
          error_callback.add_error(error_format_integer_is_required(format, element.type()));
          return false;
      }
      break;
    case 'o':
      switch (element.numeric_type()) {
        case starlark_numeric_type::kInt64:
          result += std::format("{:o}", element.as_int64());
          break;
        case starlark_numeric_type::kBigInt:
          result += element.as_bigint().to_string(8, false);
          break;
        case starlark_numeric_type::kFloat: {
          auto* as_integer = create_integer_from_float(element.as_float(), ctx, error_callback);
          if (as_integer == nullptr) {
            return false;
          }
          return interpolation_convertion(result, *create_integer_from_float(element.as_float(), ctx, error_callback), format, index, ctx, error_callback);
        }
        default:
          error_callback.add_error(error_format_integer_is_required(format, element.type()));
          return false;
      }
      break;
    case 'x':
      switch (element.numeric_type()) {
        case starlark_numeric_type::kInt64:
          result += std::format("{:x}", element.as_int64());
          break;
        case starlark_numeric_type::kBigInt:
          result += element.as_bigint().to_string(16, false);
          break;
        case starlark_numeric_type::kFloat: {
          auto* as_integer = create_integer_from_float(element.as_float(), ctx, error_callback);
          if (as_integer == nullptr) {
            return false;
          }
          return interpolation_convertion(result, *create_integer_from_float(element.as_float(), ctx, error_callback), format, index, ctx, error_callback);
        }
        default:
          error_callback.add_error(error_format_integer_is_required(format, element.type()));
          return false;
      }
      break;
    case 'X':
      switch (element.numeric_type()) {
        case starlark_numeric_type::kInt64:
          result += std::format("{:X}", element.as_int64());
          break;
        case starlark_numeric_type::kBigInt:
          result += element.as_bigint().to_string(16, true);
          break;
        case starlark_numeric_type::kFloat: {
          auto* as_integer = create_integer_from_float(element.as_float(), ctx, error_callback);
          if (as_integer == nullptr) {
            return false;
          }
          return interpolation_convertion(result, *create_integer_from_float(element.as_float(), ctx, error_callback), format, index, ctx, error_callback);
        }
        default:
          error_callback.add_error(error_format_integer_is_required(format, element.type()));
          return false;
      }
      break;
    case 'e':
      switch (element.numeric_type()) {
        case starlark_numeric_type::kInt64:
          result += std::format("{:e}", static_cast<double>(element.as_int64()));
          break;
        case starlark_numeric_type::kBigInt:
          result += std::format("{:e}", to_double(element.as_bigint()));
          break;
        case starlark_numeric_type::kFloat:
          if (std::isfinite(element.as_float())) {
            result += std::format("{:e}", element.as_float());
          } else {
            result += element.str();
          }
          break;
        default:
          error_callback.add_error(error_format_real_is_required(format, element.type()));
          return false;
      }
      break;
    case 'E':
      switch (element.numeric_type()) {
        case starlark_numeric_type::kInt64:
          result += std::format("{:E}", static_cast<double>(element.as_int64()));
          break;
        case starlark_numeric_type::kBigInt: {
          auto value = to_double(element.as_bigint());
          if (std::isfinite(value)) {
            result += std::format("{:E}", value);
          } else {
            result += std::format("{:e}", value);
          }
          break;
        }
        case starlark_numeric_type::kFloat:
          if (std::isfinite(element.as_float())) {
            result += std::format("{:E}", element.as_float());
          } else {
            result += element.str();
          }
          break;
        default:
          error_callback.add_error(error_format_real_is_required(format, element.type()));
          return false;
      }
      break;
    case 'f':
      switch (element.numeric_type()) {
        case starlark_numeric_type::kInt64:
          result += std::format("{:f}", static_cast<double>(element.as_int64()));
          break;
        case starlark_numeric_type::kBigInt:
          result += std::format("{:f}", to_double(element.as_bigint()));
          break;
        case starlark_numeric_type::kFloat:
          if (std::isfinite(element.as_float())) {
            result += std::format("{:f}", element.as_float());
          } else {
            result += element.str();
          }
          break;
        default:
          error_callback.add_error(error_format_real_is_required(format, element.type()));
          return false;
      }
      break;
    case 'F':
      switch (element.numeric_type()) {
        case starlark_numeric_type::kInt64:
          result += std::format("{:F}", static_cast<double>(element.as_int64()));
          break;
        case starlark_numeric_type::kBigInt: {
          auto value = to_double(element.as_bigint());
          if (std::isfinite(value)) {
            result += std::format("{:F}", value);
          } else {
            result += std::format("{:f}", value);
          }
          break;
        }
        case starlark_numeric_type::kFloat:
          if (std::isfinite(element.as_float())) {
            result += std::format("{:F}", element.as_float());
          } else {
            result += element.str();
          }
          break;
        default:
          error_callback.add_error(error_format_real_is_required(format, element.type()));
          return false;
      }
      break;
    case 'g':
      switch (element.numeric_type()) {
        case starlark_numeric_type::kInt64:
          result += float_to_string(static_cast<double>(element.as_int64()), false);
          break;
        case starlark_numeric_type::kBigInt:
          result += float_to_string(to_double(element.as_bigint()), false);
          break;
        case starlark_numeric_type::kFloat:
          result += float_to_string(element.as_float(), false);
          break;
        default:
          error_callback.add_error(error_format_real_is_required(format, element.type()));
          return false;
      }
      break;
    case 'G':
      switch (element.numeric_type()) {
        case starlark_numeric_type::kInt64:
          result += float_to_string(static_cast<double>(element.as_int64()), true);
          break;
        case starlark_numeric_type::kBigInt: {
          result += float_to_string(to_double(element.as_bigint()), true);
          break;
        }
        case starlark_numeric_type::kFloat:
          result += float_to_string(element.as_float(), true);
          break;
        default:
          error_callback.add_error(error_format_real_is_required(format, element.type()));
          return false;
      }
      break;
    default:
      error_callback.add_error(error_unsupported_format_character(format, index));
      return false;
  }
  return true;
}

starlark_obj* percent_op(const starlark_string& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  std::vector<std::string> parts;
  std::vector<std::pair<char, std::size_t>> convertions;
  if (!parse_interpolation(this_obj.as_string(), parts, convertions, error_callback)) {
    return nullptr;
  }
  std::string result = parts[0];
  if (other.type() != starlark_types::tuple_t) {
    if (parts.size() != 2) {
      error_callback.add_error(error_not_enough_arguments_for_format_string());
      return nullptr;
    }
    if (!interpolation_convertion(result, other, convertions[0].first, convertions[0].second, ctx, error_callback)) {
      return nullptr;
    }
    result += parts[1];
    return Arena::Create<starlark_string>(&ctx.arena(), std::move(result));
  }

  const starlark_tuple& t_other = static_cast<const starlark_tuple&>(other);
  if (t_other.size() != convertions.size()) {
    if (t_other.size() < convertions.size()) {
      error_callback.add_error(error_not_enough_arguments_for_format_string());
    } else {
      error_callback.add_error(error_not_all_arguments_converted_during_string_formatting());
    }
    return nullptr;
  }

  for (std::size_t i = 0; i < convertions.size(); ++i) {
    if (!interpolation_convertion(result, *t_other.at(i), convertions[i].first, convertions[i].second, ctx, error_callback)) {
      return nullptr;
    }
    result += parts[i + 1];
  }
  return Arena::Create<starlark_string>(&ctx.arena(), std::move(result));
}

enum class case_condition {
  kNone,
  kFinalSigma,
};

struct case_convertion {
  std::string prefix;
  std::string if_condition;
  std::string else_condition;
  case_condition condition;
  std::size_t pos;
};

bool test_final_sigma_before(const std::vector<std::uint32_t>& code_points, std::size_t start) {
  for (auto pos = start; pos > 0; --pos) {
    if (is_Cased(code_points[pos - 1])) {
      return true;
    }
    if (!is_Case_Ignorable(code_points[pos - 1])) {
      return false;
    }
  }
  return false;
}

bool test_final_sigma_after(const std::vector<std::uint32_t>& code_points, std::size_t start) {
  for (auto pos = start + 1; pos < code_points.size(); ++pos) {
    if (is_Cased(code_points[pos])) {
      return false;
    }
    if (is_Case_Ignorable(code_points[pos])) {
      continue;
    }
  }
  return true;
}

std::string merge_parts(const std::vector<case_convertion>& parts, const std::vector<std::uint32_t>& code_points) {
  std::string result;
  for (const auto& part : parts) {
    result += part.prefix;
    if (part.condition == case_condition::kFinalSigma) {
      if (test_final_sigma_before(code_points, part.pos) &&
          test_final_sigma_after(code_points, part.pos)) {
        result += part.if_condition;
      } else {
        result += part.else_condition;
      }
    }
  }
  return result;
}

std::string to_title_string(std::string_view value, bool& found_cased) {
  std::vector<std::uint32_t> code_points;
  utf8_reader reader(value, false, false);
  while (reader.pending()) {
    auto code_point = reader.read_code_point();
    code_points.push_back(code_point);
  }
  std::vector<std::uint64_t> word_boundaries;
  word_break(code_points, word_boundaries);

  found_cased = false;
  bool first_letter_of_word = true;
  std::size_t word_boundary = 0;
  std::vector<case_convertion> parts;
  parts.emplace_back();
  parts.back().condition = case_condition::kNone;
  for (std::size_t pos = 0; pos < code_points.size(); ++pos) {
    if (word_boundaries[word_boundary] == pos) {
      first_letter_of_word = true;
      word_boundary++;
    }
    auto code_point = code_points[pos];
    auto cased = is_Cased(code_point);
    found_cased |= cased;
    if (first_letter_of_word && cased) {
      found_cased = true;
      auto new_code_points = to_title(code_point);
      for (const auto& c : new_code_points) {
        utf8_encode_code_point(c == 0x110000 ? code_point : c, parts.back().prefix, true, false);
      }
      first_letter_of_word = false;
    } else {
      auto new_code_points = to_lower(code_point);
      if (new_code_points.second.has_value()) {
        for (const auto& c : new_code_points.first) {
          utf8_encode_code_point(c == 0x110000 ? code_point : c, parts.back().else_condition, true, false);
        }
        for (const auto& c : new_code_points.second.value()) {
          utf8_encode_code_point(c, parts.back().if_condition, true, false);
        }
        parts.back().condition = case_condition::kFinalSigma;
        parts.back().pos = pos;
        parts.emplace_back();
        parts.back().condition = case_condition::kNone;
      } else {
        for (const auto& c : new_code_points.first) {
          utf8_encode_code_point(c == 0x110000 ? code_point : c, parts.back().prefix, true, false);
        }
      }
    }
  }
  return merge_parts(parts, code_points);
}

}  // namespace

starlark_obj* starlark_string::binary_plus(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return plus_op(*this, other, "+", ctx, error_callback);
}

starlark_obj* starlark_string::plus_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return plus_op(*this, other, "+=", ctx, error_callback);
}

starlark_obj* starlark_string::binary_star(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return star_op(*this, other, "*", ctx, error_callback);
}

starlark_obj* starlark_string::star_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return star_op(*this, other, "*=", ctx, error_callback);
}

starlark_obj* starlark_string::binary_percent(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return percent_op(*this, other, "%", ctx, error_callback);
}

starlark_obj* starlark_string::percent_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return percent_op(*this, other, "%=", ctx, error_callback);
}

starlark_obj* starlark_string::index(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  auto idx = inner_index(other, value.size(), error_callback);
  if (!idx.ok()) {
    return nullptr;
  }
  return Arena::Create<starlark_string>(&ctx.arena(), value.substr(*idx, 1));
}

starlark_obj* starlark_string::slice_range(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, context& ctx, error_fn& error_callback) const {
  auto slice_result = inner_slice_range(start, stop, stride, value.size(), error_callback);
  if (!slice_result.ok()) {
    return nullptr;
  }
  auto i_start = std::get<0>(*slice_result);
  auto i_end = std::get<1>(*slice_result);
  auto i_stride = std::get<2>(*slice_result);

  decltype(value) result;
  if (i_stride > 0) {
    for (auto i = i_start; i < i_end; i += i_stride) {
      result += value[i];
    }
  } else {
    for (auto i = i_start; i > i_end; i += i_stride) {
      result += value[i];
    }
  }
  return Arena::Create<starlark_string>(&ctx.arena(), result);
}

std::string_view starlark_string::as_string() const {
  return value;
}

int64_t starlark_string::count(std::string_view sub, int64_t start, int64_t end) const {
  if (start < 0) {
    start = std::max<int64_t>(start + value.size(), 0);
  } else {
    start = std::min<int64_t>(start, value.size());
  }
  if (end < 0) {
    end = std::max<int64_t>(end + value.size(), 0);
  } else {
    end = std::min<int64_t>(end, value.size());
  }
  if (start > end) {
    start = end + 1;
  }
  if (sub.empty()) {
    return end - start + 1;
  }
  std::string_view view = value;
  std::string_view reduced_view = view.substr(0, end);
  int64_t count = 0;
  for (auto pos = reduced_view.find(sub, start); start <= end && pos != std::string_view::npos; pos = reduced_view.find(sub, start)) {
    count++;
    start = pos + sub.length();
  }
  return count;
}

bool starlark_string::endswith(std::vector<std::string_view> ends, int64_t start, int64_t end) const {
  if (start < 0) {
    start = std::max<int64_t>(start + value.size(), 0);
  }
  if (end < 0) {
    end = std::max<int64_t>(end + value.size(), 0);
  } else {
    end = std::min<int64_t>(end, value.size());
  }
  if (start > end) {
    return false;
  }
  std::string_view reduced_view = ((std::string_view)value).substr(start, end - start);
  for (const auto& end : ends) {
    if (reduced_view.ends_with(end)) {
      return true;
    }
  }
  return false;
}

bool starlark_string::startswith(const std::vector<std::string_view>& begins, int64_t start, int64_t end) const {
  if (start < 0) {
    start = std::max<int64_t>(start + value.size(), 0);
  }
  if (end < 0) {
    end = std::max<int64_t>(end + value.size(), 0);
  } else {
    end = std::min<int64_t>(end, value.size());
  }
  if (start > end) {
    return false;
  }
  std::string_view reduced_view = ((std::string_view)value).substr(start, end - start);
  for (const auto& begin : begins) {
    if (reduced_view.starts_with(begin)) {
      return true;
    }
  }
  return false;
}

int64_t starlark_string::find(std::string_view sub, int64_t start, int64_t end) const {
  if (start < 0) {
    start = std::max<int64_t>(start + value.size(), 0);
  }
  if (end < 0) {
    end = std::max<int64_t>(end + value.size(), 0);
  } else {
    end = std::min<int64_t>(end, value.size());
  }
  if (start > end) {
    return -1;
  }

  std::string_view reduced_view = ((std::string_view)value).substr(start, end - start);
  auto result = reduced_view.find(sub);
  if (result == std::string_view::npos) {
    return -1;
  }
  return result + start;
}

int64_t starlark_string::rfind(std::string_view sub, int64_t start, int64_t end) const {
  if (start < 0) {
    start = std::max<int64_t>(start + value.size(), 0);
  }
  if (end < 0) {
    end = std::max<int64_t>(end + value.size(), 0);
  } else {
    end = std::min<int64_t>(end, value.size());
  }
  if (start > end) {
    return -1;
  }

  std::string_view reduced_view = ((std::string_view)value).substr(start, end - start);
  auto result = reduced_view.rfind(sub);
  if (result == std::string_view::npos) {
    return -1;
  }
  return result + start;
}

starlark_obj* starlark_string::join(const std::vector<std::string_view>& elements, context& ctx) const {
  bool first = true;
  std::string result;
  for (auto element : elements) {
    if (!first) {
      result += value;
    } else {
      first = false;
    }
    result += element;
  }
  return Arena::Create<starlark_string>(&ctx.arena(), std::move(result));
}

starlark_obj* starlark_string::partition(std::string_view sub, context& ctx, error_fn& error_callback) {
  if (sub.empty()) {
    error_callback.add_error(error_empty_separator());
    return nullptr;
  }
  auto pos = value.find(sub);
  auto* result = Arena::Create<starlark_tuple>(&ctx.arena(), 3);
  if (pos == std::string::npos) {
    result->add(this);
    result->add(ctx.empty_string());
    result->add(ctx.empty_string());
  } else {
    result->add(Arena::Create<starlark_string>(&ctx.arena(), value.substr(0, pos)));
    result->add(Arena::Create<starlark_string>(&ctx.arena(), sub));
    result->add(Arena::Create<starlark_string>(&ctx.arena(), value.substr(pos + sub.length())));
  }
  return result;
}

starlark_obj* starlark_string::rpartition(std::string_view sub, context& ctx, error_fn& error_callback) {
  if (sub.empty()) {
    error_callback.add_error(error_empty_separator());
    return nullptr;
  }
  auto pos = value.rfind(sub);
  auto* result = Arena::Create<starlark_tuple>(&ctx.arena(), 3);
  if (pos == std::string::npos) {
    result->add(ctx.empty_string());
    result->add(ctx.empty_string());
    result->add(this);
  } else {
    result->add(Arena::Create<starlark_string>(&ctx.arena(), value.substr(0, pos)));
    result->add(Arena::Create<starlark_string>(&ctx.arena(), sub));
    result->add(Arena::Create<starlark_string>(&ctx.arena(), value.substr(pos + sub.length())));
  }
  return result;
}

starlark_obj* starlark_string::replace(std::string_view old, std::string_view new_, int64_t count, context& ctx) const {
  if (count < 0) {
    count = std::numeric_limits<int64_t>::max();
  }
  std::string result;
  std::string_view remaining_view = value;
  if (old.empty()) {
    while (count > 0) {
      result += new_;
      if (remaining_view.empty()) {
        break;
      }
      result += remaining_view[0];
      remaining_view = remaining_view.substr(1);
      count--;
    }
  } else {
    while (count > 0 && !remaining_view.empty()) {
      auto pos = remaining_view.find(old);
      if (pos == std::string_view::npos) {
        break;
      }
      result += remaining_view.substr(0, pos);
      result += new_;
      remaining_view = remaining_view.substr(pos + old.length());
      count--;
    }
  }
  result += remaining_view;
  return Arena::Create<starlark_string>(&ctx.arena(), std::move(result));
}

starlark_obj* starlark_string::rsplit(int64_t maxsplit, context& ctx) const {
  auto* result = Arena::Create<starlark_list>(&ctx.arena(), 0);
  if (maxsplit < 0) {
    maxsplit = std::numeric_limits<int64_t>::max();
  }
  utf8_reverse_reader reader(value, false);
  while (reader.pending() && is_space(reader.peek_code_point())) {
    reader.read_code_point();
  }
  while (reader.pending() && maxsplit > 0) {
    auto end = reader.pending();
    while (reader.pending() && !is_space(reader.peek_code_point())) {
      reader.read_code_point();
    }
    result->unsafe_append(Arena::Create<starlark_string>(&ctx.arena(), value.substr(reader.pending(), end - reader.pending())));
    while (reader.pending() && is_space(reader.peek_code_point())) {
      reader.read_code_point();
    }
    maxsplit--;
  }
  if (reader.pending()) {
    result->unsafe_append(Arena::Create<starlark_string>(&ctx.arena(), value.substr(0, reader.pending())));
  }
  result->unsafe_reverse();
  return result;
}

starlark_obj* starlark_string::rsplit(std::string_view sep, int64_t maxsplit, context& ctx, error_fn& error_callback) const {
  if (sep.empty()) {
    error_callback.add_error(error_empty_separator());
    return nullptr;
  }
  auto* result = Arena::Create<starlark_list>(&ctx.arena(), 0);
  std::string_view remaining_view = value;
  if (maxsplit < 0) {
    maxsplit = std::numeric_limits<int64_t>::max();
  }
  while (maxsplit > 0) {
    auto pos = remaining_view.rfind(sep);
    if (pos == std::string_view::npos) {
      break;
    }
    result->unsafe_append(Arena::Create<starlark_string>(&ctx.arena(), remaining_view.substr(pos + sep.length())));
    remaining_view = remaining_view.substr(0, pos);
    maxsplit--;
  }
  result->unsafe_append(Arena::Create<starlark_string>(&ctx.arena(), remaining_view));
  result->unsafe_reverse();
  return result;
}

starlark_obj* starlark_string::split(int64_t maxsplit, context& ctx) const {
  auto* result = Arena::Create<starlark_list>(&ctx.arena(), 0);
  if (maxsplit < 0) {
    maxsplit = std::numeric_limits<int64_t>::max();
  }
  utf8_reader reader(value, false, false);
  while (reader.pending() && is_space(reader.peek_code_point())) {
    reader.read_code_point();
  }
  while (reader.pending() && maxsplit > 0) {
    auto start = reader.pos();
    while (reader.pending() && !is_space(reader.peek_code_point())) {
      reader.read_code_point();
    }
    result->unsafe_append(Arena::Create<starlark_string>(&ctx.arena(), value.substr(start, reader.pos() - start)));
    while (reader.pending() && is_space(reader.peek_code_point())) {
      reader.read_code_point();
    }
    maxsplit--;
  }
  if (reader.pending()) {
    result->unsafe_append(Arena::Create<starlark_string>(&ctx.arena(), value.substr(reader.pos())));
  }
  return result;
}

starlark_obj* starlark_string::split(std::string_view sep, int64_t maxsplit, context& ctx, error_fn& error_callback) const {
  if (sep.empty()) {
    error_callback.add_error(error_empty_separator());
    return nullptr;
  }
  auto* result = Arena::Create<starlark_list>(&ctx.arena(), 0);
  std::string_view remaining_view = value;
  if (maxsplit < 0) {
    maxsplit = std::numeric_limits<int64_t>::max();
  }
  while (maxsplit > 0) {
    auto pos = remaining_view.find(sep);
    if (pos == std::string_view::npos) {
      break;
    }
    result->unsafe_append(Arena::Create<starlark_string>(&ctx.arena(), remaining_view.substr(0, pos)));
    remaining_view = remaining_view.substr(pos + sep.length());
    maxsplit--;
  }
  result->unsafe_append(Arena::Create<starlark_string>(&ctx.arena(), remaining_view));
  return result;
}

bool starlark_string::isalnum() const {
  if (value.empty()) {
    return false;
  }
  utf8_reader reader(value, false, false);
  while (reader.pending()) {
    auto code_point = reader.read_code_point();
    if (!is_alpha(code_point) && !is_numeric(code_point)) {
      return false;
    }
  }
  return true;
}

bool starlark_string::isalpha() const {
  if (value.empty()) {
    return false;
  }
  utf8_reader reader(value, false, false);
  while (reader.pending()) {
    auto code_point = reader.read_code_point();
    if (!is_alpha(code_point)) {
      return false;
    }
  }
  return true;
}

bool starlark_string::isdigit() const {
  if (value.empty()) {
    return false;
  }
  utf8_reader reader(value, false, false);
  while (reader.pending()) {
    auto code_point = reader.read_code_point();
    if (!is_digit(code_point)) {
      return false;
    }
  }
  return true;
}

bool starlark_string::isspace() const {
  if (value.empty()) {
    return false;
  }
  utf8_reader reader(value, false, false);
  while (reader.pending()) {
    auto code_point = reader.read_code_point();
    if (!is_space(code_point)) {
      return false;
    }
  }
  return true;
}

bool starlark_string::islower() const {
  bool cased_found = false;
  utf8_reader reader(value, false, false);
  while (reader.pending()) {
    auto code_point = reader.read_code_point();
    if (!is_Cased(code_point)) {
      continue;
    }
    cased_found = true;
    if (!is_Lowercase(code_point)) {
      return false;
    }
  }
  return cased_found;
}

bool starlark_string::istitle() const {
  bool found_cased;
  auto titled_value = to_title_string(value, found_cased);
  // The underlying algorithm is too complex to have a special case.
  return found_cased && value == titled_value;
}

bool starlark_string::isupper() const {
  bool cased_found = false;
  utf8_reader reader(value, false, false);
  while (reader.pending()) {
    auto code_point = reader.read_code_point();
    if (!is_Cased(code_point)) {
      continue;
    }
    cased_found = true;
    if (!is_Uppercase(code_point)) {
      return false;
    }
  }
  return cased_found;
}

starlark_obj* starlark_string::lower(context& ctx) {
  std::vector<case_convertion> parts;
  std::vector<std::uint32_t> code_points;
  utf8_reader reader(value, false, false);
  std::size_t pos = 0;
  parts.emplace_back();
  parts.back().condition = case_condition::kNone;
  while (reader.pending()) {
    auto code_point = reader.read_code_point();
    code_points.push_back(code_point);
    auto new_code_points = to_lower(code_point);
    if (new_code_points.second.has_value()) {
      for (const auto& c : new_code_points.first) {
        utf8_encode_code_point(c == 0x110000 ? code_point : c, parts.back().else_condition, true, false);
      }
      for (const auto& c : new_code_points.second.value()) {
        utf8_encode_code_point(c, parts.back().if_condition, true, false);
      }
      parts.back().condition = case_condition::kFinalSigma;
      parts.back().pos = pos;
      parts.emplace_back();
      parts.back().condition = case_condition::kNone;
    } else {
      for (const auto& c : new_code_points.first) {
        utf8_encode_code_point(c == 0x110000 ? code_point : c, parts.back().prefix, true, false);
      }
    }
    pos++;
  }
  return Arena::Create<starlark_string>(&ctx.arena(), merge_parts(parts, code_points));
}

starlark_obj* starlark_string::title(context& ctx) {
  bool ignore;
  return Arena::Create<starlark_string>(&ctx.arena(), to_title_string(value, ignore));
}

starlark_obj* starlark_string::upper(context& ctx) {
  std::string result;
  utf8_reader reader(value, false, false);
  while (reader.pending()) {
    auto code_point = reader.read_code_point();
    auto new_code_points = to_upper(code_point);
    for (const auto& c : new_code_points) {
      utf8_encode_code_point(c == 0x110000 ? code_point : c, result, true, false);
    }
  }
  return Arena::Create<starlark_string>(&ctx.arena(), std::move(result));
}

starlark_obj* starlark_string::capitalize(context& ctx) {
  std::vector<case_convertion> parts;
  std::vector<std::uint32_t> code_points;
  utf8_reader reader(value, false, false);
  std::size_t pos = 0;
  parts.emplace_back();
  parts.back().condition = case_condition::kNone;
  bool is_first = true;
  while (reader.pending()) {
    auto code_point = reader.read_code_point();
    code_points.push_back(code_point);
    if (is_first) {
      auto new_code_points = to_title(code_point);
      for (const auto& c : new_code_points) {
        utf8_encode_code_point(c == 0x110000 ? code_point : c, parts.back().prefix, true, false);
      }
      is_first = false;
    } else {
      auto new_code_points = to_lower(code_point);
      if (new_code_points.second.has_value()) {
        for (const auto& c : new_code_points.first) {
          utf8_encode_code_point(c == 0x110000 ? code_point : c, parts.back().else_condition, true, false);
        }
        for (const auto& c : new_code_points.second.value()) {
          utf8_encode_code_point(c, parts.back().if_condition, true, false);
        }
        parts.back().condition = case_condition::kFinalSigma;
        parts.back().pos = pos;
        parts.emplace_back();
        parts.back().condition = case_condition::kNone;
      } else {
        for (const auto& c : new_code_points.first) {
          utf8_encode_code_point(c == 0x110000 ? code_point : c, parts.back().prefix, true, false);
        }
      }
    }
    pos++;
  }
  return Arena::Create<starlark_string>(&ctx.arena(), merge_parts(parts, code_points));
}

starlark_obj* starlark_string::removeprefix(std::string_view sub, context& ctx) {
  if (value.starts_with(sub)) {
    return Arena::Create<starlark_string>(&ctx.arena(), value.substr(sub.length()));
  }
  return this;
}

starlark_obj* starlark_string::removesuffix(std::string_view sub, context& ctx) {
  if (value.ends_with(sub)) {
    return Arena::Create<starlark_string>(&ctx.arena(), value.substr(0, value.length() - sub.length()));
  }
  return this;
}

bool starlark_string::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
  return type() == other->type() &&
         value == other->as_string();
}

void starlark_string::inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, error_fn& error_callback) const {
  if (other->type() != type()) {
    starlark_obj::inner_cmp(comp, other, op, error_callback);
    return;
  }
  auto result = value <=> static_cast<const starlark_string*>(other)->value;
  if (result != 0) {
    comp.add_task(result > 0 ? order_comparator::pending_task_type::kGreaterThan : order_comparator::pending_task_type::kLessThan);
  }
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_string::inner_hash() const {
  if (value.length() == 0) {
    return 0;
  }
  return static_cast<int64_t>(siphash(value.data(), value.length(), 0x243F6A8885A308D3, 0x13198A2E03707344));
}

namespace {

status_or<std::string_view> string_as_string(const starlark_obj* element, std::string_view fn_name, int64_t arg_pos, error_fn& error_callback) {
  if (element->type() != starlark_types::string_t) {
    error_callback.add_error(error_argument_must_be_type(fn_name, arg_pos, starlark_types::string_t, element->type()));
    return status_or<std::string_view>(status_code::kError);
  }
  return status_or<std::string_view>(element->as_string());
}

status_or<std::vector<std::string_view>> string_or_tuple_as_vector_of_string(const starlark_obj* element, std::string_view fn_name, int64_t arg_pos, error_fn& error_callback) {
  std::vector<std::string_view> result;
  if (element->type() == starlark_types::tuple_t) {
    auto* tuple = static_cast<const starlark_tuple*>(element);
    for (int i = 0; i < tuple->size(); ++i) {
      auto* entry = tuple->at(i);
      if (entry->type() != starlark_types::string_t) {
        error_callback.add_error(error_tuple_must_contain_type(fn_name, starlark_types::string_t, entry->type()));
        return status_or<std::vector<std::string_view>>(status_code::kError);
      }
      result.emplace_back(entry->as_string());
    }
  } else {
    if (element->type() != starlark_types::string_t) {
      error_callback.add_error(error_argument_must_be_type(fn_name, arg_pos, starlark_types::string_t, element->type()));
      return status_or<std::vector<std::string_view>>(status_code::kError);
    }
    result.emplace_back(element->as_string());
  }
  return status_or<std::vector<std::string_view>>(result);
}

status_or<std::pair<int64_t, int64_t>> get_start_and_end(const starlark_obj::pos_args_t& pos_args, error_fn& error_callback) {
  int64_t start = std::numeric_limits<int64_t>::min();
  int64_t end = std::numeric_limits<int64_t>::max();
  if (pos_args.size() >= 2) {
    if (!to_int64_with_clamping_for_index_allow_none(*pos_args[1], start, error_callback).ok()) {
      return status_or<std::pair<int64_t, int64_t>>(status_code::kError);
    }
    if (pos_args.size() >= 3) {
      if (!to_int64_with_clamping_for_index_allow_none(*pos_args[2], end, error_callback).ok()) {
        return status_or<std::pair<int64_t, int64_t>>(status_code::kError);
      }
    }
  }
  return status_or<std::pair<int64_t, int64_t>>(std::make_pair(start, end));
}

}  // namespace

starlark_obj* starlark_string_fn_capitalize(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_arg(pos_args, named_args, error_callback, "capitalize").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  return static_cast<starlark_string*>(this_obj)->capitalize(ctx);
}

starlark_obj* starlark_string_fn_count(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "string.count").ok() ||
      !min_args(pos_args, error_callback, "count", 1).ok() ||
      !max_args(pos_args, error_callback, "count", 3).ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  auto sub = string_as_string(pos_args.front(), "count", 1, error_callback);
  if (!sub.ok()) {
    return nullptr;
  }
  auto start_end = get_start_and_end(pos_args, error_callback);
  if (!start_end.ok()) {
    return nullptr;
  }
  return create_integer(static_cast<starlark_string*>(this_obj)->count(*sub, start_end->first, start_end->second), ctx);
}

starlark_obj* starlark_string_fn_elem_ords(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_elems(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_endswith(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "string.endswith").ok() ||
      !min_args(pos_args, error_callback, "endswith", 1).ok() ||
      !max_args(pos_args, error_callback, "endswith", 3).ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  auto subs = string_or_tuple_as_vector_of_string(pos_args.front(), "endswith", 1, error_callback);
  if (!subs.ok()) {
    return nullptr;
  }
  auto start_end = get_start_and_end(pos_args, error_callback);
  if (!start_end.ok()) {
    return nullptr;
  }
  return static_cast<starlark_string*>(this_obj)->endswith(*subs, start_end->first, start_end->second) ? ctx.true_value() : ctx.false_value();
}

starlark_obj* starlark_string_fn_find(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "string.find").ok() ||
      !min_args(pos_args, error_callback, "find", 1).ok() ||
      !max_args(pos_args, error_callback, "find", 3).ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  auto sub = string_as_string(pos_args.front(), "find", 1, error_callback);
  if (!sub.ok()) {
    return nullptr;
  }
  auto start_end = get_start_and_end(pos_args, error_callback);
  if (!start_end.ok()) {
    return nullptr;
  }
  return create_integer(static_cast<starlark_string*>(this_obj)->find(*sub, start_end->first, start_end->second), ctx);
}

starlark_obj* starlark_string_fn_format(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_index(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "string.index").ok() ||
      !min_args(pos_args, error_callback, "index", 1).ok() ||
      !max_args(pos_args, error_callback, "index", 3).ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  auto sub = string_as_string(pos_args.front(), "index", 1, error_callback);
  if (!sub.ok()) {
    return nullptr;
  }
  auto start_end = get_start_and_end(pos_args, error_callback);
  if (!start_end.ok()) {
    return nullptr;
  }
  auto result = static_cast<starlark_string*>(this_obj)->find(*sub, start_end->first, start_end->second);
  if (result < 0) {
    error_callback.add_error(error_substring_not_found());
    return nullptr;
  }
  return create_integer(result, ctx);
}

starlark_obj* starlark_string_fn_isalnum(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_arg(pos_args, named_args, error_callback, "isalnum").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  return static_cast<starlark_string*>(this_obj)->isalnum() ? ctx.true_value() : ctx.false_value();
}

starlark_obj* starlark_string_fn_isalpha(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_arg(pos_args, named_args, error_callback, "isalpha").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  return static_cast<starlark_string*>(this_obj)->isalpha() ? ctx.true_value() : ctx.false_value();
}

starlark_obj* starlark_string_fn_isdigit(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_arg(pos_args, named_args, error_callback, "isdigit").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  return static_cast<starlark_string*>(this_obj)->isdigit() ? ctx.true_value() : ctx.false_value();
}

starlark_obj* starlark_string_fn_islower(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_arg(pos_args, named_args, error_callback, "islower").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  return static_cast<starlark_string*>(this_obj)->islower() ? ctx.true_value() : ctx.false_value();
}

starlark_obj* starlark_string_fn_isspace(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_arg(pos_args, named_args, error_callback, "isspace").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  return static_cast<starlark_string*>(this_obj)->isspace() ? ctx.true_value() : ctx.false_value();
}

starlark_obj* starlark_string_fn_istitle(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_arg(pos_args, named_args, error_callback, "istitle").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  return static_cast<starlark_string*>(this_obj)->istitle() ? ctx.true_value() : ctx.false_value();
}

starlark_obj* starlark_string_fn_isupper(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_arg(pos_args, named_args, error_callback, "isupper").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  return static_cast<starlark_string*>(this_obj)->isupper() ? ctx.true_value() : ctx.false_value();
}

starlark_obj* starlark_string_fn_join(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "join").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);

  auto it = pos_args.front()->get_iterator(true, ctx, error_callback);
  if (it == nullptr) {
    // TODO(lmirelmann): The error is not exactly the same, Python produces the following error:
    // `TypeError: can only join an iterable`
    return nullptr;
  }
  std::vector<std::string_view> elements;
  while (it->has_next()) {
    auto element = string_as_string(it->next(), "join", 1, error_callback);
    if (!element.ok()) {
      return nullptr;
      it->end_iterator();
    }
    elements.push_back(*element);
  }
  it->end_iterator();
  return static_cast<starlark_string*>(this_obj)->join(elements, ctx);
}

starlark_obj* starlark_string_fn_lower(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_arg(pos_args, named_args, error_callback, "lower").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  return static_cast<starlark_string*>(this_obj)->lower(ctx);
}

starlark_obj* starlark_string_fn_lstrip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_partition(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "partition").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);

  auto separator = string_as_string(pos_args.front(), "partition", 1, error_callback);
  if (!separator.ok()) {
    return nullptr;
  }
  return static_cast<starlark_string*>(this_obj)->partition(*separator, ctx, error_callback);
}

starlark_obj* starlark_string_fn_replace(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "string.replace").ok() ||
      !min_args(pos_args, error_callback, "replace", 2).ok() ||
      !max_args(pos_args, error_callback, "replace", 3).ok()) {
    return nullptr;
  }
  auto old = string_as_string(pos_args.front(), "replace", 1, error_callback);
  if (!old.ok()) {
    return nullptr;
  }
  auto new_ = string_as_string(pos_args[1], "replace", 2, error_callback);
  if (!new_.ok()) {
    return nullptr;
  }
  int64_t count = -1;
  if (pos_args.size() >= 3) {
    auto status_or_count = to_int64_with_clamping(*pos_args[2], error_callback);
    if (!status_or_count.ok()) {
      return nullptr;
    }
    count = *status_or_count;
  }
  return static_cast<starlark_string*>(this_obj)->replace(*old, *new_, count, ctx);
}

starlark_obj* starlark_string_fn_removeprefix(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "removeprefix").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);

  auto sub = string_as_string(pos_args.front(), "removeprefix", 1, error_callback);
  if (!sub.ok()) {
    return nullptr;
  }
  return static_cast<starlark_string*>(this_obj)->removeprefix(*sub, ctx);
}

starlark_obj* starlark_string_fn_removesuffix(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "removesuffix").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);

  auto sub = string_as_string(pos_args.front(), "removesuffix", 1, error_callback);
  if (!sub.ok()) {
    return nullptr;
  }
  return static_cast<starlark_string*>(this_obj)->removesuffix(*sub, ctx);
}

starlark_obj* starlark_string_fn_rfind(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "string.rfind").ok() ||
      !min_args(pos_args, error_callback, "rfind", 1).ok() ||
      !max_args(pos_args, error_callback, "rfind", 3).ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  auto sub = string_as_string(pos_args.front(), "rfind", 1, error_callback);
  if (!sub.ok()) {
    return nullptr;
  }
  auto start_end = get_start_and_end(pos_args, error_callback);
  if (!start_end.ok()) {
    return nullptr;
  }
  return create_integer(static_cast<starlark_string*>(this_obj)->rfind(*sub, start_end->first, start_end->second), ctx);
}

starlark_obj* starlark_string_fn_rindex(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "string.rindex").ok() ||
      !min_args(pos_args, error_callback, "rindex", 1).ok() ||
      !max_args(pos_args, error_callback, "rindex", 3).ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  auto sub = string_as_string(pos_args.front(), "rindex", 1, error_callback);
  if (!sub.ok()) {
    return nullptr;
  }
  auto start_end = get_start_and_end(pos_args, error_callback);
  if (!start_end.ok()) {
    return nullptr;
  }
  auto result = static_cast<starlark_string*>(this_obj)->rfind(*sub, start_end->first, start_end->second);
  if (result < 0) {
    error_callback.add_error(error_substring_not_found());
    return nullptr;
  }
  return create_integer(result, ctx);
}

starlark_obj* starlark_string_fn_rpartition(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "rpartition").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);

  auto separator = string_as_string(pos_args.front(), "rpartition", 1, error_callback);
  if (!separator.ok()) {
    return nullptr;
  }
  return static_cast<starlark_string*>(this_obj)->rpartition(*separator, ctx, error_callback);
}

starlark_obj* starlark_string_fn_rsplit(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "string.rsplit").ok() ||
      !max_args(pos_args, error_callback, "rsplit", 2).ok()) {
    return nullptr;
  }
  if (pos_args.empty() || pos_args.front()->type() == starlark_types::none_t) {
    int64_t maxsplit = -1;
    if (pos_args.size() >= 2) {
      auto status_or_maxsplit = to_int64_with_clamping(*pos_args[1], error_callback);
      if (!status_or_maxsplit.ok()) {
        return nullptr;
      }
      maxsplit = *status_or_maxsplit;
    }
    return static_cast<starlark_string*>(this_obj)->rsplit(maxsplit, ctx);
  }
  auto sep = string_as_string(pos_args.front(), "rsplit", 1, error_callback);
  if (!sep.ok()) {
    return nullptr;
  }
  int64_t maxsplit = -1;
  if (pos_args.size() >= 2) {
    auto status_or_maxsplit = to_int64_with_clamping(*pos_args[1], error_callback);
    if (!status_or_maxsplit.ok()) {
      return nullptr;
    }
    maxsplit = *status_or_maxsplit;
  }
  return static_cast<starlark_string*>(this_obj)->rsplit(*sep, maxsplit, ctx, error_callback);
}

starlark_obj* starlark_string_fn_rstrip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_split(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "string.split").ok() ||
      !max_args(pos_args, error_callback, "split", 2).ok()) {
    return nullptr;
  }
  if (pos_args.empty() || pos_args.front()->type() == starlark_types::none_t) {
    int64_t maxsplit = -1;
    if (pos_args.size() >= 2) {
      auto status_or_maxsplit = to_int64_with_clamping(*pos_args[1], error_callback);
      if (!status_or_maxsplit.ok()) {
        return nullptr;
      }
      maxsplit = *status_or_maxsplit;
    }
    return static_cast<starlark_string*>(this_obj)->split(maxsplit, ctx);
  }
  auto sep = string_as_string(pos_args.front(), "split", 1, error_callback);
  if (!sep.ok()) {
    return nullptr;
  }
  int64_t maxsplit = -1;
  if (pos_args.size() >= 2) {
    auto status_or_maxsplit = to_int64_with_clamping(*pos_args[1], error_callback);
    if (!status_or_maxsplit.ok()) {
      return nullptr;
    }
    maxsplit = *status_or_maxsplit;
  }
  return static_cast<starlark_string*>(this_obj)->split(*sep, maxsplit, ctx, error_callback);
}

starlark_obj* starlark_string_fn_splitlines(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_startswith(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "string.startswith").ok() ||
      !min_args(pos_args, error_callback, "startswith", 1).ok() ||
      !max_args(pos_args, error_callback, "startswith", 3).ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  auto subs = string_or_tuple_as_vector_of_string(pos_args.front(), "startswith", 1, error_callback);
  if (!subs.ok()) {
    return nullptr;
  }
  auto start_end = get_start_and_end(pos_args, error_callback);
  if (!start_end.ok()) {
    return nullptr;
  }
  return static_cast<starlark_string*>(this_obj)->startswith(*subs, start_end->first, start_end->second) ? ctx.true_value() : ctx.false_value();
}

starlark_obj* starlark_string_fn_strip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_title(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_arg(pos_args, named_args, error_callback, "title").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  return static_cast<starlark_string*>(this_obj)->title(ctx);
}

starlark_obj* starlark_string_fn_upper(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_arg(pos_args, named_args, error_callback, "upper").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  return static_cast<starlark_string*>(this_obj)->upper(ctx);
}

}  // namespace runtime
}  // namespace starlark

