// Copyright 2025-2026 Lucas Mirelmann

#include "runtime/starlark_bytes.hpp"

#include <cassert>

#include <algorithm>
#include <functional>
#include <limits>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "runtime/error_messages.hpp"
#include "runtime/hex_encoder.hpp"
#include "runtime/options.hpp"
#include "runtime/siphash.hpp"
#include "runtime/starlark_list.hpp"
#include "runtime/starlark_numeric.hpp"
#include "runtime/starlark_tuple.hpp"
#include "runtime/starlark_types.hpp"
#include "unicode/ucd_code_points.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;
using ::starlark::ucd::is_White_Space;

namespace starlark {
namespace runtime {

starlark_obj* starlark_bytes_fn_count(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_elems(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_endswith(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_find(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_index(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_join(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_lstrip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_partition(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_replace(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_rfind(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_rindex(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_rpartition(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_rsplit(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_rstrip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_split(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_startswith(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_bytes_fn_strip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);

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
        error_callback.add_error(error_integer_or_like(type(), other.type()));
        return false;
      }
      const starlark_bytes& s_other = static_cast<const starlark_bytes&>(other);
      return value.contains(s_other.value);
    }
    default:
     error_callback.add_error(error_integer_or_like(type(), other.type()));
     return false;
  }
}

namespace {

starlark_obj* plus_op(const starlark_bytes& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  if (other.type() != this_obj.type()) {
    error_callback.add_error(error_no_concat(this_obj.type(), other.type()));
    return nullptr;
  }
  // TODO(lmirelmann): Check that the value length would not go over the limit.
  std::string value{this_obj.as_string()};
  value += other.as_string();
  return Arena::Create<starlark_bytes>(&ctx.arena(), std::move(value));
}

starlark_obj* star_op(const starlark_bytes& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64: {
      if (this_obj.as_string().empty()) {
        return Arena::Create<starlark_bytes>(&ctx.arena(), std::string_view{});
      }
      auto multiplier = other.as_int64();
      // TODO(lmirelmann): Check whether the size will be over the maximum allowed.
      std::string result;
      for (int64_t i = 0; i < multiplier; ++i) {
        result += this_obj.as_string();
      }
      return Arena::Create<starlark_bytes>(&ctx.arena(), std::move(result));
    }
    case starlark_numeric_type::kBigInt: {
      if (this_obj.as_string().empty()) {
        return Arena::Create<starlark_bytes>(&ctx.arena(), std::string_view{});
      }
      const auto& multiplier = other.as_bigint();
      if (multiplier <= number::zero()) {
        return Arena::Create<starlark_bytes>(&ctx.arena(), std::string_view{});
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
      return Arena::Create<starlark_bytes>(&ctx.arena(), std::move(result));
    }
    default:
      error_callback.add_error(error_no_multiply_sequence(other.type()));
      return nullptr;
  }
}

}  // namespace

starlark_obj* starlark_bytes::binary_plus(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return plus_op(*this, other, "+", ctx, error_callback);
}

starlark_obj* starlark_bytes::plus_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return plus_op(*this, other, "+=", ctx, error_callback);
}

starlark_obj* starlark_bytes::binary_star(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return star_op(*this, other, "*", ctx, error_callback);
}

starlark_obj* starlark_bytes::star_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return star_op(*this, other, "*=", ctx, error_callback);
}

starlark_obj* starlark_bytes::index(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  auto idx = inner_index(other, value.size(), error_callback);
  if (idx < 0) {
    return nullptr;
  }
  return Arena::Create<starlark_bytes>(&ctx.arena(), value.substr(idx, 1));
}

starlark_obj* starlark_bytes::slice_range(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, context& ctx, error_fn& error_callback) const {
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
  return Arena::Create<starlark_bytes>(&ctx.arena(), result);
}

std::string_view starlark_bytes::as_string() const {
  return value;
}

int64_t starlark_bytes::count(std::string_view sub, int64_t start, int64_t end) const {
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
  std::string_view reduced_view = ((std::string_view)value).substr(0, end);
  int64_t count = 0;
  for (auto pos = reduced_view.find(sub, start); start <= end && pos != std::string_view::npos; pos = reduced_view.find(sub, start)) {
    count++;
    start = pos + sub.length();
  }
  return count;
}

/*
TODO(lmirelmann): Implement:

starlark_obj* starlark_bytes::elems(context& ctx) const;
*/

bool starlark_bytes::endswith(const std::vector<std::string_view>& ends, int64_t start, int64_t end) const {
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

int64_t starlark_bytes::find(std::string_view sub, int64_t start, int64_t end) const {
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

starlark_obj* starlark_bytes::join(const std::vector<std::string_view>& elements, context& ctx) const {
  bool first = true;
  std::string result;
  for (auto element: elements) {
    if (!first) {
      result += value;
    } else {
      first = false;
    }
    result += element;
  }
  return Arena::Create<starlark_bytes>(&ctx.arena(), std::move(result));
}

starlark_obj* starlark_bytes::lstrip(context& ctx) const {
  std::string::size_type i = 0;
  while (i < value.size() && is_White_Space(value[i])) {
    ++i;
  }
  return Arena::Create<starlark_bytes>(&ctx.arena(), value.substr(i));
}

starlark_obj* starlark_bytes::lstrip(std::string_view cutset, context& ctx) const {
  auto pos = value.find_first_not_of(cutset);
  if (pos == std::string::npos) {
    return Arena::Create<starlark_bytes>(&ctx.arena(), std::string_view());
  }
  return Arena::Create<starlark_bytes>(&ctx.arena(), value.substr(pos));
}

starlark_obj* starlark_bytes::partition(std::string_view sub, context& ctx, error_fn& error_callback) {
  if (sub.empty()) {
    error_callback.add_error(error_empty_separator());
    return nullptr;
  }
  auto pos = value.find(sub);
  auto* result = Arena::Create<starlark_tuple>(&ctx.arena(), 3);
  if (pos == std::string::npos) {
    result->add(this);
    result->add(ctx.empty_bytes());
    result->add(ctx.empty_bytes());
  } else {
    result->add(Arena::Create<starlark_bytes>(&ctx.arena(), value.substr(0, pos)));
    result->add(Arena::Create<starlark_bytes>(&ctx.arena(), sub));
    result->add(Arena::Create<starlark_bytes>(&ctx.arena(), value.substr(pos + sub.length())));
  }
  return result;
}

/*
TODO(lmirelmann): Implement:

starlark_obj* starlark_bytes::replace(std::string_view old, std::string_view new_, int64_t count) const;
*/

int64_t starlark_bytes::rfind(std::string_view sub, int64_t start, int64_t end) const {
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

starlark_obj* starlark_bytes::rpartition(std::string_view sub, context& ctx, error_fn& error_callback) {
  if (sub.empty()) {
    error_callback.add_error(error_empty_separator());
    return nullptr;
  }
  auto pos = value.rfind(sub);
  auto* result = Arena::Create<starlark_tuple>(&ctx.arena(), 3);
  if (pos == std::string::npos) {
    result->add(ctx.empty_bytes());
    result->add(ctx.empty_bytes());
    result->add(this);
  } else {
    result->add(Arena::Create<starlark_bytes>(&ctx.arena(), value.substr(0, pos)));
    result->add(Arena::Create<starlark_bytes>(&ctx.arena(), sub));
    result->add(Arena::Create<starlark_bytes>(&ctx.arena(), value.substr(pos + sub.length())));
  }
  return result;
}

starlark_obj* starlark_bytes::rsplit(int64_t maxsplit, context& ctx) const {
  auto* result = Arena::Create<starlark_list>(&ctx.arena(), 0);
  std::string_view remaining_view = value;
  if (maxsplit < 0) {
    maxsplit = std::numeric_limits<int64_t>::max();
  }
  std::string_view::size_type pos = remaining_view.size();
  for (pos = remaining_view.size(); pos > 0 && is_White_Space(remaining_view[pos - 1]); --pos) {}
  remaining_view = remaining_view.substr(0, pos);
  while (!remaining_view.empty() && maxsplit > 0) {
    for (pos = remaining_view.size(); pos > 0 && !is_White_Space(remaining_view[pos - 1]); --pos) {}
    result->unsafe_append(Arena::Create<starlark_bytes>(&ctx.arena(), remaining_view.substr(pos)));
    remaining_view = remaining_view.substr(0, pos);
    for (pos = remaining_view.size(); pos > 0 && is_White_Space(remaining_view[pos - 1]); --pos) {}
    remaining_view = remaining_view.substr(0, pos);
    maxsplit--;
  }
  if (!remaining_view.empty()) {
    result->unsafe_append(Arena::Create<starlark_bytes>(&ctx.arena(), remaining_view));
  }
  result->unsafe_reverse();
  return result;
}

starlark_obj* starlark_bytes::rsplit(std::string_view sep, int64_t maxsplit, context& ctx, error_fn& error_callback) const {
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
    result->unsafe_append(Arena::Create<starlark_bytes>(&ctx.arena(), remaining_view.substr(pos + sep.length())));
    remaining_view = remaining_view.substr(0, pos);
    maxsplit--;
  }
  result->unsafe_append(Arena::Create<starlark_bytes>(&ctx.arena(), remaining_view));
  result->unsafe_reverse();
  return result;

}

starlark_obj* starlark_bytes::rstrip(context& ctx) const {
  auto i = value.size();
  while (i > 0 && is_White_Space(value[i - 1])) {
    --i;
  }
  return Arena::Create<starlark_bytes>(&ctx.arena(), value.substr(0, i));
}

starlark_obj* starlark_bytes::rstrip(std::string_view cutset, context& ctx) const {
  auto pos = value.find_last_not_of(cutset);
  if (pos == std::string::npos) {
    return Arena::Create<starlark_bytes>(&ctx.arena(), std::string_view());
  }
  return Arena::Create<starlark_bytes>(&ctx.arena(), value.substr(0, pos + 1));
}

starlark_obj* starlark_bytes::split(int64_t maxsplit, context& ctx) const {
  auto* result = Arena::Create<starlark_list>(&ctx.arena(), 0);
  std::string_view remaining_view = value;
  if (maxsplit < 0) {
    maxsplit = std::numeric_limits<int64_t>::max();
  }
  std::string_view::size_type pos = 0;
  for (pos = 0; pos < remaining_view.size() && is_White_Space(remaining_view[pos]); ++pos) {}
  remaining_view = remaining_view.substr(pos);
  while (!remaining_view.empty() && maxsplit > 0) {
    for (pos = 0; pos < remaining_view.size() && !is_White_Space(remaining_view[pos]); ++pos) {}
    result->unsafe_append(Arena::Create<starlark_bytes>(&ctx.arena(), remaining_view.substr(0, pos)));
    remaining_view = remaining_view.substr(pos);
    for (pos = 0; pos < remaining_view.size() && is_White_Space(remaining_view[pos]); ++pos) {}
    remaining_view = remaining_view.substr(pos);
    maxsplit--;
  }
  if (!remaining_view.empty()) {
    result->unsafe_append(Arena::Create<starlark_bytes>(&ctx.arena(), remaining_view));
  }
  return result;
}

starlark_obj* starlark_bytes::split(std::string_view sep, int64_t maxsplit, context& ctx, error_fn& error_callback) const {
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
    result->unsafe_append(Arena::Create<starlark_bytes>(&ctx.arena(), remaining_view.substr(0, pos)));
    remaining_view = remaining_view.substr(pos + sep.length());
    maxsplit--;
  }
  result->unsafe_append(Arena::Create<starlark_bytes>(&ctx.arena(), remaining_view));
  return result;
}

bool starlark_bytes::startswith(const std::vector<std::string_view>& begins, int64_t start, int64_t end) const {
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

starlark_obj* starlark_bytes::strip(context& ctx) const {
  std::string_view view = value;
  std::string::size_type start, end;
  for (start = 0; start < view.size() && is_White_Space(view[start]); ++start) {}
  for (end = view.size(); end > start && is_White_Space(view[end - 1]); --end) {}
  return Arena::Create<starlark_bytes>(&ctx.arena(), view.substr(start, end - start));
}

starlark_obj* starlark_bytes::strip(std::string_view cutset, context& ctx) const {
  std::string_view view = value;
  auto pos = view.find_first_not_of(cutset);
  if (pos == std::string_view::npos) {
    return Arena::Create<starlark_bytes>(&ctx.arena(), std::string_view());
  }
  view = view.substr(pos);
  pos = view.find_last_not_of(cutset);
  // `pos != std::string_view::npos` as in the previous check, we know that
  // there is at least one element that does not match cutset.
  return Arena::Create<starlark_bytes>(&ctx.arena(), view.substr(0, pos + 1));
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

namespace {

status_or<std::string_view> bytes_or_int_as_bytes(const starlark_obj* element, error_fn& error_callback) {
  static char all_chars[257] =
      "\000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037"
      "\040\041\042\043\044\045\046\047\050\051\052\053\054\055\056\057\060\061\062\063\064\065\066\067\070\071\072\073\074\075\076\077"
      "\100\101\102\103\104\105\106\107\110\111\112\113\114\115\116\117\120\121\122\123\124\125\126\127\130\131\132\133\134\135\136\137"
      "\140\141\142\143\144\145\146\147\150\151\152\153\154\155\156\157\160\161\162\163\164\165\166\167\170\171\172\173\174\175\176\177"
      "\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217\220\221\222\223\224\225\226\227\230\231\232\233\234\235\236\237"
      "\240\241\242\243\244\245\246\247\250\251\252\253\254\255\256\257\260\261\262\263\264\265\266\267\270\271\272\273\274\275\276\277"
      "\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337"
      "\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356\357\360\361\362\363\364\365\366\367\370\371\372\373\374\375\376\377";
  switch (element->numeric_type()) {
    case starlark_numeric_type::kInt64: {
      auto other_value = element->as_int64();
      if (other_value < 0 || 255 < other_value) {
        error_callback.add_error(error_byte_in_range());
        return status_or<std::string_view>(status_code::kError);
      }
      return status_or<std::string_view>(std::string_view(&all_chars[other_value], 1));
    }
    case starlark_numeric_type::kBigInt: {
      auto& other_value = element->as_bigint();
      if (other_value.sign() || other_value.bit_size() >= 8) {
        error_callback.add_error(error_byte_in_range());
        return status_or<std::string_view>(status_code::kError);
      }
      return status_or<std::string_view>(std::string_view(&all_chars[other_value.at(0)], 1));
    }
    case starlark_numeric_type::kNotNumeric: {
      if (element->type() != starlark_types::bytes_t) {
        error_callback.add_error(error_integer_or_like(starlark_types::bytes_t, element->type()));
        return status_or<std::string_view>(status_code::kError);
      }
      return status_or<std::string_view>(element->as_string());
    }
    default:
      error_callback.add_error(error_integer_or_like(starlark_types::bytes_t, element->type()));
      return status_or<std::string_view>(status_code::kError);
  }
}

status_or<std::vector<std::string_view>> bytes_int_or_tuple_as_vector_of_bytes(const starlark_obj* element, error_fn& error_callback) {
  std::vector<std::string_view> result;
  if (element->type() == starlark_types::tuple_t) {
    auto* tuple = static_cast<const starlark_tuple*>(element);
    for (int i = 0; i < tuple->size(); ++i) {
      // TODO(lmirelmann): The error is not the same, it should be
      // TypeError: tuple for endswith must only contain str, not int
      auto entry = bytes_or_int_as_bytes(tuple->at(i), error_callback);
      if (!entry.ok()) {
        return status_or<std::vector<std::string_view>>(status_code::kError);
      }
      result.emplace_back(*entry);
    }
  } else {
    auto entry = bytes_or_int_as_bytes(element, error_callback);
    if (!entry.ok()) {
      return status_or<std::vector<std::string_view>>(status_code::kError);
    }
    result.emplace_back(*entry);
  }
  return status_or<std::vector<std::string_view>>(result);
}

status_or<std::pair<int64_t, int64_t>> get_start_and_end(const starlark_obj::pos_args_t& pos_args, error_fn& error_callback) {
  int64_t start = std::numeric_limits<int64_t>::min();
  int64_t end = std::numeric_limits<int64_t>::max();
  if (pos_args.size() >= 2) {
    if (!to_int64_with_clamping_for_index_allow_none(*pos_args[1], start, error_callback)) {
      return status_or<std::pair<int64_t, int64_t>>(status_code::kError);
    }
    if (pos_args.size() >= 3) {
      if (!to_int64_with_clamping_for_index_allow_none(*pos_args[2], end, error_callback)) {
        return status_or<std::pair<int64_t, int64_t>>(status_code::kError);
      }
    }
  }
  return status_or<std::pair<int64_t, int64_t>>(std::make_pair(start, end));
}

}  // namespace

starlark_obj* starlark_bytes_fn_count(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "bytes.count") ||
      !min_args(pos_args, error_callback, "count", 1) ||
      !max_args(pos_args, error_callback, "count", 3)) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::bytes_t);
  auto sub = bytes_or_int_as_bytes(pos_args.front(), error_callback);
  if (!sub.ok()) {
    return nullptr;
  }
  auto start_end = get_start_and_end(pos_args, error_callback);
  if (!start_end.ok()) {
    return nullptr;
  }
  return create_integer(static_cast<starlark_bytes*>(this_obj)->count(*sub, start_end->first, start_end->second), ctx);
}

starlark_obj* starlark_bytes_fn_elems(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_bytes_fn_endswith(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "bytes.endswith") ||
      !min_args(pos_args, error_callback, "endswith", 1) ||
      !max_args(pos_args, error_callback, "endswith", 3)) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::bytes_t);
  auto subs = bytes_int_or_tuple_as_vector_of_bytes(pos_args.front(), error_callback);
  if (!subs.ok()) {
    return nullptr;
  }
  auto start_end = get_start_and_end(pos_args, error_callback);
  if (!start_end.ok()) {
    return nullptr;
  }
  return static_cast<starlark_bytes*>(this_obj)->endswith(*subs, start_end->first, start_end->second) ? ctx.true_value() : ctx.false_value();
}

starlark_obj* starlark_bytes_fn_find(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "bytes.find") ||
      !min_args(pos_args, error_callback, "find", 1) ||
      !max_args(pos_args, error_callback, "find", 3)) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::bytes_t);
  auto sub = bytes_or_int_as_bytes(pos_args.front(), error_callback);
  if (!sub.ok()) {
    return nullptr;
  }
  auto start_end = get_start_and_end(pos_args, error_callback);
  if (!start_end.ok()) {
    return nullptr;
  }
  return create_integer(static_cast<starlark_bytes*>(this_obj)->find(*sub, start_end->first, start_end->second), ctx);
}

starlark_obj* starlark_bytes_fn_index(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "bytes.index") ||
      !min_args(pos_args, error_callback, "index", 1) ||
      !max_args(pos_args, error_callback, "index", 3)) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::bytes_t);
  auto sub = bytes_or_int_as_bytes(pos_args.front(), error_callback);
  if (!sub.ok()) {
    return nullptr;
  }
  auto start_end = get_start_and_end(pos_args, error_callback);
  if (!start_end.ok()) {
    return nullptr;
  }
  auto result = static_cast<starlark_bytes*>(this_obj)->find(*sub, start_end->first, start_end->second);
  if (result < 0) {
    error_callback.add_error(error_substring_not_found());
    return nullptr;
  }
  return create_integer(result, ctx);
}

starlark_obj* starlark_bytes_fn_join(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "join")) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::bytes_t);

  auto it = pos_args.front()->get_iterator(true, ctx, error_callback);
  if (it == nullptr) {
    // TODO(lmirelmann): The error is not exactly the same, Python produces the following error:
    // `TypeError: can only join an iterable`
    return nullptr;
  }
  std::vector<std::string_view> elements;
  while (it->has_next()) {
    auto element = bytes_or_int_as_bytes(it->next(), error_callback);
    if (!element.ok()) {
      return nullptr;
      it->end_iterator();
    }
    elements.push_back(*element);
  }
  it->end_iterator();
  return static_cast<starlark_bytes*>(this_obj)->join(elements, ctx);
}

