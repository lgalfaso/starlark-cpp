// Copyright 2024-2026 Lucas Mirelmann

#include <cstdint>
#include <cstdlib>

#include <algorithm>
#include <cctype>
#include <filesystem>  // NOLINT(build/c++17)
#include <format>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "unicode/extract/extract.hpp"

namespace {

constexpr char HPP_HEADER[] = R"CPP(// Copyright 2024-2025 Lucas Mirelmann

// Generated file, do not edit.

#ifndef @@@HEADER_GUARD@@@
#define @@@HEADER_GUARD@@@

#include <cstdint>

#include <array>
#include <optional>
#include <span>

#include "containers/flat_map.hpp"

namespace starlark {
namespace ucd {

enum class word_break_type {
  kCR,
  kLF,
  kNewline,
  kExtend,
  kZWJ,
  kRegional_Indicator,
  kFormat,
  kKatakana,
  kHebrew_Letter,
  kALetter,
  kSingle_Quote,
  kDouble_Quote,
  kMidNumLet,
  kMidLetter,
  kMidNum,
  kNumeric,
  kExtendNumLet,
  kE_Base,
  kE_Modifier,
  kGlue_After_Zwj,
  kE_Base_GAZ,
  kWSegSpace,
  kOther,
};

bool is_assigned(char32_t code_point);

bool is_printable(char32_t code_point);

bool is_compatibility_decomposition(char32_t code_point);

bool is_alpha(char32_t code_point);

bool is_digit(char32_t code_point);

bool is_numeric(char32_t code_point);

int32_t decimal_value(char32_t code_point);

bool is_space(char32_t code_point);

std::span<const char32_t> decomposition(char32_t code_point);

int ccc(char32_t code_point);

std::optional<char32_t> canonical_composition(char32_t lhs, char32_t rhs);

std::optional<std::span<const char32_t>> to_upper(char32_t code_point);

std::optional<std::span<const char32_t>> to_title(char32_t code_point);

std::optional<std::pair<std::span<const char32_t>, std::optional<std::span<const char32_t>>>> to_lower(char32_t code_point);

word_break_type word_break(char32_t code_point);

)CPP";

constexpr char HPP_FOOTER[] = R"CPP(}  // namespace ucd
}  // namespace starlark

#endif  // @@@HEADER_GUARD@@@

)CPP";

constexpr char CPP_HEADER[] = R"CPP(// Copyright 2024-2026 Lucas Mirelmann

// Generated file, do not edit.

#include <bitset>
#include <map>

#include "@@@INCLUDE_H@@@"

namespace starlark {
namespace ucd {

)CPP";

constexpr char CPP_FOOTER[] = R"CPP(}  // namespace ucd
}  // namespace starlark

)CPP";

constexpr int CODEPOINTS_PER_LINE = 64;

std::set<std::string> binary_unicode_properties = {
  "XID_Continue", "XID_Start", "Case_Ignorable", "Cased",
  "Lowercase", "Uppercase", "Extended_Pictographic",
};

std::set<std::string> normalization_properties = {
  "NFC_QC", "NFD_QC", "NFKC_QC", "NFKD_QC",
};

std::set<std::string> numeric_properties = {
  "Numeric", "Digit", "Decimal",
};

template<typename T>
void ostream_write(std::ostream& output, const T& value) {
  output << value;
}

uint32_t format_code_point(char32_t code_point) {
  return static_cast<uint32_t>(code_point);
}

std::string replace_all(std::string_view text, std::string_view placeholder, std::string_view value) {
  std::string result(text);
  std::size_t pos = 0;
  while ((pos = result.find(placeholder, pos)) != std::string::npos) {
    result.replace(pos, placeholder.size(), value);
    pos += value.size();
  }
  return result;
}


void print_in_multiple_lines(const char* characters, int& count, int to_print, std::ostream& output) {
  while (to_print != 0) {
    if (count % CODEPOINTS_PER_LINE == 0) {
      if (count != 0) {
        ostream_write(output, "\"\n");
      }
      ostream_write(output, "    \"");
    }
    int will_print = std::min<int>(CODEPOINTS_PER_LINE - (count % CODEPOINTS_PER_LINE), to_print);
    output.write(characters, will_print);
    count += will_print;
    to_print -= will_print;
  }
}

