// Copyright 2024-2025 Lucas Mirelmann

#include "grammar/token.hpp"

#include <string>

namespace grammar {

position position::operator-(std::size_t places) const {
  return position{
    .row = row,
    .column = column - places,
    .pos = pos - places,
  };
}

token::token(token_type tok_type, position tok_start, position tok_end) : tok_type(tok_type), tok_start(tok_start), tok_end(tok_end) {}

token::token(token_type tok_type, position tok_start, position tok_end, const bignum::number& value) : tok_type(tok_type), tok_start(tok_start), tok_end(tok_end), value(value) {}

token::token(token_type tok_type, position tok_start, position tok_end, double value) : tok_type(tok_type), tok_start(tok_start), tok_end(tok_end), value(value) {}

token::token(token_type tok_type, position tok_start, position tok_end, const std::string& value) : tok_type(tok_type), tok_start(tok_start), tok_end(tok_end), value(value) {}

token_type token::type() const {
  return tok_type;
}

void token::set_type(token_type new_type) {
  tok_type = new_type;
}

const bignum::number& token::int_value() const {
  if (std::holds_alternative<bignum::number>(value)) {
    return std::get<bignum::number>(value);
  }
  return bignum::number::zero;
}

double token::double_value() const {
  if (std::holds_alternative<double>(value)) {
    return std::get<double>(value);
  }
  return 0;
}

const std::string& token::string_value() const {
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

const std::string token::empty_string;

}  // namespace grammar
