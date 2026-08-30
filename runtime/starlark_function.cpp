// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_function.hpp"

#include "runtime/builtin_pos.hpp"

#include <algorithm>
#include <limits>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "errors/runtime_error_messages.hpp"
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
using ::starlark::error_messages::error_v2_argument_bad_operand_type;
using ::starlark::error_messages::error_v2_argument_interpreted_as_integer;
using ::starlark::error_messages::error_v2_argument_interpreted_as_string;
using ::starlark::error_messages::error_v2_argument_non_zero;
using ::starlark::error_messages::error_v2_argument_string_int_bool_or_real;
using ::starlark::error_messages::error_v2_argument_string_or_real;
using ::starlark::error_messages::error_v2_arguments_exactly_one;
using ::starlark::error_messages::error_v2_arguments_one_or_two;
using ::starlark::error_messages::error_v2_arguments_too_many;
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
using ::starlark::error_messages::error_v2_max_bytes_length;
using ::starlark::error_messages::error_v2_missing_argument;
using ::starlark::error_messages::error_v2_missing_typed_argument;
using ::starlark::error_messages::error_v2_multiple_values_for_argument;
using ::starlark::error_messages::error_v2_named_argument_must_be_type;
using ::starlark::error_messages::error_v2_non_string_with_base;
using ::starlark::error_messages::error_v2_overflow;
using ::starlark::error_messages::error_v2_overflow_float_too_large;
using ::starlark::error_messages::error_v2_unknown_argument;
using ::starlark::result::error_status;
using ::starlark::result::ok_status;
using ::starlark::result::status;
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

starlark_built_in_function::starlark_built_in_function(starlark_obj* this_obj, builtin_entrypoints entrypoints, std::string_view fn_name) :
  this_obj(this_obj), entrypoints(entrypoints), fn_name(fn_name) {}

std::string_view starlark_built_in_function::type() const {
  return starlark_types::builtin_function_or_method_t;
}

bool starlark_built_in_function::inner_repr(printer& print, printer_action action) const {
  if (this_obj != nullptr) {
    print.append(std::format("<built-in method {} of {} value>", fn_name, this_obj->type()));
    return false;
  }
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
  return fn_name == fother->fn_name && entrypoints.call == fother->entrypoints.call;
}

void starlark_built_in_function::inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, bool extended, error_fn& error_callback) const {
  if (extended && type() == other->type() && equals(*other)) {
    return;
  }
  starlark_obj::inner_cmp(comp, other, op, extended, error_callback);
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_built_in_function::inner_hash() const {
  if (fn_name.length() == 0) {
    return 0;
  }
  return static_cast<int64_t>(siphash(fn_name.data(), fn_name.length(), 0x452821E638D01377, 0xBE5466CF34E90C6C));
}

starlark_obj* starlark_built_in_function::call(const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args,
    context& ctx, error_fn& error_callback) {
  return entrypoints.call(this_obj, pos_args, named_args, ctx, error_callback);
}

starlark_obj* starlark_built_in_function::call_pos(std::span<starlark_obj*> pos_args, context& ctx, error_fn& error_callback) {
  switch (pos_args.size()) {
    case 0:
      if (entrypoints.pos0 != nullptr) {
        return entrypoints.pos0(this_obj, ctx, error_callback);
      }
      break;
    case 1:
      if (entrypoints.pos1 != nullptr) {
        return entrypoints.pos1(this_obj, pos_args[0], ctx, error_callback);
      }
      break;
    case 2:
      if (entrypoints.pos2 != nullptr) {
        return entrypoints.pos2(this_obj, pos_args[0], pos_args[1], ctx, error_callback);
      }
      break;
    case 3:
      if (entrypoints.pos3 != nullptr) {
        return entrypoints.pos3(this_obj, pos_args[0], pos_args[1], pos_args[2], ctx, error_callback);
      }
      break;
    default:
      break;
  }
  static thread_local starlark_obj::pos_args_t pos_args_vec;
  pos_args_vec.assign(pos_args.begin(), pos_args.end());
  starlark_obj::named_args_t named_args;
  return entrypoints.call(this_obj, pos_args_vec, named_args, ctx, error_callback);
}

starlark_function::starlark_function(std::string_view fn_name, std::string_view module_name) : fn_name(fn_name), module_name(module_name) {}

std::string_view starlark_function::type() const {
  return starlark_types::function_t;
}

bool starlark_function::inner_repr(printer& print, printer_action action) const {
  print.append(std::format("<function {} from {}>", fn_name, module_name));
  return false;
}

bool starlark_function::truthy() const {
  return true;
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_function::inner_hash() const {
  if (fn_name.length() == 0) {
    return 0;
  }
  return static_cast<int64_t>(siphash(fn_name.data(), fn_name.length(), 0xC0AC29B7C97C50DD, 0x3F84D5B5B5470917));
}

starlark_obj* starlark_fn_abs(starlark_obj* this_obj, const std::vector<starlark_obj*>& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::abs_f).ok()) {
    return nullptr;
  }
  return builtin_pos::abs_pos1(this_obj, pos_args.front(), ctx, error_callback);
}

