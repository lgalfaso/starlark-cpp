// Copyright 2024 Lucas Mirelmann

#ifndef UNICODE_UTF8_READER_HPP_
#define UNICODE_UTF8_READER_HPP_

#include <cstdint>
#include <string_view>

#pragma GCC visibility push(default)

namespace unicode {

// Whether the code point is within the Unicode range.
bool is_in_range(std::uint32_t code_point);

bool is_surrogate(std::uint32_t code_point);

class utf8_reader {
 public:
  explicit utf8_reader(std::string_view input, bool strict);
  bool empty() const;
  std::size_t pending() const;
  std::size_t pos() const;
  char peek(std::size_t delta = 0) const;
  std::uint32_t peek_code_point() const;
  void skip(std::size_t delta = 1);
  void skip_code_point();
  bool next(std::string_view candidate);
  bool capture(std::string_view candidate);

  // Unicode replacement character.
  static constexpr std::uint32_t replacement_character = 0xfffdu;
  // Unicode byte order mark.
  static constexpr std::uint32_t bom_character = 0xfeffu;
  // Unicode maximum Unicode code point.
  static constexpr std::uint32_t max_code_point = 0x10'ffffu;

 private:
  const std::string_view input;
  std::size_t input_pos = 0;
  bool strict;
};

}  // namespace unicode

#pragma GCC visibility pop

#endif  // UNICODE_UTF8_READER_HPP_

