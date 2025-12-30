// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <cstdio>
#include <string>
#include <vector>

#include "unicode/encode.hpp"
#include "unicode/normalization.hpp"
#include "unicode/ucd_code_points.hpp"

using starlark::ucd::is_assigned;
using starlark::unicode::to_nfc;
using starlark::unicode::to_nfc_x;
using starlark::unicode::to_nfd;
using starlark::unicode::to_nfd_x;
using starlark::unicode::to_nfkc;
using starlark::unicode::to_nfkc_x;
using starlark::unicode::to_nfkd;
using starlark::unicode::to_nfkd_x;
using starlark::unicode::utf8_encode_code_point;
using testing::SizeIs;

namespace {

struct test_case {
  std::vector<std::uint32_t> source;
  std::vector<std::uint32_t> NFC;
  std::vector<std::uint32_t> NFD;
  std::vector<std::uint32_t> NFKC;
  std::vector<std::uint32_t> NFKD;
  std::string line;
  int part;
};

void read_column(char** line, std::vector<std::uint32_t>& column) {
  int code, length;
  do {
    int count = std::sscanf(*line, "%x%n", &code, &length);
    if (count != 1) {
      exit(1);
    }
    column.push_back(code);
    *line += length;
  } while ((*line)[0] == ' ');
  *line += 1;
}

std::vector<test_case> read_file(const char* file) {
  std::vector<test_case> result;

  FILE* fp = fopen(file, "r");
  char* line = nullptr;
  size_t len = 0;

  if (fp == nullptr) {
    exit(1);
  }
  int part = -1;
  while ((getline(&line, &len, fp)) != -1) {
    if (len != 0 && line[0] != '#' && line[0] != '@') {
      char* sline = line;
      test_case tc;
      tc.line = line;
      tc.part = part;
      read_column(&sline, tc.source);
      read_column(&sline, tc.NFC);
      read_column(&sline, tc.NFD);
      read_column(&sline, tc.NFKC);
      read_column(&sline, tc.NFKD);
      result.emplace_back(tc);
    }
    if (len != 0 && line[0] == '@') {
      part++;
    }
  }
  fclose(fp);
  if (line) {
    free(line);
  }
  return result;
}

std::string utf8_encode(const std::vector<std::uint32_t>& code_points) {
  std::string result;
  for (auto c : code_points) {
    utf8_encode_code_point(c, result, true, false);
  }
  return result;
}

TEST(Normalization, Empty) {
  EXPECT_EQ(std::vector<std::uint32_t>{}, to_nfc_x({}));
  EXPECT_EQ(std::vector<std::uint32_t>{}, to_nfd_x({}));
  EXPECT_EQ(std::vector<std::uint32_t>{}, to_nfkc_x({}));
  EXPECT_EQ(std::vector<std::uint32_t>{}, to_nfkd_x({}));
}

TEST(Normalization, UTF8) {
  const auto& argv = ::testing::internal::GetArgvs();
  ASSERT_THAT(argv, SizeIs(2));
  std::string path = argv[1];
  std::vector<test_case> test_cases = read_file(path.c_str());

  for (const auto& tc : test_cases) {
    // NFC
    EXPECT_EQ(utf8_encode(tc.NFC), to_nfc(utf8_encode(tc.source))) << tc.line;
    EXPECT_EQ(utf8_encode(tc.NFC), to_nfc(utf8_encode(tc.NFC))) << tc.line;
    EXPECT_EQ(utf8_encode(tc.NFC), to_nfc(utf8_encode(tc.NFD))) << tc.line;
    EXPECT_EQ(utf8_encode(tc.NFKC), to_nfc(utf8_encode(tc.NFKC))) << tc.line;
    EXPECT_EQ(utf8_encode(tc.NFKC), to_nfc(utf8_encode(tc.NFKD))) << tc.line;

    // NFD
    EXPECT_EQ(utf8_encode(tc.NFD), to_nfd(utf8_encode(tc.source))) << tc.line;
    EXPECT_EQ(utf8_encode(tc.NFD), to_nfd(utf8_encode(tc.NFC))) << tc.line;
    EXPECT_EQ(utf8_encode(tc.NFD), to_nfd(utf8_encode(tc.NFD))) << tc.line;
    EXPECT_EQ(utf8_encode(tc.NFKD), to_nfd(utf8_encode(tc.NFKC))) << tc.line;
    EXPECT_EQ(utf8_encode(tc.NFKD), to_nfd(utf8_encode(tc.NFKD))) << tc.line;

    // NFKC
    EXPECT_EQ(utf8_encode(tc.NFKC), to_nfkc(utf8_encode(tc.source))) << tc.line;
    EXPECT_EQ(utf8_encode(tc.NFKC), to_nfkc(utf8_encode(tc.NFC))) << tc.line;
    EXPECT_EQ(utf8_encode(tc.NFKC), to_nfkc(utf8_encode(tc.NFD))) << tc.line;
    EXPECT_EQ(utf8_encode(tc.NFKC), to_nfkc(utf8_encode(tc.NFKC))) << tc.line;
    EXPECT_EQ(utf8_encode(tc.NFKC), to_nfkc(utf8_encode(tc.NFKD))) << tc.line;

    // NFKD
    EXPECT_EQ(utf8_encode(tc.NFKD), to_nfkd(utf8_encode(tc.source))) << tc.line;
    EXPECT_EQ(utf8_encode(tc.NFKD), to_nfkd(utf8_encode(tc.NFC))) << tc.line;
    EXPECT_EQ(utf8_encode(tc.NFKD), to_nfkd(utf8_encode(tc.NFD))) << tc.line;
    EXPECT_EQ(utf8_encode(tc.NFKD), to_nfkd(utf8_encode(tc.NFKC))) << tc.line;
    EXPECT_EQ(utf8_encode(tc.NFKD), to_nfkd(utf8_encode(tc.NFKD))) << tc.line;
  }
}

TEST(Normalization, UCD) {
  const auto& argv = ::testing::internal::GetArgvs();
  ASSERT_THAT(argv, SizeIs(2));
  std::string path = argv[1];
  std::vector<test_case> test_cases = read_file(path.c_str());

  // Parts 0-5 of the conformance test
  for (const auto& tc : test_cases) {
    // NFC
    EXPECT_EQ(tc.NFC, to_nfc_x(tc.source)) << tc.line;
    EXPECT_EQ(tc.NFC, to_nfc_x(tc.NFC)) << tc.line;
    EXPECT_EQ(tc.NFC, to_nfc_x(tc.NFD)) << tc.line;
    EXPECT_EQ(tc.NFKC, to_nfc_x(tc.NFKC)) << tc.line;
    EXPECT_EQ(tc.NFKC, to_nfc_x(tc.NFKD)) << tc.line;

    // NFD
    EXPECT_EQ(tc.NFD, to_nfd_x(tc.source)) << tc.line;
    EXPECT_EQ(tc.NFD, to_nfd_x(tc.NFC)) << tc.line;
    EXPECT_EQ(tc.NFD, to_nfd_x(tc.NFD)) << tc.line;
    EXPECT_EQ(tc.NFKD, to_nfd_x(tc.NFKC)) << tc.line;
    EXPECT_EQ(tc.NFKD, to_nfd_x(tc.NFKD)) << tc.line;

    // NFKC
    EXPECT_EQ(tc.NFKC, to_nfkc_x(tc.source)) << tc.line;
    EXPECT_EQ(tc.NFKC, to_nfkc_x(tc.NFC)) << tc.line;
    EXPECT_EQ(tc.NFKC, to_nfkc_x(tc.NFD)) << tc.line;
    EXPECT_EQ(tc.NFKC, to_nfkc_x(tc.NFKC)) << tc.line;
    EXPECT_EQ(tc.NFKC, to_nfkc_x(tc.NFKD)) << tc.line;

    // NFKD
    EXPECT_EQ(tc.NFKD, to_nfkd_x(tc.source)) << tc.line;
    EXPECT_EQ(tc.NFKD, to_nfkd_x(tc.NFC)) << tc.line;
    EXPECT_EQ(tc.NFKD, to_nfkd_x(tc.NFD)) << tc.line;
    EXPECT_EQ(tc.NFKD, to_nfkd_x(tc.NFKC)) << tc.line;
    EXPECT_EQ(tc.NFKD, to_nfkd_x(tc.NFKD)) << tc.line;
  }

  std::bitset<0x110000> part1;
  for (const auto& tc : test_cases) {
    if (tc.part == 1) {
      ASSERT_THAT(tc.source, SizeIs(1)) << tc.line;
      part1.set(tc.source.front());
    }
  }

  // Check all assigned code points not in Part 1.
  for (std::uint32_t i = 0; i <= 0x10FFFF; ++i) {
    if (is_assigned(i) && !part1[i]) {
      std::vector<std::uint32_t> source = {i};
      EXPECT_EQ(source, to_nfc_x(source)) << "Source: " << i;
      EXPECT_EQ(source, to_nfd_x(source)) << "Source: " << i;
      EXPECT_EQ(source, to_nfkc_x(source)) << "Source: " << i;
      EXPECT_EQ(source, to_nfkd_x(source)) << "Source: " << i;
    }
  }
}

}  // namespace