starlark_obj* starlark_bytes_fn_lstrip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!zero_or_one_pos_arg(pos_args, named_args, error_callback, "lstrip")) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::bytes_t);
  if (pos_args.empty()) {
    return static_cast<starlark_bytes*>(this_obj)->lstrip(ctx);
  }
  auto cutset = bytes_or_int_as_bytes(pos_args.front(), error_callback);
  if (!cutset.ok()) {
    return nullptr;
  }
  return static_cast<starlark_bytes*>(this_obj)->lstrip(*cutset, ctx);
}

starlark_obj* starlark_bytes_fn_partition(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "partition")) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::bytes_t);

  auto separator = bytes_or_int_as_bytes(pos_args.front(), error_callback);
  if (!separator.ok()) {
    return nullptr;
  }
  return static_cast<starlark_bytes*>(this_obj)->partition(*separator, ctx, error_callback);
}

starlark_obj* starlark_bytes_fn_replace(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_bytes_fn_rfind(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "bytes.rfind") ||
      !min_args(pos_args, error_callback, "rfind", 1) ||
      !max_args(pos_args, error_callback, "rfind", 3)) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::bytes_t);
  auto sub = bytes_or_int_as_bytes(pos_args.front(), error_callback);
  if (!sub.ok()) {
    return nullptr;
  }
  auto start_end = get_start_and_end(pos_args, error_callback);
  if (!start_end.ok()) {
    return nullptr;
  }
  return create_integer(static_cast<starlark_bytes*>(this_obj)->rfind(*sub, start_end->first, start_end->second), ctx);
}

