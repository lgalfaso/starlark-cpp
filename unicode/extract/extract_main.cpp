// Copyright 2024 Lucas Mirelmann

#include <iostream>
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

  std::vector<std::tuple<std::uint64_t, std::uint64_t, bool>> blocks;
  // TODO(lmirelmann): Given that this method takes non-overlapping,
  // ranges, then it should be possible to implement this using only the
  // ranges without transfoming it to a flat structure.
  std::set<std::uint64_t> all_cps;
  {
    constexpr int MAX_GAP_SIZE = (1 << 16) - 1;
    std::set<std::pair<std::uint64_t, std::uint64_t>> new_set;
    for (const auto& cps : set) {
      if (cps.second - cps.first >= MAX_GAP_SIZE) {
        blocks.emplace_back(cps.first, cps.second, false);
      } else {
        new_set.insert(cps);
      }
    }

    if (!new_set.empty()) {
      std::uint64_t min_cp = new_set.begin()->first;
      for (const auto& cps : new_set) {
        min_cp = std::min(min_cp, cps.first);
        for (std::uint64_t cp = cps.first; cp <= cps.second; ++cp) {
          all_cps.insert(cp);
        }
      }
      std::uint64_t max_cp = *all_cps.rbegin();

      std::uint64_t gap = 0;
      bool found_zero = false;
      std::uint64_t start = min_cp;
      std::uint64_t end = min_cp;
      bool in_gap = true;
      for (std::uint64_t i = min_cp; i <= max_cp; ++i) {
        if (in_gap) {
          if (all_cps.contains(i)) {
            start = i;
            end = i;
            found_zero = false;
            gap = 0;
            in_gap = false;
          }
        } else {
          if (all_cps.contains(i)) {
            gap = 0;
            found_zero |= (end + 1 != i);
            end = i;
          } else {
            if (gap == MAX_GAP_SIZE) {
              blocks.emplace_back(start, end, found_zero);
              in_gap = true;
            } else {
              ++gap;
            }
          }
        }
      }
      blocks.emplace_back(start, end, found_zero);
    }
  }

  constexpr int CODEPOINTS_PER_LINE = 64;

  for (std::size_t pos = 0; pos < blocks.size(); ++pos) {
    const auto& [min_cp, max_cp, found_zero] = blocks[pos];
    if (!found_zero) {
      continue;
    }
    fprintf(output, "constexpr const char* %s_bitset_%lu =\n", fn.c_str(), pos);
    for (std::uint64_t i = min_cp; i <= max_cp; ++i) {
      if ((i - min_cp) % CODEPOINTS_PER_LINE == 0) {
        if ((i - min_cp) != 0) {
          fprintf(output, "\"\n");
        }
        fprintf(output, "    \"");
      }
      fprintf(output, all_cps.contains(max_cp + min_cp - i) ? "1" : "0");
    }
    fprintf(output, "\";\n\n");
  }

  fprintf(output, "bool %s(std::uint64_t codepoint) {\n", fn.c_str());
  bool add_blank_line = false;
  for (std::size_t pos = 0; pos < blocks.size(); ++pos) {
    const auto& [min_cp, max_cp, found_zero] = blocks[pos];
    if (!found_zero) {
      continue;
    }
    add_blank_line = true;
    fprintf(output,
            "  static constexpr std::bitset<0x%llX>"
            " all_cp_%lu(%s_bitset_%lu);\n",
            max_cp + 1 - min_cp, pos, fn.c_str(), pos);
  }
  if (add_blank_line) {
    fprintf(output, "\n");
  }
  for (std::size_t pos = 0; pos < blocks.size(); ++pos) {
    const auto& [min_cp, max_cp, found_zero] = blocks[pos];
    if (pos == 0) {
      fprintf(output, "  return ");
    } else {
      fprintf(output, " ||\n         ");
    }
    if (min_cp == max_cp) {
      fprintf(output, "(codepoint == 0x%llX)", min_cp);
    } else {
      fprintf(output, "(0x%llX <= codepoint && codepoint <= 0x%llX", min_cp, max_cp);
      if (found_zero) {
        fprintf(output, " && all_cp_%lu[codepoint - 0x%llX]", pos, min_cp);
      }
      fprintf(output, ")");
    }
  }
  fprintf(output, ";\n}\n\n");
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
