// Copyright 2024 Lucas Mirelmann

#include "unicode/extract/extract.hpp"

namespace ucd {

std::set<std::string> binary_unicode_properties = {
  "XID_Continue", "XID_Start"
};

void read_all_codepoints(const char* file,
    std::map<std::string,
             std::set<std::pair<std::uint64_t, std::uint64_t>>>& set) {
  FILE* fp = fopen(file, "r");
  char* line = nullptr;
  size_t len = 0;

  if (fp == nullptr) {
    exit(1);
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
        exit(1);
      }
      ++sline;
      while (std::sscanf(sline, " %99[0-9a-zA-Z_]%n", alias, &count1) > 0) {
        if (strlen(alias) == 99) {
          exit(1);
        }
        sline += count1;
        if (!binary_unicode_properties.contains(alias)) {
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

}  // namespace ucd
