// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_function.hpp"

#include <format>
#include <map>
#include <string>
#include <vector>

#include "runtime/starlark_bool.hpp"
#include "runtime/starlark_bytes.hpp"
#include "runtime/starlark_dictionary.hpp"
#include "runtime/starlark_list.hpp"
#include "runtime/starlark_numeric.hpp"
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

starlark_built_in_function::starlark_built_in_function(fn* native_fn, const std::string& fn_name) :
  native_fn(native_fn), fn_name(fn_name) {}

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
  // TODO(lmirelmann): Implement.
  return false;
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_built_in_function::inner_hash() const {
  // TODO(lmirelmann): Implement.
  return 0;
}

starlark_obj* starlark_built_in_function::call(
    const std::vector<starlark_obj*>& pos_args,
    const std::map<std::string, starlark_obj*>& named_args,
    Arena& arena,
    error_fn& error_callback) {
  return native_fn(pos_args, named_args, arena, error_callback);
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
    const std::vector<starlark_obj*>& pos_args,
    const std::map<std::string, starlark_obj*>& named_args,
    Arena& arena,
    error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  return nullptr;
}

namespace {

bool one_pos_arg(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, error_fn& error_callback, std::string_view fn_name) {
  if (!named_args.empty()) {
    error_callback.add_error(std::format("TypeError: {}() takes no keyword arguments", fn_name));
    return false;
  }
  if (pos_args.size() != 1) {
    error_callback.add_error(std::format("TypeError: {}() takes exactly one argument ({} given)", fn_name, pos_args.size()));
    return false;
  }
  return true;
}

bool zero_or_one_pos_arg(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, error_fn& error_callback, std::string_view fn_name) {
  if (!named_args.empty()) {
    error_callback.add_error(std::format("TypeError: {}() takes no keyword arguments", fn_name));
    return false;
  }
  if (pos_args.size() > 1) {
    error_callback.add_error(std::format("TypeError: {} expected at most 1 argument, got {}", fn_name, pos_args.size()));
    return false;
  }
  return true;
}

}  // namespace

starlark_obj* starlark_fn_abs(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "abs")) {
    return nullptr;
  }
  auto* value = pos_args.front();
  switch (value->numeric_type()) {
    case starlark_numeric_type::kFloat: {
      auto fvalue = value->as_float();
      if (std::signbit(fvalue)) {
        return create_float(std::abs(value->as_float()), arena);
      }
      return value;
    }
    case starlark_numeric_type::kInt64: {
      int64_t ivalue = value->as_int64();
      if (ivalue < 0) {
        return value->unary_minus(arena, error_callback);
      }
      return value;
    }
    case starlark_numeric_type::kBigInt: {
      const auto& bvalue = value->as_bigint();
      if (bvalue.sign()) {
        return value->unary_minus(arena, error_callback);
      }
      return value;
    }
    default:
      error_callback.add_error(std::format("TypeError: bad operand type for abs(): '{}'", value->type()));
      return nullptr;
  }
}

starlark_obj* starlark_fn_all(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "all")) {
    return nullptr;
  }
  auto* it = pos_args.front()->get_iterator(true, arena, error_callback);
  if (it == nullptr) {
    return nullptr;
  }
  bool result = true;
  while (result && it->has_next()) {
    result = it->next()->truthy();
  }
  it->end_iterator();
  // TODO(lmirelmann): Use the instance of bool from the context.
  return Arena::Create<starlark_bool>(&arena, result);
}

starlark_obj* starlark_fn_any(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "any")) {
    return nullptr;
  }
  auto* it = pos_args.front()->get_iterator(true, arena, error_callback);
  if (it == nullptr) {
    return nullptr;
  }
  bool result = false;
  while (!result && it->has_next()) {
    result = it->next()->truthy();
  }
  it->end_iterator();
  // TODO(lmirelmann): Use the instance of bool from the context.
  return Arena::Create<starlark_bool>(&arena, result);
}

starlark_obj* starlark_fn_bool(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "bool")) {
    return nullptr;
  }
  // TODO(lmirelmann): Use the instance of bool from the context.
  return Arena::Create<starlark_bool>(&arena, pos_args.front()->truthy());
}

