// Copyright 2024 Lucas Mirelmann

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

namespace ucd {

)CPP";

const char* HPP_FOOTER = R"CPP(}  // namespace ucd

#endif  // %s

)CPP";

const char* CPP_HEADER = R"CPP(// Copyright 2024 Lucas Mirelmann

// Generated file, do not edit.

#include <bitset>
#include <set>

#include "%s"

namespace ucd {

)CPP";

const char* CPP_FOOTER = R"CPP(}  // namespace ucd

)CPP";

constexpr int CODEPOINTS_PER_LINE = 64;

void print_in_multiple_lines(const char* characters, int& count, uint64_t to_print, FILE* output) {
  while (to_print != 0) {
    if (count % CODEPOINTS_PER_LINE == 0) {
      if (count != 0) {
        fwrite("\"\n", 1, 2, output);
      }
      fwrite("    \"", 1, 5, output);
    }
    uint64_t will_print = std::min<uint64_t>(CODEPOINTS_PER_LINE - (count % CODEPOINTS_PER_LINE), to_print);
    fwrite(characters, 1, will_print, output);
    count += will_print;
    to_print -= will_print;
  }
}

void print_codepoints(
      FILE* output,
      const std::set<std::pair<std::uint64_t, std::uint64_t>>& set,
                      const std::string& fn) {
  if (set.size() == 0) {
    fprintf(output,
            "bool %s(std::uint64_t codepoint) {\n"
            "  return false;\n"
            "}\n\n",
            fn.c_str());
    return;
  }

  std::vector<std::vector<std::pair<std::uint64_t, std::uint64_t>>> blocks;
  {
    // Split into chunks. Each chunk will be a single bitset.
    constexpr int MAX_GAP_SIZE = (1 << 16) - 1;

    bool create_new_block = true;
    std::uint64_t previous_max = 0;
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
      std::uint64_t previous_min = mini_block.back().second + 1;
      for (auto it = mini_block.rbegin(); it < mini_block.rend(); ++it) {
        auto [min_cp, max_cp] = *it;
        print_in_multiple_lines(zeros_string.c_str(), count, previous_min - max_cp - 1, output);
        print_in_multiple_lines(ones_string.c_str(), count, max_cp - min_cp + 1, output);
        previous_min = min_cp;
      }
      fwrite("\";\n\n", 1, 4, output);
    }
  }
  {
    // Print the functions that check for the codepoints.
    fprintf(output, "bool %s(std::uint64_t codepoint) {\n", fn.c_str());
    bool add_blank_line = false;
    for (std::size_t pos = 0; pos < blocks.size(); ++pos) {
      const auto& mini_block = blocks[pos];
      if (mini_block.size() == 1) {
        continue;
      }
      add_blank_line = true;
      fprintf(output,
              "  static constexpr std::bitset<0x%llX> all_cp_%lu(%s_bitset_%lu);\n",
              mini_block.back().second + 1 - mini_block.front().first,
              pos, fn.c_str(), pos);
    }
    if (add_blank_line) {
      fwrite("\n", 1, 1, output);
    }
    for (std::size_t pos = 0; pos < blocks.size(); ++pos) {
      const auto& mini_block = blocks[pos];
      if (pos == 0) {
        fwrite("  return ", 1, 9, output);
      } else {
        fwrite(" ||\n         ", 1, 13, output);
      }
      auto min_cp = mini_block.front().first;
      auto max_cp = mini_block.back().second;
      if (min_cp == max_cp) {
        fprintf(output, "(codepoint == 0x%llX)", min_cp);
      } else if (mini_block.size() != 1) {
        fprintf(output, "(0x%llX <= codepoint && codepoint <= 0x%llX && all_cp_%lu[codepoint - 0x%llX])", min_cp, max_cp, pos, min_cp);
      } else {
        fprintf(output, "(0x%llX <= codepoint && codepoint <= 0x%llX)", min_cp, max_cp);
      }
    }
    fwrite(";\n}\n\n", 1, 5, output);
  }
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

  for (const auto& binary_property : ucd::binary_unicode_properties) {
    fprintf(h_output, "bool is_%s(std::uint64_t);\n\n",
            binary_property.c_str());
  }

  fprintf(h_output, HPP_FOOTER, header_guard.c_str());
  fclose(h_output);
}

void write_impl(const char* derived_core_properties_file,
                const char* output_file,
                const char* include_h) {
  FILE* cc_output = fopen(output_file, "w");
  fprintf(cc_output, CPP_HEADER, include_h);

  std::map<std::string,
           std::set<std::pair<std::uint64_t,
                                std::uint64_t>>> binary_properties;
  ucd::read_all_codepoints(derived_core_properties_file, binary_properties);

  for (const auto& binary_property : ucd::binary_unicode_properties) {
    if (binary_properties[binary_property].empty()) {
      exit(1);
    }
    std::string name = "is_" + binary_property;
    print_codepoints(cc_output, binary_properties[binary_property],
                     name);
  }

  fwrite(CPP_FOOTER, sizeof CPP_FOOTER[0], strlen(CPP_FOOTER), cc_output);
  fclose(cc_output);
}

}  // namespace

int main(int argc, char *argv[]) {
  if (argc == 5) {
    const char* derived_core_properties_file = argv[1];
    const char* output_cpp_file = argv[2];
    const char* output_hpp_file = argv[3];
    const char* include_h = argv[4];

    write_header(output_hpp_file, include_h);
    write_impl(derived_core_properties_file,
               output_cpp_file,
               include_h);
  }
}
