// Copyright 2024-2026 Lucas Mirelmann

#include <cstring>

#include <algorithm>
#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "unicode/extract/extract.hpp"

namespace {

const char* HPP_HEADER = R"CPP(// Copyright 2024-2025 Lucas Mirelmann

// Generated file, do not edit.

#ifndef %s
#define %s

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

bool is_assigned(std::uint32_t code_point);

bool is_printable(std::uint32_t code_point);

bool is_compatibility_decomposition(std::uint32_t code_point);

bool is_alpha(std::uint32_t code_point);

bool is_digit(std::uint32_t code_point);

bool is_numeric(std::uint32_t code_point);

bool is_space(std::uint32_t code_point);

std::span<const std::uint32_t> decomposition(std::uint32_t code_point);

int ccc(std::uint32_t code_point);

std::optional<std::uint32_t> canonical_composition(std::uint32_t lhs, std::uint32_t rhs);

// TODO(lmirelmann): Should be possible to output the encoded bytes un a string_view. This will
// make the execution of whomever need this much faster as it would not need to re-encode things
// multiple times. It hsould also sabe a few bytes as keeping the code points takes more space
// than keeping the encoded bytes.
std::span<const std::uint32_t> to_upper(std::uint32_t code_point);

std::span<const std::uint32_t> to_title(std::uint32_t code_point);

std::pair<std::span<const std::uint32_t>, std::optional<std::span<const std::uint32_t>>> to_lower(std::uint32_t code_point);

word_break_type word_break(std::uint32_t code_point);

)CPP";

const char* HPP_FOOTER = R"CPP(}  // namespace ucd
}  // namespace starlark

#endif  // %s

)CPP";

const char* CPP_HEADER = R"CPP(// Copyright 2024-2026 Lucas Mirelmann

// Generated file, do not edit.

#include <bitset>
#include <map>

#include "%s"

namespace starlark {
namespace ucd {

)CPP";

const char* CPP_FOOTER = R"CPP(}  // namespace ucd
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

#define FWRITE(STR, OUTPUT) fwrite(STR, sizeof(char), std::strlen(STR), OUTPUT)

void print_in_multiple_lines(const char* characters, int& count, uint32_t to_print, FILE* output) {
  while (to_print != 0) {
    if (count % CODEPOINTS_PER_LINE == 0) {
      if (count != 0) {
        FWRITE("\"\n", output);
      }
      FWRITE("    \"", output);
    }
    uint32_t will_print = std::min<uint32_t>(CODEPOINTS_PER_LINE - (count % CODEPOINTS_PER_LINE), to_print);
    fwrite(characters, sizeof(char), will_print, output);
    count += will_print;
    to_print -= will_print;
  }
}

void print_code_points(
      FILE* output,
      const std::set<std::pair<std::uint32_t, std::uint32_t>>& set,
                      const std::string& fn) {
  if (set.size() == 0) {
    fprintf(output,
            "bool %s(std::uint32_t code_point) {\n"
            "  return false;\n"
            "}\n\n",
            fn.c_str());
    return;
  }

  std::vector<std::vector<std::pair<std::uint32_t, std::uint32_t>>> blocks;
  {
    // Split into chunks. Each chunk will be a single bitset.
    // There are some oportunities to tweak this number to balance the binary size and speed.
    constexpr int MAX_GAP_SIZE = (1 << 16) - 1;

    bool create_new_block = true;
    std::uint32_t previous_max = 0;
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
      fprintf(output, "constexpr const char* %s_bitset_%lu =\n", fn.c_str(), pos);
      int count = 0;
      std::uint32_t previous_min = mini_block.back().second + 1;
      for (auto it = mini_block.rbegin(); it < mini_block.rend(); ++it) {
        auto [min_cp, max_cp] = *it;
        print_in_multiple_lines(zeros_string.c_str(), count, previous_min - max_cp - 1, output);
        print_in_multiple_lines(ones_string.c_str(), count, max_cp - min_cp + 1, output);
        previous_min = min_cp;
      }
      FWRITE("\";\n\n", output);
    }
  }
  {
    // Print the functions that check for the code points.
    fprintf(output, "bool %s(std::uint32_t code_point) {\n", fn.c_str());
    bool add_blank_line = false;
    for (std::size_t pos = 0; pos < blocks.size(); ++pos) {
      const auto& mini_block = blocks[pos];
      if (mini_block.size() == 1) {
        continue;
      }
      add_blank_line = true;
      fprintf(output,
              "  static constexpr std::bitset<0x%X> all_cp_%lu(%s_bitset_%lu);\n",
              mini_block.back().second + 1 - mini_block.front().first,
              pos, fn.c_str(), pos);
    }
    if (add_blank_line) {
      FWRITE("\n", output);
    }
    for (std::size_t pos = 0; pos < blocks.size(); ++pos) {
      const auto& mini_block = blocks[pos];
      if (pos == 0) {
        FWRITE("  return ", output);
      } else {
        FWRITE(" ||\n         ", output);
      }
      auto min_cp = mini_block.front().first;
      auto max_cp = mini_block.back().second;
      if (min_cp == max_cp) {
        fprintf(output, "(code_point == 0x%X)", min_cp);
      } else if (mini_block.size() != 1) {
        fprintf(output, "(0x%X <= code_point && code_point <= 0x%X && all_cp_%lu[code_point - 0x%X])", min_cp, max_cp, pos, min_cp);
      } else {
        fprintf(output, "(0x%X <= code_point && code_point <= 0x%X)", min_cp, max_cp);
      }
    }
    FWRITE(";\n}\n\n", output);
  }
}

