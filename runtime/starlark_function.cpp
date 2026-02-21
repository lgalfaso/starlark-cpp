// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_function.hpp"

#include <algorithm>
#include <limits>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "runtime/error_messages.hpp"
#include "runtime/siphash.hpp"
#include "runtime/starlark_bool.hpp"
#include "runtime/starlark_bytes.hpp"
#include "runtime/starlark_dictionary.hpp"
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
using ::starlark::unicode::utf8_encode_code_point;
using ::starlark::unicode::utf8_reader;

namespace starlark {
namespace runtime {

const char starlark_built_in_functions::abs_f[] = "abs";
const char starlark_built_in_functions::all_f[] = "all";
const char starlark_built_in_functions::any_f[] = "any";
const char starlark_built_in_functions::bool_f[] = "bool";
const char starlark_built_in_functions::bytes_f[] = "bytes";
const char starlark_built_in_functions::chr_f[] = "chr";
const char starlark_built_in_functions::dict_f[] = "dict";
const char starlark_built_in_functions::dir_f[] = "dir";
const char starlark_built_in_functions::enumerate_f[] = "enumerate";
const char starlark_built_in_functions::fail_f[] = "fail";
const char starlark_built_in_functions::float_f[] = "float";
const char starlark_built_in_functions::getattr_f[] = "getattr";
const char starlark_built_in_functions::hasattr_f[] = "hasattr";
const char starlark_built_in_functions::hash_f[] = "hash";
const char starlark_built_in_functions::int_f[] = "int";
const char starlark_built_in_functions::len_f[] = "len";
const char starlark_built_in_functions::list_f[] = "list";
const char starlark_built_in_functions::max_f[] = "max";
const char starlark_built_in_functions::min_f[] = "min";
const char starlark_built_in_functions::ord_f[] = "ord";
const char starlark_built_in_functions::print_f[] = "print";
const char starlark_built_in_functions::range_f[] = "range";
const char starlark_built_in_functions::repr_f[] = "repr";
const char starlark_built_in_functions::reversed_f[] = "reversed";
const char starlark_built_in_functions::set_f[] = "set";
const char starlark_built_in_functions::sorted_f[] = "sorted";
const char starlark_built_in_functions::str_f[] = "str";
const char starlark_built_in_functions::tuple_f[] = "tuple";
const char starlark_built_in_functions::type_f[] = "type";
const char starlark_built_in_functions::zip_f[] = "zip";

starlark_built_in_function::starlark_built_in_function(starlark_obj* this_obj, fn* native_fn, std::string_view fn_name) :
  this_obj(this_obj), native_fn(native_fn), fn_name(fn_name) {}

std::string_view starlark_built_in_function::type() const {
  return starlark_types::builtin_function_or_method_t;
}

bool starlark_built_in_function::inner_repr(printer& print, printer_action action) const {
  print.append(std::format("<built-in function {}>", fn_name));
  return false;
}

bool starlark_built_in_function::truthy() const {
  return true;
}

bool starlark_built_in_function::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
  if (other->type() != starlark_types::builtin_function_or_method_t) {
    return false;
  }
  const starlark_built_in_function* fother = static_cast<const starlark_built_in_function*>(other);
  if ((this_obj == nullptr) ^ (fother->this_obj == nullptr)) {
    return false;
  }
  if (this_obj != nullptr) {
    if (this_obj->primitive()) {
      comp.add_task(equals_comparator::pending_task{
        .lhs = this_obj,
        .rhs = fother->this_obj,
      });
    } else {
      if (this_obj != fother->this_obj) {
        return false;
      }
    }
  }
  return fn_name == fother->fn_name && native_fn == fother->native_fn;
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_built_in_function::inner_hash() const {
  if (fn_name.length() == 0) {
    return 0;
  }
  return static_cast<int64_t>(siphash(fn_name.data(), fn_name.length(), 0x452821E638D01377, 0xBE5466CF34E90C6C));
}

starlark_obj* starlark_built_in_function::call(const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args,
    context& ctx, error_fn& error_callback) {
  return native_fn(this_obj, pos_args, named_args, ctx, error_callback);
}

std::string_view starlark_function::type() const {
  return starlark_types::function_t;
}

bool starlark_function::inner_repr(printer& print, printer_action action) const {
  // TODO(lmirelmann): Replace `FUNCTION_NAME` and `MODULE` with the correct values. Eg:
  //     <function cc_fuzz_test from @@rules_fuzzing+//fuzzing/private:fuzz_test.bzl>
  //     <function _starlark_proto_encoder_rule_impl from //grammar:starlark_proto_encoder.bzl>
  print.append("<function $FUNCTION_NAME from $MODULE>");
  return false;
}

bool starlark_function::truthy() const {
  return true;
}

bool starlark_function::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
  // TODO(lmirelmann): Implement.
  return false;
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_function::inner_hash() const {
  // TODO(lmirelmann): Implement.
  return 0;
}

starlark_obj* starlark_function::call(
    const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args,
    context& ctx, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_fn_abs(starlark_obj* this_obj, const std::vector<starlark_obj*>& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::abs_f)) {
    return nullptr;
  }
  auto* value = pos_args.front();
  switch (value->numeric_type()) {
    case starlark_numeric_type::kFloat: {
      auto fvalue = value->as_float();
      if (std::signbit(fvalue)) {
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
      error_callback.add_error(error_argument_bad_operand_type(starlark_built_in_functions::abs_f, value->type()));
      return nullptr;
  }
}

starlark_obj* starlark_fn_all(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::all_f)) {
    return nullptr;
  }
  auto* it = pos_args.front()->get_iterator(true, ctx, error_callback);
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

starlark_obj* starlark_fn_any(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::any_f)) {
    return nullptr;
  }
  auto* it = pos_args.front()->get_iterator(true, ctx, error_callback);
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

starlark_obj* starlark_fn_bool(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::bool_f)) {
    return nullptr;
  }
  return pos_args.front()->truthy() ? ctx.true_value() : ctx.false_value();
}

