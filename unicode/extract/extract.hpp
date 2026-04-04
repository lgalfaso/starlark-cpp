// Copyright 2024-2025 Lucas Mirelmann

#ifndef UNICODE_EXTRACT_EXTRACT_HPP_
#define UNICODE_EXTRACT_EXTRACT_HPP_

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace starlark {
namespace ucd {

struct unicode_data_record {
  std::uint32_t canonical_combining_class;
  bool canonical_character_decomposition_mapping;
  std::vector<std::uint32_t> character_decomposition_mapping;
  std::string general_category;
  std::string bidirectional_category;
  bool is_digit;
  std::uint32_t uppercase_mapping;
  std::uint32_t lowercase_mapping;
  std::uint32_t titlecase_mapping;
};

struct special_casing_record {
  std::vector<std::uint32_t> lower;
  std::vector<std::uint32_t> title;
  std::vector<std::uint32_t> upper;
  std::vector<std::string> conditions;
};

void read_raw_code_points(const char* file,
                          std::set<std::uint32_t>& set);

void read_all_code_points(const char* file,
    std::map<std::string,
             std::set<std::pair<std::uint32_t, std::uint32_t>>>& set,
    const std::set<std::string>& properties);

/*
 * The output parameter `unicode_data` maps the unicode code point using the logic from
 * https://www.unicode.org/L2/L1999/UnicodeData.html
 */
void read_unicode_data(const char* file, std::map<std::uint32_t, unicode_data_record>& unicode_data);

void read_special_casing(const char* file, std::map<std::uint32_t, special_casing_record>& special_casing);

void read_word_break(const char* file, std::map<std::pair<std::uint32_t, std::uint32_t>, std::string>& word_break);

}  // namespace ucd
}  // namespace starlark

#endif  // UNICODE_EXTRACT_EXTRACT_HPP_