starlark_obj* starlark_bytes_fn_rindex(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "bytes.rindex") ||
      !min_args(pos_args, error_callback, "rindex", 1) ||
      !max_args(pos_args, error_callback, "rindex", 3)) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::bytes_t);
  auto sub = bytes_or_int_as_bytes(pos_args.front(), error_callback);
  if (!sub.ok()) {
    return nullptr;
  }
  auto start_end = get_start_and_end(pos_args, error_callback);
  if (!start_end.ok()) {
    return nullptr;
  }
  auto result = static_cast<starlark_bytes*>(this_obj)->rfind(*sub, start_end->first, start_end->second);
  if (result < 0) {
    error_callback.add_error(error_substring_not_found());
    return nullptr;
  }
  return create_integer(result, ctx);
}

starlark_obj* starlark_bytes_fn_rpartition(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "rpartition")) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::bytes_t);

  auto separator = bytes_or_int_as_bytes(pos_args.front(), error_callback);
  if (!separator.ok()) {
    return nullptr;
  }
  return static_cast<starlark_bytes*>(this_obj)->rpartition(*separator, ctx, error_callback);
}

starlark_obj* starlark_bytes_fn_rsplit(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "bytes.rsplit") ||
      !max_args(pos_args, error_callback, "rsplit", 2)) {
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
    return static_cast<starlark_bytes*>(this_obj)->rsplit(maxsplit, ctx);
  }
  auto sep = bytes_or_int_as_bytes(pos_args.front(), error_callback);
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
  return static_cast<starlark_bytes*>(this_obj)->rsplit(*sep, maxsplit, ctx, error_callback);
}