starlark_obj* starlark_fn_all(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::all_f).ok()) {
    return nullptr;
  }
  return builtin_pos::all_pos1(this_obj, pos_args.front(), ctx, error_callback);
}

starlark_obj* starlark_fn_any(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::any_f).ok()) {
    return nullptr;
  }
  return builtin_pos::any_pos1(this_obj, pos_args.front(), ctx, error_callback);
}

starlark_obj* starlark_fn_bool(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!zero_or_one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::bool_f).ok()) {
    return nullptr;
  }
  if (pos_args.empty()) {
    return builtin_pos::bool_pos0(this_obj, ctx, error_callback);
  }
  return builtin_pos::bool_pos1(this_obj, pos_args.front(), ctx, error_callback);
}

starlark_obj* starlark_fn_bytes(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // The Python version of `bytes` can take zero arguments and returns `b''`.
  // The spec is not clear whether zero arguments is ok, but a strict reading would be that this parameter is mandatory.
  // See: https://github.com/bazelbuild/starlark/issues/351
  starlark_obj* source = nullptr;
  for (auto& [key, value] : named_args) {
    if (key->as_string() == "source") {
      assert(value != nullptr);
      source = value;
    } else {
      error_callback.add_error(error_v2_unknown_argument(key->as_string()));
      return nullptr;
    }
  }
  if (pos_args.size() >= 2) {
    error_callback.add_error(error_v2_arguments_too_many(starlark_built_in_functions::bytes_f, pos_args.size(), 1));
    return nullptr;
  }
  if (pos_args.size() >= 1) {
    if (source != nullptr) {
      error_callback.add_error(error_v2_multiple_values_for_argument(
          starlark_built_in_functions::bytes_f,
          "source"));
      return nullptr;
    }
    source = pos_args.front();
  }
  if (source == nullptr) {
    return builtin_pos::bytes_pos0(this_obj, ctx, error_callback);
  }
  return builtin_pos::bytes_pos1(this_obj, source, ctx, error_callback);
}

starlark_obj* starlark_fn_chr(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::chr_f).ok()) {
    return nullptr;
  }
  return builtin_pos::chr_pos1(this_obj, pos_args.front(), ctx, error_callback);
}

starlark_obj* starlark_fn_dict(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!max_args(pos_args, error_callback, starlark_built_in_functions::dict_f, 1).ok()) {
    return nullptr;
  }
  if (pos_args.empty()) {
    starlark_dictionary* result = Arena::Create<starlark_dictionary>(&ctx.arena());
    if (!result->update(nullptr, named_args, ctx, error_callback).ok()) {
      return nullptr;
    }
    return result;
  }
  starlark_dictionary* result = Arena::Create<starlark_dictionary>(&ctx.arena());
  if (!result->update(pos_args.front(), named_args, ctx, error_callback).ok()) {
    return nullptr;
  }
  return result;
}

