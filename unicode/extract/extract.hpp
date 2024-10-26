// Copyright 2024 Lucas Mirelmann

#ifndef UNICODE_EXTRACT_EXTRACT_HPP_
#define UNICODE_EXTRACT_EXTRACT_HPP_

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <utility>

namespace ucd {

extern std::set<std::string> binary_unicode_properties;

void read_all_codepoints(const char* file,
    std::map<std::string,
             std::set<std::pair<std::uint64_t, std::uint64_t>>>& set);

}  // namespace ucd

#endif  // UNICODE_EXTRACT_EXTRACT_HPP_

