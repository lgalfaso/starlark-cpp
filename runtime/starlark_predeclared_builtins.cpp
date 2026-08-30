// Copyright 2026 Lucas Mirelmann

#include "runtime/builtin_pos.hpp"

#include <algorithm>
#include <cmath>
#include <cerrno>
#include <cstdlib>
#include <limits>
#include <string>
#include <vector>

#include "errors/runtime_error_messages.hpp"
#include "runtime/starlark_bool.hpp"
#include "runtime/starlark_bytes.hpp"
#include "runtime/starlark_dictionary.hpp"
#include "runtime/starlark_function.hpp"
#include "runtime/starlark_list.hpp"
#include "runtime/starlark_numeric.hpp"
#include "runtime/starlark_range.hpp"
#include "runtime/starlark_set.hpp"
#include "runtime/starlark_string.hpp"
#include "runtime/starlark_tuple.hpp"
#include "runtime/starlark_types.hpp"
#include "unicode/encode.hpp"
#include "unicode/utf8_reader.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::parse_number;
using ::starlark::error_messages::error_v2_argument_bad_operand_type;
using ::starlark::error_messages::error_v2_argument_interpreted_as_integer;
using ::starlark::error_messages::error_v2_argument_non_zero;
using ::starlark::error_messages::error_v2_argument_string_int_bool_or_real;
using ::starlark::error_messages::error_v2_argument_string_or_real;
using ::starlark::error_messages::error_v2_attribute_string;
using ::starlark::error_messages::error_v2_bytes_in_range;
using ::starlark::error_messages::error_v2_convert;
using ::starlark::error_messages::error_v2_convert_string;
using ::starlark::error_messages::error_v2_empty_iterator;
using ::starlark::error_messages::error_v2_expect_character;
using ::starlark::error_messages::error_v2_expect_one_character_or_one_byte;
using ::starlark::error_messages::error_v2_int_base;
using ::starlark::error_messages::error_v2_interpreted_as_integer;
using ::starlark::error_messages::error_v2_invalid_literal_with_base;
using ::starlark::error_messages::error_v2_non_string_with_base;
using ::starlark::error_messages::error_v2_max_bytes_length;
using ::starlark::error_messages::error_v2_overflow;
using ::starlark::error_messages::error_v2_overflow_float_too_large;
using ::starlark::unicode::utf8_encode_code_point;
using ::starlark::unicode::utf8_reader;
using ::starlark::result::error_status;
using ::starlark::result::ok_status;
using ::starlark::result::status;