starlark_obj* starlark_fn_dir(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // Python has a variation of this that takes zero arguments. This was discussed, but probably will never happen.
  // Context: https://github.com/bazelbuild/starlark/issues/218
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::dir_f).ok()) {
    return nullptr;
  }
  return builtin_pos::dir_pos1(this_obj, pos_args.front(), ctx, error_callback);
}

starlark_obj* starlark_fn_enumerate(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // Bazel allows the first parameter to be named with the name `list`, Python does the same with the name `iterable`, the spec calls this parameter `x`.
  // Check https://github.com/bazelbuild/starlark/issues/355
  starlark_obj* iterable = nullptr;
  starlark_obj* start = nullptr;
  for (auto& [key, value] : named_args) {
    if (key->as_string() == "start") {
      assert(value != nullptr);
      start = value;
    } else if (key->as_string() == "iterable") {
      assert(value != nullptr);
      iterable = value;
    } else {
      error_callback.add_error(error_v2_unknown_argument(key->as_string()));
      return nullptr;
    }
  }
  if (!max_args(pos_args, error_callback, starlark_built_in_functions::enumerate_f, 2).ok()) {
    return nullptr;
  }
  if (!pos_args.empty()) {
    if (iterable != nullptr) {
      error_callback.add_error(error_v2_multiple_values_for_argument(
          starlark_built_in_functions::enumerate_f,
          "iterable"));
      return nullptr;
    }
    iterable = pos_args.front();
  }
  if (pos_args.size() >= 2) {
    if (start != nullptr) {
      error_callback.add_error(error_v2_multiple_values_for_argument(
          starlark_built_in_functions::enumerate_f,
          "start"));
      return nullptr;
    }
    start = pos_args[1];
  }
  if (iterable == nullptr) {
    error_callback.add_error(error_v2_missing_argument(starlark_built_in_functions::enumerate_f, "iterable"));
    return nullptr;
  }
  if (pos_args.size() == 1 && named_args.empty()) {
    return builtin_pos::enumerate_pos1(this_obj, iterable, ctx, error_callback);
  }
  if (pos_args.size() == 2 && named_args.empty()) {
    return builtin_pos::enumerate_pos2(this_obj, iterable, start, ctx, error_callback);
  }
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

starlark_obj* starlark_fn_fail(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, starlark_built_in_functions::fail_f).ok()) {
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
  if (!zero_or_one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::float_f).ok()) {
    return nullptr;
  }
  if (pos_args.empty()) {
    return builtin_pos::float_pos0(this_obj, ctx, error_callback);
  }
  return builtin_pos::float_pos1(this_obj, pos_args.front(), ctx, error_callback);
}

starlark_obj* starlark_fn_getattr(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, starlark_built_in_functions::getattr_f).ok() ||
      !min_args(pos_args, error_callback, starlark_built_in_functions::getattr_f, 2).ok() ||
      !max_args(pos_args, error_callback, starlark_built_in_functions::getattr_f, 3).ok()) {
    return nullptr;
  }
  auto* element = pos_args.front();
  auto* name = pos_args[1];
  if (pos_args.size() == 2 && named_args.empty()) {
    return builtin_pos::getattr_pos2(this_obj, element, name, ctx, error_callback);
  }
  if (pos_args.size() == 3 && named_args.empty()) {
    return builtin_pos::getattr_pos3(this_obj, element, name, pos_args[2], ctx, error_callback);
  }
  if (name->type() != starlark_types::string_t) {
    error_callback.add_error(error_v2_attribute_string(name->type()));
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
  if (!n_pos_args(pos_args, named_args, 2, error_callback, starlark_built_in_functions::hasattr_f).ok()) {
    return nullptr;
  }
  return builtin_pos::hasattr_pos2(this_obj, pos_args.front(), pos_args.back(), ctx, error_callback);
}

starlark_obj* starlark_fn_hash(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // Python allows many things to be hashed, but the Starlark spec is very clear that only string and bytes are alloed in Starlark.
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::hash_f).ok()) {
    return nullptr;
  }
  return builtin_pos::hash_pos1(this_obj, pos_args.front(), ctx, error_callback);
}

