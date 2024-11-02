// Copyright 2024 Lucas Mirelmann

#ifndef UNICODE_UTF8_READER_HPP_
#define UNICODE_UTF8_READER_HPP_

#include <string_view>

#pragma GCC visibility push(default)

namespace unicode {

class utf8_reader {
 public:
  explicit utf8_reader(std::string_view input);
  bool empty() const;
  std::size_t pending() const;
  std::size_t pos() const;
  char peek(std::size_t delta = 0) const;
  std::uint64_t peek_code_point() const;
  void skip(std::size_t delta = 1);
  void skip_code_point();
  bool next(std::string_view candidate);
  bool capture(std::string_view candidate);

  // Unicode replacement character.
  static const std::uint64_t replacement_character = 0xfffdul;
  // Unicode byte order mark.
  static const std::uint64_t bom_character = 0xfefful;

 private:
  const std::string_view input;
  std::size_t input_pos = 0;
};

}  // namespace unicode

#pragma GCC visibility pop

#endif  // UNICODE_UTF8_READER_HPP_