starlark_obj* starlark_fn_bytes(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // TODO(lmirelmann): The Python version of `bytes` can take zero arguments and returns `b''`. It is not clear whether this is desired in this case.
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::bytes_f)) {
    return nullptr;
  }
  if (pos_args.front()->type() == starlark_types::bytes_t) {
    return pos_args[0];
  }
  if (pos_args.front()->type() == starlark_types::string_t) {
    std::string result;
    utf8_reader reader(pos_args.front()->as_string(), false, false);
    while (reader.pending()) {
      utf8_encode_code_point(reader.peek_code_point(), result, false, true);
      reader.skip_code_point();
    }
    return Arena::Create<starlark_bytes>(&ctx.arena(), result);
  }
  auto* it = pos_args.front()->get_iterator(false, ctx, error_callback);
  if (it == nullptr) {
    error_callback.add_error(error_convert(pos_args.front()->type(), starlark_types::bytes_t));
    return nullptr;
  }
  std::string result;
  while (it->has_next()) {
    auto value = it->next();
    switch (value->numeric_type()) {
      case starlark_numeric_type::kInt64: {
        auto ivalue = value->as_int64();
        if (ivalue < 0 || 255 < ivalue) {
          error_callback.add_error(error_bytes_in_range());
          return nullptr;
        }
        result += static_cast<char>(ivalue);
        break;
      }
      case starlark_numeric_type::kBigInt: {
        const auto& bvalue = value->as_bigint();
        if (bvalue.sign() || bvalue.bit_size() > 8) {
          error_callback.add_error(error_bytes_in_range());
          return nullptr;
        }
        result += static_cast<char>(bvalue.at(0));
        break;
      }
      default:
        error_callback.add_error(error_interpreted_as_integer(value->type()));
        return nullptr;
    }
  }
  it->end_iterator();
  return Arena::Create<starlark_bytes>(&ctx.arena(), result);
}

starlark_obj* starlark_fn_chr(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::chr_f)) {
    return nullptr;
  }
  std::string result;
  auto* value = pos_args.front();
  switch (value->numeric_type()) {
    case starlark_numeric_type::kInt64: {
      auto ivalue = value->as_int64();
      if (ivalue < 0 || 0x10ffff < ivalue) {
        error_callback.add_error(error_unicode_in_range());
        return nullptr;
      }
      utf8_encode_code_point(ivalue, result, false, true);
      break;
    }
    case starlark_numeric_type::kBigInt: {
      const auto& bvalue = value->as_bigint();
      if (bvalue.sign() || bvalue.bit_size() > 21) {
        error_callback.add_error(error_unicode_in_range());
        return nullptr;
      }
      auto ivalue = bvalue.at(0);
      if (0x10ffff < ivalue) {
        error_callback.add_error(error_unicode_in_range());
        return nullptr;
      }
      utf8_encode_code_point(ivalue, result, false, true);
      break;
    }
    default:
      error_callback.add_error(error_interpreted_as_integer(value->type()));
      return nullptr;
  }
  return Arena::Create<starlark_string>(&ctx.arena(), result);
}

