// Copyright 2024 Lucas Mirelmann

#ifndef UNICODE_ENCODE_HPP_
#define UNICODE_ENCODE_HPP_

#include <cstdint>
#include <string>

namespace unicode {

void utf8_encode_code_point(std::uint32_t character, std::string& output, bool strict);

}  // namespace unicode

#endif  // UNICODE_ENCODE_HPP_

