// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_string.hpp"

#include <cassert>

#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "bigint/number.hpp"
#include "runtime/error_messages.hpp"
#include "runtime/hex_encoder.hpp"
#include "runtime/options.hpp"
#include "runtime/siphash.hpp"
#include "runtime/starlark_numeric.hpp"
#include "runtime/starlark_types.hpp"
#include "unicode/utf8_reader.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;
using ::starlark::unicode::utf8_reader;

namespace starlark {
namespace runtime {

starlark_obj* starlark_string_fn_capitalize(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_codepoint_ords(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_codepoints(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_count(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_elem_ords(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_elems(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_endswith(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_find(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_format(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_index(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_isalnum(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_isalpha(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_isdigit(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_islower(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_isspace(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_istitle(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_isupper(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_join(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_lower(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_lstrip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_partition(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_replace(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_removeprefix(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_removesuffix(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_rfind(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_rindex(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_rpartition(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_rsplit(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_rstrip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_split(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_splitlines(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_startswith(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_strip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_title(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_string_fn_upper(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);

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
    auto code_point = reader.peek_code_point();
    if (code_point != utf8_reader::kReplacementCharacter) {
      write_printable(reader.peek_code_point(), /*allow_non_ascii_printable=*/ true, result);
      reader.skip_code_point();
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

starlark_obj* plus_op(const starlark_string& this_obj, const starlark_obj& other, std::string_view op, Arena& arena, error_fn& error_callback) {
  if (other.type() != this_obj.type()) {
    error_callback.add_error(error_no_concat(this_obj.type(), other.type()));
    return nullptr;
  }
  // TODO(lmirelmann): Check that the value length would not go over the limit.
  std::string result{this_obj.as_string()};
  result += other.as_string();
  return Arena::Create<starlark_string>(&arena, std::move(result));
}

starlark_obj* star_op(const starlark_string& this_obj, const starlark_obj& other, std::string_view op, Arena& arena, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64: {
      if (this_obj.as_string().empty()) {
        return Arena::Create<starlark_string>(&arena, std::string_view{});
      }
      // TODO(lmirelmann): Check whether the size will be over the maximum allowed.
      auto multiplier = other.as_int64();
      std::string result;
      for (int64_t i = 0; i < multiplier; ++i) {
        result += this_obj.as_string();
      }
      return Arena::Create<starlark_string>(&arena, std::move(result));
    }
    case starlark_numeric_type::kBigInt: {
      if (this_obj.as_string().empty()) {
        return Arena::Create<starlark_string>(&arena, std::string_view{});
      }
      const auto& multiplier = other.as_bigint();
      if (multiplier <= number::zero()) {
        return Arena::Create<starlark_string>(&arena, std::string_view{});
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
      return Arena::Create<starlark_string>(&arena, std::move(result));
    }
    default:
      error_callback.add_error(error_no_multiply_sequence(other.type()));
      return nullptr;
  }
}

starlark_obj* percent_op(const starlark_string& this_obj, const starlark_obj& other, std::string_view op, Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

}  // namespace

starlark_obj* starlark_string::binary_plus(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  return plus_op(*this, other, "+", arena, error_callback);
}

starlark_obj* starlark_string::plus_equals_assign(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) {
  return plus_op(*this, other, "+=", arena, error_callback);
}

starlark_obj* starlark_string::binary_star(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  return star_op(*this, other, "*", arena, error_callback);
}

starlark_obj* starlark_string::star_equals_assign(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) {
  return star_op(*this, other, "*=", arena, error_callback);
}

starlark_obj* starlark_string::binary_percent(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  return percent_op(*this, other, "%", arena, error_callback);
}

starlark_obj* starlark_string::percent_equals_assign(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) {
  return percent_op(*this, other, "%=", arena, error_callback);
}

starlark_obj* starlark_string::index(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  auto idx = inner_index(other, value.size(), error_callback);
  if (idx < 0) {
    return nullptr;
  }
  return Arena::Create<starlark_string>(&arena, value.substr(idx, 1));
}

std::string_view starlark_string::as_string() const {
  return value;
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

starlark_obj* starlark_string_fn_capitalize(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_codepoint_ords(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_codepoints(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_count(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): This is not the right implementation of string::count.
  assert(this_obj->type() == starlark_types::string_t);
  return create_integer(this_obj->as_string().size(), arena);
}

starlark_obj* starlark_string_fn_elem_ords(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_elems(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_endswith(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_find(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_format(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_index(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_isalnum(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_isalpha(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_isdigit(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_islower(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_isspace(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_istitle(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_isupper(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_join(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_lower(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_lstrip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_partition(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_replace(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_removeprefix(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_removesuffix(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_rfind(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_rindex(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_rpartition(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_rsplit(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_rstrip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_split(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_splitlines(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_startswith(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_strip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_title(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_string_fn_upper(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

}  // namespace runtime
}  // namespace starlark