void print_decomposition(FILE* output, const std::map<std::uint32_t, starlark::ucd::unicode_data_record>& unicode_data) {
  FWRITE("std::span<const std::uint32_t> decomposition(std::uint32_t code_point) {\n", output);
  std::map<std::uint32_t, std::uint32_t> entries;
  std::vector<std::uint32_t> elements;
  for (const auto& entry : unicode_data) {
    const auto& dc = entry.second.character_decomposition_mapping;
    if (dc.size() != 0) {
      for (const auto& element : dc) {
        elements.push_back(element);
      }
      entries[entry.first] = elements.size();
    }
  }
  fprintf(output, "  static constexpr cnt::flat_map<std::uint32_t, std::uint32_t, %zu> all_dc_index = {", entries.size());
  int pos = 0;
  for (const auto& entry : entries) {
    if (pos % 6 == 0) {
      FWRITE("\n   ", output);
    }
    fprintf(output, " {0x%05X, 0x%05X}, ", entry.first, entry.second);
    ++pos;
  }
  FWRITE("\n  };\n", output);
  fprintf(output, "  static constexpr std::array<std::uint32_t, %zu> all_dc{", elements.size());
  pos = 0;
  for (const auto& element : elements) {
    if (pos % 12 == 0) {
      FWRITE("\n   ", output);
    }
    fprintf(output, " 0x%05X,", element);
    ++pos;
  }
  FWRITE("\n  };\n\n", output);
  FWRITE("  if (auto dc_candidate = all_dc_index.find(code_point); dc_candidate != all_dc_index.end()) {\n", output);
  FWRITE("    std::size_t begin = 0;\n", output);
  FWRITE("    auto end = dc_candidate->second;\n", output);
  FWRITE("    if (dc_candidate != all_dc_index.begin()) {\n", output);
  FWRITE("      begin = (--dc_candidate)->second;\n", output);
  FWRITE("    }\n", output);
  FWRITE("    return std::span<const std::uint32_t>(all_dc).subspan(begin, end - begin);\n", output);
  FWRITE("  }\n", output);
  FWRITE("  return std::span<const std::uint32_t>{};\n", output);
  FWRITE("}\n\n", output);
}