starlark_obj* starlark_fn_bytes(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): The Python version of `bytes` can take zero arguments and returns `b''`. It is not clear whether this is desired in this case.
  if (!one_pos_arg(pos_args, named_args, error_callback, "bytes")) {
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
    return Arena::Create<starlark_bytes>(&arena, result);
  }
  auto* it = pos_args.front()->get_iterator(false, arena, error_callback);
  if (it == nullptr) {
    error_callback.add_error(std::format("TypeError: cannot convert '{}' object to bytes", pos_args.front()->type()));
    return nullptr;
  }
  std::string result;
  while (it->has_next()) {
    auto value = it->next();
    switch (value->numeric_type()) {
      case starlark_numeric_type::kInt64: {
        auto ivalue = value->as_int64();
        if (ivalue < 0 || 255 < ivalue) {
          error_callback.add_error("ValueError: bytes must be in range(0, 256)");
          return nullptr;
        }
        result += static_cast<char>(ivalue);
        break;
      }
      case starlark_numeric_type::kBigInt: {
        const auto& bvalue = value->as_bigint();
        if (bvalue.sign() || bvalue.bit_size() > 8) {
          error_callback.add_error("ValueError: bytes must be in range(0, 256)");
          return nullptr;
        }
        result += static_cast<char>(bvalue.at(0));
        break;
      }
      default:
        error_callback.add_error(std::format("TypeError: '{}' object cannot be interpreted as an integer", value->type()));
        return nullptr;
    }
  }
  it->end_iterator();
  return Arena::Create<starlark_bytes>(&arena, result);
}

starlark_obj* starlark_fn_chr(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "chr")) {
    return nullptr;
  }
  std::string result;
  auto* value = pos_args.front();
  switch (value->numeric_type()) {
    case starlark_numeric_type::kInt64: {
      auto ivalue = value->as_int64();
      if (ivalue < 0 || 0x10ffff < ivalue) {
        error_callback.add_error("ValueError: Unicode code point must be in range(0, 0x110000)");
        return nullptr;
      }
      utf8_encode_code_point(ivalue, result, false, true);
      break;
    }
    case starlark_numeric_type::kBigInt: {
      const auto& bvalue = value->as_bigint();
      if (bvalue.sign() || bvalue.bit_size() > 21) {
        error_callback.add_error("ValueError: Unicode code point must be in range(0, 0x110000)");
        return nullptr;
      }
      auto ivalue = bvalue.at(0);
      if (0x10ffff < ivalue) {
        error_callback.add_error("ValueError: Unicode code point must be in range(0, 0x110000)");
        return nullptr;
      }
      utf8_encode_code_point(ivalue, result, false, true);
      break;
    }
    default:
      error_callback.add_error(std::format("TypeError: '{}' object cannot be interpreted as an integer", value->type()));
      return nullptr;
  }
  return Arena::Create<starlark_string>(&arena, result);
}

starlark_obj* starlark_fn_dict(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  if (pos_args.size() > 1) {
    error_callback.add_error(std::format("TypeError: {} expected at most 1 argument, got {}", "dict", pos_args.size()));
    return nullptr;
  }
  starlark_dictionary* result = Arena::Create<starlark_dictionary>(&arena);
  if (!pos_args.empty()) {
    auto* pos_value = pos_args.front();
    // This is a special case. This should be extended to understand any mapping, but at the moment only `dictionary` implements it.
    if (pos_value->type() == starlark_types::dict_t) {
      auto* it = pos_value->get_iterator(true, arena, error_callback);
      assert(it != nullptr);
      while (it->has_next()) {
        auto* key = it->next();
        assert(key != nullptr);
        auto* value = pos_value->index(*key, arena, error_callback);
        assert(value != nullptr);
        if (result->insert(key, value, error_callback).second) {
          // Should not happen as `pos_value` is already a dictionary.
          return nullptr;
        }
      }
      it->end_iterator();
    } else {
      int pos = 0;
      auto* it = pos_value->get_iterator(true, arena, error_callback);
      if (it == nullptr) {
        return nullptr;
      }
      while (it->has_next()) {
        auto* kv = it->next();
        assert(kv != nullptr);
        auto* it2 = kv->get_iterator(true, arena, error_callback);
        if (it2 == nullptr) {
          return nullptr;
        }
        if (!it2->has_next()) {
          error_callback.add_error(std::format("ValueError: dictionary update sequence element #{} has length 0; 2 is required", pos));
          return nullptr;
        }
        auto* key = it2->next();
        assert(key != nullptr);
        if (!it2->has_next()) {
          error_callback.add_error(std::format("ValueError: dictionary update sequence element #{} has length 1; 2 is required", pos));
          return nullptr;
        }
        auto* value = it2->next();
        assert(value != nullptr);
        if (it2->has_next()) {
          error_callback.add_error(std::format("ValueError: dictionary update sequence element #{} has length {}; 2 is required", pos, kv->len(false, error_callback)));
          return nullptr;
        }
        if (result->insert(key, value, error_callback).second) {
          return nullptr;
        }
        it2->end_iterator();
        pos++;
      }
      it->end_iterator();
    }
  }
  for (auto& [key, value] : named_args) {
    if (result->insert(Arena::Create<starlark_string>(&arena, key), value, error_callback).second) {
      // Should not happen.
      return nullptr;
    }
  }
  return result;
}

