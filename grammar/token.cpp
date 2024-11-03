// Copyright 2024 Lucas Mirelmann

#include "grammar/token.hpp"

namespace grammar {

token::token(token_type tok_type, std::size_t tok_start, std::size_t tok_end) : tok_type(tok_type), tok_start(tok_start), tok_end(tok_end) {}

token::token(token_type tok_type, std::size_t tok_start, std::size_t tok_end, std::int64_t value) : tok_type(tok_type), tok_start(tok_start), tok_end(tok_end), value(value) {}

token::token(token_type tok_type, std::size_t tok_start, std::size_t tok_end, double value) : tok_type(tok_type), tok_start(tok_start), tok_end(tok_end), value(value) {}

token::token(token_type tok_type, std::size_t tok_start, std::size_t tok_end, const std::string& value) : tok_type(tok_type), tok_start(tok_start), tok_end(tok_end), value(value) {}

token_type token::type() const {
  return tok_type;
}

void token::set_type(token_type new_type) {
  tok_type = new_type;
}

std::int64_t token::int_value() const {
  if (std::holds_alternative<std::int64_t>(value)) {
    return std::get<std::int64_t>(value);
  }
  return 0;
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

std::size_t token::start() const {
  return tok_start;
}

std::size_t token::end() const {
  return tok_end;
}

const std::string token::empty_string;

}  // namespace grammar