void print_code_points(
      std::ostream& output,
      const std::set<std::pair<char32_t, char32_t>>& set,
                      const std::string& fn) {
  if (set.size() == 0) {
    ostream_write(output, std::format("bool {}(char32_t code_point) {{\n  return false;\n}}\n\n", fn));
    return;
  }

  std::vector<std::vector<std::pair<char32_t, char32_t>>> blocks;
  {
    // Split into chunks. Each chunk will be a single bitset.
    // There are some oportunities to tweak this number to balance the binary size and speed.
    constexpr int MAX_GAP_SIZE = (1 << 16) - 1;

    bool create_new_block = true;
    char32_t previous_max = 0;
    for (const auto& cps : set) {
      if (create_new_block ||
          cps.first - previous_max > MAX_GAP_SIZE ||
          cps.second - cps.first > MAX_GAP_SIZE) {
        blocks.emplace_back();
      }
      blocks.back().push_back(cps);
      previous_max = cps.second;
      create_new_block = cps.second - cps.first > MAX_GAP_SIZE;
    }
  }
  {
    // Print the strings that will be used to initalize the bitsets.
    // The bitsets position 0 comes from the last character in the string.
    std::string zeros_string(CODEPOINTS_PER_LINE, '0');
    std::string ones_string(CODEPOINTS_PER_LINE, '1');

    for (std::size_t pos = 0; pos < blocks.size(); ++pos) {
      const auto& mini_block = blocks[pos];
      if (mini_block.size() == 1) {
        continue;
      }
      ostream_write(output, std::format("constexpr const char* {}_bitset_{} =\n", fn, pos));
      int count = 0;
      char32_t previous_min = mini_block.back().second + 1;
      for (auto it = mini_block.rbegin(); it < mini_block.rend(); ++it) {
        auto [min_cp, max_cp] = *it;
        print_in_multiple_lines(zeros_string.c_str(), count, previous_min - max_cp - 1, output);
        print_in_multiple_lines(ones_string.c_str(), count, max_cp - min_cp + 1, output);
        previous_min = min_cp;
      }
      ostream_write(output, "\";\n\n");
    }
  }
  {
    // Print the functions that check for the code points.
    ostream_write(output, std::format("bool {}(char32_t code_point) {{\n", fn));
    bool add_blank_line = false;
    for (std::size_t pos = 0; pos < blocks.size(); ++pos) {
      const auto& mini_block = blocks[pos];
      if (mini_block.size() == 1) {
        continue;
      }
      add_blank_line = true;
      ostream_write(output, std::format(
          "  static constexpr std::bitset<0x{:X}> all_cp_{}({}_bitset_{});\n",
          static_cast<uint32_t>(mini_block.back().second + 1 - mini_block.front().first),
          pos, fn, pos));
    }
    if (add_blank_line) {
      ostream_write(output, "\n");
    }
    for (std::size_t pos = 0; pos < blocks.size(); ++pos) {
      const auto& mini_block = blocks[pos];
      if (pos == 0) {
        ostream_write(output, "  return ");
      } else {
        ostream_write(output, " ||\n         ");
      }
      auto min_cp = mini_block.front().first;
      auto max_cp = mini_block.back().second;
      if (min_cp == max_cp) {
        ostream_write(output, std::format("(code_point == 0x{:X})", static_cast<uint32_t>(min_cp)));
      } else if (mini_block.size() != 1) {
        ostream_write(output, std::format("(0x{:X} <= code_point && code_point <= 0x{:X} && all_cp_{}[code_point - 0x{:X}])", static_cast<uint32_t>(min_cp), static_cast<uint32_t>(max_cp), pos, static_cast<uint32_t>(min_cp)));
      } else {
        ostream_write(output, std::format("(0x{:X} <= code_point && code_point <= 0x{:X})", static_cast<uint32_t>(min_cp), static_cast<uint32_t>(max_cp)));
      }
    }
    ostream_write(output, ";\n}\n\n");
  }
}

void print_decomposition(std::ostream& output, const std::map<char32_t, starlark::ucd::unicode_data_record>& unicode_data) {
  ostream_write(output, "std::span<const char32_t> decomposition(char32_t code_point) {\n");
  std::map<char32_t, char32_t> entries;
  std::vector<char32_t> elements;
  for (const auto& entry : unicode_data) {
    const auto& dc = entry.second.character_decomposition_mapping;
    if (dc.size() != 0) {
      for (const auto& element : dc) {
        elements.push_back(element);
      }
      entries[entry.first] = elements.size();
    }
  }
  ostream_write(output, std::format("  static constexpr cnt::flat_map<char32_t, char32_t, {}> all_dc_index = {{", entries.size()));
  int pos = 0;
  for (const auto& entry : entries) {
    if (pos % 6 == 0) {
      ostream_write(output, "\n   ");
    }
    ostream_write(output, std::format(" {{0x{:05X}, 0x{:05X}}}, ", format_code_point(entry.first), format_code_point(entry.second)));
    ++pos;
  }
  ostream_write(output, "\n  };\n");
  ostream_write(output, std::format("  static constexpr std::array<char32_t, {}> all_dc{{", elements.size()));
  pos = 0;
  for (const auto& element : elements) {
    if (pos % 12 == 0) {
      ostream_write(output, "\n   ");
    }
    ostream_write(output, std::format(" 0x{:05X},", format_code_point(element)));
    ++pos;
  }
  ostream_write(output, "\n  };\n\n");
  ostream_write(output, "  if (auto dc_candidate = all_dc_index.find(code_point); dc_candidate != all_dc_index.end()) {\n");
  ostream_write(output, "    std::size_t begin = 0;\n");
  ostream_write(output, "    auto end = dc_candidate->second;\n");
  ostream_write(output, "    if (dc_candidate != all_dc_index.begin()) {\n");
  ostream_write(output, "      begin = (--dc_candidate)->second;\n");
  ostream_write(output, "    }\n");
  ostream_write(output, "    return std::span<const char32_t>(all_dc).subspan(begin, end - begin);\n");
  ostream_write(output, "  }\n");
  ostream_write(output, "  return std::span<const char32_t>{};\n");
  ostream_write(output, "}\n\n");
}

