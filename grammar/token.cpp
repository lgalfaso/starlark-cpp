// Copyright 2024-2025 Lucas Mirelmann

#include "grammar/token.hpp"

#include <string>

namespace starlark {
namespace grammar {

position operator-(const position& pos, std::size_t places) {
  return position{
    .row = pos.row,
    .column = pos.column - places,
    .pos = pos.pos - places,
  };
}

token::token(token_type tok_type, position tok_start, position tok_end) : tok_type(tok_type), tok_start(tok_start), tok_end(tok_end) {}

token::token(token_type tok_type, position tok_start, position tok_end, const bigint::number& value) : tok_type(tok_type), tok_start(tok_start), tok_end(tok_end), value(value) {}

token::token(token_type tok_type, position tok_start, position tok_end, double value) : tok_type(tok_type), tok_start(tok_start), tok_end(tok_end), value(value) {}

token::token(token_type tok_type, position tok_start, position tok_end, const std::string& value) : tok_type(tok_type), tok_start(tok_start), tok_end(tok_end), value(value) {}

token_type token::type() const {
  return tok_type;
}

void token::set_type(token_type new_type) {
  tok_type = new_type;
}

const bigint::number& token::int_value() const {
  if (std::holds_alternative<bigint::number>(value)) {
    return std::get<bigint::number>(value);
  }
  return bigint::number::zero;
}

double token::double_value() const {
  if (std::holds_alternative<double>(value)) {
    return std::get<double>(value);
  }
  return 0;
}

const std::string& token::string_value() const {
  static const std::string empty_string;

  if (std::holds_alternative<std::string>(value)) {
    return std::get<std::string>(value);
  }
  return empty_string;
}

position token::start() const {
  return tok_start;
}

position token::end() const {
  return tok_end;
}

}  // namespace grammar
}  // namespace starlark