starlark_obj* starlark_fn_int(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // Python allows the variation with zero parameters, so we allow it here.
  starlark_obj* base_param = nullptr;
  for (auto& [key, value] : named_args) {
    if (key->as_string() == "base") {
      assert(value != nullptr);
      if (value->type() != starlark_types::int_t) {
        error_callback.add_error(error_v2_argument_interpreted_as_integer("base", value->type()));
        return nullptr;
      }
      base_param = value;
    } else {
      error_callback.add_error(error_v2_unknown_argument(key->as_string()));
      return nullptr;
    }
  }
  if (pos_args.size() > 2) {
    error_callback.add_error(error_v2_arguments_one_or_two(starlark_built_in_functions::int_f, pos_args.size()));
    return nullptr;
  }
  if (pos_args.size() >= 2) {
    if (base_param != nullptr) {
      error_callback.add_error(error_v2_multiple_values_for_argument(
          starlark_built_in_functions::int_f,
          "base"));
      return nullptr;
    }
    base_param = pos_args[1];
  }
  if (pos_args.empty()) {
    if (base_param != nullptr) {
      error_callback.add_error(error_v2_missing_typed_argument(starlark_built_in_functions::int_f, starlark_types::string_t));
      return nullptr;
    }
    return builtin_pos::int_pos0(this_obj, ctx, error_callback);
  }
  auto* value = pos_args.front();
  if (pos_args.size() == 1 && named_args.empty()) {
    return builtin_pos::int_pos1(this_obj, value, ctx, error_callback);
  }
  if (pos_args.size() == 2 && named_args.empty()) {
    return builtin_pos::int_pos2(this_obj, value, pos_args[1], ctx, error_callback);
  }
  if (value->type() != starlark_types::string_t && base_param != nullptr) {
    error_callback.add_error(error_v2_non_string_with_base());
    return nullptr;
  }
  if (value->type() == starlark_types::int_t) {
    return value;
  } else if (value->type() == starlark_types::float_t) {
    return create_integer_from_float(value->as_float(), ctx, error_callback);
  } else if (value->type() == starlark_types::bool_t) {
    return value->truthy() ? ctx.one() : ctx.zero();
  } else if (value->type() == starlark_types::string_t) {
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
  } else {
    error_callback.add_error(error_v2_argument_string_int_bool_or_real(starlark_built_in_functions::int_f, value->type()));
    return nullptr;
  }
}

starlark_obj* starlark_fn_len(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::len_f).ok()) {
    return nullptr;
  }
  return builtin_pos::len_pos1(this_obj, pos_args.front(), ctx, error_callback);
}

starlark_obj* starlark_fn_list(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!zero_or_one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::list_f).ok()) {
    return nullptr;
  }
  if (pos_args.empty()) {
    return builtin_pos::list_pos0(this_obj, ctx, error_callback);
  }
  return builtin_pos::list_pos1(this_obj, pos_args.front(), ctx, error_callback);
}

