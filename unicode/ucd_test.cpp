// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <utility>

#include "unicode/ucd_code_points.hpp"
#include "unicode/extract/extract.hpp"

using testing::SizeIs;

namespace {

const char* XID_Start = "XID_Start";
const char* XID_Continue = "XID_Continue";
const char* White_Space = "White_Space";

void read_code_points(const char* file,
                     std::set<char32_t>& set,
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
  std::set<std::pair<char32_t, char32_t>> ranges;
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
    for (char32_t cp = cps.first; cp <= cps.second; ++cp) {
      set.insert(cp);
    }
  }
}

constexpr int max_unicode = 0x10FFFF;

TEST(UcdTest, IsXIdStart) {
  const auto& argv = ::testing::internal::GetArgvs();
  ASSERT_THAT(argv, SizeIs(4));

  std::string path = argv[1];
  std::set<char32_t> all_cps;
  read_code_points(path.c_str(), all_cps, XID_Start);

  for (int i = 0; i <= max_unicode; ++i) {
    EXPECT_EQ(all_cps.contains(i), starlark::ucd::is_XID_Start(i));
  }
}

TEST(UcdTest, IsXIdContinue) {
  const auto& argv = ::testing::internal::GetArgvs();
  ASSERT_THAT(argv, SizeIs(4));

  std::string path = argv[1];
  std::set<char32_t> all_cps;
  read_code_points(path.c_str(), all_cps, XID_Continue);

  for (int i = 0; i <= max_unicode; ++i) {
    EXPECT_EQ(all_cps.contains(i), starlark::ucd::is_XID_Continue(i));
  }
}

TEST(UcdTest, IsSpace) {
  const auto& argv = ::testing::internal::GetArgvs();
  ASSERT_THAT(argv, SizeIs(4));

  std::string path = argv[2];
  std::set<char32_t> all_cps;
  read_code_points(path.c_str(), all_cps, White_Space);

  for (int i = 0; i <= max_unicode; ++i) {
    // There is a discrepancy between the expectation if `is_space` from Python and the definition of the
    // Unicode binary porperty `White_Space`. The former includes the characters that have
    // the Bidirectional cateogry "B" (Paragraph Separator). There are 4 characters that do not have the
    // binary property White_Space and have the Bidirectional cateogry "B", there are '\x1c', '\x1d', '\x1e', and '\x1f'.
    EXPECT_EQ(all_cps.contains(i) || (0x1c <= i && i <= 0x1f), starlark::ucd::is_space(i)) << "Codepoint: " << i << "\n";
  }
}

TEST(UcdTest, WordBreakType) {
  const auto& argv = ::testing::internal::GetArgvs();
  ASSERT_THAT(argv, SizeIs(4));

  std::string path = argv[3];
  std::map<std::pair<char32_t, char32_t>, std::string> word_break;
  starlark::ucd::read_word_break(path.c_str(), word_break);
  std::map<char32_t, starlark::ucd::word_break_type> expected;
  for (std::size_t i = 0; i <= max_unicode; ++i) {
    expected[i] = starlark::ucd::word_break_type::kOther;
  }
  for (const auto& entry : word_break) {
    starlark::ucd::word_break_type expected_type;
    if (entry.second == "CR") {
      expected_type = starlark::ucd::word_break_type::kCR;
    } else if (entry.second == "LF") {
      expected_type = starlark::ucd::word_break_type::kLF;
    } else if (entry.second == "Newline") {
      expected_type = starlark::ucd::word_break_type::kNewline;
    } else if (entry.second == "Extend") {
      expected_type = starlark::ucd::word_break_type::kExtend;
    } else if (entry.second == "ZWJ") {
      expected_type = starlark::ucd::word_break_type::kZWJ;
    } else if (entry.second == "Regional_Indicator") {
      expected_type = starlark::ucd::word_break_type::kRegional_Indicator;
    } else if (entry.second == "Format") {
      expected_type = starlark::ucd::word_break_type::kFormat;
    } else if (entry.second == "Katakana") {
      expected_type = starlark::ucd::word_break_type::kKatakana;
    } else if (entry.second == "Hebrew_Letter") {
      expected_type = starlark::ucd::word_break_type::kHebrew_Letter;
    } else if (entry.second == "ALetter") {
      expected_type = starlark::ucd::word_break_type::kALetter;
    } else if (entry.second == "Single_Quote") {
      expected_type = starlark::ucd::word_break_type::kSingle_Quote;
    } else if (entry.second == "Double_Quote") {
      expected_type = starlark::ucd::word_break_type::kDouble_Quote;
    } else if (entry.second == "MidNumLet") {
      expected_type = starlark::ucd::word_break_type::kMidNumLet;
    } else if (entry.second == "MidLetter") {
      expected_type = starlark::ucd::word_break_type::kMidLetter;
    } else if (entry.second == "MidNum") {
      expected_type = starlark::ucd::word_break_type::kMidNum;
    } else if (entry.second == "Numeric") {
      expected_type = starlark::ucd::word_break_type::kNumeric;
    } else if (entry.second == "ExtendNumLet") {
      expected_type = starlark::ucd::word_break_type::kExtendNumLet;
    } else if (entry.second == "E_Base") {
      expected_type = starlark::ucd::word_break_type::kE_Base;
    } else if (entry.second == "E_Modifier") {
      expected_type = starlark::ucd::word_break_type::kE_Modifier;
    } else if (entry.second == "Glue_After_Zwj") {
      expected_type = starlark::ucd::word_break_type::kGlue_After_Zwj;
    } else if (entry.second == "E_Base_GAZ") {
      expected_type = starlark::ucd::word_break_type::kE_Base_GAZ;
    } else if (entry.second == "WSegSpace") {
      expected_type = starlark::ucd::word_break_type::kWSegSpace;
    } else {
      FAIL() << "Unknown type " << entry.second;
    }
    for (int i = entry.first.first; i <= entry.first.second; ++i) {
      expected[i] = expected_type;
    }
  }

  for (std::size_t i = 0; i <= max_unicode; ++i) {
    EXPECT_EQ(expected[i], starlark::ucd::word_break(i));
  }
}

}  // namespace