starlark_obj* starlark_fn_dir(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_fn_enumerate(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  starlark_obj* start = nullptr;
  for (auto& [key, value] : named_args) {
    if (key == "start") {
      assert(value != nullptr);
      if (value->type() != starlark_types::int_t) {
        error_callback.add_error(std::format("TypeError: parameter 'start' cannot be interpreted as an integer ({}).", value->type()));
        return nullptr;
      }
      start = value;
    } else {
      error_callback.add_error(std::format("Unknown named argument '{}'.", key));
      return nullptr;
    }
  }
  if (pos_args.size() != 1) {
    error_callback.add_error(std::format("TypeError: {}() takes exactly one argument ({} given)", "enumerate", pos_args.size()));
    return nullptr;
  }
  auto* it = pos_args.front()->get_iterator(true, arena, error_callback);
  if (it == nullptr) {
    return nullptr;
  }
  auto* result = Arena::Create<starlark_list>(&arena, std::max<int64_t>(0, pos_args.front()->len(false, error_callback)));
  if (start == nullptr) {
    start = create_integer(0, arena);
  }
  auto* one = create_integer(1, arena);
  while (it->has_next()) {
    auto* tuple = Arena::Create<starlark_tuple>(&arena, 2);
    tuple->add(start);
    tuple->add(it->next());
    result->add(tuple, error_callback);
    start = start->binary_plus(*one, arena, error_callback);
  }
  it->end_iterator();
  return result;
}

starlark_obj* starlark_fn_fail(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  if (!named_args.empty()) {
    error_callback.add_error(std::format("TypeError: {}() takes no keyword arguments", "fail"));
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

starlark_obj* starlark_fn_float(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "float")) {
    return nullptr;
  }
  auto* value = pos_args.front();
  switch (value->numeric_type()) {
    case starlark_numeric_type::kFloat:
      return value;
    case starlark_numeric_type::kInt64:
      return create_float(value->as_int64(), arena);
    case starlark_numeric_type::kBigInt: {
      auto fvalue = to_double(value->as_bigint());
      if (std::isinf(fvalue)) {
        error_callback.add_error("OverflowError: int too large to convert to float");
        return nullptr;
      }
      return create_float(fvalue, arena);
    }
    case starlark_numeric_type::kNotNumeric:
      if (value->type() == starlark_types::string_t) {
        auto svalue = value->as_string();
        errno = 0;
        char* end;
        double double_value = std::strtod(svalue.data(), &end);
        if (end != &svalue.back() + 1) {
          error_callback.add_error(std::format("ValueError: could not convert string to float: '{}'", svalue));
          return nullptr;
        }
        if (errno != 0) {
          error_callback.add_error("OverflowError: floating-point number too large");
          return nullptr;
        }
        return create_float(double_value, arena);
      } else if (value->type() == starlark_types::bool_t) {
        if (value->truthy()) {
          return create_float(1.0, arena);
        } else {
          return create_float(0.0, arena);
        }
      } else {
        error_callback.add_error(std::format("TypeError: float() argument must be a string or a real number, not '{}'", value->type()));
        return nullptr;
      }
  }
}

starlark_obj* starlark_fn_getattr(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_fn_hasattr(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_fn_hash(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "hash")) {
    return nullptr;
  }
  auto* value = pos_args.front();
  if (value->type() == starlark_types::string_t || value->type() == starlark_types::bytes_t) {
    return create_integer(value->hash(), arena);
  } else {
    error_callback.add_error(std::format("TypeError: in call to hash(), got value of type '{}', want 'string' or 'bytes'", value->type()));
    return nullptr;
  }
}

starlark_obj* starlark_fn_int(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  if (!named_args.empty()) {
    error_callback.add_error(std::format("TypeError: {}() takes no keyword arguments", "int"));
    return nullptr;
  }
  if (pos_args.size() != 1 && pos_args.size() != 2) {
    error_callback.add_error(std::format("TypeError: {}() takes one or two argument ({} given)", "int", pos_args.size()));
    return nullptr;
  }
  auto* value = pos_args.front();
  if (value->type() == starlark_types::int_t) {
    if (pos_args.size() == 2) {
      error_callback.add_error("TypeError: int() can't convert non-string with explicit base");
      return nullptr;
    }
    return value;
  } else if (value->type() == starlark_types::float_t) {
    if (pos_args.size() == 2) {
      error_callback.add_error("TypeError: int() can't convert non-string with explicit base");
      return nullptr;
    }
    auto fvalue = value->as_float();
    if (!std::isfinite(fvalue)) {
      if (std::isinf(fvalue)) {
        error_callback.add_error("OverflowError: cannot convert float infinity to integer");
      } else {
        error_callback.add_error("ValueError: cannot convert float NaN to integer");
      }
      return nullptr;
    }
    return create_integer_from_float(fvalue, arena);
  } else if (value->type() == starlark_types::bool_t) {
    if (pos_args.size() == 2) {
      error_callback.add_error("TypeError: int() can't convert non-string with explicit base");
      return nullptr;
    }
    return create_integer(value->truthy() ? 1 : 0, arena);
  } else if (value->type() == starlark_types::string_t) {
    int base = 10;
    if (pos_args.size() == 2) {
      auto* base_param = pos_args[1];
      switch (base_param->numeric_type()) {
        case starlark_numeric_type::kInt64: {
          auto ibase = base_param->as_int64();
          if (ibase != 0 && !(2 <= ibase && ibase <= 36)) {
            error_callback.add_error("ValueError: int() base must be >= 2 and <= 36, or 0");
            return nullptr;
          }
          base = ibase;
          break;
        }
        case starlark_numeric_type::kBigInt: {
          const auto& bbase = base_param->as_bigint();
          if (bbase.sign() || bbase.length() > 1) {
            error_callback.add_error("ValueError: int() base must be >= 2 and <= 36, or 0");
            return nullptr;
          }
          auto ibase = bbase.at(0);
          if (ibase != 0 && !(2 <= ibase && ibase <= 36)) {
            error_callback.add_error("ValueError: int() base must be >= 2 and <= 36, or 0");
            return nullptr;
          }
          base = ibase;
          break;
        }
        default:
          error_callback.add_error(std::format("TypeError: '{}' object cannot be interpreted as an integer", base_param->type()));
          return nullptr;
      }
    }
    auto svalue = value->as_string();
    const char* end;
    auto result = parse_number(svalue, &end, base);
    if (end != (&svalue.back() + 1)) {
      error_callback.add_error(std::format("ValueError: invalid literal for int() with base {}: '{}'", base, svalue));
      return nullptr;
    }
    return create_integer(std::move(result), arena);
  } else {
    error_callback.add_error(std::format("TypeError: int() argument must be a string, int, bool or a real number, not '{}'", value->type()));
    return nullptr;
  }
}

starlark_obj* starlark_fn_len(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "len")) {
    return nullptr;
  }
  auto result = pos_args.front()->len(true, error_callback);
  if (result < 0) {
    return nullptr;
  }
  return create_integer(result, arena);
}

