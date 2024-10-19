// Copyright 2024 Lucas Mirelmann

#ifndef GRAMMAR_SOURCE_HPP_
#define GRAMMAR_SOURCE_HPP_

#include <string_view>

#pragma GCC visibility push(default)

namespace grammar {

class source {
 public:
  explicit source(std::string_view source_code);
  std::uint64_t peek_codepoint() const;

  static const std::uint64_t invalid_codepoint = 0xffff'ffff'ffff'fffful;

 private:
  const std::string_view source_code;
  std::size_t pos = 0;
};

}  // namespace grammar

#pragma GCC visibility pop

#endif  // GRAMMAR_SOURCE_HPP_

