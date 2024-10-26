// Copyright 2024 Lucas Mirelmann

#ifndef UNICODE_ENCODE_HPP_
#define UNICODE_ENCODE_HPP_

#include <cstdint>
#include <string>

namespace ucd {

std::string utf8_encode_code_point(std::uint64_t character);

}  // namespace ucd

#endif  // UNICODE_ENCODE_HPP_