starlark_obj* starlark_fn_list(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  if (!zero_or_one_pos_arg(pos_args, named_args, error_callback, "list")) {
    return nullptr;
  }
  if (pos_args.empty()) {
    return Arena::Create<starlark_list>(&arena, 0);
  }
  auto* it = pos_args.front()->get_iterator(true, arena, error_callback);
  if (it == nullptr) {
    return nullptr;
  }
  auto* result = Arena::Create<starlark_list>(&arena, std::max<int64_t>(0, pos_args.front()->len(false, error_callback)));
  while (it->has_next()) {
    result->add(it->next(), error_callback);
  }
  it->end_iterator();
  return result;
}

starlark_obj* starlark_fn_max(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_fn_min(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_fn_ord(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "ord")) {
    return nullptr;
  }
  auto* value = pos_args.front();
  if (value->type() == starlark_types::string_t) {
    utf8_reader reader(value->as_string(), false, false);
    if (!reader.pending()) {
      error_callback.add_error(std::format("TypeError: ord() expected a character, but {} of length {} found", value->type(), value->len(false, error_callback)));
      return nullptr;
    }
    auto result = reader.peek_code_point();
    reader.skip_code_point();
    if (reader.pending()) {
      error_callback.add_error(std::format("TypeError: ord() expected a character, but {} of length {} found", value->type(), value->len(false, error_callback)));
      return nullptr;
    }
    return create_integer(result, arena);
  } else if (value->type() == starlark_types::bytes_t) {
    if (value->len(false, error_callback) != 1) {
      error_callback.add_error(std::format("TypeError: ord() expected a character, but {} of length {} found", value->type(), value->len(false, error_callback)));
      return nullptr;
    }
    return create_integer(static_cast<unsigned char>(value->as_string()[0]), arena);
  } else {
    error_callback.add_error(std::format("TypeError: ord() expected bytes of length 1 or string with one character, but '{}' found", value->type()));
    return nullptr;
  }
}

starlark_obj* starlark_fn_print(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_fn_range(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_fn_repr(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_fn_reversed(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_fn_set(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_fn_sorted(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_fn_str(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_fn_tuple(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_fn_type(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_fn_zip(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

}  // namespace runtime
}  // namespace starlark


