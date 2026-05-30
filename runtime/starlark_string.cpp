// Copyright 2025-2026 Lucas Mirelmann

#include "runtime/starlark_string.hpp"

#include <stdckdint.h>
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
using ::starlark::result::error_status;
using ::starlark::result::ok_status;
using ::starlark::result::status;
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
using ::starlark::unicode::replacement_character_utf8;
using ::starlark::unicode::utf8_encode_code_point;
using ::starlark::unicode::utf8_reader;
using ::starlark::unicode::utf8_reverse_reader;
using ::starlark::unicode::word_break;

namespace starlark {
namespace runtime {

starlark_obj* starlark_string_fn_capitalize(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_codepoint_ords(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_codepoints(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
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
starlark_obj* starlark_string_fn_removeprefix(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_removesuffix(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_string_fn_replace(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
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

void append_for_repr(std::string& output, std::string_view input) {
  utf8_reader reader(input, false, false);
  while (reader.pending()) {
    auto code_point = reader.peek_code_point();
    if (code_point.first != utf8_reader::kReplacementCharacter) {
      write_printable(code_point.first, /*allow_non_ascii_printable=*/ true, output);
      reader.skip(code_point.second);
    } else {
      output += input[reader.pos()];
      reader.skip();
    }
  }
}

status chr_fn(std::string& output, int64_t input, error_fn& error_callback) {
  if (input < 0 || 0x10ffff < input) {
    error_callback.add_error(error_unicode_in_range());
    return error_status();
  }
  utf8_encode_code_point(input, output, false, true);
  return ok_status();
}

status chr_fn(std::string& output, const starlark::bigint::number& input, error_fn& error_callback) {
  if (input.sign() || input.bit_size() > 21) {
    error_callback.add_error(error_unicode_in_range());
    return error_status();
  }
  auto ivalue = input.at(0);
  if (0x10ffff < ivalue) {
    error_callback.add_error(error_unicode_in_range());
    return error_status();
  }
  utf8_encode_code_point(ivalue, output, false, true);
  return ok_status();
}

constexpr std::string::size_type starlark_string::index_step;

const std::map<std::string, starlark_obj::fn*, std::less<>>& starlark_string::method_refs() {
  static const std::map<std::string, starlark_obj::fn*, std::less<>>* result =
    new std::map<std::string, starlark_obj::fn*, std::less<>>{
      {"capitalize", starlark_string_fn_capitalize},
      {"codepoint_ords", starlark_string_fn_codepoint_ords},
      {"codepoints", starlark_string_fn_codepoints},
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
      {"removeprefix", starlark_string_fn_removeprefix},
      {"removesuffix", starlark_string_fn_removesuffix},
      {"replace", starlark_string_fn_replace},
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

starlark_string::starlark_string(std::string&& value) : value(std::forward<std::string>(value)) {
  build_index();
}

starlark_string::starlark_string(std::string_view value) : value(value) {
  build_index();
}

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
  return size;
}

bool starlark_string::inner_repr(printer& print, printer_action action) const {
  assert(action == printer_action::kPrintTop);
  // If this function were to be executed a lot, then there are a
  // few things that can we can try:
  // - Check whether the original value can be used just adding quotes
  // - Keep the value of `result` in a mutable field
  std::string result = "\"";
  append_for_repr(result, value);
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
  std::size_t expected_size;
  if (ckd_add(&expected_size, this_obj.as_string().size(), other.as_string().size()) ||
      expected_size > ctx.options().max_string_length) {
    error_callback.add_error(error_max_sequence_length(ctx.options().max_string_length));
    return nullptr;
  }
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
      auto multiplier = other.as_int64();
      std::string result;
      if (multiplier > 0) {
        std::size_t expected_size;
        if (ckd_mul(&expected_size, this_obj.as_string().size(), multiplier) ||
            expected_size > ctx.options().max_string_length) {
          error_callback.add_error(error_max_sequence_length(ctx.options().max_string_length));
          return nullptr;
        }
        for (int64_t i = 0; i < multiplier; ++i) {
          result += this_obj.as_string();
        }
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
        error_callback.add_error(error_max_sequence_length(ctx.options().max_string_length));
        return nullptr;
      }
      int64_t int_value = multiplier.at(0);
      std::size_t expected_size;
      if (ckd_mul(&expected_size, this_obj.as_string().size(), int_value) ||
          expected_size > ctx.options().max_string_length) {
        error_callback.add_error(error_max_sequence_length(ctx.options().max_string_length));
        return nullptr;
      }
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

status parse_interpolation(std::string_view format, std::vector<std::string>& parts, std::vector<std::pair<char32_t, std::size_t>>& convertions, error_fn& error_callback) {
  bool last_is_percent = false;
  parts.emplace_back();
  std::size_t pos = 0;
  utf8_reader reader{format, false, false};
  while (reader.pending()) {
    auto start = reader.pos();
    auto cp = reader.read_code_point();
    if (last_is_percent) {
      if (cp == '%') {
        parts.back() += format.substr(start, reader.pos() - start);
      } else {
        convertions.push_back(std::make_pair(cp, pos));
        parts.emplace_back();
      }
      last_is_percent = false;
    } else if (cp == '%') {
      last_is_percent = true;
    } else {
      parts.back() += format.substr(start, reader.pos() - start);
    }
    ++pos;
  }
  if (last_is_percent) {
    error_callback.add_error(error_incomplete_format());
    return error_status();
  }
  return ok_status();
}

status interpolation_convertion(std::string& result, const starlark_obj& element, char32_t format, std::size_t index, context& ctx, error_fn& error_callback) {
  switch (format) {
    case 's':
      result += element.str();
      break;
    case 'r':
      result += element.repr();
      break;
    case 'c':
      switch (element.numeric_type()) {
        case starlark_numeric_type::kInt64:
          if (!chr_fn(result, element.as_int64(), error_callback).ok()) {
            return error_status();
          }
          break;
        case starlark_numeric_type::kBigInt:
          if (!chr_fn(result, element.as_bigint(), error_callback).ok()) {
            return error_status();
          }
          break;
        case starlark_numeric_type::kNotNumeric: {
          if (element.type() != starlark_types::string_t) {
            error_callback.add_error(error_integer_or_unicode_character(element.type()));
            return error_status();
          }
          auto len = element.len(false, error_callback);
          if (len != 1) {
            error_callback.add_error(error_integer_or_unicode_character_type_and_length(element.type(), len));
            return error_status();
          }
          result += element.as_string();
          break;
        }
        default:
          error_callback.add_error(error_integer_or_unicode_character(element.type()));
          return error_status();
      }
      break;
    case 'd':
    case 'i':
    case 'o':
    case 'x':
    case 'X': {
      const starlark_obj* n_element;
      switch (element.numeric_type()) {
        case starlark_numeric_type::kInt64:
        case starlark_numeric_type::kBigInt:
          n_element = &element;
          break;
        case starlark_numeric_type::kFloat:
          n_element = create_integer_from_float(element.as_float(), ctx, error_callback);
          if (n_element == nullptr) {
            return error_status();
          }
          break;
        default:
          error_callback.add_error(error_format_integer_is_required(format, element.type()));
          return error_status();
      }
      switch (format) {
        case 'd':
        case 'i':
          switch (n_element->numeric_type()) {
            case starlark_numeric_type::kInt64:
            default:
              result += std::format("{:d}", n_element->as_int64());
              break;
            case starlark_numeric_type::kBigInt:
              result += n_element->as_bigint().to_string(10, false);
              break;
          }
          break;
        case 'o':
          switch (n_element->numeric_type()) {
            case starlark_numeric_type::kInt64:
            default:
              result += std::format("{:o}", n_element->as_int64());
              break;
            case starlark_numeric_type::kBigInt:
              result += n_element->as_bigint().to_string(8, false);
              break;
          }
          break;
        case 'x':
          switch (n_element->numeric_type()) {
            case starlark_numeric_type::kInt64:
            default:
              result += std::format("{:x}", n_element->as_int64());
              break;
            case starlark_numeric_type::kBigInt:
              result += n_element->as_bigint().to_string(16, false);
              break;
          }
          break;
        case 'X':
          switch (n_element->numeric_type()) {
            case starlark_numeric_type::kInt64:
            default:
              result += std::format("{:X}", n_element->as_int64());
              break;
            case starlark_numeric_type::kBigInt:
              result += n_element->as_bigint().to_string(16, true);
              break;
          }
          break;
      }
      break;
    }
    case 'e':
    case 'E':
    case 'f':
    case 'F':
    case 'g':
    case 'G': {
      double float_value;
      switch (element.numeric_type()) {
        case starlark_numeric_type::kInt64:
          float_value = static_cast<double>(element.as_int64());
          break;
        case starlark_numeric_type::kBigInt: {
          float_value = to_double(element.as_bigint());
          break;
        }
        case starlark_numeric_type::kFloat:
          float_value = element.as_float();
          break;
        default:
          error_callback.add_error(error_format_real_is_required(format, element.type()));
          return error_status();
      }
      if (std::isfinite(float_value)) {
        switch (format) {
          case 'e':
            result += std::format("{:e}", float_value);
            break;
          case 'E':
            result += std::format("{:E}", float_value);
            break;
          case 'f':
            result += std::format("{:f}", float_value);
            break;
          case 'F':
            result += std::format("{:F}", float_value);
            break;
          case 'g':
            result += float_to_string(float_value, false);
            break;
          case 'G':
            result += float_to_string(float_value, true);
            break;
        }
      } else {
        result += float_to_string(float_value, false);
      }
      break;
    }
    default:
      error_callback.add_error(error_unsupported_format_character(format, index));
      return error_status();
  }
  return ok_status();
}

status parse_format(std::string_view format, std::vector<std::string>& parts, std::vector<std::string>& names, error_fn& error_callback) {
  enum class state_t {
    kText,
    kLastElementWasOpenCurlyBraces,
    kLastElementWasCloseCurlyBraces,
    kName,
    kNumber,
  };
  bool has_blanks = false;
  bool has_numbers = false;
  state_t state = state_t::kText;
  parts.emplace_back();
  utf8_reader reader{format, false, false};
  while (reader.pending()) {
    auto start = reader.pos();
    auto cp = reader.read_code_point();
    switch (state) {
      case state_t::kText:
        if (cp == '{') {
          state = state_t::kLastElementWasOpenCurlyBraces;
        } else if (cp == '}') {
          state = state_t::kLastElementWasCloseCurlyBraces;
        } else {
          parts.back() += format.substr(start, reader.pos() - start);
        }
        break;
      case state_t::kLastElementWasOpenCurlyBraces:
        if (cp == '{') {
          parts.back() += format.substr(start, reader.pos() - start);
          state = state_t::kText;
        } else if (cp == '}') {
          names.emplace_back();
          parts.emplace_back();
          state = state_t::kText;
          has_blanks = true;
        } else if ('0' <= cp && cp <= '9') {
          // In Python, the logic is much more complex as it allows any character with Unicode General Category Nd
          has_numbers = true;
          names.emplace_back();
          names.back() += format.substr(start, reader.pos() - start);
          state = state_t::kNumber;
        } else if (cp == '!' || cp == '.' || cp == ':' || cp == '[') {  // https://github.com/python/cpython/issues/150626
          error_callback.add_error(error_unexpected_in_field_name(format.substr(start, reader.pos() - start)));
          return error_status();
        } else {
          names.emplace_back();
          names.back() += format.substr(start, reader.pos() - start);
          state = state_t::kName;
        }
        break;
      case state_t::kLastElementWasCloseCurlyBraces:
        if (cp == '}') {
          parts.back() += format.substr(start, reader.pos() - start);
          state = state_t::kText;
        } else {
          error_callback.add_error(error_single_format_element_in_string("}"));
          return error_status();
        }
        break;
      case state_t::kName:
        if (cp == '}') {
          parts.emplace_back();
          state = state_t::kText;
        } else if (cp == '!' || cp == '.' || cp == ':' || cp == '[' || cp == '{') {  // https://github.com/python/cpython/issues/150626
          error_callback.add_error(error_unexpected_in_field_name(format.substr(start, reader.pos() - start)));
          return error_status();
        } else {
          names.back() += cp;
        }
        break;
      case state_t::kNumber:
        if (cp == '}') {
          parts.emplace_back();
          state = state_t::kText;
        } else if ('0' <= cp && cp <= '9') {
          names.back() += format.substr(start, reader.pos() - start);
        } else {
          error_callback.add_error(error_unexpected_in_field_name(format.substr(start, reader.pos() - start)));
          return error_status();
        }
        break;
    }
  }
  if (has_blanks && has_numbers) {
    error_callback.add_error(error_switch_from_manual_to_automatic_numbering());
    return error_status();
  }
  switch (state) {
    default:
    case state_t::kText:
      return ok_status();
    case state_t::kLastElementWasOpenCurlyBraces:
      error_callback.add_error(error_single_format_element_in_string("{"));
      return error_status();
    case state_t::kLastElementWasCloseCurlyBraces:
      error_callback.add_error(error_single_format_element_in_string("}"));
      return error_status();
    case state_t::kName:
    case state_t::kNumber:
      error_callback.add_error(error_expected_format_element_before_end_of_string("}"));
      return error_status();
  }
}

starlark_obj* percent_op(const starlark_string& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  std::vector<std::string> parts;
  std::vector<std::pair<char32_t, std::size_t>> convertions;
  if (!parse_interpolation(this_obj.as_string(), parts, convertions, error_callback).ok()) {
    return nullptr;
  }
  std::string result = parts[0];
  if (other.type() != starlark_types::tuple_t) {
    if (parts.size() != 2) {
      error_callback.add_error(error_not_enough_arguments_for_format_string());
      return nullptr;
    }
    if (!interpolation_convertion(result, other, convertions[0].first, convertions[0].second, ctx, error_callback).ok()) {
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
    if (!interpolation_convertion(result, *t_other.at(i), convertions[i].first, convertions[i].second, ctx, error_callback).ok()) {
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

bool test_final_sigma_before(const std::vector<char32_t>& code_points, std::size_t start) {
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

bool test_final_sigma_after(const std::vector<char32_t>& code_points, std::size_t start) {
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

std::string merge_parts(const std::vector<case_convertion>& parts, const std::vector<char32_t>& code_points) {
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
  std::vector<char32_t> code_points;
  std::vector<std::size_t> code_points_pos;
  utf8_reader reader(value, false, false);
  while (reader.pending()) {
    code_points_pos.push_back(reader.pos());
    auto code_point = reader.read_code_point();
    code_points.push_back(code_point);
  }
  code_points_pos.push_back(reader.pos());
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
      auto new_code_points = to_title(code_point);
      if (new_code_points.has_value()) {
        parts.back().prefix += new_code_points.value();
      } else {
        parts.back().prefix += value.substr(code_points_pos[pos], code_points_pos[pos + 1] - code_points_pos[pos]);
      }
      first_letter_of_word = false;
    } else {
      auto new_code_points = to_lower(code_point);
      if (!new_code_points.has_value()) {
        parts.back().prefix += value.substr(code_points_pos[pos], code_points_pos[pos + 1] - code_points_pos[pos]);
      } else if (!new_code_points->second.has_value()) {
        parts.back().prefix += new_code_points.value().first;
      } else {
        parts.back().else_condition += new_code_points.value().first;
        parts.back().if_condition += new_code_points.value().second.value();
        parts.back().condition = case_condition::kFinalSigma;
        parts.back().pos = pos;
        parts.emplace_back();
        parts.back().condition = case_condition::kNone;
      }
    }
  }
  return merge_parts(parts, code_points);
}

enum class strip_type {
  kLeft,
  kRight,
  kBoth,
};

std::string_view strip_impl(std::string_view value, std::string_view cutset, strip_type stype) {
  std::size_t start = 0;
  if (stype == strip_type::kLeft || stype == strip_type::kBoth) {
    utf8_reader reader(value, false, false);
    while (reader.pending()) {
      auto begin = reader.pos();
      if ((reader.read_code_point() == utf8_reader::kReplacementCharacter) &&
          (value.substr(begin, reader.pos() - begin) != replacement_character_utf8())) {
        break;
      }
      auto end = reader.pos();
      if (cutset.find(value.substr(begin, end - begin)) == std::string::npos) {
        break;
      }
      start = end;
    }
  }
  std::size_t count = value.size();
  if (stype == strip_type::kRight || stype == strip_type::kBoth) {
    utf8_reverse_reader reverse_reader(value, false);
    while (reverse_reader.pos() > start) {
      auto end = reverse_reader.pos();
      if ((reverse_reader.read_code_point() == utf8_reader::kReplacementCharacter) &&
          (value.substr(reverse_reader.pos(), end - reverse_reader.pos()) != replacement_character_utf8())) {
        break;
      }
      auto begin = reverse_reader.pos();
      if (cutset.find(value.substr(begin, end - begin)) == std::string::npos) {
        break;
      }
      count = begin;
    }
  }
  return value.substr(start, count - start);
}

status_or<std::string_view> string_as_string(const starlark_obj* element, std::string_view fn_name, int64_t arg_pos, error_fn& error_callback) {
  if (element->type() != starlark_types::string_t) {
    error_callback.add_error(error_argument_must_be_type(fn_name, arg_pos, starlark_types::string_t, element->type()));
    return status_or<std::string_view>(status_code::kError);
  }
  return status_or<std::string_view>(element->as_string());
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
  auto idx = inner_index(other, size, error_callback);
  if (!idx.ok()) {
    return nullptr;
  }
  return Arena::Create<starlark_string>(&ctx.arena(), element_at(*idx));
}

starlark_obj* starlark_string::slice_range(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, context& ctx, error_fn& error_callback) const {
  auto slice_result = inner_slice_range(start, stop, stride, size, error_callback);
  if (!slice_result.ok()) {
    return nullptr;
  }
  auto i_start = std::get<0>(*slice_result);
  auto i_end = std::get<1>(*slice_result);
  auto i_stride = std::get<2>(*slice_result);

  decltype(value) result;
  if (i_stride > 0) {
    for (auto i = i_start; i < i_end; i += i_stride) {
      result += element_at(i);
    }
  } else {
    for (auto i = i_start; i > i_end; i += i_stride) {
      result += element_at(i);
    }
  }
  return Arena::Create<starlark_string>(&ctx.arena(), result);
}

std::string_view starlark_string::as_string() const {
  return value;
}

int64_t starlark_string::count(std::string_view sub, int64_t start, int64_t end) const {
  if (start < 0) {
    start = std::max<int64_t>(start + size, 0);
  } else {
    start = std::min<int64_t>(start, size);
  }
  if (end < 0) {
    end = std::max<int64_t>(end + size, 0);
  } else {
    end = std::min<int64_t>(end, size);
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

bool starlark_string::endswith(const std::vector<std::string_view>& ends, int64_t start, int64_t end) const {
  if (start < 0) {
    start = std::max<int64_t>(start + size, 0);
  }
  if (end < 0) {
    end = std::max<int64_t>(end + size, 0);
  } else {
    end = std::min<int64_t>(end, size);
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
    start = std::max<int64_t>(start + size, 0);
  }
  if (end < 0) {
    end = std::max<int64_t>(end + size, 0);
  } else {
    end = std::min<int64_t>(end, size);
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
    start = std::max<int64_t>(start + size, 0);
  }
  if (end < 0) {
    end = std::max<int64_t>(end + size, 0);
  } else {
    end = std::min<int64_t>(end, size);
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
    start = std::max<int64_t>(start + size, 0);
  }
  if (end < 0) {
    end = std::max<int64_t>(end + size, 0);
  } else {
    end = std::min<int64_t>(end, size);
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
  for (auto cp = reader.peek_code_point(); reader.pending() && is_space(cp.first); cp = reader.peek_code_point()) {
    reader.skip(cp.second);
  }
  while (reader.pending() && maxsplit > 0) {
    auto start = reader.pos();
    for (auto cp = reader.peek_code_point(); reader.pending() && !is_space(cp.first); cp = reader.peek_code_point()) {
      reader.skip(cp.second);
    }
    result->unsafe_append(Arena::Create<starlark_string>(&ctx.arena(), value.substr(start, reader.pos() - start)));
    for (auto cp = reader.peek_code_point(); reader.pending() && is_space(cp.first); cp = reader.peek_code_point()) {
      reader.skip(cp.second);
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

starlark_obj* starlark_string::lstrip(context& ctx) const {
  utf8_reader reader(value, false, false);
  for (auto cp = reader.peek_code_point(); reader.pending() && is_space(cp.first); cp = reader.peek_code_point()) {
    reader.skip(cp.second);
  }
  return Arena::Create<starlark_string>(&ctx.arena(), value.substr(reader.pos()));
}

starlark_obj* starlark_string::lstrip(std::string_view cutset, context& ctx) const {
  return Arena::Create<starlark_string>(&ctx.arena(), strip_impl(value, cutset, strip_type::kLeft));
}

starlark_obj* starlark_string::rstrip(context& ctx) const {
  utf8_reverse_reader reader(value, false);
  while (reader.pending() && is_space(reader.peek_code_point())) {
    reader.read_code_point();
  }
  return Arena::Create<starlark_string>(&ctx.arena(), value.substr(0, reader.pos()));
}

starlark_obj* starlark_string::rstrip(std::string_view cutset, context& ctx) const {
  return Arena::Create<starlark_string>(&ctx.arena(), strip_impl(value, cutset, strip_type::kRight));
}

starlark_obj* starlark_string::strip(context& ctx) const {
  utf8_reader reader(value, false, false);
  for (auto cp = reader.peek_code_point(); reader.pending() && is_space(cp.first); cp = reader.peek_code_point()) {
    reader.skip(cp.second);
  }
  utf8_reverse_reader reverse_reader(value, false);
  while (reverse_reader.pos() > reader.pos() && is_space(reverse_reader.peek_code_point())) {
    reverse_reader.read_code_point();
  }
  return Arena::Create<starlark_string>(&ctx.arena(), value.substr(reader.pos(), reverse_reader.pos() - reader.pos()));
}

starlark_obj* starlark_string::strip(std::string_view cutset, context& ctx) const {
  return Arena::Create<starlark_string>(&ctx.arena(), strip_impl(value, cutset, strip_type::kBoth));
}

starlark_obj* starlark_string::splitlines(bool keepends, context& ctx) const {
  auto* result = Arena::Create<starlark_list>(&ctx.arena(), 0);
  utf8_reader reader(value, false, false);
  std::size_t begin = 0;
  std::size_t end = 0;
  std::string_view mirror = value;
  while (reader.pending()) {
    bool found_end = false;
    auto candidate = reader.read_code_point();
    switch (candidate) {
      case '\n':
      case '\v':
      case '\f':
      case 0x1c:
      case 0x1d:
      case 0x1e:
      case 0x85:
      case 0x2028:
      case 0x2029:
        found_end = true;
        break;
      case '\r':
        // This case can be either "\r" or "\r\n".
        reader.capture("\n");
        found_end = true;
        break;
    }
    if (found_end) {
      if (keepends) {
        end = reader.pos();
      }
      result->unsafe_append(Arena::Create<starlark_string>(&ctx.arena(), mirror.substr(begin, end - begin)));
      begin = reader.pos();
    }
    end = reader.pos();
  }
  if (begin != end) {
    result->unsafe_append(Arena::Create<starlark_string>(&ctx.arena(), mirror.substr(begin, end - begin)));
  }
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
  std::vector<char32_t> code_points;
  utf8_reader reader(value, false, false);
  std::size_t pos = 0;
  parts.emplace_back();
  parts.back().condition = case_condition::kNone;
  std::string_view value_view = value;
  while (reader.pending()) {
    auto begin = reader.pos();
    auto code_point = reader.read_code_point();
    code_points.push_back(code_point);
    auto new_code_points = to_lower(code_point);
    if (!new_code_points.has_value()) {
      parts.back().prefix += value_view.substr(begin, reader.pos() - begin);
    } else if (!new_code_points->second.has_value()) {
      parts.back().prefix += new_code_points.value().first;
    } else {
      parts.back().else_condition += new_code_points.value().first;
      parts.back().if_condition += new_code_points.value().second.value();
      parts.back().condition = case_condition::kFinalSigma;
      parts.back().pos = pos;
      parts.emplace_back();
      parts.back().condition = case_condition::kNone;
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
  std::string_view value_view = value;
  while (reader.pending()) {
    auto begin = reader.pos();
    auto code_point = reader.read_code_point();
    auto new_code_points = to_upper(code_point);
    if (new_code_points.has_value()) {
      result += new_code_points.value();
    } else {
      result += value_view.substr(begin, reader.pos() - begin);
    }
  }
  return Arena::Create<starlark_string>(&ctx.arena(), std::move(result));
}

starlark_obj* starlark_string::capitalize(context& ctx) {
  std::vector<case_convertion> parts;
  std::vector<char32_t> code_points;
  utf8_reader reader(value, false, false);
  std::size_t pos = 0;
  parts.emplace_back();
  parts.back().condition = case_condition::kNone;
  bool is_first = true;
  std::string_view value_view = value;
  while (reader.pending()) {
    auto begin = reader.pos();
    auto code_point = reader.read_code_point();
    code_points.push_back(code_point);
    if (is_first) {
      auto new_code_points = to_title(code_point);
      if (new_code_points.has_value()) {
        parts.back().prefix += new_code_points.value();
      } else {
        parts.back().prefix += value_view.substr(begin, reader.pos() - begin);
      }
      is_first = false;
    } else {
      auto new_code_points = to_lower(code_point);
      if (!new_code_points.has_value()) {
        parts.back().prefix += value_view.substr(begin, reader.pos() - begin);
      } else if (!new_code_points->second.has_value()) {
        parts.back().prefix += new_code_points.value().first;
      } else {
        parts.back().else_condition += new_code_points.value().first;
        parts.back().if_condition += new_code_points.value().second.value();
        parts.back().condition = case_condition::kFinalSigma;
        parts.back().pos = pos;
        parts.emplace_back();
        parts.back().condition = case_condition::kNone;
      }
    }
    pos++;
  }
  return Arena::Create<starlark_string>(&ctx.arena(), merge_parts(parts, code_points));
}

starlark_obj* starlark_string::format(const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  std::vector<std::string> parts;
  std::vector<std::string> names;
  if (!parse_format(value, parts, names, error_callback).ok()) {
    return nullptr;
  }
  std::string result;
  std::size_t automatic_numbering = 0;
  for (std::size_t i = 0; i < names.size(); ++i) {
    result += parts[i];
    const auto& name = names[i];
    if (name.empty()) {
      if (pos_args.size() <= automatic_numbering) {
        error_callback.add_error(error_positional_argument_out_of_range(automatic_numbering));
        return nullptr;
      }
      result += pos_args[automatic_numbering]->str();
      automatic_numbering++;
    } else if ('0' <= name[0] && name[0] <= '9') {
      // This is making the assumption that the numbers are always in 0..9 instead of being of Unicode General Cathegory Nd.
      errno = 0;
      char* end;
      std::int64_t int_value = std::strtol(name.c_str(), &end, 10);
      if (errno == ERANGE || end != &*name.end()) {
        error_callback.add_error(error_positional_argument_out_of_range(name));
        return nullptr;
      }
      if (int_value < 0 || int_value >= pos_args.size()) {
        error_callback.add_error(error_positional_argument_out_of_range(name));
        return nullptr;
      }
      result += pos_args[int_value]->str();
    } else {
      auto it = named_args.find(name);
      if (it == named_args.end()) {
        error_callback.add_error(error_dictionary_key_not_found(name));
        return nullptr;
      }
      result += it->second->str();
    }
  }
  result += parts.back();
  return Arena::Create<starlark_string>(&ctx.arena(), result);
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

starlark_obj* starlark_string::elems(context& ctx) const {
  return Arena::Create<starlark_string::string_elems>(&ctx.arena(), this, calculate_state(0, size, 1), false, false);
}

starlark_obj* starlark_string::elem_ords(context& ctx) const {
  return Arena::Create<starlark_string::string_elems>(&ctx.arena(), this, calculate_state(0, size, 1), false, true);
}

starlark_obj* starlark_string::codepoints(context& ctx) const {
  return Arena::Create<starlark_string::string_elems>(&ctx.arena(), this, calculate_state(0, size, 1), true, false);
}

starlark_obj* starlark_string::codepoint_ords(context& ctx) const {
  return Arena::Create<starlark_string::string_elems>(&ctx.arena(), this, calculate_state(0, size, 1), true, true);
}

starlark_string::string_elems::string_elems(const starlark_string* str, range_state state, bool is_cp, bool ords) : str(str), state(state), is_cp(is_cp), ords(ords) {}

std::string_view starlark_string::string_elems::type() const {
  if (is_cp) {
    if (ords) {
      return "string.codepoint_ords";
    } else {
      return "string.codepoints";
    }
  } else {
    if (ords) {
      return "string.elem_ords";
    } else {
      return "string.elems";
    }
  }
}

bool starlark_string::string_elems::truthy() const {
  return state.len > 0;
}

bool starlark_string::string_elems::binary_in(const starlark_obj& other, error_fn& error_callback) const {
  if (ords) {
    int64_t value;
    switch (other.numeric_type()) {
      case starlark_numeric_type::kInt64:
        value = other.as_int64();
        break;
      case starlark_numeric_type::kBigInt: {
        auto& bvalue = other.as_bigint();
        if (bvalue.sign() || bvalue.bit_size() > 21) {
          return false;
        }
        value = bvalue.at(0);
        break;
      }
      default:
        error_callback.add_error(error_in_type_requires_type(type(), starlark_types::int_t, other.type()));
        return false;
    }
    for (std::string::size_type i = 0; i < state.len; ++i) {
      if (str->ord_element_at(state.start + i * state.step) == value) {
        return true;
      }
    }
  } else {
    if (other.type() != starlark_types::string_t) {
      error_callback.add_error(error_in_type_requires_type(type(), starlark_types::string_t, other.type()));
      return false;
    }
    for (std::string::size_type i = 0; i < state.len; ++i) {
      if (str->element_at(state.start + i * state.step) == other.as_string()) {
        return true;
      }
    }
  }
  return false;
}

int64_t starlark_string::string_elems::len(bool produce_error, error_fn& error_callback) const {
  return state.len;
}

starlark_iterator* starlark_string::string_elems::get_iterator(bool produce_error, context& ctx, error_fn& error_callback) {
  return Arena::Create<starlark_elems_iterator>(&ctx.arena(), str, state.start, state.step, state.len, ords, ctx);
}

starlark_obj* starlark_string::string_elems::index(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  auto idx = inner_index(other, state.len, error_callback);
  if (!idx.ok()) {
    return nullptr;
  }
  if (ords) {
    return create_integer(str->ord_element_at(state.start + (*idx) * state.step), ctx);
  } else {
    return Arena::Create<starlark_string>(&ctx.arena(), str->element_at(state.start + (*idx) * state.step));
  }
}

starlark_obj* starlark_string::string_elems::slice_range(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, context& ctx, error_fn& error_callback) const {
  auto slice_result = inner_slice_range_range(start, stop, stride, state.start, state.end, state.step, state.len, error_callback);
  if (!slice_result.ok()) {
    return nullptr;
  }
  return Arena::Create<string_elems>(&ctx.arena(), str, calculate_state(std::get<0>(*slice_result), std::get<1>(*slice_result), std::get<2>(*slice_result)), is_cp, ords);
}

bool starlark_string::string_elems::inner_repr(printer& print, printer_action action) const {
  assert(action == printer_action::kPrintTop);
  std::string result = "\"";
  if (state.step > 0) {
    for (auto i = state.start; i < state.end; i += state.step) {
      append_for_repr(result, str->element_at(i));
    }
  } else {
    for (auto i = state.start; i > state.end; i += state.step) {
      append_for_repr(result, str->element_at(i));
    }
  }
  if (is_cp) {
    if (ords) {
      result += "\".codepoint_ords()";
    } else {
      result += "\".codepoints()";
    }
  } else {
    if (ords) {
      result += "\".elem_ords()";
    } else {
      result += "\".elems()";
    }
  }
  print.append(result);
  return false;
}

bool starlark_string::string_elems::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
  if (type() != other->type()) {
    return false;
  }
  const string_elems* e_other = static_cast<const string_elems*>(other);
  if (state.len != e_other->state.len) {
    return false;
  }
  for (std::string::size_type i = 0; i < state.len; ++i) {
    if (str->element_at(state.start + i * state.step) != e_other->str->element_at(e_other->state.start + i * e_other->state.step)) {
      return false;
    }
  }
  return true;
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_string::string_elems::inner_hash() const {
  return -1;
}

starlark_string::starlark_elems_iterator::starlark_elems_iterator(const starlark_string* str, int64_t current_pos, int64_t step, int64_t remaining, bool ords, context& ctx)
  : str(str), current_pos(current_pos), step(step), remaining(remaining), ords(ords), ctx(ctx) {}


bool starlark_string::starlark_elems_iterator::has_next() const {
  return remaining > 0;
}

starlark_obj* starlark_string::starlark_elems_iterator::next() {
  starlark_obj* result;
  if (ords) {
    result = create_integer(str->ord_element_at(current_pos), ctx);
  } else {
    result = Arena::Create<starlark_string>(&ctx.arena(), str->element_at(current_pos));
  }
  current_pos += step;
  remaining--;
  return result;
}

void starlark_string::starlark_elems_iterator::end_iterator() {}

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
  if (value.empty()) {
    return 0;
  }
  return static_cast<int64_t>(siphash(value.data(), value.length(), 0x243F6A8885A308D3, 0x13198A2E03707344));
}

void starlark_string::build_index() {
  // TODO(lmirelmann): Build the index lazy, but still have a way to know what the size of the string is.
  utf8_reader reader(value, false, false);
  std::string::size_type code_point_count = 0;
  while (reader.pending()) {
    if (code_point_count && (code_point_count % index_step == 0)) {
      value_index.push_back(reader.pos());
    }
    reader.read_code_point();
    ++code_point_count;
  }
  size = code_point_count;
}

std::string_view starlark_string::element_at(std::size_t element) const {
  assert(element < size);
  auto big_step = element / index_step;
  auto small_step = element % index_step;
  utf8_reader reader(value, false, false);
  if (big_step) {
    reader.skip(value_index[big_step - 1]);
  }
  while (small_step) {
    small_step--;
    reader.read_code_point();
  }
  auto start = reader.pos();
  reader.read_code_point();
  std::string_view mirror = value;
  return mirror.substr(start, reader.pos() - start);
}

char32_t starlark_string::ord_element_at(std::size_t element) const {
  return utf8_reader(element_at(element), false, false).read_code_point();
}

namespace {

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
      error_callback.add_error(error_string_or_tuple_of_string(fn_name, element->type()));
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

starlark_obj* starlark_string_fn_elems(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_arg(pos_args, named_args, error_callback, "string.elems").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  return static_cast<starlark_string*>(this_obj)->elems(ctx);
}

starlark_obj* starlark_string_fn_elem_ords(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_arg(pos_args, named_args, error_callback, "string.elem_ords").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  return static_cast<starlark_string*>(this_obj)->elem_ords(ctx);
}

starlark_obj* starlark_string_fn_codepoints(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_arg(pos_args, named_args, error_callback, "string.codepoints").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  return static_cast<starlark_string*>(this_obj)->codepoints(ctx);
}

starlark_obj* starlark_string_fn_codepoint_ords(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_arg(pos_args, named_args, error_callback, "string.codepoint_ords").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  return static_cast<starlark_string*>(this_obj)->codepoint_ords(ctx);
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
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  return static_cast<starlark_string*>(this_obj)->format(pos_args, named_args, ctx, error_callback);
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

  auto it = pos_args.front()->get_iterator(false, ctx, error_callback);
  if (it == nullptr) {
    error_callback.add_error(error_can_only_join_on_iterable());
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
  if (!zero_or_one_pos_arg(pos_args, named_args, error_callback, "lstrip").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  if (pos_args.empty()) {
    return static_cast<starlark_string*>(this_obj)->lstrip(ctx);
  }
  auto cutset = string_as_string(pos_args.front(), "lstrip", 1, error_callback);
  if (!cutset.ok()) {
    return nullptr;
  }
  return static_cast<starlark_string*>(this_obj)->lstrip(*cutset, ctx);
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
  if (!zero_or_one_pos_arg(pos_args, named_args, error_callback, "rstrip").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  if (pos_args.empty()) {
    return static_cast<starlark_string*>(this_obj)->rstrip(ctx);
  }
  auto cutset = string_as_string(pos_args.front(), "rstrip", 1, error_callback);
  if (!cutset.ok()) {
    return nullptr;
  }
  return static_cast<starlark_string*>(this_obj)->rstrip(*cutset, ctx);
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
  if (!zero_or_one_pos_arg(pos_args, named_args, error_callback, "splitlines").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  bool keepends = false;
  if (!pos_args.empty()) {
    if (pos_args.front()->type() != starlark_types::bool_t) {
      error_callback.add_error(error_argument_must_be_type("splitlines", 1, starlark_types::bool_t, pos_args.front()->type()));
      return nullptr;
    }
    keepends = pos_args.front()->truthy();
  }
  return static_cast<starlark_string*>(this_obj)->splitlines(keepends, ctx);
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
  if (!zero_or_one_pos_arg(pos_args, named_args, error_callback, "strip").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::string_t);
  if (pos_args.empty()) {
    return static_cast<starlark_string*>(this_obj)->strip(ctx);
  }
  auto cutset = string_as_string(pos_args.front(), "strip", 1, error_callback);
  if (!cutset.ok()) {
    return nullptr;
  }
  return static_cast<starlark_string*>(this_obj)->strip(*cutset, ctx);
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