starlark_obj* starlark_fn_dict(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!max_args(pos_args, error_callback, starlark_built_in_functions::dict_f, 1)) {
    return nullptr;
  }
  starlark_dictionary* result = Arena::Create<starlark_dictionary>(&ctx.arena());
  starlark_obj* iterable = pos_args.empty() ? nullptr : pos_args.front();
  if (result->update(iterable, named_args, ctx, error_callback)) {
    return nullptr;
  }
  return result;
}

starlark_obj* starlark_fn_dir(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::dir_f)) {
    return nullptr;
  }
  const auto& attributes = pos_args.front()->dir();
  auto* result = Arena::Create<starlark_list>(&ctx.arena(), attributes.size());
  for (const auto& attribute : attributes) {
    // TODO(lmirelmann): This recreates the strings for every call. This can be quite wasteful. Given that this method is not called a lot, then maybe this is ok.
    result->append(Arena::Create<starlark_string>(&ctx.arena(), attribute), error_callback);
  }
  return result;
}

starlark_obj* starlark_fn_enumerate(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  starlark_obj* start = nullptr;
  for (auto& [key, value] : named_args) {
    if (key == "start") {
      assert(value != nullptr);
      if (value->type() != starlark_types::int_t) {
        error_callback.add_error(error_argument_interpreted_as_integer("start", value->type()));
        return nullptr;
      }
      start = value;
    } else {
      error_callback.add_error(error_unknown_argument(key));
      return nullptr;
    }
  }
  if (pos_args.size() != 1) {
    error_callback.add_error(error_arguments_exactly_one(starlark_built_in_functions::enumerate_f, pos_args.size()));
    return nullptr;
  }
  auto* it = pos_args.front()->get_iterator(true, ctx, error_callback);
  if (it == nullptr) {
    return nullptr;
  }
  auto* result = Arena::Create<starlark_list>(&ctx.arena(), std::max<int64_t>(0, pos_args.front()->len(false, error_callback)));
  if (start == nullptr) {
    start = ctx.zero();
  }
  auto* one = ctx.one();
  while (it->has_next()) {
    auto* tuple = Arena::Create<starlark_tuple>(&ctx.arena(), 2);
    tuple->add(start);
    tuple->add(it->next());
    result->append(tuple, error_callback);
    start = start->binary_plus(*one, ctx, error_callback);
  }
  it->end_iterator();
  return result;
}

starlark_obj* starlark_fn_fail(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, starlark_built_in_functions::fail_f)) {
    return nullptr;
  }
  std::string message = "Error:";
  for (const auto& entry : pos_args) {
    message += " ";
    message += entry->str();
  }
  error_callback.add_error(message);
  return nullptr;
}

starlark_obj* starlark_fn_float(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::float_f)) {
    return nullptr;
  }
  auto* value = pos_args.front();
  switch (value->numeric_type()) {
    case starlark_numeric_type::kFloat:
      return value;
    case starlark_numeric_type::kInt64:
      return create_float(value->as_int64(), ctx);
    case starlark_numeric_type::kBigInt: {
      auto fvalue = to_double(value->as_bigint());
      if (std::isinf(fvalue)) {
        error_callback.add_error(error_overflow(value->type(), starlark_types::float_t));
        return nullptr;
      }
      return create_float(fvalue, ctx);
    }
    case starlark_numeric_type::kNotNumeric:
      if (value->type() == starlark_types::string_t) {
        auto svalue = value->as_string();
        errno = 0;
        char* end;
        double double_value = std::strtod(svalue.data(), &end);
        if (end != &svalue.back() + 1) {
          error_callback.add_error(error_convert_string(starlark_types::float_t, svalue));
          return nullptr;
        }
        if (errno != 0) {
          error_callback.add_error(error_overflow_float_too_large());
          return nullptr;
        }
        return create_float(double_value, ctx);
      } else if (value->type() == starlark_types::bool_t) {
        if (value->truthy()) {
          return create_float(1.0, ctx);
        } else {
          return create_float(0.0, ctx);
        }
      } else {
        error_callback.add_error(error_argument_string_or_real(starlark_built_in_functions::float_f, value->type()));
        return nullptr;
      }
  }
}

