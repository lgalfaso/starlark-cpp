// Copyright 2024-2025 Lucas Mirelmann

#ifndef UNICODE_EXTRACT_EXTRACT_HPP_
#define UNICODE_EXTRACT_EXTRACT_HPP_

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace ucd {

void read_raw_code_points(const char* file,
                          std::set<std::uint32_t>& set);

void read_all_code_points(const char* file,
    std::map<std::string,
             std::set<std::pair<std::uint32_t, std::uint32_t>>>& set,
    const std::set<std::string>& properties);

void read_unicode_data(const char* file,
    std::map<std::uint32_t,
             std::tuple<std::uint32_t, bool, std::vector<std::uint32_t>>>& unicode_data);

}  // namespace ucd

#endif  // UNICODE_EXTRACT_EXTRACT_HPP_

