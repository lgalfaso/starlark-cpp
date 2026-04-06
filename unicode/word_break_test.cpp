// Copyright 2026 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <cstdio>

#include <string>
#include <utility>
#include <vector>

#include "unicode/extract/extract.hpp"
#include "unicode/word_break.hpp"

using testing::IsEmpty;
using testing::SizeIs;

namespace {

std::vector<std::tuple<std::vector<std::uint32_t>, std::vector<std::uint64_t>, std::string>>
read_unicode_word_break_tests(const char* file) {
  auto content = starlark::ucd::read_file(file);
  std::vector<std::tuple<std::vector<std::uint32_t>, std::vector<std::uint64_t>, std::string>> result;
  for (const auto& entry : content) {
    std::uint64_t pos = 0;
    std::vector<std::uint32_t> code_points;
    std::vector<std::uint64_t> break_points;
    std::string pending = entry.front();
    pending.erase(0, pending.find_first_not_of(" \t"));
    while (!pending.empty()) {
      if (pending.starts_with("÷")) {
        break_points.push_back(pos);
      } else if (pending.starts_with("×")) {
      } else {
        int code_point;
        int count = std::sscanf(pending.c_str(), "%x", &code_point);
        if (count != 1) {
          exit(1);
        }
        code_points.push_back(code_point);
        pos++;
      }
      pending.erase(0, pending.find_first_of(" \t"));
      pending.erase(0, pending.find_first_not_of(" \t"));
    }
    result.emplace_back(std::move(code_points), std::move(break_points), entry.front());
  }
  return result;
}

TEST(WordBreak, EmptyString) {
  std::vector<std::uint64_t> actual;
  starlark::unicode::word_break({}, actual);
  EXPECT_THAT(actual, IsEmpty());
}

TEST(WordBreak, UnicodeTests) {
  const auto& argv = ::testing::internal::GetArgvs();
  ASSERT_THAT(argv, SizeIs(2));

  std::string path = argv[1];
  auto test_cases = read_unicode_word_break_tests(path.c_str());
  for (const auto& entry : test_cases) {
    std::vector<std::uint64_t> actual;
    starlark::unicode::word_break(std::get<0>(entry), actual);
    EXPECT_EQ(std::get<1>(entry), actual) << std::get<2>(entry);
  }
}

}  // namespace