starlark_obj* starlark_fn_getattr(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, starlark_built_in_functions::getattr_f) ||
      !min_args(pos_args, error_callback, starlark_built_in_functions::getattr_f, 2) ||
      !max_args(pos_args, error_callback, starlark_built_in_functions::getattr_f, 3)) {
    return nullptr;
  }
  auto* element = pos_args.front();
  auto* name = pos_args[1];
  if (name->type() != starlark_types::string_t) {
    error_callback.add_error(error_attribute_string(name->type()));
    return nullptr;
  }
  auto* result = element->get_attr(pos_args.size() == 2, name->as_string(), ctx, error_callback);
  if (result == nullptr) {
    if (pos_args.size() <= 2) {
      return nullptr;
    }
    result = pos_args[2];
  }
  return result;
}

starlark_obj* starlark_fn_hasattr(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!n_pos_args(pos_args, named_args, 2, error_callback, starlark_built_in_functions::hasattr_f)) {
    return nullptr;
  }
  auto* attr = pos_args.back();
  if (attr->type() != starlark_types::string_t) {
    error_callback.add_error(error_attribute_string(attr->type()));
    return nullptr;
  }
  const auto& attributes = pos_args.front()->dir();
  auto it = std::lower_bound(attributes.begin(), attributes.end(), attr->as_string());
  if (it == attributes.end() || *it != attr->as_string()) {
    return ctx.false_value();
  }
  return ctx.true_value();
}

starlark_obj* starlark_fn_hash(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::hash_f)) {
    return nullptr;
  }
  auto* value = pos_args.front();
  if (value->type() == starlark_types::string_t || value->type() == starlark_types::bytes_t) {
    return create_integer(value->hash(), ctx);
  } else {
    error_callback.add_error(error_argument_bad_operand_type(starlark_built_in_functions::hash_f, value->type(), starlark_types::string_t, starlark_types::bytes_t));
    return nullptr;
  }
}

starlark_obj* starlark_fn_int(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, starlark_built_in_functions::int_f)) {
    return nullptr;
  }
  if (pos_args.size() != 1 && pos_args.size() != 2) {
    error_callback.add_error(error_arguments_one_or_two(starlark_built_in_functions::int_f, pos_args.size()));
    return nullptr;
  }
  auto* value = pos_args.front();
  if (value->type() == starlark_types::int_t) {
    if (pos_args.size() == 2) {
      error_callback.add_error(error_convert_non_string_with_base(starlark_built_in_functions::int_f));
      return nullptr;
    }
    return value;
  } else if (value->type() == starlark_types::float_t) {
    if (pos_args.size() == 2) {
      error_callback.add_error(error_convert_non_string_with_base(starlark_built_in_functions::int_f));
      return nullptr;
    }
    auto fvalue = value->as_float();
    if (!std::isfinite(fvalue)) {
      if (std::isinf(fvalue)) {
        error_callback.add_error(error_convert_float_infinity_to_integer());
      } else {
        error_callback.add_error(error_convert_float_nan_to_integer());
      }
      return nullptr;
    }
    return create_integer_from_float(fvalue, ctx);
  } else if (value->type() == starlark_types::bool_t) {
    if (pos_args.size() == 2) {
      error_callback.add_error(error_convert_non_string_with_base(starlark_built_in_functions::int_f));
      return nullptr;
    }
    return value->truthy() ? ctx.one() : ctx.zero();
  } else if (value->type() == starlark_types::string_t) {
    int base = 10;
    if (pos_args.size() == 2) {
      auto* base_param = pos_args[1];
      switch (base_param->numeric_type()) {
        case starlark_numeric_type::kInt64: {
          auto ibase = base_param->as_int64();
          if (ibase != 0 && !(2 <= ibase && ibase <= 36)) {
            error_callback.add_error(error_int_base(starlark_built_in_functions::int_f));
            return nullptr;
          }
          base = ibase;
          break;
        }
        case starlark_numeric_type::kBigInt: {
          const auto& bbase = base_param->as_bigint();
          if (bbase.sign() || bbase.length() > 1) {
            error_callback.add_error(error_int_base(starlark_built_in_functions::int_f));
            return nullptr;
          }
          auto ibase = bbase.at(0);
          if (ibase != 0 && !(2 <= ibase && ibase <= 36)) {
            error_callback.add_error(error_int_base(starlark_built_in_functions::int_f));
            return nullptr;
          }
          base = ibase;
          break;
        }
        default:
          error_callback.add_error(error_interpreted_as_integer(base_param->type()));
          return nullptr;
      }
    }
    auto svalue = value->as_string();
    const char* end;
    auto result = parse_number(svalue, &end, base);
    if (end != (&svalue.back() + 1)) {
      error_callback.add_error(error_invalid_literal_with_base(starlark_built_in_functions::int_f, base, svalue));
      return nullptr;
    }
    return create_integer(std::move(result), ctx);
  } else {
    error_callback.add_error(error_argument_string_int_bool_or_real(starlark_built_in_functions::int_f, value->type()));
    return nullptr;
  }
}

