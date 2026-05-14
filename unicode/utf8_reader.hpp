// Copyright 2024-2025 Lucas Mirelmann

#ifndef UNICODE_UTF8_READER_HPP_
#define UNICODE_UTF8_READER_HPP_

#include <cstdint>
#include <string_view>
#include <utility>

#pragma GCC visibility push(default)

namespace starlark {
namespace unicode {

// Whether the code point is within the Unicode range.
bool is_in_range(char32_t code_point);

bool is_surrogate(char32_t code_point);

bool is_utf8_continue(char input);

std::string_view replacement_character_utf8();

class utf8_reader {
 public:
  explicit utf8_reader(std::string_view input, bool strict, bool remove_bom);
  bool empty() const;
  std::size_t pending() const;
  std::size_t pos() const;
  char peek(std::size_t delta = 0) const;
  std::pair<char32_t, int> peek_code_point();
  void skip(std::size_t delta = 1);
  char32_t read_code_point();
  bool next(std::string_view candidate);
  bool capture(std::string_view candidate);

  // Unicode replacement character.
  static constexpr char32_t kReplacementCharacter = 0xfffdu;
  // Unicode byte order mark.
  static constexpr char32_t kBomCharacter = 0xfeffu;
  // Unicode maximum Unicode code point.
  static constexpr char32_t kMaxCodePoint = 0x10'ffffu;

 private:
  std::pair<char32_t, int> read_code_point(bool move_forward);

  std::string_view input;
  std::size_t input_pos = 0;
  bool strict;
};

}  // namespace unicode
}  // namespace starlark

#pragma GCC visibility pop

#endif  // UNICODE_UTF8_READER_HPP_

