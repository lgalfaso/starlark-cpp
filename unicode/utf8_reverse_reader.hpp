// Copyright 2026 Lucas Mirelmann

#ifndef UNICODE_UTF8_REVERSE_READER_HPP_
#define UNICODE_UTF8_REVERSE_READER_HPP_

#include <cstdint>
#include <string_view>

#pragma GCC visibility push(default)

namespace starlark {
namespace unicode {

class utf8_reverse_reader {
 public:
  explicit utf8_reverse_reader(std::string_view input, bool strict);
  std::size_t pending() const;
  std::uint32_t peek_code_point();
  std::uint32_t read_code_point();

 private:
  std::uint32_t read_code_point(bool move_forward);

  std::string_view input;
  std::size_t input_pos;
  bool strict;
};

}  // namespace unicode
}  // namespace starlark

#pragma GCC visibility pop

#endif  // UNICODE_UTF8_REVERSE_READER_HPP_