starlark_obj* starlark_fn_max(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!min_args(pos_args, error_callback, starlark_built_in_functions::max_f, 1).ok()) {
    return nullptr;
  }
  starlark_obj* key_fn = nullptr;
  for (auto& [key, value] : named_args) {
    if (key->as_string() == "key") {
      assert(value != nullptr);
      if (value->type() != starlark_types::none_t) {
        key_fn = const_cast<starlark_obj*>(value);
      }
    } else {
      error_callback.add_error(error_v2_unknown_argument(key->as_string()));
      return nullptr;
    }
  }
  if (pos_args.size() > 1) {
    starlark_obj* candidate = pos_args.front();
    if (key_fn == nullptr) {
      for (std::size_t pos = 1; pos < pos_args.size(); ++pos) {
        auto cmp = candidate->cmp(*pos_args[pos], "<", error_callback);
        if (!cmp.ok()) {
          return nullptr;
        }
        if (*cmp < 0) {
          candidate = pos_args[pos];
        }
      }
    } else {
      const starlark_obj* candidate_key = key_fn->call({pos_args.front()}, {}, ctx, error_callback);
      if (candidate_key == nullptr) {
        return nullptr;
      }
      for (std::size_t pos = 1; pos < pos_args.size(); ++pos) {
        const starlark_obj* element_key = key_fn->call({pos_args[pos]}, {}, ctx, error_callback);
        if (element_key == nullptr) {
          return nullptr;
        }
        auto cmp = candidate_key->cmp(*element_key, "<", error_callback);
        if (!cmp.ok()) {
          return nullptr;
        }
        if (*cmp < 0) {
          candidate = pos_args[pos];
          candidate_key = element_key;
        }
      }
    }
    return candidate;
  }
  auto* it = pos_args.front()->get_iterator(true, ctx, error_callback);
  if (it == nullptr) {
    return nullptr;
  }
  if (!it->has_next()) {
    it->end_iterator();
    error_callback.add_error(error_v2_empty_iterator(starlark_built_in_functions::max_f));
    return nullptr;
  }
  starlark_obj* candidate = it->next();
  if (key_fn == nullptr) {
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
  } else {
    const starlark_obj* candidate_key = key_fn->call({candidate}, {}, ctx, error_callback);
    if (candidate_key == nullptr) {
      return nullptr;
    }
    while (it->has_next()) {
      auto* element = it->next();
      const starlark_obj* element_key = key_fn->call({element}, {}, ctx, error_callback);
      if (element_key == nullptr) {
        return nullptr;
      }
      auto cmp = candidate_key->cmp(*element_key, "<", error_callback);
      if (!cmp.ok()) {
        return nullptr;
      }
      if (*cmp < 0) {
        candidate = element;
        candidate_key = element_key;
      }
    }
  }
  it->end_iterator();
  return candidate;
}

starlark_obj* starlark_fn_min(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!min_args(pos_args, error_callback, starlark_built_in_functions::min_f, 1).ok()) {
    return nullptr;
  }
  starlark_obj* key_fn = nullptr;
  for (auto& [key, value] : named_args) {
    if (key->as_string() == "key") {
      assert(value != nullptr);
      if (value->type() != starlark_types::none_t) {
        key_fn = const_cast<starlark_obj*>(value);
      }
    } else {
      error_callback.add_error(error_v2_unknown_argument(key->as_string()));
      return nullptr;
    }
  }
  if (pos_args.size() > 1) {
    starlark_obj* candidate = pos_args.front();
    if (key_fn == nullptr) {
      for (std::size_t pos = 1; pos < pos_args.size(); ++pos) {
        starlark_obj* element = pos_args[pos];
        auto cmp = candidate->cmp(*element, "<", error_callback);
        if (!cmp.ok()) {
          return nullptr;
        }
        if (*cmp > 0) {
          candidate = element;
        }
      }
    } else {
      const starlark_obj* candidate_key = key_fn->call({pos_args.front()}, {}, ctx, error_callback);
      if (candidate_key == nullptr) {
        return nullptr;
      }
      for (std::size_t pos = 1; pos < pos_args.size(); ++pos) {
        starlark_obj* element = pos_args[pos];
        const starlark_obj* element_key = key_fn->call({element}, {}, ctx, error_callback);
        if (element_key == nullptr) {
          return nullptr;
        }
        auto cmp = candidate_key->cmp(*element_key, "<", error_callback);
        if (!cmp.ok()) {
          return nullptr;
        }
        if (*cmp > 0) {
          candidate = element;
          candidate_key = element_key;
        }
      }
    }
    return candidate;
  }
  auto* it = pos_args.front()->get_iterator(true, ctx, error_callback);
  if (it == nullptr) {
    return nullptr;
  }
  if (!it->has_next()) {
    it->end_iterator();
    error_callback.add_error(error_v2_empty_iterator(starlark_built_in_functions::min_f));
    return nullptr;
  }
  starlark_obj* candidate = it->next();
  if (key_fn == nullptr) {
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
  } else {
    const starlark_obj* candidate_key = key_fn->call({candidate}, {}, ctx, error_callback);
    if (candidate_key == nullptr) {
      return nullptr;
    }
    while (it->has_next()) {
      auto* element = it->next();
      const starlark_obj* element_key = key_fn->call({element}, {}, ctx, error_callback);
      if (element_key == nullptr) {
        return nullptr;
      }
      auto cmp = candidate_key->cmp(*element_key, "<", error_callback);
      if (!cmp.ok()) {
        return nullptr;
      }
      if (*cmp > 0) {
        candidate = element;
        candidate_key = element_key;
      }
    }
  }
  it->end_iterator();
  return candidate;
}