namespace starlark {
namespace runtime {
namespace {

status read_range_int64(starlark_obj* value, int64_t& output, error_fn& error_callback) {
  switch (value->numeric_type()) {
    case starlark_numeric_type::kInt64:
      output = value->as_int64();
      return ok_status();
    case starlark_numeric_type::kBigInt: {
      const auto& bvalue = value->as_bigint();
      if (!bvalue.fits_in_int64()) {
        error_callback.add_error(error_v2_overflow(value->type(), starlark_types::int64));
        return error_status();
      }
      output = bvalue.as_int64();
      return ok_status();
    }
    default:
      error_callback.add_error(error_v2_interpreted_as_integer(value->type()));
      return error_status();
  }
}

starlark_obj* make_range_int64(int64_t start, int64_t end, int64_t step, context& ctx, error_fn& error_callback) {
  auto* result = Arena::Create<starlark_range>(&ctx.arena(), start, end, step);
  if (!result->valid()) {
    error_callback.add_error(error_v2_overflow(starlark_types::int_t, starlark_types::int64));
    return nullptr;
  }
  return result;
}

starlark_obj* enumerate_from(starlark_obj* iterable, starlark_obj* start, context& ctx, error_fn& error_callback) {
  if (start != nullptr && start->type() != starlark_types::int_t) {
    error_callback.add_error(error_v2_argument_interpreted_as_integer("start", start->type()));
    return nullptr;
  }
  auto* it = iterable->get_iterator(true, ctx, error_callback);
  if (it == nullptr) {
    return nullptr;
  }
  auto* result = Arena::Create<starlark_list>(&ctx.arena(), std::max<int64_t>(0, iterable->unsafe_len()));
  if (start == nullptr) {
    start = ctx.zero();
  }
  auto* one = ctx.one();
  while (it->has_next()) {
    auto* tuple = Arena::Create<starlark_tuple>(&ctx.arena(), 2);
    tuple->add(start);
    tuple->add(it->next());
    result->unsafe_append(tuple);
    start = start->binary_plus(*one, ctx, error_callback);
  }
  it->end_iterator();
  return result;
}

starlark_obj* bytes_from_source(starlark_obj* source, context& ctx, error_fn& error_callback) {
  if (source->type() == starlark_types::bytes_t) {
    return source;
  }
  if (source->type() == starlark_types::string_t) {
    std::string result;
    utf8_reader reader(source->as_string(), false, false);
    while (reader.pending()) {
      utf8_encode_code_point(reader.read_code_point(), result, false, true);
    }
    if (result.length() > ctx.options().max_string_length) {
      error_callback.add_error(error_v2_max_bytes_length(ctx.options().max_string_length));
      return nullptr;
    }
    return Arena::Create<starlark_bytes>(&ctx.arena(), result);
  }
  auto* it = source->get_iterator(false, ctx, error_callback);
  if (it == nullptr) {
    error_callback.add_error(error_v2_convert(source->type(), starlark_types::bytes_t));
    return nullptr;
  }
  if (source->unsafe_len() > ctx.options().max_string_length) {
    error_callback.add_error(error_v2_max_bytes_length(ctx.options().max_string_length));
    return nullptr;
  }
  std::string result;
  while (it->has_next()) {
    auto value = it->next();
    switch (value->numeric_type()) {
      case starlark_numeric_type::kInt64: {
        auto ivalue = value->as_int64();
        if (ivalue < 0 || 255 < ivalue) {
          error_callback.add_error(error_v2_bytes_in_range());
          return nullptr;
        }
        result += static_cast<char>(ivalue);
        break;
      }
      case starlark_numeric_type::kBigInt: {
        const auto& bvalue = value->as_bigint();
        if (bvalue.sign() || bvalue.bit_size() > 8) {
          error_callback.add_error(error_v2_bytes_in_range());
          return nullptr;
        }
        result += static_cast<char>(bvalue.at(0));
        break;
      }
      default:
        error_callback.add_error(error_v2_interpreted_as_integer(value->type()));
        return nullptr;
    }
  }
  it->end_iterator();
  return Arena::Create<starlark_bytes>(&ctx.arena(), result);
}

starlark_obj* int_from_value(starlark_obj* value, starlark_obj* base_param, context& ctx, error_fn& error_callback) {
  if (value->type() != starlark_types::string_t && base_param != nullptr) {
    error_callback.add_error(error_v2_non_string_with_base());
    return nullptr;
  }
  if (value->type() == starlark_types::int_t) {
    return value;
  }
  if (value->type() == starlark_types::float_t) {
    return create_integer_from_float(value->as_float(), ctx, error_callback);
  }
  if (value->type() == starlark_types::bool_t) {
    return value->truthy() ? ctx.one() : ctx.zero();
  }
  if (value->type() == starlark_types::string_t) {
    int base = 10;
    if (base_param != nullptr) {
      switch (base_param->numeric_type()) {
        case starlark_numeric_type::kInt64: {
          auto ibase = base_param->as_int64();
          if (ibase != 0 && !(2 <= ibase && ibase <= 36)) {
            error_callback.add_error(error_v2_int_base(starlark_built_in_functions::int_f));
            return nullptr;
          }
          base = ibase;
          break;
        }
        case starlark_numeric_type::kBigInt: {
          const auto& bbase = base_param->as_bigint();
          if (bbase.sign() || bbase.length() > 1) {
            error_callback.add_error(error_v2_int_base(starlark_built_in_functions::int_f));
            return nullptr;
          }
          auto ibase = bbase.at(0);
          if (ibase != 0 && !(2 <= ibase && ibase <= 36)) {
            error_callback.add_error(error_v2_int_base(starlark_built_in_functions::int_f));
            return nullptr;
          }
          base = ibase;
          break;
        }
        default:
          error_callback.add_error(error_v2_interpreted_as_integer(base_param->type()));
          return nullptr;
      }
    }
    auto svalue = value->as_string();
    if (svalue.empty()) {
      error_callback.add_error(error_v2_invalid_literal_with_base(starlark_built_in_functions::int_f, base, svalue));
      return nullptr;
    }
    const char* end;
    auto result = parse_number(svalue, &end, base);
    if (end != (&svalue.back() + 1)) {
      error_callback.add_error(error_v2_invalid_literal_with_base(starlark_built_in_functions::int_f, base, svalue));
      return nullptr;
    }
    return create_integer(std::move(result), ctx);
  }
  error_callback.add_error(error_v2_argument_string_int_bool_or_real(starlark_built_in_functions::int_f, value->type()));
  return nullptr;
}

starlark_obj* max_from_iterable(starlark_obj* iterable, context& ctx, error_fn& error_callback) {
  auto* it = iterable->get_iterator(true, ctx, error_callback);
  if (it == nullptr) {
    return nullptr;
  }
  if (!it->has_next()) {
    it->end_iterator();
    error_callback.add_error(error_v2_empty_iterator(starlark_built_in_functions::max_f));
    return nullptr;
  }
  starlark_obj* candidate = it->next();
  while (it->has_next()) {
    auto* element = it->next();
    auto cmp = candidate->cmp(*element, "<", error_callback);
    if (!cmp.ok()) {
      return nullptr;
    }
    if (*cmp < 0) {
      candidate = element;
    }
  }
  it->end_iterator();
  return candidate;
}

starlark_obj* min_from_iterable(starlark_obj* iterable, context& ctx, error_fn& error_callback) {
  auto* it = iterable->get_iterator(true, ctx, error_callback);
  if (it == nullptr) {
    return nullptr;
  }
  if (!it->has_next()) {
    it->end_iterator();
    error_callback.add_error(error_v2_empty_iterator(starlark_built_in_functions::min_f));
    return nullptr;
  }
  starlark_obj* candidate = it->next();
  while (it->has_next()) {
    auto* element = it->next();
    auto cmp = candidate->cmp(*element, "<", error_callback);
    if (!cmp.ok()) {
      return nullptr;
    }
    if (*cmp > 0) {
      candidate = element;
    }
  }
  it->end_iterator();
  return candidate;
}

starlark_obj* max_from_args(std::initializer_list<starlark_obj*> args, error_fn& error_callback) {
  auto it = args.begin();
  starlark_obj* candidate = *it;
  for (++it; it != args.end(); ++it) {
    auto cmp = candidate->cmp(**it, "<", error_callback);
    if (!cmp.ok()) {
      return nullptr;
    }
    if (*cmp < 0) {
      candidate = *it;
    }
  }
  return candidate;
}

starlark_obj* min_from_args(std::initializer_list<starlark_obj*> args, error_fn& error_callback) {
  auto it = args.begin();
  starlark_obj* candidate = *it;
  for (++it; it != args.end(); ++it) {
    auto cmp = candidate->cmp(**it, "<", error_callback);
    if (!cmp.ok()) {
      return nullptr;
    }
    if (*cmp > 0) {
      candidate = *it;
    }
  }
  return candidate;
}

starlark_obj* zip_from(std::span<starlark_obj*> pos_args, context& ctx, error_fn& error_callback) {
  auto all_available = [](const std::vector<starlark_iterator*>& its) -> bool {
    for (const auto* it : its) {
      if (!it->has_next()) {
        return false;
      }
    }
    return true;
  };

  if (pos_args.empty()) {
    return Arena::Create<starlark_list>(&ctx.arena(), 0);
  }
  auto len = std::numeric_limits<int64_t>::max();
  std::vector<starlark_iterator*> its;
  its.reserve(pos_args.size());
  for (auto* element : pos_args) {
    auto* it = element->get_iterator(true, ctx, error_callback);
    if (it == nullptr) {
      for (auto& iit : its) {
        iit->end_iterator();
      }
      return nullptr;
    }
    its.push_back(it);
    len = std::min<int64_t>(len, element->unsafe_len());
  }
  starlark_list* result = Arena::Create<starlark_list>(&ctx.arena(), std::max<int64_t>(0, len));
  while (all_available(its)) {
    auto* tuple = Arena::Create<starlark_tuple>(&ctx.arena(), its.size());
    for (auto* it : its) {
      tuple->add(it->next());
    }
    result->unsafe_append(tuple);
  }
  for (auto* it : its) {
    it->end_iterator();
  }
  return result;
}

}  // namespace

namespace builtin_pos {

starlark_obj* abs_pos1(starlark_obj* this_obj, starlark_obj* value, context& ctx, error_fn& error_callback) {
  switch (value->numeric_type()) {
    case starlark_numeric_type::kFloat: {
      if (std::signbit(value->as_float())) {
        return create_float(std::abs(value->as_float()), ctx);
      }
      return value;
    }
    case starlark_numeric_type::kInt64: {
      int64_t ivalue = value->as_int64();
      if (ivalue < 0) {
        return value->unary_minus(ctx, error_callback);
      }
      return value;
    }
    case starlark_numeric_type::kBigInt: {
      const auto& bvalue = value->as_bigint();
      if (bvalue.sign()) {
        return value->unary_minus(ctx, error_callback);
      }
      return value;
    }
    default:
      error_callback.add_error(error_v2_argument_bad_operand_type(starlark_built_in_functions::abs_f, value->type(), starlark_types::int_t, starlark_types::float_t));
      return nullptr;
  }
}

starlark_obj* all_pos1(starlark_obj* this_obj, starlark_obj* iterable, context& ctx, error_fn& error_callback) {
  auto* it = iterable->get_iterator(true, ctx, error_callback);
  if (it == nullptr) {
    return nullptr;
  }
  bool result = true;
  while (result && it->has_next()) {
    result = it->next()->truthy();
  }
  it->end_iterator();
  return result ? ctx.true_value() : ctx.false_value();
}

starlark_obj* any_pos1(starlark_obj* this_obj, starlark_obj* iterable, context& ctx, error_fn& error_callback) {
  auto* it = iterable->get_iterator(true, ctx, error_callback);
  if (it == nullptr) {
    return nullptr;
  }
  bool result = false;
  while (!result && it->has_next()) {
    result = it->next()->truthy();
  }
  it->end_iterator();
  return result ? ctx.true_value() : ctx.false_value();
}

starlark_obj* bool_pos0(starlark_obj* this_obj, context& ctx, error_fn& error_callback) {
  return ctx.false_value();
}

starlark_obj* bool_pos1(starlark_obj* this_obj, starlark_obj* value, context& ctx, error_fn& error_callback) {
  return value->truthy() ? ctx.true_value() : ctx.false_value();
}

starlark_obj* bytes_pos0(starlark_obj* this_obj, context& ctx, error_fn& error_callback) {
  return ctx.empty_bytes();
}

starlark_obj* bytes_pos1(starlark_obj* this_obj, starlark_obj* source, context& ctx, error_fn& error_callback) {
  return bytes_from_source(source, ctx, error_callback);
}

starlark_obj* chr_pos1(starlark_obj* this_obj, starlark_obj* value, context& ctx, error_fn& error_callback) {
  std::string result;
  switch (value->numeric_type()) {
    case starlark_numeric_type::kInt64:
      if (!chr_fn(result, value->as_int64(), error_callback).ok()) {
        return nullptr;
      }
      break;
    case starlark_numeric_type::kBigInt:
      if (!chr_fn(result, value->as_bigint(), error_callback).ok()) {
        return nullptr;
      }
      break;
    default:
      error_callback.add_error(error_v2_interpreted_as_integer(value->type()));
      return nullptr;
  }
  return Arena::Create<starlark_string>(&ctx.arena(), result);
}

starlark_obj* dict_pos0(starlark_obj* this_obj, context& ctx, error_fn& error_callback) {
  return Arena::Create<starlark_dictionary>(&ctx.arena());
}

starlark_obj* dict_pos1(starlark_obj* this_obj, starlark_obj* iterable, context& ctx, error_fn& error_callback) {
  starlark_dictionary* result = Arena::Create<starlark_dictionary>(&ctx.arena());
  starlark_obj::named_args_t named_args;
  if (!result->update(iterable, named_args, ctx, error_callback).ok()) {
    return nullptr;
  }
  return result;
}

starlark_obj* dir_pos1(starlark_obj* this_obj, starlark_obj* value, context& ctx, error_fn& error_callback) {
  const auto& attributes = value->dir();
  auto* result = Arena::Create<starlark_list>(&ctx.arena(), attributes.size());
  for (const auto& attribute : attributes) {
    result->unsafe_append(Arena::Create<starlark_string>(&ctx.arena(), attribute));
  }
  return result;
}

starlark_obj* enumerate_pos1(starlark_obj* this_obj, starlark_obj* iterable, context& ctx, error_fn& error_callback) {
  return enumerate_from(iterable, nullptr, ctx, error_callback);
}

starlark_obj* enumerate_pos2(starlark_obj* this_obj, starlark_obj* iterable, starlark_obj* start, context& ctx, error_fn& error_callback) {
  return enumerate_from(iterable, start, ctx, error_callback);
}

starlark_obj* float_pos0(starlark_obj* this_obj, context& ctx, error_fn& error_callback) {
  return create_float(0.0, ctx);
}

starlark_obj* float_pos1(starlark_obj* this_obj, starlark_obj* value, context& ctx, error_fn& error_callback) {
  switch (value->numeric_type()) {
    case starlark_numeric_type::kFloat:
      return value;
    case starlark_numeric_type::kInt64:
      return create_float(value->as_int64(), ctx);
    case starlark_numeric_type::kBigInt: {
      auto fvalue = to_double(value->as_bigint());
      if (std::isinf(fvalue)) {
        error_callback.add_error(error_v2_overflow(value->type(), starlark_types::float_t));
        return nullptr;
      }
      return create_float(fvalue, ctx);
    }
    case starlark_numeric_type::kNotNumeric:
    default:
      if (value->type() == starlark_types::string_t) {
        auto svalue = value->as_string();
        errno = 0;
        char* end;
        double double_value = std::strtod(svalue.data(), &end);
        if (end != &svalue.back() + 1) {
          error_callback.add_error(error_v2_convert_string(starlark_types::float_t, svalue));
          return nullptr;
        }
        if (errno != 0) {
          error_callback.add_error(error_v2_overflow_float_too_large());
          return nullptr;
        }
        return create_float(double_value, ctx);
      }
      if (value->type() == starlark_types::bool_t) {
        return create_float(value->truthy() ? 1.0 : 0.0, ctx);
      }
      error_callback.add_error(error_v2_argument_string_or_real(starlark_built_in_functions::float_f, value->type()));
      return nullptr;
  }
}

starlark_obj* getattr_pos2(starlark_obj* this_obj, starlark_obj* element, starlark_obj* name, context& ctx, error_fn& error_callback) {
  if (name->type() != starlark_types::string_t) {
    error_callback.add_error(error_v2_attribute_string(name->type()));
    return nullptr;
  }
  return element->get_attr(true, name->as_string(), ctx, error_callback);
}

starlark_obj* getattr_pos3(starlark_obj* this_obj, starlark_obj* element, starlark_obj* name, starlark_obj* default_value, context& ctx, error_fn& error_callback) {
  if (name->type() != starlark_types::string_t) {
    error_callback.add_error(error_v2_attribute_string(name->type()));
    return nullptr;
  }
  auto* result = element->get_attr(false, name->as_string(), ctx, error_callback);
  if (result == nullptr) {
    return default_value;
  }
  return result;
}

starlark_obj* hasattr_pos2(starlark_obj* this_obj, starlark_obj* element, starlark_obj* attr, context& ctx, error_fn& error_callback) {
  if (attr->type() != starlark_types::string_t) {
    error_callback.add_error(error_v2_attribute_string(attr->type()));
    return nullptr;
  }
  const auto& attributes = element->dir();
  auto it = std::lower_bound(attributes.begin(), attributes.end(), attr->as_string());
  if (it == attributes.end() || *it != attr->as_string()) {
    return ctx.false_value();
  }
  return ctx.true_value();
}

starlark_obj* hash_pos1(starlark_obj* this_obj, starlark_obj* value, context& ctx, error_fn& error_callback) {
  if (value->type() == starlark_types::string_t || value->type() == starlark_types::bytes_t) {
    return create_integer(value->hash(), ctx);
  }
  error_callback.add_error(error_v2_argument_bad_operand_type(starlark_built_in_functions::hash_f, value->type(), starlark_types::string_t, starlark_types::bytes_t));
  return nullptr;
}

starlark_obj* int_pos0(starlark_obj* this_obj, context& ctx, error_fn& error_callback) {
  return ctx.zero();
}

starlark_obj* int_pos1(starlark_obj* this_obj, starlark_obj* value, context& ctx, error_fn& error_callback) {
  return int_from_value(value, nullptr, ctx, error_callback);
}

starlark_obj* int_pos2(starlark_obj* this_obj, starlark_obj* value, starlark_obj* base, context& ctx, error_fn& error_callback) {
  return int_from_value(value, base, ctx, error_callback);
}

starlark_obj* len_pos1(starlark_obj* this_obj, starlark_obj* obj, context& ctx, error_fn& error_callback) {
  auto result = obj->len(error_callback);
  if (result < 0) {
    return nullptr;
  }
  return create_integer(result, ctx);
}

starlark_obj* list_pos0(starlark_obj* this_obj, context& ctx, error_fn& error_callback) {
  return Arena::Create<starlark_list>(&ctx.arena(), 0);
}

starlark_obj* list_pos1(starlark_obj* this_obj, starlark_obj* iterable, context& ctx, error_fn& error_callback) {
  auto* it = iterable->get_iterator(true, ctx, error_callback);
  if (it == nullptr) {
    return nullptr;
  }
  auto* result = Arena::Create<starlark_list>(&ctx.arena(), std::max<int64_t>(0, iterable->unsafe_len()));
  while (it->has_next()) {
    result->unsafe_append(it->next());
  }
  it->end_iterator();
  return result;
}

starlark_obj* max_pos1(starlark_obj* this_obj, starlark_obj* iterable, context& ctx, error_fn& error_callback) {
  return max_from_iterable(iterable, ctx, error_callback);
}

starlark_obj* max_pos2(starlark_obj* this_obj, starlark_obj* a0, starlark_obj* a1, context& ctx, error_fn& error_callback) {
  return max_from_args({a0, a1}, error_callback);
}

starlark_obj* max_pos3(starlark_obj* this_obj, starlark_obj* a0, starlark_obj* a1, starlark_obj* a2, context& ctx, error_fn& error_callback) {
  return max_from_args({a0, a1, a2}, error_callback);
}

starlark_obj* min_pos1(starlark_obj* this_obj, starlark_obj* iterable, context& ctx, error_fn& error_callback) {
  return min_from_iterable(iterable, ctx, error_callback);
}

starlark_obj* min_pos2(starlark_obj* this_obj, starlark_obj* a0, starlark_obj* a1, context& ctx, error_fn& error_callback) {
  return min_from_args({a0, a1}, error_callback);
}

starlark_obj* min_pos3(starlark_obj* this_obj, starlark_obj* a0, starlark_obj* a1, starlark_obj* a2, context& ctx, error_fn& error_callback) {
  return min_from_args({a0, a1, a2}, error_callback);
}

starlark_obj* ord_pos1(starlark_obj* this_obj, starlark_obj* value, context& ctx, error_fn& error_callback) {
  if (value->type() == starlark_types::string_t) {
    utf8_reader reader(value->as_string(), false, false);
    if (!reader.pending()) {
      error_callback.add_error(error_v2_expect_character(starlark_built_in_functions::ord_f, value->type(), value->unsafe_len()));
      return nullptr;
    }
    auto result = reader.read_code_point();
    if (reader.pending()) {
      error_callback.add_error(error_v2_expect_character(starlark_built_in_functions::ord_f, value->type(), value->unsafe_len()));
      return nullptr;
    }
    return create_integer(result, ctx);
  }
  if (value->type() == starlark_types::bytes_t) {
    if (auto value_len = value->unsafe_len(); value_len != 1) {
      error_callback.add_error(error_v2_expect_character(starlark_built_in_functions::ord_f, value->type(), value_len));
      return nullptr;
    }
    return create_integer(static_cast<unsigned char>(value->as_string()[0]), ctx);
  }
  error_callback.add_error(error_v2_expect_one_character_or_one_byte(starlark_built_in_functions::ord_f, value->type()));
  return nullptr;
}

starlark_obj* print_pos0(starlark_obj* this_obj, context& ctx, error_fn& error_callback) {
  ctx.options().out << "\n";
  return ctx.none_value();
}

starlark_obj* print_pos1(starlark_obj* this_obj, starlark_obj* value, context& ctx, error_fn& error_callback) {
  ctx.options().out << value->str() << "\n";
  return ctx.none_value();
}

starlark_obj* range_pos1(starlark_obj* this_obj, starlark_obj* end, context& ctx, error_fn& error_callback) {
  int64_t end_val;
  if (!read_range_int64(end, end_val, error_callback).ok()) {
    return nullptr;
  }
  return make_range_int64(0, end_val, 1, ctx, error_callback);
}

starlark_obj* range_pos2(starlark_obj* this_obj, starlark_obj* start, starlark_obj* end, context& ctx, error_fn& error_callback) {
  int64_t start_val;
  int64_t end_val;
  if (!read_range_int64(start, start_val, error_callback).ok() ||
      !read_range_int64(end, end_val, error_callback).ok()) {
    return nullptr;
  }
  return make_range_int64(start_val, end_val, 1, ctx, error_callback);
}

starlark_obj* range_pos3(starlark_obj* this_obj, starlark_obj* start, starlark_obj* end, starlark_obj* step, context& ctx, error_fn& error_callback) {
  int64_t start_val;
  int64_t end_val;
  int64_t step_val;
  if (!read_range_int64(start, start_val, error_callback).ok() ||
      !read_range_int64(end, end_val, error_callback).ok() ||
      !read_range_int64(step, step_val, error_callback).ok()) {
    return nullptr;
  }
  if (step_val == 0) {
    error_callback.add_error(error_v2_argument_non_zero(starlark_built_in_functions::range_f, 3));
    return nullptr;
  }
  return make_range_int64(start_val, end_val, step_val, ctx, error_callback);
}

starlark_obj* repr_pos1(starlark_obj* this_obj, starlark_obj* value, context& ctx, error_fn& error_callback) {
  return Arena::Create<starlark_string>(&ctx.arena(), value->repr());
}

starlark_obj* reversed_pos1(starlark_obj* this_obj, starlark_obj* iterable, context& ctx, error_fn& error_callback) {
  auto* it = iterable->get_iterator(true, ctx, error_callback);
  if (it == nullptr) {
    return nullptr;
  }
  std::vector<starlark_obj*> elements;
  while (it->has_next()) {
    elements.push_back(it->next());
  }
  it->end_iterator();
  std::reverse(elements.begin(), elements.end());
  auto* result = Arena::Create<starlark_list>(&ctx.arena(), elements.size());
  for (auto* element : elements) {
    result->unsafe_append(element);
  }
  return result;
}

starlark_obj* set_pos0(starlark_obj* this_obj, context& ctx, error_fn& error_callback) {
  return Arena::Create<starlark_set>(&ctx.arena());
}

starlark_obj* set_pos1(starlark_obj* this_obj, starlark_obj* iterable, context& ctx, error_fn& error_callback) {
  auto* it = iterable->get_iterator(true, ctx, error_callback);
  if (it == nullptr) {
    return nullptr;
  }
  auto* result = Arena::Create<starlark_set>(&ctx.arena());
  while (it->has_next()) {
    if (!result->add(it->next(), error_callback).ok()) {
      return nullptr;
    }
  }
  it->end_iterator();
  return result;
}

starlark_obj* sorted_pos1(starlark_obj* this_obj, starlark_obj* iterable, context& ctx, error_fn& error_callback) {
  std::vector<std::pair<starlark_obj*, starlark_obj*>> elems;
  auto* it = iterable->get_iterator(true, ctx, error_callback);
  if (it == nullptr) {
    return nullptr;
  }
  while (it->has_next()) {
    starlark_obj* element = it->next();
    elems.emplace_back(element, element);
  }
  it->end_iterator();

  bool found_error = false;
  std::stable_sort(elems.begin(), elems.end(), [&error_callback, &found_error](const std::pair<starlark_obj*, starlark_obj*>& lhs, const std::pair<starlark_obj*, starlark_obj*>& rhs) -> bool {
    if (found_error) {
      return false;
    }
    auto cmp = lhs.first->cmp(*rhs.first, "<", error_callback);
    if (!cmp.ok()) {
      found_error = true;
      return false;
    }
    return *cmp < 0;
  });
  if (found_error) {
    return nullptr;
  }

  auto* result = Arena::Create<starlark_list>(&ctx.arena(), elems.size());
  for (auto& elem : elems) {
    result->unsafe_append(elem.second);
  }
  return result;
}

starlark_obj* str_pos0(starlark_obj* this_obj, context& ctx, error_fn& error_callback) {
  return ctx.empty_string();
}

starlark_obj* str_pos1(starlark_obj* this_obj, starlark_obj* object, context& ctx, error_fn& error_callback) {
  return Arena::Create<starlark_string>(&ctx.arena(), object->str());
}

starlark_obj* tuple_pos0(starlark_obj* this_obj, context& ctx, error_fn& error_callback) {
  return Arena::Create<starlark_tuple>(&ctx.arena(), 0);
}

starlark_obj* tuple_pos1(starlark_obj* this_obj, starlark_obj* iterable, context& ctx, error_fn& error_callback) {
  auto* it = iterable->get_iterator(true, ctx, error_callback);
  if (it == nullptr) {
    return nullptr;
  }
  auto* result = Arena::Create<starlark_tuple>(&ctx.arena(), std::max<int64_t>(0, iterable->unsafe_len()));
  while (it->has_next()) {
    result->add(it->next());
  }
  it->end_iterator();
  return result;
}

starlark_obj* type_pos1(starlark_obj* this_obj, starlark_obj* value, context& ctx, error_fn& error_callback) {
  return Arena::Create<starlark_string>(&ctx.arena(), value->type());
}

starlark_obj* zip_pos0(starlark_obj* this_obj, context& ctx, error_fn& error_callback) {
  return Arena::Create<starlark_list>(&ctx.arena(), 0);
}

starlark_obj* zip_pos1(starlark_obj* this_obj, starlark_obj* a0, context& ctx, error_fn& error_callback) {
  starlark_obj* args[] = {a0};
  return zip_from(args, ctx, error_callback);
}

starlark_obj* zip_pos2(starlark_obj* this_obj, starlark_obj* a0, starlark_obj* a1, context& ctx, error_fn& error_callback) {
  starlark_obj* args[] = {a0, a1};
  return zip_from(args, ctx, error_callback);
}

starlark_obj* zip_pos3(starlark_obj* this_obj, starlark_obj* a0, starlark_obj* a1, starlark_obj* a2, context& ctx, error_fn& error_callback) {
  starlark_obj* args[] = {a0, a1, a2};
  return zip_from(args, ctx, error_callback);
}

}  // namespace builtin_pos

void add_predeclared_builtins(std::map<std::string, starlark_obj*, std::less<>>& global_context, context& ctx) {
  global_context[starlark_built_in_functions::abs_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_abs,
    .pos1 = builtin_pos::abs_pos1,
  }, starlark_built_in_functions::abs_f);
  global_context[starlark_built_in_functions::all_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_all,
    .pos1 = builtin_pos::all_pos1,
  }, starlark_built_in_functions::all_f);
  global_context[starlark_built_in_functions::any_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_any,
    .pos1 = builtin_pos::any_pos1,
  }, starlark_built_in_functions::any_f);
  global_context[starlark_built_in_functions::bool_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_bool,
    .pos0 = builtin_pos::bool_pos0,
    .pos1 = builtin_pos::bool_pos1,
  }, starlark_built_in_functions::bool_f);
  global_context[starlark_built_in_functions::bytes_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_bytes,
    .pos0 = builtin_pos::bytes_pos0,
    .pos1 = builtin_pos::bytes_pos1,
  }, starlark_built_in_functions::bytes_f);
  global_context[starlark_built_in_functions::chr_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_chr,
    .pos1 = builtin_pos::chr_pos1,
  }, starlark_built_in_functions::chr_f);
  global_context[starlark_built_in_functions::dict_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_dict,
    .pos0 = builtin_pos::dict_pos0,
    .pos1 = builtin_pos::dict_pos1,
  }, starlark_built_in_functions::dict_f);
  global_context[starlark_built_in_functions::dir_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_dir,
    .pos1 = builtin_pos::dir_pos1,
  }, starlark_built_in_functions::dir_f);
  global_context[starlark_built_in_functions::enumerate_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_enumerate,
    .pos1 = builtin_pos::enumerate_pos1,
    .pos2 = builtin_pos::enumerate_pos2,
  }, starlark_built_in_functions::enumerate_f);
  global_context[starlark_built_in_functions::fail_f] = create_function(ctx, nullptr, starlark_fn_fail, starlark_built_in_functions::fail_f);
  global_context[starlark_built_in_functions::float_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_float,
    .pos0 = builtin_pos::float_pos0,
    .pos1 = builtin_pos::float_pos1,
  }, starlark_built_in_functions::float_f);
  global_context[starlark_built_in_functions::getattr_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_getattr,
    .pos2 = builtin_pos::getattr_pos2,
    .pos3 = builtin_pos::getattr_pos3,
  }, starlark_built_in_functions::getattr_f);
  global_context[starlark_built_in_functions::hasattr_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_hasattr,
    .pos2 = builtin_pos::hasattr_pos2,
  }, starlark_built_in_functions::hasattr_f);
  global_context[starlark_built_in_functions::hash_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_hash,
    .pos1 = builtin_pos::hash_pos1,
  }, starlark_built_in_functions::hash_f);
  global_context[starlark_built_in_functions::int_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_int,
    .pos0 = builtin_pos::int_pos0,
    .pos1 = builtin_pos::int_pos1,
    .pos2 = builtin_pos::int_pos2,
  }, starlark_built_in_functions::int_f);
  global_context[starlark_built_in_functions::len_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_len,
    .pos1 = builtin_pos::len_pos1,
  }, starlark_built_in_functions::len_f);
  global_context[starlark_built_in_functions::list_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_list,
    .pos0 = builtin_pos::list_pos0,
    .pos1 = builtin_pos::list_pos1,
  }, starlark_built_in_functions::list_f);
  global_context[starlark_built_in_functions::max_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_max,
    .pos1 = builtin_pos::max_pos1,
    .pos2 = builtin_pos::max_pos2,
    .pos3 = builtin_pos::max_pos3,
  }, starlark_built_in_functions::max_f);
  global_context[starlark_built_in_functions::min_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_min,
    .pos1 = builtin_pos::min_pos1,
    .pos2 = builtin_pos::min_pos2,
    .pos3 = builtin_pos::min_pos3,
  }, starlark_built_in_functions::min_f);
  global_context[starlark_built_in_functions::ord_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_ord,
    .pos1 = builtin_pos::ord_pos1,
  }, starlark_built_in_functions::ord_f);
  global_context[starlark_built_in_functions::print_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_print,
    .pos0 = builtin_pos::print_pos0,
    .pos1 = builtin_pos::print_pos1,
  }, starlark_built_in_functions::print_f);
  global_context[starlark_built_in_functions::range_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_range,
    .pos1 = builtin_pos::range_pos1,
    .pos2 = builtin_pos::range_pos2,
    .pos3 = builtin_pos::range_pos3,
  }, starlark_built_in_functions::range_f);
  global_context[starlark_built_in_functions::repr_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_repr,
    .pos1 = builtin_pos::repr_pos1,
  }, starlark_built_in_functions::repr_f);
  global_context[starlark_built_in_functions::reversed_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_reversed,
    .pos1 = builtin_pos::reversed_pos1,
  }, starlark_built_in_functions::reversed_f);
  global_context[starlark_built_in_functions::set_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_set,
    .pos0 = builtin_pos::set_pos0,
    .pos1 = builtin_pos::set_pos1,
  }, starlark_built_in_functions::set_f);
  global_context[starlark_built_in_functions::sorted_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_sorted,
    .pos1 = builtin_pos::sorted_pos1,
  }, starlark_built_in_functions::sorted_f);
  global_context[starlark_built_in_functions::str_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_str,
    .pos0 = builtin_pos::str_pos0,
    .pos1 = builtin_pos::str_pos1,
  }, starlark_built_in_functions::str_f);
  global_context[starlark_built_in_functions::tuple_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_tuple,
    .pos0 = builtin_pos::tuple_pos0,
    .pos1 = builtin_pos::tuple_pos1,
  }, starlark_built_in_functions::tuple_f);
  global_context[starlark_built_in_functions::type_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_type,
    .pos1 = builtin_pos::type_pos1,
  }, starlark_built_in_functions::type_f);
  global_context[starlark_built_in_functions::zip_f] = create_function(ctx, nullptr, builtin_entrypoints{
    .call = starlark_fn_zip,
    .pos0 = builtin_pos::zip_pos0,
    .pos1 = builtin_pos::zip_pos1,
    .pos2 = builtin_pos::zip_pos2,
    .pos3 = builtin_pos::zip_pos3,
  }, starlark_built_in_functions::zip_f);
}

}  // namespace runtime
}  // namespace starlark