void print_decimal_value(std::ostream& output, const std::map<char32_t, starlark::ucd::unicode_data_record>& unicode_data) {
  ostream_write(output, "int32_t decimal_value(char32_t code_point) {\n");
  std::map<char32_t, int> entries;
  for (const auto& entry : unicode_data) {
    if (entry.second.decimal_value != -1) {
      entries[entry.first] = entry.second.decimal_value;
    }
  }
  ostream_write(output, std::format("  static constexpr cnt::flat_map<char32_t, int32_t, {}> all_decimal_values = {{", entries.size()));
  int pos = 0;
  for (const auto& entry : entries) {
    if (pos % 6 == 0) {
      ostream_write(output, "\n   ");
    }
    ostream_write(output, std::format(" {{0x{:05X}, {}}},", format_code_point(entry.first), entry.second));
    ++pos;
  }
  ostream_write(output, "\n  };\n\n");
  ostream_write(output, "  if (auto decimal_value_candidate = all_decimal_values.find(code_point); decimal_value_candidate != all_decimal_values.end()) {\n");
  ostream_write(output, "    return decimal_value_candidate->second;\n");
  ostream_write(output, "  }\n");
  ostream_write(output, "  return -1;\n");
  ostream_write(output, "}\n\n");
}

void print_ccc(std::ostream& output, const std::map<char32_t, starlark::ucd::unicode_data_record>& unicode_data) {
  ostream_write(output, "int ccc(char32_t code_point) {\n");
  std::map<char32_t, int> entries;
  for (const auto& entry : unicode_data) {
    if (entry.second.canonical_combining_class != 0) {
      entries[entry.first] = entry.second.canonical_combining_class;
    }
  }
  ostream_write(output, std::format("  static constexpr cnt::flat_map<char32_t, int, {}> all_ccc = {{", entries.size()));
  int pos = 0;
  for (const auto& entry : entries) {
    if (pos % 6 == 0) {
      ostream_write(output, "\n   ");
    }
    ostream_write(output, std::format(" {{0x{:05X}, {:3d}}},", format_code_point(entry.first), entry.second));
    ++pos;
  }
  ostream_write(output, "\n  };\n\n");
  ostream_write(output, "  if (auto ccc_candidate = all_ccc.find(code_point); ccc_candidate != all_ccc.end()) {\n");
  ostream_write(output, "    return ccc_candidate->second;\n");
  ostream_write(output, "  }\n");
  ostream_write(output, "  return 0;\n");
  ostream_write(output, "}\n\n");
}

void print_canonical_composition(std::ostream& output, const std::map<char32_t, starlark::ucd::unicode_data_record>& unicode_data,
               const std::set<char32_t>& comp_exclusions) {
  ostream_write(output, "std::optional<char32_t> canonical_composition(char32_t lhs, char32_t rhs) {\n");
  std::map<std::pair<char32_t, char32_t>, char32_t> entries;
  for (const auto& entry : unicode_data) {
    if (entry.second.canonical_combining_class == 0 && entry.second.canonical_character_decomposition_mapping && !comp_exclusions.contains(entry.first)) {
      auto& cc = entry.second.character_decomposition_mapping;
      if (cc.size() == 0 || cc.size() == 1) {
        continue;
      }
      if (cc.size() != 2) {
        exit(1);
      }
      // If the decomposition begins with a non-starter, then this is not a candidate for composition.
      if (unicode_data.at(cc[0]).canonical_combining_class != 0) {
        continue;
      }
      entries[std::make_pair(cc[0], cc[1])] = entry.first;
    }
  }
  ostream_write(output, std::format("  static constexpr cnt::flat_map<std::pair<char32_t, char32_t>, char32_t, {}> all_cc = {{", entries.size()));
  int pos = 0;
  for (const auto& entry : entries) {
    if (pos % 6 == 0) {
      ostream_write(output, "\n   ");
    }
    ostream_write(output, std::format(" {{{{0x{:05X}, 0x{:05X}}}, 0x{:05X}}},",
        format_code_point(entry.first.first), format_code_point(entry.first.second), format_code_point(entry.second)));
    ++pos;
  }
  ostream_write(output, "\n  };\n\n");
  ostream_write(output, "  if (auto cc_candidate = all_cc.find(std::make_pair(lhs, rhs)); cc_candidate != all_cc.end()) {\n");
  ostream_write(output, "    return cc_candidate->second;\n");
  ostream_write(output, "  }\n");
  ostream_write(output, "  return {};\n");
  ostream_write(output, "}\n\n");
}