void print_ccc(FILE* output, const std::map<std::uint32_t, starlark::ucd::unicode_data_record>& unicode_data) {
  FWRITE("int ccc(std::uint32_t code_point) {\n", output);
  std::map<std::uint32_t, int> entries;
  for (const auto& entry : unicode_data) {
    if (entry.second.canonical_combining_class != 0) {
      entries[entry.first] = entry.second.canonical_combining_class;
    }
  }
  fprintf(output, "  static constexpr cnt::flat_map<std::uint32_t, int, %zu> all_ccc = {", entries.size());
  int pos = 0;
  for (const auto& entry : entries) {
    if (pos % 6 == 0) {
      FWRITE("\n   ", output);
    }
    fprintf(output, " {0x%05X, %3d},", entry.first, entry.second);
    ++pos;
  }
  FWRITE("\n  };\n\n", output);
  FWRITE("  if (auto ccc_candidate = all_ccc.find(code_point); ccc_candidate != all_ccc.end()) {\n", output);
  FWRITE("    return ccc_candidate->second;\n", output);
  FWRITE("  }\n", output);
  FWRITE("  return 0;\n", output);
  FWRITE("}\n\n", output);
}

void print_canonical_composition(FILE* output, const std::map<std::uint32_t, starlark::ucd::unicode_data_record>& unicode_data,
               const std::set<std::uint32_t>& comp_exclusions) {
  FWRITE("std::optional<std::uint32_t> canonical_composition(std::uint32_t lhs, std::uint32_t rhs) {\n", output);
  std::map<std::pair<std::uint32_t, std::uint32_t>, std::uint32_t> entries;
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
  fprintf(output, "  static constexpr cnt::flat_map<std::pair<std::uint32_t, std::uint32_t>, std::uint32_t, %zu> all_cc = {", entries.size());
  int pos = 0;
  for (const auto& entry : entries) {
    if (pos % 6 == 0) {
      FWRITE("\n   ", output);
    }
    fprintf(output, " {{0x%05X, 0x%05X}, 0x%05X},", entry.first.first, entry.first.second, entry.second);
    ++pos;
  }
  FWRITE("\n  };\n\n", output);
  FWRITE("  if (auto cc_candidate = all_cc.find(std::make_pair(lhs, rhs)); cc_candidate != all_cc.end()) {\n", output);
  FWRITE("    return cc_candidate->second;\n", output);
  FWRITE("  }\n", output);
  FWRITE("  return {};\n", output);
  FWRITE("}\n\n", output);
}

