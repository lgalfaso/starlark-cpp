// Copyright 2024 Lucas Mirelmann

#ifndef GRAMMAR_TOKEN_HPP_
#define GRAMMAR_TOKEN_HPP_

#include <string>
#include <string_view>
#include <variant>

#include "bignum/number.hpp"

#pragma GCC visibility push(default)

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

class token {
 public:
  token(token_type type, std::size_t start, std::size_t end);
  token(token_type type, std::size_t start, std::size_t end, const bignum::number& value);
  token(token_type type, std::size_t start, std::size_t end, double value);
  token(token_type type, std::size_t start, std::size_t end, const std::string& value);
  token_type type() const;
  void set_type(token_type new_type);
  const bignum::number& int_value() const;
  double double_value() const;
  const std::string& string_value() const;
  std::size_t start() const;
  std::size_t end() const;

 private:
  token_type tok_type;
  std::size_t tok_start;
  std::size_t tok_end;
  std::variant<double, bignum::number, std::string> value;

  static const std::string empty_string;
};

}  // namespace grammar

#pragma GCC visibility pop

#endif  // GRAMMAR_TOKEN_HPP_