void print_to_upper(std::ostream& output,
                    const std::map<char32_t, starlark::ucd::unicode_data_record>& unicode_data,
                    const std::map<char32_t, starlark::ucd::special_casing_record>& special_casing) {
  ostream_write(output, "std::optional<std::span<const char32_t>> to_upper(char32_t code_point) {\n");
  std::map<char32_t, char32_t> entries;
  std::vector<char32_t> elements;
  for (const auto& entry : unicode_data) {
    if (special_casing.contains(entry.first)) {
      const auto& special_case = special_casing.at(entry.first);
      // If this special case does no mapping at all, then skip.
      if (special_case.upper.size() == 1 && special_case.upper.front() == entry.first && entry.second.uppercase_mapping == 0x110000) {
        continue;
      }

      // If there is a condition, check whether the condition would cause issues.
      if (!special_case.conditions.empty() &&
          (special_case.upper.size() != 1 || special_case.upper.front() != entry.second.uppercase_mapping)) {
        if (!special_case.conditions.empty()) {
          // We have a case that there is a condition and this condition would not produce the same sequence.
          // This is somethign that we are currently not supporting, so abort.
          exit(1);
        }
      }

      // We are safe, use the sequence from special_cases.
      for (const auto& element : special_case.upper) {
        elements.push_back(element);
      }
      entries[entry.first] = elements.size();
    } else if (entry.second.uppercase_mapping != 0x110000) {
      elements.push_back(entry.second.uppercase_mapping);
      entries[entry.first] = elements.size();
    }
  }
  ostream_write(output, std::format("  static constexpr cnt::flat_map<char32_t, char32_t, {}> all_upper_index = {{", entries.size()));
  int pos = 0;
  for (const auto& entry : entries) {
    if (pos % 6 == 0) {
      ostream_write(output, "\n   ");
    }
    ostream_write(output, std::format(" {{0x{:05X}, 0x{:05X}}},", format_code_point(entry.first), format_code_point(entry.second)));
    ++pos;
  }
  ostream_write(output, "\n  };\n\n");
  ostream_write(output, std::format("  static constexpr std::array<char32_t, {}> all_upper{{", elements.size()));
  pos = 0;
  for (const auto& element : elements) {
    if (pos % 12 == 0) {
      ostream_write(output, "\n   ");
    }
    ostream_write(output, std::format(" 0x{:05X},", format_code_point(element)));
    ++pos;
  }
  ostream_write(output, "\n  };\n\n");
  ostream_write(output, "  if (auto upper_candidate = all_upper_index.find(code_point); upper_candidate != all_upper_index.end()) {\n");
  ostream_write(output, "    std::size_t begin = 0;\n");
  ostream_write(output, "    auto end = upper_candidate->second;\n");
  ostream_write(output, "    if (upper_candidate != all_upper_index.begin()) {\n");
  ostream_write(output, "      begin = (--upper_candidate)->second;\n");
  ostream_write(output, "    }\n");
  ostream_write(output, "    return std::span<const char32_t>(all_upper).subspan(begin, end - begin);\n");
  ostream_write(output, "  }\n");
  ostream_write(output, "  return {};\n");
  ostream_write(output, "}\n\n");
}