starlark_obj* starlark_fn_len(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::len_f)) {
    return nullptr;
  }
  auto result = pos_args.front()->len(true, error_callback);
  if (result < 0) {
    return nullptr;
  }
  return create_integer(result, ctx);
}

starlark_obj* starlark_fn_list(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!zero_or_one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::list_f)) {
    return nullptr;
  }
  if (pos_args.empty()) {
    return Arena::Create<starlark_list>(&ctx.arena(), 0);
  }
  auto* it = pos_args.front()->get_iterator(true, ctx, error_callback);
  if (it == nullptr) {
    return nullptr;
  }
  auto* result = Arena::Create<starlark_list>(&ctx.arena(), std::max<int64_t>(0, pos_args.front()->len(false, error_callback)));
  while (it->has_next()) {
    result->append(it->next(), error_callback);
  }
  it->end_iterator();
  return result;
}

starlark_obj* starlark_fn_max(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_fn_min(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_fn_ord(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::ord_f)) {
    return nullptr;
  }
  auto* value = pos_args.front();
  if (value->type() == starlark_types::string_t) {
    utf8_reader reader(value->as_string(), false, false);
    if (!reader.pending()) {
      error_callback.add_error(error_expect_character(starlark_built_in_functions::ord_f, value->type(), value->len(false, error_callback)));
      return nullptr;
    }
    auto result = reader.peek_code_point();
    reader.skip_code_point();
    if (reader.pending()) {
      error_callback.add_error(error_expect_character(starlark_built_in_functions::ord_f, value->type(), value->len(false, error_callback)));
      return nullptr;
    }
    return create_integer(result, ctx);
  } else if (value->type() == starlark_types::bytes_t) {
    if (value->len(false, error_callback) != 1) {
      error_callback.add_error(error_expect_character(starlark_built_in_functions::ord_f, value->type(), value->len(false, error_callback)));
      return nullptr;
    }
    return create_integer(static_cast<unsigned char>(value->as_string()[0]), ctx);
  } else {
    error_callback.add_error(error_expect_one_character_or_one_byte(starlark_built_in_functions::ord_f, value->type()));
    return nullptr;
  }
}

starlark_obj* starlark_fn_print(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_fn_range(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, starlark_built_in_functions::range_f) ||
      !min_args(pos_args, error_callback, starlark_built_in_functions::range_f, 1) ||
      !max_args(pos_args, error_callback, starlark_built_in_functions::range_f, 3)) {
    return nullptr;
  }

  auto read_int64 = [](starlark_obj* value, int64_t& output, error_fn& error_callback) -> bool {
    switch (value->numeric_type()) {
    case starlark_numeric_type::kInt64:
      output = value->as_int64();
      return true;
    case starlark_numeric_type::kBigInt: {
      const auto& bvalue = value->as_bigint();
      if (!bvalue.fits_in_int64()) {
        error_callback.add_error(error_overflow(value->type(), starlark_types::int64));
        return false;
      }
      output = bvalue.as_int64();
      return true;
    }
    default:
      error_callback.add_error(error_interpreted_as_integer(value->type()));
      return false;
    }
  };

  int64_t start = 0;
  int64_t end;
  int64_t step = 1;
  switch (pos_args.size()) {
    case 1:
      if (!read_int64(pos_args[0], end, error_callback)) {
        return nullptr;
      }
      break;
    case 2:
      if (!read_int64(pos_args[0], start, error_callback) ||
          !read_int64(pos_args[1], end, error_callback)) {
        return nullptr;
      }
      break;
    case 3:
      if (!read_int64(pos_args[0], start, error_callback) ||
          !read_int64(pos_args[1], end, error_callback) ||
          !read_int64(pos_args[2], step, error_callback)) {
        return nullptr;
      }
      if (step == 0) {
        error_callback.add_error(error_argument_non_zero(starlark_built_in_functions::range_f, 3));
        return nullptr;
      }
      break;
  }


  auto* result = Arena::Create<starlark_range>(&ctx.arena(), start, end, step);
  if (result->len(false, error_callback) < 0) {
    error_callback.add_error(error_overflow(starlark_types::int_t, starlark_types::int64));
    return nullptr;
  }
  return result;
}

