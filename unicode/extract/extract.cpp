// Copyright 2024 Lucas Mirelmann

#include <cstdlib>
#include <cstring>

#include "unicode/extract/extract.hpp"

namespace ucd {

void read_raw_code_points(const char* file,
                          std::set<std::uint32_t>& set) {
  FILE* fp = fopen(file, "r");
  char* line = nullptr;
  size_t len = 0;

  if (fp == nullptr) {
    std::exit(1);
  }

  while ((getline(&line, &len, fp)) != -1) {
    int code_point;
    int count = std::sscanf(line, "%x", &code_point);
    if (count > 0) {
      set.insert(code_point);
    }
  }

  fclose(fp);
  if (line) {
    free(line);
  }
}

void read_all_code_points(const char* file,
    std::map<std::string,
             std::set<std::pair<std::uint32_t, std::uint32_t>>>& set,
    const std::set<std::string>& properties) {
  FILE* fp = fopen(file, "r");
  char* line = nullptr;
  size_t len = 0;

  if (fp == nullptr) {
    std::exit(1);
  }

  int start, end, count1, count2;
  char alias[100];
  while ((getline(&line, &len, fp)) != -1) {
    char* sline = line;
    int count = std::sscanf(sline, "%x%n..%x%n",
                            &start, &count1, &end, &count2);
    if (count > 0) {
      if (count == 1) {
        sline += count1;
      } else if (count == 2) {
        sline += count2;
      }
      while (sline[0] == ' ') {
        ++sline;
      }
      if (sline[0] != ';') {
        std::exit(1);
      }
      ++sline;
      while (std::sscanf(sline, " %99[0-9a-zA-Z_]%n", alias, &count1) > 0) {
        if (strlen(alias) == 99) {
          std::exit(1);
        }
        sline += count1;
        if (!properties.contains(alias)) {
          continue;
        }
        if (count == 1) {
          set[alias].insert(std::make_pair(start, start));
        } else if (count == 2) {
          set[alias].insert(std::make_pair(start, end));
        }
      }
    }
  }

  fclose(fp);
  if (line) {
    free(line);
  }
}

void read_unicode_data(const char* file,
    std::map<std::uint32_t,
             std::tuple<std::uint32_t, bool, std::vector<std::uint32_t>>>& unicode_data) {
  FILE* fp = fopen(file, "r");
  char* line = nullptr;
  size_t len = 0;

  if (fp == nullptr) {
    std::exit(1);
  }

  int previous_code = 0;
  while ((getline(&line, &len, fp)) != -1) {
    if (len > 0) {
      char* sline = line;
      int code, length;
      int count = std::sscanf(sline, "%x%n", &code, &length);
      if (count != 1) {
        exit(1);
      }
      sline += length;
      for (int i = 0; i < 3; sline++) {
        if (sline[0] == ';') {
          ++i;
        }
      }
      std::uint32_t ccc;
      count = std::sscanf(sline, "%d%n", &ccc, &length);
      if (count != 1) {
        exit(1);
      }
      sline += length;
      for (int i = 0; i < 2; sline++) {
        if (sline[0] == ';') {
          ++i;
        }
      }
      bool canonical = true;
      if (sline[0] == '<') {
        canonical = false;
        while (sline[0] != ' ') {
          ++sline;
        }
      }
      std::vector<std::uint32_t> decomposition;
      while (sline[0] != ';') {
        int decomposition_code;
        count = std::sscanf(sline, "%x%n", &decomposition_code, &length);
        if (count != 1) {
          exit(1);
        }
        decomposition.push_back(decomposition_code);
        sline += length;
      }
      if (std::strstr(line, "Last>") != nullptr) {
        if (decomposition.size() > 0) {
          exit(1);
        }
        for (int i = previous_code + 1; i < code; ++i) {
          unicode_data.emplace(i, std::make_tuple(ccc, canonical, std::move(decomposition)));
        }
      }
      unicode_data.emplace(code, std::make_tuple(ccc, canonical, std::move(decomposition)));
      previous_code = code;
    }
  }

  fclose(fp);
  if (line) {
    free(line);
  }
}

}  // namespace ucd