void print_to_title(std::ostream& output,
                    const std::map<char32_t, starlark::ucd::unicode_data_record>& unicode_data,
                    const std::map<char32_t, starlark::ucd::special_casing_record>& special_casing) {
  ostream_write(output, "std::optional<std::span<const char32_t>> to_title(char32_t code_point) {\n");
  std::map<char32_t, char32_t> entries;
  std::vector<char32_t> elements;
  for (const auto& entry : unicode_data) {
    if (special_casing.contains(entry.first)) {
      const auto& special_case = special_casing.at(entry.first);
      // If this special case does no mapping at all, then skip.
      if (special_case.title.size() == 1 && special_case.title.front() == entry.first && entry.second.titlecase_mapping == 0x110000) {
        continue;
      }

      // If there is a condition, check whether the condition would cause issues.
      if (!special_case.conditions.empty() &&
          (special_case.title.size() != 1 || special_case.title.front() != entry.second.titlecase_mapping)) {
        if (!special_case.conditions.empty()) {
          // We have a case that there is a condition and this condition would not produce the same sequence.
          // This is somethign that we are currently not supporting, so abort.
          exit(1);
        }
      }

      // We are safe, use the sequence from special_cases.
      for (const auto& element : special_case.title) {
        elements.push_back(element);
      }
      entries[entry.first] = elements.size();
    } else if (entry.second.titlecase_mapping != 0x110000) {
      elements.push_back(entry.second.titlecase_mapping);
      entries[entry.first] = elements.size();
    }
  }
  ostream_write(output, std::format("  static constexpr cnt::flat_map<char32_t, char32_t, {}> all_title_index = {{", entries.size()));
  int pos = 0;
  for (const auto& entry : entries) {
    if (pos % 6 == 0) {
      ostream_write(output, "\n   ");
    }
    ostream_write(output, std::format(" {{0x{:05X}, 0x{:05X}}},", format_code_point(entry.first), format_code_point(entry.second)));
    ++pos;
  }
  ostream_write(output, "\n  };\n\n");
  ostream_write(output, std::format("  static constexpr std::array<char32_t, {}> all_title{{", elements.size()));
  pos = 0;
  for (const auto& element : elements) {
    if (pos % 12 == 0) {
      ostream_write(output, "\n   ");
    }
    ostream_write(output, std::format(" 0x{:05X},", format_code_point(element)));
    ++pos;
  }
  ostream_write(output, "\n  };\n\n");
  ostream_write(output, "  if (auto title_candidate = all_title_index.find(code_point); title_candidate != all_title_index.end()) {\n");
  ostream_write(output, "    std::size_t begin = 0;\n");
  ostream_write(output, "    auto end = title_candidate->second;\n");
  ostream_write(output, "    if (title_candidate != all_title_index.begin()) {\n");
  ostream_write(output, "      begin = (--title_candidate)->second;\n");
  ostream_write(output, "    }\n");
  ostream_write(output, "    return std::span<const char32_t>(all_title).subspan(begin, end - begin);\n");
  ostream_write(output, "  }\n");
  ostream_write(output, "  return {};\n");
  ostream_write(output, "}\n\n");
}

