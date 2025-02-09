// Copyright 2024-2025 Lucas Mirelmann

#ifndef UNICODE_NORMALIZATION_HPP_
#define UNICODE_NORMALIZATION_HPP_

#include <string>
#include <string_view>
#include <vector>

namespace starlark {
namespace unicode {

std::string to_nfc(std::string_view input);
std::string to_nfd(std::string_view input);
std::string to_nfkc(std::string_view input);
std::string to_nfkd(std::string_view input);

std::vector<std::uint32_t> to_nfc_x(const std::vector<std::uint32_t>& input);
std::vector<std::uint32_t> to_nfd_x(const std::vector<std::uint32_t>& input);
std::vector<std::uint32_t> to_nfkc_x(const std::vector<std::uint32_t>& input);
std::vector<std::uint32_t> to_nfkd_x(const std::vector<std::uint32_t>& input);

}  // namespace unicode
}  // namespace starlark

#endif  // UNICODE_NORMALIZATION_HPP_

