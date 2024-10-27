// Copyright 2024 Lucas Mirelmann

#ifndef GRAMMAR_SOURCE_HPP_
#define GRAMMAR_SOURCE_HPP_

#include <string_view>

#pragma GCC visibility push(default)

namespace grammar {

class source {
 public:
  explicit source(std::string_view input);
  bool pending() const;
  std::size_t pos() const;
  char peek(std::size_t delta = 0) const;
  std::uint64_t peek_codepoint() const;
  void skip(std::size_t delta = 1);
  void skip_codepoint();
  bool capture(std::string_view candidate);

  // Unicode replacement character.
  static const std::uint64_t replacement_character = 0xfffdul;
  // Unicode byte order mark.
  static const std::uint64_t bom_character = 0xfefful;

 private:
  const std::string_view input;
  std::size_t input_pos = 0;
};

}  // namespace grammar

#pragma GCC visibility pop

#endif  // GRAMMAR_SOURCE_HPP_