void print_to_lower(std::ostream& output,
                    const std::map<char32_t, starlark::ucd::unicode_data_record>& unicode_data,
                    const std::map<char32_t, starlark::ucd::special_casing_record>& special_casing) {
  ostream_write(output, "std::optional<std::pair<std::span<const char32_t>, std::optional<std::span<const char32_t>>>> to_lower(char32_t code_point) {\n");
  std::map<char32_t, char32_t> entries;
  std::vector<char32_t> elements;
  std::map<char32_t, char32_t> conditional_entries;
  std::vector<char32_t> conditional_elements;
  for (const auto& entry : unicode_data) {
    if (special_casing.contains(entry.first)) {
      const auto& special_case = special_casing.at(entry.first);
      // If this special case does no mapping at all, then skip.
      if (special_case.lower.size() == 1 && special_case.lower.front() == entry.first && entry.second.lowercase_mapping == 0x110000) {
        continue;
      }

      // If there is a condition, check whether the condition would cause issues.
      if (!special_case.conditions.empty() &&
          (special_case.lower.size() != 1 || special_case.lower.front() != entry.second.lowercase_mapping)) {
        bool skip = false;
        for (const auto& condition : special_case.conditions) {
          if (condition == "Final_Sigma") {
            continue;
          }
          // We have a case that there is a condition and this condition would not produce the same sequence.
          // This is somethign that we are currently not supporting, so abort.
          exit(1);
        }
        elements.push_back(entry.second.lowercase_mapping);
        entries[entry.first] = elements.size();
        if (skip) {
          continue;
        }
        for (const auto& element : special_case.lower) {
          conditional_elements.push_back(element);
        }
        conditional_entries[entry.first] = conditional_elements.size();
      } else {
        // We are safe, use the sequence from special_cases.
        for (const auto& element : special_case.lower) {
          elements.push_back(element);
        }
        entries[entry.first] = elements.size();
      }
    } else if (entry.second.lowercase_mapping != 0x110000) {
      elements.push_back(entry.second.lowercase_mapping);
      entries[entry.first] = elements.size();
    }
  }
  ostream_write(output, std::format("  static constexpr cnt::flat_map<char32_t, char32_t, {}> all_lower_index = {{", entries.size()));
  int pos = 0;
  for (const auto& entry : entries) {
    if (pos % 6 == 0) {
      ostream_write(output, "\n   ");
    }
    ostream_write(output, std::format(" {{0x{:05X}, 0x{:05X}}},", format_code_point(entry.first), format_code_point(entry.second)));
    ++pos;
  }
  ostream_write(output, "\n  };\n");
  ostream_write(output, std::format("  static constexpr std::array<char32_t, {}> all_lower{{", elements.size()));
  pos = 0;
  for (const auto& element : elements) {
    if (pos % 12 == 0) {
      ostream_write(output, "\n   ");
    }
    ostream_write(output, std::format(" 0x{:05X},", format_code_point(element)));
    ++pos;
  }
  ostream_write(output, "\n  };\n");

  ostream_write(output, std::format("  static constexpr cnt::flat_map<char32_t, char32_t, {}> conditional_all_lower_index = {{", conditional_entries.size()));
  pos = 0;
  for (const auto& entry : conditional_entries) {
    if (pos % 6 == 0) {
      ostream_write(output, "\n   ");
    }
    ostream_write(output, std::format(" {{0x{:05X}, 0x{:05X}}},", format_code_point(entry.first), format_code_point(entry.second)));
    ++pos;
  }
  ostream_write(output, "\n  };\n");
  ostream_write(output, std::format("  static constexpr std::array<char32_t, {}> conditional_all_lower{{", conditional_elements.size()));
  pos = 0;
  for (const auto& element : conditional_elements) {
    if (pos % 12 == 0) {
      ostream_write(output, "\n   ");
    }
    ostream_write(output, std::format(" 0x{:05X},", format_code_point(element)));
    ++pos;
  }
  ostream_write(output, "\n  };\n\n");

  ostream_write(output, "  if (auto lower_candidate = all_lower_index.find(code_point); lower_candidate != all_lower_index.end()) {\n");
  ostream_write(output, "    std::size_t begin = 0;\n");
  ostream_write(output, "    auto end = lower_candidate->second;\n");
  ostream_write(output, "    if (lower_candidate != all_lower_index.begin()) {\n");
  ostream_write(output, "      begin = (--lower_candidate)->second;\n");
  ostream_write(output, "    }\n");
  ostream_write(output, "    auto entries = std::span<const char32_t>(all_lower).subspan(begin, end - begin);\n");
  ostream_write(output, "    if (auto conditional_lower_candidate = conditional_all_lower_index.find(code_point); conditional_lower_candidate != conditional_all_lower_index.end()) {\n");

  ostream_write(output, "      std::size_t conditional_begin = 0;\n");
  ostream_write(output, "      auto conditional_end = conditional_lower_candidate->second;\n");
  ostream_write(output, "      if (conditional_lower_candidate != conditional_all_lower_index.begin()) {\n");
  ostream_write(output, "        conditional_begin = (--conditional_lower_candidate)->second;\n");
  ostream_write(output, "      }\n");
  ostream_write(output, "      auto conditional_entries = std::span<const char32_t>(conditional_all_lower).subspan(conditional_begin, conditional_end - conditional_begin);\n");
  ostream_write(output, "      return std::pair<std::span<const char32_t>, std::optional<std::span<const char32_t>>>(entries, conditional_entries);\n");

  ostream_write(output, "    }\n");
  ostream_write(output, "    return std::pair<std::span<const char32_t>, std::optional<std::span<const char32_t>>>(entries, {});\n");
  ostream_write(output, "  }\n");
  ostream_write(output, "  return {};\n");
  ostream_write(output, "}\n\n");
}

void print_word_break(std::ostream& output,
                      const std::map<std::pair<char32_t, char32_t>, std::string>& word_break) {
  ostream_write(output, "word_break_type word_break(char32_t code_point) {\n");
  ostream_write(output, std::format("  static constexpr cnt::flat_map<std::pair<char32_t, char32_t>, word_break_type, {}> all_word_break = {{", word_break.size()));
  int pos = 0;
  for (const auto& entry : word_break) {
    if (pos % 6 == 0) {
      ostream_write(output, "\n   ");
    }
    ostream_write(output, std::format(" {{{{0x{:05X}, 0x{:05X}}}, word_break_type::k{}}},",
        format_code_point(entry.first.first), format_code_point(entry.first.second), entry.second));
    ++pos;
  }
  ostream_write(output, "\n  };\n\n");
  ostream_write(output, "  auto up_bound = all_word_break.upper_bound(std::pair<char32_t, char32_t>(code_point, 0x110000));\n");
  ostream_write(output, "  if (up_bound == all_word_break.begin()) {\n");
  ostream_write(output, "    return word_break_type::kOther;\n");
  ostream_write(output, "  }\n");
  ostream_write(output, "  --up_bound;\n");
  ostream_write(output, "  if (up_bound->first.first <= code_point && code_point <= up_bound->first.second) {\n");
  ostream_write(output, "    return up_bound->second;\n");
  ostream_write(output, "  }\n");
  ostream_write(output, "  return word_break_type::kOther;\n");
  ostream_write(output, "}\n\n");
}

