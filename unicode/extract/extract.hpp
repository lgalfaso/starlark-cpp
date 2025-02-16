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

namespace starlark {
namespace ucd {

void read_raw_code_points(const char* file,
                          std::set<std::uint32_t>& set);

void read_all_code_points(const char* file,
    std::map<std::string,
             std::set<std::pair<std::uint32_t, std::uint32_t>>>& set,
    const std::set<std::string>& properties);

struct unicode_data_record {
  std::uint32_t canonical_combining_class;
  bool canonical_character_decomposition_mapping;
  std::vector<std::uint32_t> character_decomposition_mapping;
  std::string general_category;
};

/*
 * The output parameter `unicode_data` maps the unicode code point to a tuple with the following information:
 * - The Canonical combining class
 * - Whether the Character decomposition mapping is canonical
 * - The Character decomposition mapping
 * - The 
 */
void read_unicode_data(const char* file, std::map<std::uint32_t, unicode_data_record>& unicode_data);

void read_unicode_data(const char* file,
    std::map<std::uint32_t, 
             std::tuple<std::uint32_t, bool, std::vector<std::uint32_t>>>& unicode_data);

}  // namespace ucd
}  // namespace starlark

#endif  // UNICODE_EXTRACT_EXTRACT_HPP_

