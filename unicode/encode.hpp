// Copyright 2024-2025 Lucas Mirelmann

#ifndef UNICODE_ENCODE_HPP_
#define UNICODE_ENCODE_HPP_

#include <cstdint>

#include <string>
#include <vector>

namespace starlark {
namespace unicode {

void utf8_encode_code_point(char32_t code_point, std::string& output, bool strict, bool encode_surrogate);
void utf8_encode(const std::vector<char32_t>& code_points, std::string& output);

}  // namespace unicode
}  // namespace starlark

#endif  // UNICODE_ENCODE_HPP_