starlark_obj* starlark_fn_ord(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::ord_f).ok()) {
    return nullptr;
  }
  return builtin_pos::ord_pos1(this_obj, pos_args.front(), ctx, error_callback);
}

starlark_obj* starlark_fn_print(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (named_args.empty() && pos_args.size() == 1) {
    return builtin_pos::print_pos1(this_obj, pos_args.front(), ctx, error_callback);
  }
  if (named_args.empty() && pos_args.empty()) {
    return builtin_pos::print_pos0(this_obj, ctx, error_callback);
  }
  std::string sep = " ";
  for (auto& [key, value] : named_args) {
    if (key->as_string() == "sep") {
      assert(value != nullptr);
      if (value->type() != starlark_types::string_t) {
        error_callback.add_error(error_v2_argument_interpreted_as_string("sep", value->type()));
        return nullptr;
      }
      sep = value->str();
    } else {
      error_callback.add_error(error_v2_unknown_argument(key->as_string()));
      return nullptr;
    }
  }
  bool first = true;
  for (const auto* element : pos_args) {
    if (!first) {
      ctx.options().out << sep;
    }
    first = false;
    ctx.options().out << element->str();
  }
  ctx.options().out << "\n";
  return ctx.none_value();
}

starlark_obj* starlark_fn_range(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, starlark_built_in_functions::range_f).ok() ||
      !min_args(pos_args, error_callback, starlark_built_in_functions::range_f, 1).ok() ||
      !max_args(pos_args, error_callback, starlark_built_in_functions::range_f, 3).ok()) {
    return nullptr;
  }

  switch (pos_args.size()) {
    case 1:
      return builtin_pos::range_pos1(this_obj, pos_args[0], ctx, error_callback);
    case 2:
      return builtin_pos::range_pos2(this_obj, pos_args[0], pos_args[1], ctx, error_callback);
    case 3:
      return builtin_pos::range_pos3(this_obj, pos_args[0], pos_args[1], pos_args[2], ctx, error_callback);
    default:
      return nullptr;
  }
}

starlark_obj* starlark_fn_repr(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::repr_f).ok()) {
    return nullptr;
  }
  return builtin_pos::repr_pos1(this_obj, pos_args.front(), ctx, error_callback);
}

starlark_obj* starlark_fn_reversed(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::reversed_f).ok()) {
    return nullptr;
  }
  return builtin_pos::reversed_pos1(this_obj, pos_args.front(), ctx, error_callback);
}

starlark_obj* starlark_fn_set(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!zero_or_one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::set_f).ok()) {
    return nullptr;
  }
  if (pos_args.empty()) {
    return builtin_pos::set_pos0(this_obj, ctx, error_callback);
  }
  return builtin_pos::set_pos1(this_obj, pos_args.front(), ctx, error_callback);
}

