// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_bytes.hpp"

#include <cassert>

#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "runtime/error_messages.hpp"
#include "runtime/hex_encoder.hpp"
#include "runtime/options.hpp"
#include "runtime/siphash.hpp"
#include "runtime/starlark_types.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;

namespace starlark {
namespace runtime {

starlark_obj* starlark_bytes_fn_count(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_elems(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_endswith(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_find(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_index(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_join(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_lstrip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_partition(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_replace(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_rfind(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_rindex(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_rpartition(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_rsplit(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_rstrip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_split(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_startswith(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_strip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);

/*
  Note: It is unclear what methods are needed. This is following
  (https://github.com/bazelbuild/starlark/issues/112) that states:

    - The following string methods would have byte-string counterparts:
      [count endswith find index join lstrip partition replace rfind rindex rpartition
       rsplit rstrip split startswith strip].
      This set excludes methods related to textual concepts such as letter vs number,
      or upper case vs lower. We should perhaps start with a smaller set.
    - The elems method would iterate over the 1-byte substrings, and elem_ords would
      iterate over the numeric byte values.
*/
const std::map<std::string, starlark_obj::fn*, std::less<>>& starlark_bytes::method_refs() {
  static const std::map<std::string, starlark_obj::fn*, std::less<>>* result =
    new std::map<std::string, starlark_obj::fn*, std::less<>>{
      {"count", starlark_bytes_fn_count},
      {"elems", starlark_bytes_fn_elems},
      {"endswith", starlark_bytes_fn_endswith},
      {"find", starlark_bytes_fn_find},
      {"index", starlark_bytes_fn_index},
      {"join", starlark_bytes_fn_join},
      {"lstrip", starlark_bytes_fn_lstrip},
      {"partition", starlark_bytes_fn_partition},
      {"replace", starlark_bytes_fn_replace},
      {"rfind", starlark_bytes_fn_rfind},
      {"rindex", starlark_bytes_fn_rindex},
      {"rpartition", starlark_bytes_fn_rpartition},
      {"rsplit", starlark_bytes_fn_rsplit},
      {"rstrip", starlark_bytes_fn_rstrip},
      {"split", starlark_bytes_fn_split},
      {"startswith", starlark_bytes_fn_startswith},
      {"strip", starlark_bytes_fn_strip},
    };

  return *result;
}

const std::vector<std::string>& starlark_bytes::attributes() {
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

starlark_bytes::starlark_bytes(std::string&& value) : value(value) {}
starlark_bytes::starlark_bytes(std::string_view value) : value(value) {}

std::string_view starlark_bytes::type() const {
  return starlark_types::bytes_t;
}

bool starlark_bytes::primitive() const {
  return true;
}

const std::vector<std::string>& starlark_bytes::dir() const {
  return attributes();
}

const std::map<std::string, starlark_obj::fn*, std::less<>>& starlark_bytes::methods_meta() const {
  return method_refs();
}

int64_t starlark_bytes::len(bool produce_error, error_fn& error_callback) const {
  return value.size();
}

bool starlark_bytes::inner_repr(printer& print, printer_action action) const {
  assert(action == printer_action::kPrintTop);
  std::string result = "b\"";
  for (unsigned char c : value) {
    write_printable(c, /*allow_non_ascii_printable=*/ false, result);
  }
  result += "\"";
  print.append(result);
  return false;
}

bool starlark_bytes::truthy() const {
  return !value.empty();
}

bool starlark_bytes::binary_in(const starlark_obj& other, error_fn& error_callback) const {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64: {
      auto other_value = other.as_int64();
      if (other_value < 0 || 255 < other_value) {
        error_callback.add_error(error_byte_in_range());
        return false;
      }
      return value.contains(static_cast<char>(other.as_int64()));
    }
    case starlark_numeric_type::kBigInt: {
      auto& other_value = other.as_bigint();
      if (other_value.sign() || other_value.bit_size() >= 8) {
        error_callback.add_error(error_byte_in_range());
        return false;
      }
      return value.contains(static_cast<char>(other_value.at(0)));
    }
    case starlark_numeric_type::kNotNumeric: {
      if (other.type() != type()) {
        error_callback.add_error(error_like_required(type(), other.type()));
        return false;
      }
      const starlark_bytes& s_other = static_cast<const starlark_bytes&>(other);
      return value.contains(s_other.value);
    }
    default:
     error_callback.add_error(error_like_required(type(), other.type()));
     return false;
  }
}

namespace {

starlark_obj* plus_op(const starlark_bytes& this_obj, const starlark_obj& other, std::string_view op, Arena& arena, error_fn& error_callback) {
  if (other.type() != this_obj.type()) {
    error_callback.add_error(error_no_concat(this_obj.type(), other.type()));
    return nullptr;
  }
  // TODO(lmirelmann): Check that the value length would not go over the limit.
  std::string value{this_obj.as_string()};
  value += other.as_string();
  return Arena::Create<starlark_bytes>(&arena, std::move(value));
}

starlark_obj* star_op(const starlark_bytes& this_obj, const starlark_obj& other, std::string_view op, Arena& arena, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64: {
      if (this_obj.as_string().empty()) {
        return Arena::Create<starlark_bytes>(&arena, std::string_view{});
      }
      auto multiplier = other.as_int64();
      // TODO(lmirelmann): Check whether the size will be over the maximum allowed.
      std::string result;
      for (int64_t i = 0; i < multiplier; ++i) {
        result += this_obj.as_string();
      }
      return Arena::Create<starlark_bytes>(&arena, std::move(result));
    }
    case starlark_numeric_type::kBigInt: {
      if (this_obj.as_string().empty()) {
        return Arena::Create<starlark_bytes>(&arena, std::string_view{});
      }
      const auto& multiplier = other.as_bigint();
      if (multiplier <= number::zero()) {
        return Arena::Create<starlark_bytes>(&arena, std::string_view{});
      }
      if (multiplier.bit_size() >= 63) {
        error_callback.add_error(error_max_sequence_length(max_string_length()));
        return nullptr;
      }
      int64_t int_value = multiplier.at(0);
      std::string result;
      // TODO(lmirelmann): Check whether the size will be over the maximum allowed.
      for (int64_t i = 0; i < int_value; ++i) {
        result += this_obj.as_string();
      }
      return Arena::Create<starlark_bytes>(&arena, std::move(result));
    }
    default:
      error_callback.add_error(error_no_multiply_sequence(other.type()));
      return nullptr;
  }
}

}  // namespace

starlark_obj* starlark_bytes::binary_plus(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  return plus_op(*this, other, "+", arena, error_callback);
}

starlark_obj* starlark_bytes::plus_equals_assign(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) {
  return plus_op(*this, other, "+=", arena, error_callback);
}

starlark_obj* starlark_bytes::binary_star(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  return star_op(*this, other, "*", arena, error_callback);
}

starlark_obj* starlark_bytes::star_equals_assign(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) {
  return star_op(*this, other, "*=", arena, error_callback);
}

starlark_obj* starlark_bytes::index(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  auto idx = inner_index(other, value.size(), error_callback);
  if (idx < 0) {
    return nullptr;
  }
  return Arena::Create<starlark_bytes>(&arena, value.substr(idx, 1));
}

std::string_view starlark_bytes::as_string() const {
  return value;
}

bool starlark_bytes::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
  return type() == other->type() &&
      value == (static_cast<const starlark_bytes*>(other))->value;
}

void starlark_bytes::inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, error_fn& error_callback) const {
  if (other->type() != type()) {
    starlark_obj::inner_cmp(comp, other, op, error_callback);
    return;
  }
  auto result = value <=> static_cast<const starlark_bytes*>(other)->value;
  if (result != 0) {
    comp.add_task(result > 0 ? order_comparator::pending_task_type::kGreaterThan : order_comparator::pending_task_type::kLessThan);
  }
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_bytes::inner_hash() const {
  if (value.length() == 0) {
    return 0;
  }
  return static_cast<int64_t>(siphash(value.data(), value.length(), 0xA4093822299F31D0, 0x082EFA98EC4E6C89));
}

starlark_obj* starlark_bytes_fn_count(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_bytes_fn_elems(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_bytes_fn_endswith(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_bytes_fn_find(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_bytes_fn_index(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_bytes_fn_join(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_bytes_fn_lstrip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_bytes_fn_partition(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_bytes_fn_replace(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_bytes_fn_rfind(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_bytes_fn_rindex(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_bytes_fn_rpartition(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_bytes_fn_rsplit(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_bytes_fn_rstrip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_bytes_fn_split(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_bytes_fn_startswith(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_bytes_fn_strip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

}  // namespace runtime
}  // namespace starlark