std::set<std::pair<char32_t, char32_t>> create_ranges(const std::set<char32_t>& input) {
  std::set<std::pair<char32_t, char32_t>> result;
  char32_t min = 0;
  char32_t previous = 0;
  bool first = true;
  for (const auto& entry : input) {
    if (first) {
      min = entry;
      first = false;
    } else if (previous + 1 != entry) {
      result.emplace(min, previous);
      min = entry;
    }
    previous = entry;
  }
  if (!first) {
    result.emplace(min, previous);
  }
  return result;
}

template<typename T>
std::set<std::pair<char32_t, char32_t>> create_ranges(const std::map<char32_t, T>& input) {
  std::set<char32_t> keys;
  for (const auto& entry : input) {
    keys.insert(entry.first);
  }
  return create_ranges(keys);
}

std::set<std::pair<char32_t, char32_t>> compatibility_set(
    const std::map<char32_t, starlark::ucd::unicode_data_record>& unicode_data) {
  std::set<char32_t> keys;
  for (const auto& entry : unicode_data) {
    if (!entry.second.canonical_character_decomposition_mapping) {
      keys.insert(entry.first);
    }
  }
  return create_ranges(keys);
}

std::set<std::pair<char32_t, char32_t>> printable_set(
    const std::map<char32_t, starlark::ucd::unicode_data_record>& unicode_data) {
  std::set<char32_t> keys;
  for (const auto& entry : unicode_data) {
    if (entry.second.general_category[0] != 'C' && entry.second.general_category[0] != 'Z') {
      keys.insert(entry.first);
    }
  }
  keys.insert(0x20);  // The space character is considered a printable character.
  return create_ranges(keys);
}

std::set<std::pair<char32_t, char32_t>> alpha_set(
    const std::map<char32_t, starlark::ucd::unicode_data_record>& unicode_data) {
  std::set<char32_t> keys;
  for (const auto& entry : unicode_data) {
    if (entry.second.general_category == "Lm" ||
        entry.second.general_category == "Lt" ||
        entry.second.general_category == "Lu" ||
        entry.second.general_category == "Ll" ||
        entry.second.general_category == "Lo") {
      keys.insert(entry.first);
    }
  }
  return create_ranges(keys);
}

std::set<std::pair<char32_t, char32_t>> digits_set(
    const std::map<char32_t, starlark::ucd::unicode_data_record>& unicode_data) {
  std::set<char32_t> keys;
  for (const auto& entry : unicode_data) {
    if (entry.second.is_digit) {
      keys.insert(entry.first);
    }
  }
  return create_ranges(keys);
}

std::set<std::pair<char32_t, char32_t>> numeric_set(
    const std::map<std::string,
             std::set<std::pair<char32_t, char32_t>>>& extracted_numeric_properties) {
  std::set<char32_t> keys;
  for (const auto& entry : extracted_numeric_properties) {
    for (auto r : entry.second) {
      for (int i = r.first; i <= r.second; ++i) {
        keys.insert(i);
      }
    }
  }
  return create_ranges(keys);
}

std::set<std::pair<char32_t, char32_t>> space_set(
    const std::map<char32_t, starlark::ucd::unicode_data_record>& unicode_data) {
  std::set<char32_t> keys;
  for (const auto& entry : unicode_data) {
    if (entry.second.general_category == "Zs" ||
        entry.second.bidirectional_category == "WS" ||
        entry.second.bidirectional_category == "B" ||
        entry.second.bidirectional_category == "S") {
      keys.insert(entry.first);
    }
  }
  return create_ranges(keys);
}

void write_header(const std::filesystem::path& output_file,
                  const char* include_h) {
  std::string header_guard{include_h};
  for (auto& c : header_guard) {
    if (!std::isalnum(c)) {
      c = '_';
    }
    c = std::toupper(c);
  }
  header_guard += '_';

  std::ofstream h_output(output_file);
  if (!h_output) {
    std::exit(1);
  }
  ostream_write(h_output, replace_all(HPP_HEADER, "@@@HEADER_GUARD@@@", header_guard));

  for (const auto& normalization_property : normalization_properties) {
    ostream_write(h_output, std::format("bool is_{}_nm(char32_t);\n\n", normalization_property));
  }
  for (const auto& binary_property : binary_unicode_properties) {
    ostream_write(h_output, std::format("bool is_{}(char32_t);\n\n", binary_property));
  }

  ostream_write(h_output, replace_all(HPP_FOOTER, "@@@HEADER_GUARD@@@", header_guard));
}

