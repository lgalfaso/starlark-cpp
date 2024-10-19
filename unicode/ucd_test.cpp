// Copyright 2024 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include "unicode/ucd_codepoints.hpp"

namespace {

const char* XID_Start = "XID_Start";
const char* XID_Continue = "XID_Continue";

void read_codepoints(const char* file,
                     std::set<std::uint64_t>& set,
                     const char* category) {
  FILE* fp = fopen(file, "r");
  char* line = nullptr;
  size_t len = 0;

  if (fp == nullptr) {
    exit(1);
  }
  std::string cat = "; ";
  cat += category;
  cat += " ";
  std::set<std::pair<std::uint64_t, std::uint64_t>> ranges;
  while ((getline(&line, &len, fp)) != -1) {
    if (strstr(line, cat.c_str()) != nullptr) {
      int start, end;
      int count = std::sscanf(line, "%x..%x", &start, &end);
      if (count == 1) {
        ranges.insert(std::make_pair(start, start));
      } else if (count == 2) {
        ranges.insert(std::make_pair(start, end));
      }
    }
  }
  fclose(fp);
  if (line) {
    free(line);
  }

  for (const auto& cps : ranges) {
    for (std::uint64_t cp = cps.first; cp <= cps.second; ++cp) {
      set.insert(cp);
    }
  }
}

constexpr int max_unicode = 0x10FFFF;

TEST(UcdTest, IsXIdStart) {
  auto test_srcdir = std::getenv("TEST_SRCDIR");
  if (test_srcdir == nullptr) {
    GTEST_SKIP() << "TEST_SRCDIR not defined";
  }
  std::string path = std::string {test_srcdir} +
      "/_main~_repo_rules~ucd/DerivedCoreProperties.txt";

  std::set<std::uint64_t> all_cps;
  read_codepoints(path.c_str(), all_cps, XID_Start);

  for (int i = 0; i <= max_unicode; ++i) {
    EXPECT_EQ(all_cps.contains(i), ucd::is_XID_Start(i));
  }
}

TEST(UcdTest, XIsIdContinue) {
  auto test_srcdir = std::getenv("TEST_SRCDIR");
  if (test_srcdir == nullptr) {
    GTEST_SKIP() << "TEST_SRCDIR not defined";
  }
  std::string path = std::string {test_srcdir} +
      "/_main~_repo_rules~ucd/DerivedCoreProperties.txt";

  std::set<std::uint64_t> all_cps;
  read_codepoints(path.c_str(), all_cps, XID_Continue);

  for (int i = 0; i <= max_unicode; ++i) {
    EXPECT_EQ(all_cps.contains(i), ucd::is_XID_Continue(i));
  }
}

}  // namespace
