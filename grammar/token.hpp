// Copyright 2024-2025 Lucas Mirelmann

#ifndef GRAMMAR_TOKEN_HPP_
#define GRAMMAR_TOKEN_HPP_

#include <string>
#include <string_view>
#include <variant>

#include "bigint/number.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace grammar {

enum class token_type {
  bof,
  eof,

  ampersand,
  ampersand_equals,
  and_,
  as,
  assert,
  async,
  await,
  break_,
  bytes,
  caret,
  caret_equals,
  class_,
  colon,
  comma,
  continue_,
  def,
  del,
  dot,
  elif,
  else_,
  equals,
  equals_equals,
  except,
  finally,
  float_,
  for_,
  from,
  global,
  greater,
  greater_equals,
  greater_greater,
  greater_greater_equals,
  identifier,
  if_,
  illegal,
  import,
  in,
  indent,
  int_,
  is,
  lambda,
  lbrace,
  lbracket,
  less,
  less_equals,
  less_less,
  less_less_equals,
  load,
  lparen,
  minus,
  minus_equals,
  newline,
  nonlocal,
  not_,
  not_equals,
  or_,
  outdent,
  pass,
  percent,
  percent_equals,
  pipe,
  pipe_equals,
  plus,
  plus_equals,
  raise,
  rbrace,
  rbracket,
  return_,
  rparen,
  semi,
  slash,
  slash_equals,
  slash_slash,
  slash_slash_equals,
  star,
  star_equals,
  star_star,
  string,
  tilde,
  try_,
  while_,
  with,
  yield,
};

struct position {
  std::size_t row;
  std::size_t column;
  std::size_t pos;

  position operator-(std::size_t places) const;
};

class token {
 public:
  token(token_type type, position start, position end);
  token(token_type type, position start, position end, const bigint::number& value);
  token(token_type type, position start, position end, double value);
  token(token_type type, position start, position end, const std::string& value);
  token_type type() const;
  void set_type(token_type new_type);
  const bigint::number& int_value() const;
  double double_value() const;
  const std::string& string_value() const;
  position start() const;
  position end() const;

 private:
  token_type tok_type;
  position tok_start;
  position tok_end;
  std::variant<double, bigint::number, std::string> value;
};

}  // namespace grammar
}  // namespace starlark

#pragma GCC visibility pop

#endif  // GRAMMAR_TOKEN_HPP_