void write_impl(const std::filesystem::path& derived_core_properties_file,
                const std::filesystem::path& unicode_data_file,
                const std::filesystem::path& composition_exclusions,
                const std::filesystem::path& derived_normalization_props,
                const std::filesystem::path& prop_list,
                const std::filesystem::path& derived_numeric_type,
                const std::filesystem::path& special_casing_file,
                const std::filesystem::path& word_break_file,
                const std::filesystem::path& emoji_data,
                const std::filesystem::path& output_file,
                const std::filesystem::path& include_h) {
  std::ofstream cc_output(output_file);
  if (!cc_output) {
    std::exit(1);
  }
  ostream_write(cc_output, replace_all(CPP_HEADER, "@@@INCLUDE_H@@@", include_h.string()));

  {
    std::map<char32_t, starlark::ucd::unicode_data_record> unicode_data;
    std::map<char32_t, starlark::ucd::special_casing_record> special_casing;
    std::map<std::pair<char32_t, char32_t>, std::string> word_break;
    std::set<char32_t> comp_exclusions;
    starlark::ucd::read_unicode_data(unicode_data_file, unicode_data);
    starlark::ucd::read_special_casing(special_casing_file, special_casing);
    starlark::ucd::read_word_break(word_break_file, word_break);
    starlark::ucd::read_raw_code_points(composition_exclusions, comp_exclusions);

    print_code_points(cc_output, create_ranges(unicode_data), "is_assigned");
    print_code_points(cc_output, compatibility_set(unicode_data), "is_compatibility_decomposition");
    print_code_points(cc_output, printable_set(unicode_data), "is_printable");
    print_code_points(cc_output, alpha_set(unicode_data), "is_alpha");
    print_code_points(cc_output, digits_set(unicode_data), "is_digit");
    print_code_points(cc_output, space_set(unicode_data), "is_space");
    print_decomposition(cc_output, unicode_data);
    print_decimal_value(cc_output, unicode_data);
    print_ccc(cc_output, unicode_data);
    print_canonical_composition(cc_output, unicode_data, comp_exclusions);
    print_to_upper(cc_output, unicode_data, special_casing);
    print_to_title(cc_output, unicode_data, special_casing);
    print_to_lower(cc_output, unicode_data, special_casing);
    print_word_break(cc_output, word_break);
  }
  {
    std::map<std::string,
             std::set<std::pair<char32_t, char32_t>>> binary_properties;
    starlark::ucd::read_all_code_points(derived_normalization_props,
                              binary_properties,
                              normalization_properties);

    starlark::ucd::read_all_code_points(derived_core_properties_file,
                              binary_properties,
                              binary_unicode_properties);
    starlark::ucd::read_all_code_points(prop_list,
                              binary_properties,
                              binary_unicode_properties);
    starlark::ucd::read_all_code_points(emoji_data,
                              binary_properties,
                              binary_unicode_properties);

    for (const auto& normalization_property : normalization_properties) {
      if (binary_properties[normalization_property].empty()) {
        exit(1);
      }
      std::string name = "is_" + normalization_property + "_nm";
      print_code_points(cc_output, binary_properties[normalization_property],
                       name);
    }
    for (const auto& binary_property : binary_unicode_properties) {
      if (binary_properties[binary_property].empty()) {
        exit(1);
      }
      std::string name = "is_" + binary_property;
      print_code_points(cc_output, binary_properties[binary_property],
                       name);
    }
  }
  {
    std::map<std::string,
             std::set<std::pair<char32_t, char32_t>>> extracted_numeric_properties;
    starlark::ucd::read_all_code_points(derived_numeric_type,
                              extracted_numeric_properties,
                              numeric_properties);
    print_code_points(cc_output, numeric_set(extracted_numeric_properties), "is_numeric");
  }

  ostream_write(cc_output, CPP_FOOTER);
}

}  // namespace

int main(int argc, char *argv[]) {
  if (argc == 13) {
    const std::filesystem::path derived_core_properties_file = argv[1];
    const std::filesystem::path unicode_data_file = argv[2];
    const std::filesystem::path composition_exclusions = argv[3];
    const std::filesystem::path derived_normalization_props = argv[4];
    const std::filesystem::path prop_list = argv[5];
    const std::filesystem::path derived_numeric_type = argv[6];
    const std::filesystem::path special_casing = argv[7];
    const std::filesystem::path word_break = argv[8];
    const std::filesystem::path emoji_data = argv[9];
    const std::filesystem::path output_cpp_file = argv[10];
    const std::filesystem::path output_hpp_file = argv[11];
    const std::filesystem::path include_h = argv[12];

    write_header(output_hpp_file, include_h.c_str());
    write_impl(derived_core_properties_file,
               unicode_data_file,
               composition_exclusions,
               derived_normalization_props,
               prop_list,
               derived_numeric_type,
               special_casing,
               word_break,
               emoji_data,
               output_cpp_file,
               include_h);
  }
}
