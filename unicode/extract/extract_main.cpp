// Copyright 2024 Lucas Mirelmann

#include <cstring>

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "unicode/extract/extract.hpp"

namespace {

const char* HPP_HEADER = R"CPP(// Copyright 2024 Lucas Mirelmann

// Generated file, do not edit.

#ifndef %s
#define %s

#include <cstdint>

#include <optional>
#include <vector>

namespace ucd {

bool is_assigned(std::uint32_t code_point);

bool is_compatibility_decomposition(std::uint32_t code_point);

const std::vector<std::uint32_t>& decomposition(std::uint32_t code_point);

int ccc(std::uint32_t code_point);

std::optional<std::uint32_t> canonical_composition(std::uint32_t lhs, std::uint32_t rhs);

)CPP";

const char* HPP_FOOTER = R"CPP(}  // namespace ucd

#endif  // %s

)CPP";

const char* CPP_HEADER = R"CPP(// Copyright 2024 Lucas Mirelmann

// Generated file, do not edit.

#include <bitset>
#include <map>

#include "%s"

namespace ucd {

)CPP";

const char* CPP_FOOTER = R"CPP(}  // namespace ucd

)CPP";

constexpr int CODEPOINTS_PER_LINE = 64;

std::set<std::string> binary_unicode_properties = {
  "XID_Continue", "XID_Start"
};

std::set<std::string> normalization_properties = {
  "NFC_QC", "NFD_QC", "NFKC_QC", "NFKD_QC",
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

void print_decomposition(FILE* output, const std::map<std::uint32_t,
               std::tuple<std::uint32_t, bool, std::vector<std::uint32_t>>>& unicode_data) {
  FWRITE("const std::vector<std::uint32_t>& decomposition(std::uint32_t code_point) {\n", output);
  FWRITE("  static const std::vector<std::uint32_t> default_value;\n", output);
  FWRITE("  static const std::map<std::uint32_t, std::vector<std::uint32_t>> all_dc = {", output);
  int pos = 0;
  for (const auto& entry : unicode_data) {
    const auto& dc = std::get<2>(entry.second);
    if (dc.size() != 0) {
      if (pos % 6 == 0) {
        FWRITE("\n   ", output);
      }
      fprintf(output, " {0x%05X, {", entry.first);
      for (auto c : dc) {
        fprintf(output, " 0x%05X,", c);
      }
      FWRITE("}},", output);
      ++pos;
    }
  }
  FWRITE("\n  };\n\n", output);
  FWRITE("  if (auto dc_candidate = all_dc.find(code_point); dc_candidate != all_dc.end()) {\n", output);
  FWRITE("    return dc_candidate->second;\n", output);
  FWRITE("  }\n", output);
  FWRITE("  return default_value;\n", output);
  FWRITE("}\n\n", output);
}

void print_ccc(FILE* output, const std::map<std::uint32_t,
               std::tuple<std::uint32_t, bool, std::vector<std::uint32_t>>>& unicode_data) {
  FWRITE("int ccc(std::uint32_t code_point) {\n", output);
  FWRITE("  static const std::map<std::uint32_t, int> all_ccc = {", output);
  int pos = 0;
  for (const auto& entry : unicode_data) {
    if (std::get<0>(entry.second) != 0) {
      if (pos % 6 == 0) {
        FWRITE("\n   ", output);
      }
      fprintf(output, " {0x%05X, %3d},", entry.first, std::get<0>(entry.second));
      ++pos;
    }
  }
  FWRITE("\n  };\n\n", output);
  FWRITE("  if (auto ccc_candidate = all_ccc.find(code_point); ccc_candidate != all_ccc.end()) {\n", output);
  FWRITE("    return ccc_candidate->second;\n", output);
  FWRITE("  }\n", output);
  FWRITE("  return 0;\n", output);
  FWRITE("}\n\n", output);
}

void print_canonical_composition(FILE* output, const std::map<std::uint32_t,
               std::tuple<std::uint32_t, bool, std::vector<std::uint32_t>>>& unicode_data,
               const std::set<std::uint32_t>& comp_exclusions) {
  FWRITE("std::optional<std::uint32_t> canonical_composition(std::uint32_t lhs, std::uint32_t rhs) {\n", output);
  FWRITE("  static const std::map<std::pair<std::uint32_t, std::uint32_t>, std::uint32_t> all_cc = {", output);
  int pos = 0;
  for (const auto& entry : unicode_data) {
    if (std::get<0>(entry.second) == 0 && std::get<1>(entry.second) && !comp_exclusions.contains(entry.first)) {
      auto& cc = std::get<2>(entry.second);
      if (cc.size() == 0 || cc.size() == 1) {
        continue;
      }
      if (cc.size() != 2) {
        exit(1);
      }
      // If the decomposition begins with a non-starter, then this is not a candidate for composition.
      if (std::get<0>(unicode_data.at(cc[0])) != 0) {
        continue;
      }
      if (pos % 6 == 0) {
        FWRITE("\n   ", output);
      }
      fprintf(output, " {{0x%05X, 0x%05X}, 0x%05X},", cc[0], cc[1], entry.first);
      ++pos;
    }
  }
  FWRITE("\n  };\n\n", output);
  FWRITE("  if (auto cc_candidate = all_cc.find(std::make_pair(lhs, rhs)); cc_candidate != all_cc.end()) {\n", output);
  FWRITE("    return cc_candidate->second;\n", output);
  FWRITE("  }\n", output);
  FWRITE("  return {};\n", output);
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
    const std::map<std::uint32_t, std::tuple<std::uint32_t, bool, std::vector<std::uint32_t>>>& unicode_data) {
  std::set<std::uint32_t> keys;
  for (const auto& entry : unicode_data) {
    if (!std::get<1>(entry.second)) {
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
                const char* output_file,
                const char* include_h) {
  FILE* cc_output = fopen(output_file, "w");
  fprintf(cc_output, CPP_HEADER, include_h);

  {
    std::map<std::uint32_t, std::tuple<std::uint32_t, bool, std::vector<std::uint32_t>>> unicode_data;
    ucd::read_unicode_data(unicode_data_file, unicode_data);
    std::set<std::uint32_t> comp_exclusions;
    ucd::read_raw_code_points(composition_exclusions, comp_exclusions);

    print_code_points(cc_output, create_ranges(unicode_data), "is_assigned");
    print_code_points(cc_output, compatibility_set(unicode_data), "is_compatibility_decomposition");
    print_decomposition(cc_output, unicode_data);
    print_ccc(cc_output, unicode_data);
    print_canonical_composition(cc_output, unicode_data, comp_exclusions);
  }
  {
    std::map<std::string,
             std::set<std::pair<std::uint32_t,
                                  std::uint32_t>>> binary_properties;
    ucd::read_all_code_points(derived_normalization_props,
                              binary_properties,
                              normalization_properties);

    ucd::read_all_code_points(derived_core_properties_file,
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

  FWRITE(CPP_FOOTER, cc_output);
  fclose(cc_output);
}

}  // namespace

int main(int argc, char *argv[]) {
  if (argc == 8) {
    const char* derived_core_properties_file = argv[1];
    const char* unicode_data_file = argv[2];
    const char* composition_exclusions = argv[3];
    const char* derived_normalization_props = argv[4];
    const char* output_cpp_file = argv[5];
    const char* output_hpp_file = argv[6];
    const char* include_h = argv[7];

    write_header(output_hpp_file, include_h);
    write_impl(derived_core_properties_file,
               unicode_data_file,
               composition_exclusions,
               derived_normalization_props,
               output_cpp_file,
               include_h);
  }
}