starlark_obj* starlark_bytes_fn_rstrip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!zero_or_one_pos_arg(pos_args, named_args, error_callback, "rstrip")) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::bytes_t);
  if (pos_args.empty()) {
    return static_cast<starlark_bytes*>(this_obj)->rstrip(ctx);
  }
  auto cutset = bytes_or_int_as_bytes(pos_args.front(), error_callback);
  if (!cutset.ok()) {
    return nullptr;
  }
  return static_cast<starlark_bytes*>(this_obj)->rstrip(*cutset, ctx);
}

starlark_obj* starlark_bytes_fn_split(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "bytes.split") ||
      !max_args(pos_args, error_callback, "split", 2)) {
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
    return static_cast<starlark_bytes*>(this_obj)->split(maxsplit, ctx);
  }
  auto sep = bytes_or_int_as_bytes(pos_args.front(), error_callback);
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
  return static_cast<starlark_bytes*>(this_obj)->split(*sep, maxsplit, ctx, error_callback);
}

starlark_obj* starlark_bytes_fn_startswith(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "bytes.startswith") ||
      !min_args(pos_args, error_callback, "startswith", 1) ||
      !max_args(pos_args, error_callback, "startswith", 3)) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::bytes_t);
  auto subs = bytes_int_or_tuple_as_vector_of_bytes(pos_args.front(), error_callback);
  if (!subs.ok()) {
    return nullptr;
  }
  int64_t start = std::numeric_limits<int64_t>::min();
  int64_t end = std::numeric_limits<int64_t>::max();
  if (pos_args.size() >= 2) {
    if (!to_int64_with_clamping_for_index_allow_none(*pos_args[1], start, error_callback)) {
      return nullptr;
    }
    if (pos_args.size() >= 3) {
      if (!to_int64_with_clamping_for_index_allow_none(*pos_args[2], end, error_callback)) {
        return nullptr;
      }
    }
  }
  return static_cast<starlark_bytes*>(this_obj)->startswith(*subs, start, end) ? ctx.true_value() : ctx.false_value();
}

starlark_obj* starlark_bytes_fn_strip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!zero_or_one_pos_arg(pos_args, named_args, error_callback, "strip")) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::bytes_t);
  if (pos_args.empty()) {
    return static_cast<starlark_bytes*>(this_obj)->strip(ctx);
  }
  auto cutset = bytes_or_int_as_bytes(pos_args.front(), error_callback);
  if (!cutset.ok()) {
    return nullptr;
  }
  return static_cast<starlark_bytes*>(this_obj)->strip(*cutset, ctx);
}

}  // namespace runtime
}  // namespace starlark