void print_to_upper(FILE* output,
                    const std::map<std::uint32_t, starlark::ucd::unicode_data_record>& unicode_data,
                    const std::map<std::uint32_t, starlark::ucd::special_casing_record>& special_casing) {
  FWRITE("std::span<const std::uint32_t> to_upper(std::uint32_t code_point) {\n", output);
  FWRITE("  static constexpr std::array<std::uint32_t, 1> default_value{0x110000};\n", output);
  std::map<std::uint32_t, std::uint32_t> entries;
  std::vector<std::uint32_t> elements;
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
        for (const auto& condition : special_case.conditions) {
          // If this is a language ID, then ignore as we only support the root language.
          // The only languages that currently have special casings are Lithuanian, Turkish and Azeri.
          if (condition == "lt" || condition == "tr" || condition == "az") {
            break;
          }
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
  fprintf(output, "  static constexpr cnt::flat_map<std::uint32_t, std::uint32_t, %zu> all_upper_index = {", entries.size());
  int pos = 0;
  for (const auto& entry : entries) {
    if (pos % 6 == 0) {
      FWRITE("\n   ", output);
    }
    fprintf(output, " {0x%05X, 0x%05X},", entry.first, entry.second);
    ++pos;
  }
  FWRITE("\n  };\n\n", output);
  fprintf(output, "  static constexpr std::array<std::uint32_t, %zu> all_upper{", elements.size());
  pos = 0;
  for (const auto& element : elements) {
    if (pos % 12 == 0) {
      FWRITE("\n   ", output);
    }
    fprintf(output, " 0x%05X,", element);
    ++pos;
  }
  FWRITE("\n  };\n\n", output);
  FWRITE("  if (auto upper_candidate = all_upper_index.find(code_point); upper_candidate != all_upper_index.end()) {\n", output);
  FWRITE("    std::size_t begin = 0;\n", output);
  FWRITE("    auto end = upper_candidate->second;\n", output);
  FWRITE("    if (upper_candidate != all_upper_index.begin()) {\n", output);
  FWRITE("      begin = (--upper_candidate)->second;\n", output);
  FWRITE("    }\n", output);
  FWRITE("    return std::span<const std::uint32_t>(all_upper).subspan(begin, end - begin);\n", output);
  FWRITE("  }\n", output);
  FWRITE("  return std::span<const std::uint32_t>(default_value);\n", output);
  FWRITE("}\n\n", output);
}

void print_to_title(FILE* output,
                    const std::map<std::uint32_t, starlark::ucd::unicode_data_record>& unicode_data,
                    const std::map<std::uint32_t, starlark::ucd::special_casing_record>& special_casing) {
  FWRITE("std::span<const std::uint32_t> to_title(std::uint32_t code_point) {\n", output);
  FWRITE("  static constexpr std::array<std::uint32_t, 1> default_value{0x110000};\n", output);
  std::map<std::uint32_t, std::uint32_t> entries;
  std::vector<std::uint32_t> elements;
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
        for (const auto& condition : special_case.conditions) {
          // If this is a language ID, then ignore as we only support the root language.
          // The only languages that currently have special casings are Lithuanian, Turkish and Azeri.
          if (condition == "lt" || condition == "tr" || condition == "az") {
            break;
          }
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
  fprintf(output, "  static constexpr cnt::flat_map<std::uint32_t, std::uint32_t, %zu> all_title_index = {", entries.size());
  int pos = 0;
  for (const auto& entry : entries) {
    if (pos % 6 == 0) {
      FWRITE("\n   ", output);
    }
    fprintf(output, " {0x%05X, 0x%05X},", entry.first, entry.second);
    ++pos;
  }
  FWRITE("\n  };\n\n", output);
  fprintf(output, "  static constexpr std::array<std::uint32_t, %zu> all_title{", elements.size());
  pos = 0;
  for (const auto& element : elements) {
    if (pos % 12 == 0) {
      FWRITE("\n   ", output);
    }
    fprintf(output, " 0x%05X,", element);
    ++pos;
  }
  FWRITE("\n  };\n\n", output);
  FWRITE("  if (auto title_candidate = all_title_index.find(code_point); title_candidate != all_title_index.end()) {\n", output);
  FWRITE("    std::size_t begin = 0;\n", output);
  FWRITE("    auto end = title_candidate->second;\n", output);
  FWRITE("    if (title_candidate != all_title_index.begin()) {\n", output);
  FWRITE("      begin = (--title_candidate)->second;\n", output);
  FWRITE("    }\n", output);
  FWRITE("    return std::span<const std::uint32_t>(all_title).subspan(begin, end - begin);\n", output);
  FWRITE("  }\n", output);
  FWRITE("  return std::span<const std::uint32_t>(default_value);\n", output);
  FWRITE("}\n\n", output);
}

void print_to_lower(FILE* output,
                    const std::map<std::uint32_t, starlark::ucd::unicode_data_record>& unicode_data,
                    const std::map<std::uint32_t, starlark::ucd::special_casing_record>& special_casing) {
  FWRITE("std::pair<std::span<const std::uint32_t>, std::optional<std::span<const std::uint32_t>>> to_lower(std::uint32_t code_point) {\n", output);
  FWRITE("  static constexpr std::array<std::uint32_t, 1> default_value{0x110000};\n", output);
  std::map<std::uint32_t, std::uint32_t> entries;
  std::vector<std::uint32_t> elements;
  std::map<std::uint32_t, std::uint32_t> conditional_entries;
  std::vector<std::uint32_t> conditional_elements;
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
          // If this is a language ID, then ignore as we only support the root language.
          // The only languages that currently have special casings are Lithuanian, Turkish and Azeri.
          if (condition == "lt" || condition == "tr" || condition == "az") {
            skip = true;
            break;
          }
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
  fprintf(output, "  static constexpr cnt::flat_map<std::uint32_t, std::uint32_t, %zu> all_lower_index = {", entries.size());
  int pos = 0;
  for (const auto& entry : entries) {
    if (pos % 6 == 0) {
      FWRITE("\n   ", output);
    }
    fprintf(output, " {0x%05X, 0x%05X},", entry.first, entry.second);
    ++pos;
  }
  FWRITE("\n  };\n", output);
  fprintf(output, "  static constexpr std::array<std::uint32_t, %zu> all_lower{", elements.size());
  pos = 0;
  for (const auto& element : elements) {
    if (pos % 12 == 0) {
      FWRITE("\n   ", output);
    }
    fprintf(output, " 0x%05X,", element);
    ++pos;
  }
  FWRITE("\n  };\n", output);

  fprintf(output, "  static constexpr cnt::flat_map<std::uint32_t, std::uint32_t, %zu> conditional_all_lower_index = {", conditional_entries.size());
  pos = 0;
  for (const auto& entry : conditional_entries) {
    if (pos % 6 == 0) {
      FWRITE("\n   ", output);
    }
    fprintf(output, " {0x%05X, 0x%05X},", entry.first, entry.second);
    ++pos;
  }
  FWRITE("\n  };\n", output);
  fprintf(output, "  static constexpr std::array<std::uint32_t, %zu> conditional_all_lower{", conditional_elements.size());
  pos = 0;
  for (const auto& element : conditional_elements) {
    if (pos % 12 == 0) {
      FWRITE("\n   ", output);
    }
    fprintf(output, " 0x%05X,", element);
    ++pos;
  }
  FWRITE("\n  };\n\n", output);

  FWRITE("  if (auto lower_candidate = all_lower_index.find(code_point); lower_candidate != all_lower_index.end()) {\n", output);
  FWRITE("    std::size_t begin = 0;\n", output);
  FWRITE("    auto end = lower_candidate->second;\n", output);
  FWRITE("    if (lower_candidate != all_lower_index.begin()) {\n", output);
  FWRITE("      begin = (--lower_candidate)->second;\n", output);
  FWRITE("    }\n", output);
  FWRITE("    auto entries = std::span<const std::uint32_t>(all_lower).subspan(begin, end - begin);\n", output);
  FWRITE("    if (auto conditional_lower_candidate = conditional_all_lower_index.find(code_point); conditional_lower_candidate != conditional_all_lower_index.end()) {\n", output);

  FWRITE("      std::size_t conditional_begin = 0;\n", output);
  FWRITE("      auto conditional_end = conditional_lower_candidate->second;\n", output);
  FWRITE("      if (conditional_lower_candidate != conditional_all_lower_index.begin()) {\n", output);
  FWRITE("        conditional_begin = (--conditional_lower_candidate)->second;\n", output);
  FWRITE("      }\n", output);
  FWRITE("      auto conditional_entries = std::span<const std::uint32_t>(conditional_all_lower).subspan(conditional_begin, conditional_end - conditional_begin);\n", output);
  FWRITE("      return std::pair<std::span<const uint32_t>, std::optional<std::span<const std::uint32_t>>>(entries, conditional_entries);\n", output);

  FWRITE("    }\n", output);
  FWRITE("    return std::pair<std::span<const uint32_t>, std::optional<std::span<const std::uint32_t>>>(entries, {});\n", output);
  FWRITE("  }\n", output);
  FWRITE("  return std::pair<std::span<const uint32_t>, std::optional<std::span<const std::uint32_t>>>(default_value, {});\n", output);
  FWRITE("}\n\n", output);
}


void print_word_break(FILE* output,
                      const std::map<std::pair<std::uint32_t, std::uint32_t>, std::string>& word_break) {
  FWRITE("word_break_type word_break(std::uint32_t code_point) {\n", output);
  fprintf(output, "  static constexpr cnt::flat_map<std::pair<std::uint32_t, std::uint32_t>, word_break_type, %zu> all_word_break = {", word_break.size());
  int pos = 0;
  for (const auto& entry : word_break) {
    if (pos % 6 == 0) {
      FWRITE("\n   ", output);
    }
    fprintf(output, " {{0x%05X, 0x%05X}, word_break_type::k%s},", entry.first.first, entry.first.second, entry.second.c_str());
    ++pos;
  }
  FWRITE("\n  };\n\n", output);
  FWRITE("  auto up_bound = all_word_break.upper_bound(std::pair<std::uint32_t, std::uint32_t>(code_point, 0x110000));\n", output);
  FWRITE("  if (up_bound == all_word_break.begin()) {\n", output);
  FWRITE("    return word_break_type::kOther;\n", output);
  FWRITE("  }\n", output);
  FWRITE("  --up_bound;\n", output);
  FWRITE("  if (up_bound->first.first <= code_point && code_point <= up_bound->first.second) {\n", output);
  FWRITE("    return up_bound->second;\n", output);
  FWRITE("  }\n", output);
  FWRITE("  return word_break_type::kOther;\n", output);
  FWRITE("}\n\n", output);
}

std::set<std::pair<std::uint32_t, std::uint32_t>> create_ranges(const std::set<std::uint32_t>& input) {
  std::set<std::pair<std::uint32_t, std::uint32_t>> result;
  std::uint32_t min = 0;
  std::uint32_t previous = 0;
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
std::set<std::pair<std::uint32_t, std::uint32_t>> create_ranges(const std::map<std::uint32_t, T>& input) {
  std::set<std::uint32_t> keys;
  for (const auto& entry : input) {
    keys.insert(entry.first);
  }
  return create_ranges(keys);
}

std::set<std::pair<std::uint32_t, std::uint32_t>> compatibility_set(
    const std::map<std::uint32_t, starlark::ucd::unicode_data_record>& unicode_data) {
  std::set<std::uint32_t> keys;
  for (const auto& entry : unicode_data) {
    if (!entry.second.canonical_character_decomposition_mapping) {
      keys.insert(entry.first);
    }
  }
  return create_ranges(keys);
}

std::set<std::pair<std::uint32_t, std::uint32_t>> printable_set(
    const std::map<std::uint32_t, starlark::ucd::unicode_data_record>& unicode_data) {
  std::set<std::uint32_t> keys;
  for (const auto& entry : unicode_data) {
    if (entry.second.general_category[0] != 'C' && entry.second.general_category[0] != 'Z') {
      keys.insert(entry.first);
    }
  }
  keys.insert(0x20);  // The space character is considered a printable character.
  return create_ranges(keys);
}

std::set<std::pair<std::uint32_t, std::uint32_t>> alpha_set(
    const std::map<std::uint32_t, starlark::ucd::unicode_data_record>& unicode_data) {
  std::set<std::uint32_t> keys;
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

std::set<std::pair<std::uint32_t, std::uint32_t>> digits_set(
    const std::map<std::uint32_t, starlark::ucd::unicode_data_record>& unicode_data) {
  std::set<std::uint32_t> keys;
  for (const auto& entry : unicode_data) {
    if (entry.second.is_digit) {
      keys.insert(entry.first);
    }
  }
  return create_ranges(keys);
}

std::set<std::pair<std::uint32_t, std::uint32_t>> numeric_set(
    const std::map<std::string,
             std::set<std::pair<std::uint32_t,
                                  std::uint32_t>>>& extracted_numeric_properties) {
  std::set<std::uint32_t> keys;
  for (const auto& entry : extracted_numeric_properties) {
    for (auto r : entry.second) {
      for (int i = r.first; i <= r.second; ++i) {
        keys.insert(i);
      }
    }
  }
  return create_ranges(keys);
}

std::set<std::pair<std::uint32_t, std::uint32_t>> space_set(
    const std::map<std::uint32_t, starlark::ucd::unicode_data_record>& unicode_data) {
  std::set<std::uint32_t> keys;
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

void write_header(const char* output_file,
                  const char* include_h) {
  std::string header_guard{include_h};
  for (auto& c : header_guard) {
    if (!std::isalnum(c)) {
      c = '_';
    }
    c = std::toupper(c);
  }
  header_guard += '_';

  FILE* h_output = fopen(output_file, "w");
  fprintf(h_output, HPP_HEADER, header_guard.c_str(), header_guard.c_str());

  for (const auto& normalization_property : normalization_properties) {
    fprintf(h_output, "bool is_%s_nm(std::uint32_t);\n\n",
            normalization_property.c_str());
  }
  for (const auto& binary_property : binary_unicode_properties) {
    fprintf(h_output, "bool is_%s(std::uint32_t);\n\n",
            binary_property.c_str());
  }

  fprintf(h_output, HPP_FOOTER, header_guard.c_str());
  fclose(h_output);
}

void write_impl(const char* derived_core_properties_file,
                const char* unicode_data_file,
                const char* composition_exclusions,
                const char* derived_normalization_props,
                const char* prop_list,
                const char* derived_numeric_type,
                const char* special_casing_file,
                const char* word_break_file,
                const char* emoji_data,
                const char* output_file,
                const char* include_h) {
  FILE* cc_output = fopen(output_file, "w");
  fprintf(cc_output, CPP_HEADER, include_h);

  {
    std::map<std::uint32_t, starlark::ucd::unicode_data_record> unicode_data;
    std::map<std::uint32_t, starlark::ucd::special_casing_record> special_casing;
    std::map<std::pair<std::uint32_t, std::uint32_t>, std::string> word_break;
    std::set<std::uint32_t> comp_exclusions;
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
    print_ccc(cc_output, unicode_data);
    print_canonical_composition(cc_output, unicode_data, comp_exclusions);
    print_to_upper(cc_output, unicode_data, special_casing);
    print_to_title(cc_output, unicode_data, special_casing);
    print_to_lower(cc_output, unicode_data, special_casing);
    print_word_break(cc_output, word_break);
  }
  {
    std::map<std::string,
             std::set<std::pair<std::uint32_t,
                                  std::uint32_t>>> binary_properties;
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
             std::set<std::pair<std::uint32_t,
                                  std::uint32_t>>> extracted_numeric_properties;
    starlark::ucd::read_all_code_points(derived_numeric_type,
                              extracted_numeric_properties,
                              numeric_properties);
    print_code_points(cc_output, numeric_set(extracted_numeric_properties), "is_numeric");
  }

  FWRITE(CPP_FOOTER, cc_output);
  fclose(cc_output);
}

}  // namespace

int main(int argc, char *argv[]) {
  if (argc == 13) {
    const char* derived_core_properties_file = argv[1];
    const char* unicode_data_file = argv[2];
    const char* composition_exclusions = argv[3];
    const char* derived_normalization_props = argv[4];
    const char* prop_list = argv[5];
    const char* derived_numeric_type = argv[6];
    const char* special_casing = argv[7];
    const char* word_break = argv[8];
    const char* emoji_data = argv[9];
    const char* output_cpp_file = argv[10];
    const char* output_hpp_file = argv[11];
    const char* include_h = argv[12];

    write_header(output_hpp_file, include_h);
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
