// Copyright 2024-2026 Lucas Mirelmann

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "unicode/extract/extract.hpp"

namespace starlark {
namespace ucd {

std::vector<std::vector<std::string>> read_file(const char* file) {
  FILE* fp = fopen(file, "r");
  if (fp == nullptr) {
    std::exit(1);
  }
  char* line = nullptr;
  size_t len = 0;
  std::vector<std::vector<std::string>> result;
  while ((getline(&line, &len, fp)) != -1) {
    std::vector<std::string> entry;
    std::string element;
    bool found = false;
    for (int i = 0; i < len; ++i) {
      if (line[i] == '#' || line[i] == '\n') {
        break;
      }
      found = true;
      if (line[i] == ';') {
        entry.emplace_back(element);
        element.clear();
      } else {
        element += line[i];
      }
    }
    if (found) {
      entry.emplace_back(element);
      result.emplace_back(entry);
    }
  }

  fclose(fp);
  if (line) {
    free(line);
  }
  return result;
}

namespace {

char32_t parse_code_point(const std::string& input) {
  int code_point;
  int count = std::sscanf(input.c_str(), "%x", &code_point);
  if (count != 1) {
    exit(1);
  }
  return code_point;
}

std::pair<char32_t, char32_t> parse_code_point_or_range(const std::string& input) {
  int start, end;
  int count = std::sscanf(input.c_str(), "%x..%x", &start, &end);
  if (count == 0) {
    exit(1);
  }
  if (count == 1) {
    end = start;
  }
  return std::make_pair(start, end);
}

void parse_code_point_sequence(std::vector<char32_t>& code_points, const std::string& input) {
  auto copy = input;
  while (!copy.empty()) {
    code_points.push_back(parse_code_point(copy));
    auto pos = copy.find(" ");
    if (pos != std::string::npos) {
      copy.erase(0, pos + 1);
    } else {
      copy.clear();
    }
  }
}

int parse_decimal_value(const std::string& input) {
  int code_point;
  int count = std::sscanf(input.c_str(), "%d", &code_point);
  if (count != 1) {
    exit(1);
  }
  return code_point;
}

}  // namespace

void read_raw_code_points(const char* file, std::set<char32_t>& set) {
  auto content = read_file(file);
  for (const auto& entry : content) {
    if (entry.size() != 1) {
      exit(1);
    }
    set.insert(parse_code_point(entry.front()));
  }
}

void read_all_code_points(const char* file,
    std::map<std::string, std::set<std::pair<char32_t, char32_t>>>& set,
    const std::set<std::string>& properties) {
  auto content = read_file(file);
  for (const auto& entry : content) {
    if (entry.size() < 2) {
      exit(1);
    }
    auto alias = entry[1];
    alias.erase(0, alias.find_first_not_of(" "));
    alias.erase(alias.find_last_not_of(" ") + 1);
    if (alias.find(" ") != std::string::npos) {
      exit(1);
    }
    if (!properties.contains(alias)) {
      continue;
    }
    set[alias].insert(parse_code_point_or_range(entry[0]));
  }
}

void read_unicode_data(const char* file, std::map<char32_t, unicode_data_record>& unicode_data) {
  auto content = read_file(file);
  char32_t previous_code_point = 0;
  for (auto& entry : content) {
    if (entry.size() < 15) {
      exit(1);
    }
    char32_t code_point = parse_code_point(entry[0]);
    int ccc = parse_decimal_value(entry[3]);

    auto character_decomposition = entry[5];
    bool canonical = character_decomposition.empty() || character_decomposition[0] != '<';
    if (!canonical) {
      // If there is a tag, then remove it.
      character_decomposition.erase(0, character_decomposition.find(" ") + 1);
    }
    std::vector<char32_t> decomposition;
    parse_code_point_sequence(decomposition, character_decomposition);

    bool is_digit = !entry[6].empty() || !entry[7].empty();
    char32_t uppercase_mapping = 0x110000;
    if (!entry[12].empty()) {
      uppercase_mapping = parse_code_point(entry[12]);
    }
    char32_t lowercase_mapping = 0x110000;
    if (!entry[13].empty()) {
      lowercase_mapping = parse_code_point(entry[13]);
    }
    char32_t titlecase_mapping = 0x110000;
    if (!entry[14].empty()) {
      titlecase_mapping = parse_code_point(entry[14]);
    }
    if (entry[1].find("Last>") != std::string::npos) {
      if (decomposition.size() > 0) {
        exit(1);
      }
      for (int i = previous_code_point + 1; i < code_point; ++i) {
        if (uppercase_mapping != 0x110000 ||
            lowercase_mapping != 0x110000 ||
            titlecase_mapping != 0x110000) {
          exit(1);
        }
        unicode_data.emplace(i, unicode_data_record{
            .canonical_combining_class = ccc,
            .canonical_character_decomposition_mapping = canonical,
            .character_decomposition_mapping = decomposition,
            .general_category = entry[2],
            .bidirectional_category = entry[4],
            .is_digit = is_digit,
            .uppercase_mapping = uppercase_mapping,
            .lowercase_mapping = lowercase_mapping,
            .titlecase_mapping = titlecase_mapping,
        });
      }
    }
    unicode_data.emplace(code_point, unicode_data_record{
        .canonical_combining_class = ccc,
        .canonical_character_decomposition_mapping = canonical,
        .character_decomposition_mapping = decomposition,
        .general_category = entry[2],
        .bidirectional_category = entry[4],
        .is_digit = is_digit,
        .uppercase_mapping = uppercase_mapping,
        .lowercase_mapping = lowercase_mapping,
        .titlecase_mapping = titlecase_mapping,
    });
    previous_code_point = code_point;
  }
}

void read_special_casing(const char* file, std::map<char32_t, special_casing_record>& special_casing) {
  auto content = read_file(file);
  for (auto& entry : content) {
    if (entry.size() < 5) {
      exit(1);
    }
    char32_t code_point = parse_code_point(entry[0]);
    std::vector<char32_t> lower;
    auto element = entry[1];
    element.erase(0, element.find_first_not_of(" "));
    parse_code_point_sequence(lower, element);
    std::vector<char32_t> title;
    element = entry[2];
    element.erase(0, element.find_first_not_of(" "));
    parse_code_point_sequence(title, element);
    std::vector<char32_t> upper;
    element = entry[3];
    element.erase(0, element.find_first_not_of(" "));
    parse_code_point_sequence(upper, element);
    std::vector<std::string> condition;
    auto condition_str = entry[4];
    condition_str.erase(0, condition_str.find_first_not_of(" "));
    while (!condition_str.empty()) {
      auto pos = condition_str.find(" ");
      condition.emplace_back(condition_str.substr(0, pos));
      condition_str.erase(0, pos);
      condition_str.erase(0, condition_str.find_first_not_of(" "));
    }
    bool skip = false;
    for (const auto& c : condition) {
      // If this is a language ID, then ignore as we only support the root language.
      // The only languages that currently have special casings are Lithuanian, Turkish and Azeri.
      if (c == "lt" || c == "tr" || c == "az") {
        skip = true;
        break;
      }
    }
    if (!skip) {
      special_casing.emplace(code_point, special_casing_record{
        .lower = lower,
        .title = title,
        .upper = upper,
        .conditions = condition,
      });
    }
  }
}

void read_word_break(const char* file, std::map<std::pair<char32_t, char32_t>, std::string>& word_break) {
  auto content = read_file(file);
  for (const auto& entry : content) {
    if (entry.size() < 2) {
      exit(1);
    }
    auto alias = entry[1];
    alias.erase(0, alias.find_first_not_of(" "));
    alias.erase(alias.find_last_not_of(" ") + 1);
    if (alias.find(" ") != std::string::npos) {
      exit(1);
    }
    word_break[parse_code_point_or_range(entry[0])] = alias;
  }
}

}  // namespace ucd
}  // namespace starlark