starlark_obj* starlark_fn_sorted(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (pos_args.size() != 1) {
    error_callback.add_error(error_v2_arguments_exactly_one(starlark_built_in_functions::sorted_f, pos_args.size()));
    return nullptr;
  }
  if (named_args.empty()) {
    return builtin_pos::sorted_pos1(this_obj, pos_args.front(), ctx, error_callback);
  }
  starlark_obj* key_fn = nullptr;
  bool reverse = false;
  for (auto& [key, value] : named_args) {
    assert(value != nullptr);
    if (key->as_string() == "key") {
      if (value->type() != starlark_types::none_t) {
        key_fn = const_cast<starlark_obj*>(value);
      }
    } else if (key->as_string() == "reverse") {
      if (value->type() != starlark_types::bool_t) {
        error_callback.add_error(error_v2_named_argument_must_be_type(starlark_built_in_functions::sorted_f, "reverse", starlark_types::bool_t, value->type()));
        return nullptr;
      }
      reverse = value->truthy();
    } else {
      error_callback.add_error(error_v2_unknown_argument(key->as_string()));
      return nullptr;
    }
  }

  // Retrieve the elements.
  std::vector<std::pair<starlark_obj*, starlark_obj*>> elems;
  auto* it = pos_args.front()->get_iterator(true, ctx, error_callback);
  if (it == nullptr) {
    return nullptr;
  }
  while (it->has_next()) {
    starlark_obj* element = it->next();
    starlark_obj* element_key = element;
    if (key_fn != nullptr) {
      element_key = key_fn->call({element}, {}, ctx, error_callback);
    }
    elems.emplace_back(element_key, element);
  }
  it->end_iterator();

  // Sort and maybe revert.
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
  if (reverse) {
    std::reverse(elems.begin(), elems.end());
  }

  // Create the output.
  auto* result = Arena::Create<starlark_list>(&ctx.arena(), elems.size());
  for (auto& elem : elems) {
    result->unsafe_append(elem.second);
  }
  return result;
}

starlark_obj* starlark_fn_str(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // The Python version of `str` can take zero arguments and returns `''`.
  // The spec is not clear whether zero arguments is ok, but a strict reading would be that this parameter is mandatory.
  // See: https://github.com/bazelbuild/starlark/issues/351
  starlark_obj* object = nullptr;
  for (auto& [key, value] : named_args) {
    if (key->as_string() == "object") {
      assert(value != nullptr);
      object = value;
    } else {
      error_callback.add_error(error_v2_unknown_argument(key->as_string()));
      return nullptr;
    }
  }
  if (pos_args.size() >= 2) {
    error_callback.add_error(error_v2_arguments_too_many(starlark_built_in_functions::str_f, pos_args.size(), 1));
    return nullptr;
  }
  if (pos_args.size() >= 1) {
    if (object != nullptr) {
      error_callback.add_error(error_v2_multiple_values_for_argument(
          starlark_built_in_functions::str_f,
          "object"));
      return nullptr;
    }
    object = pos_args.front();
  }
  if (object == nullptr) {
    return builtin_pos::str_pos0(this_obj, ctx, error_callback);
  }
  return builtin_pos::str_pos1(this_obj, object, ctx, error_callback);
}

starlark_obj* starlark_fn_tuple(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!zero_or_one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::tuple_f).ok()) {
    return nullptr;
  }
  if (pos_args.empty()) {
    return builtin_pos::tuple_pos0(this_obj, ctx, error_callback);
  }
  return builtin_pos::tuple_pos1(this_obj, pos_args.front(), ctx, error_callback);
}

starlark_obj* starlark_fn_type(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, starlark_built_in_functions::type_f).ok()) {
    return nullptr;
  }
  return builtin_pos::type_pos1(this_obj, pos_args.front(), ctx, error_callback);
}

starlark_obj* starlark_fn_zip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, starlark_built_in_functions::zip_f).ok()) {
    return nullptr;
  }
  switch (pos_args.size()) {
    case 0:
      return builtin_pos::zip_pos0(this_obj, ctx, error_callback);
    case 1:
      return builtin_pos::zip_pos1(this_obj, pos_args[0], ctx, error_callback);
    case 2:
      return builtin_pos::zip_pos2(this_obj, pos_args[0], pos_args[1], ctx, error_callback);
    case 3:
      return builtin_pos::zip_pos3(this_obj, pos_args[0], pos_args[1], pos_args[2], ctx, error_callback);
    default:
      break;
  }
  auto all_available = [](const std::vector<starlark_iterator*>& its) -> bool {
    for (const auto* it : its) {
      if (!it->has_next()) {
        return false;
      }
    }
    return true;
  };
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

}  // namespace runtime
}  // namespace starlark