starlark_obj* starlark_fn_repr(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::repr_f)) {
    return nullptr;
  }
  return Arena::Create<starlark_string>(&ctx.arena(), pos_args.front()->repr());
}

starlark_obj* starlark_fn_reversed(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::reversed_f)) {
    return nullptr;
  }
  auto* it = pos_args.front()->get_iterator(true, ctx, error_callback);
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
    result->append(element, error_callback);
  }
  return result;
}

starlark_obj* starlark_fn_set(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!zero_or_one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::set_f)) {
    return nullptr;
  }
  if (pos_args.empty()) {
    return Arena::Create<starlark_set>(&ctx.arena());
  }

  auto* it = pos_args.front()->get_iterator(true, ctx, error_callback);
  if (it == nullptr) {
    return nullptr;
  }
  auto* result = Arena::Create<starlark_set>(&ctx.arena());
  while (it->has_next()) {
    if (result->add(it->next(), error_callback).second) {
      return nullptr;
    }
  }
  it->end_iterator();
  return result;
}

starlark_obj* starlark_fn_sorted(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_fn_str(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::str_f)) {
    return nullptr;
  }
  return Arena::Create<starlark_string>(&ctx.arena(), pos_args.front()->str());
}

starlark_obj* starlark_fn_tuple(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!zero_or_one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::tuple_f)) {
    return nullptr;
  }
  if (pos_args.empty()) {
    return Arena::Create<starlark_tuple>(&ctx.arena(), 0);
  }

  auto* it = pos_args.front()->get_iterator(true, ctx, error_callback);
  if (it == nullptr) {
    return nullptr;
  }
  auto* result = Arena::Create<starlark_tuple>(&ctx.arena(), std::max<int64_t>(0, pos_args.front()->len(false, error_callback)));
  while (it->has_next()) {
    result->add(it->next());
  }
  it->end_iterator();
  return result;
}

starlark_obj* starlark_fn_type(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::type_f)) {
    return nullptr;
  }
  return Arena::Create<starlark_string>(&ctx.arena(), pos_args.front()->type());
}

starlark_obj* starlark_fn_zip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  auto all_available = [](const std::vector<starlark_iterator*>& its) -> bool {
    for (const auto* it : its) {
      if (!it->has_next()) {
        return false;
      }
    }
    return true;
  };

  if (!no_named_args(named_args, error_callback, starlark_built_in_functions::zip_f)) {
    return nullptr;
  }
  if (pos_args.empty()) {
    return Arena::Create<starlark_list>(&ctx.arena(), 0);
  }
  auto len = std::numeric_limits<int64_t>::max();
  std::vector<starlark_iterator*> its;
  its.reserve(pos_args.size());
  for (auto* element : pos_args) {
    auto* it = element->get_iterator(true, ctx, error_callback);
    if (it == nullptr) {
      return nullptr;
    }
    its.push_back(it);
    len = std::min<int64_t>(len, element->len(false, error_callback));
  }
  starlark_list* result = Arena::Create<starlark_list>(&ctx.arena(), std::max<int64_t>(0, len));
  while (all_available(its)) {
    auto* tuple = Arena::Create<starlark_tuple>(&ctx.arena(), its.size());
    for (auto* it : its) {
      tuple->add(it->next());
    }
    result->append(tuple, error_callback);
  }
  for (auto* it : its) {
    it->end_iterator();
  }
  return result;
}

}  // namespace runtime
}  // namespace starlark


