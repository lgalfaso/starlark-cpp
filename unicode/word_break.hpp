// Copyright 2026 Lucas Mirelmann

#ifndef UNICODE_WORD_BREAK_HPP_
#define UNICODE_WORD_BREAK_HPP_

#include <vector>

namespace starlark {
namespace unicode {

void word_break(const std::vector<std::uint32_t>& code_points, std::vector<std::uint64_t>& output);

}  // namespace unicode
}  // namespace starlark

#endif  // UNICODE_WORD_BREAK_HPP_

