// Copyright 2024 Lucas Mirelmann

#include "unicode/normalization.hpp"

#include <algorithm>
#include <iostream>

#include "unicode/encode.hpp"
#include "unicode/ucd_code_points.hpp"
#include "unicode/utf8_reader.hpp"

namespace unicode {

namespace {

bool is_korean_leading(std::uint32_t c) {
  return c >= 0x1100 && c <= 0x1112;
}

bool is_korean_vowel(std::uint32_t c) {
  return c >= 0x1161 && c <= 0x1175;
}

bool is_korean_trailing(std::uint32_t c) {
  return c >= 0x11A8 && c <= 0x11C2;
}

std::vector<std::uint32_t> compose(const std::vector<std::uint32_t>& input) {
  if (input.empty()) {
    return input;
  }
  std::vector<std::uint32_t> result;
  result.push_back(input.front());
  int starter_pos = 0;
  int last_ccc = 0;
  bool added_non_starter = false;
  int k = is_korean_leading(input.front()) ? 1 : 0;
  for (int i = 1; i < input.size(); ++i) {
    if (k == 1) {
      if (is_korean_vowel(input[i])) {
        result[starter_pos] = (result[starter_pos] - 0x1100) * 588 + (input[i] - 0x1161) * 28 + 0xAC00;
        k = 2;
        continue;
      }
    }
    if (k == 2) {
      if (is_korean_trailing(input[i])) {
        result[starter_pos] +=  input[i] - 0x11A7;
        k = 0;
        continue;
      }
    }
    if (is_korean_leading(input[i])) {
      starter_pos = result.size();
      result.push_back(input[i]);
      k = 1;
      continue;
    }
    k = 0;
    int current_ccc = ucd::ccc(input[i]);
    if (current_ccc == 0) {
      if (added_non_starter) {
        starter_pos = result.size();
        result.push_back(input[i]);
      } else {
        auto composition = ucd::canonical_composition(result[starter_pos], input[i]);
        if (composition) {
          result[starter_pos] = composition.value();
        } else {
          starter_pos = result.size();
          result.push_back(input[i]);
        }
      }
      last_ccc = current_ccc;
    } else {
      if (current_ccc > last_ccc) {
        auto composition = ucd::canonical_composition(result[starter_pos], input[i]);
        if (composition) {
          result[starter_pos] = composition.value();
        } else {
          result.push_back(input[i]);
          added_non_starter = true;
          last_ccc = current_ccc;
        }
      } else {
        result.push_back(input[i]);
        added_non_starter = true;
        last_ccc = current_ccc;
      }
    }
  }
  return result;
}

bool decompose_korean(std::vector<std::uint32_t>& output, std::uint32_t c) {
  if (c < 0xAC00 || 0xD7A3 < c) {
    return false;
  }
  c -= 0xAC00;
  output.push_back(0x1100 + c / 588);
  output.push_back(0x1161 + (c % 588) / 28);
  auto t = c % 28;
  if (t != 0) {
    output.push_back(0x11A7 + t);
  }
  return true;
}

void do_nfd(std::vector<std::uint32_t>& output, std::uint32_t c) {
  if (decompose_korean(output,c )) {
    return;
  }
  if (ucd::is_compatibility_decomposition(c)) {
    output.push_back(c);
    return;
  }
  auto& decomp = ucd::decomposition(c);
  if (decomp.size() == 0) {
    output.push_back(c);
    return;
  }
  for (auto cc : decomp) {
    do_nfd(output, cc);
  }
}

void do_nfkd(std::vector<std::uint32_t>& output, std::uint32_t c) {
  if (decompose_korean(output,c )) {
    return;
  }
  auto& decomp = ucd::decomposition(c);
  if (decomp.size() == 0) {
    output.push_back(c);
    return;
  }
  for (auto cc : decomp) {
    do_nfkd(output, cc);
  }
}

void sort_non_starters(std::vector<std::uint32_t>& code_points) {
  auto begin = code_points.begin();
  for (auto it = code_points.begin(); it != code_points.end(); ++it) {
    if (ucd::ccc(*it) == 0) {
      std::stable_sort(begin, it, [](auto a, auto b) { return ucd::ccc(a) < ucd::ccc(b); });
      begin = it;
    }
  }
  std::stable_sort(begin, code_points.end(), [](auto a, auto b) { return ucd::ccc(a) < ucd::ccc(b); });
}

}

std::string to_nfkc(std::string_view input) {
  // TODO(lmirelmann): Check whether this is already a valid encoding using NFKC_QC
  std::vector<std::uint32_t> code_points;
  utf8_reader reader(input);
  while (reader.pending()) {
    code_points.push_back(reader.peek_code_point());
    reader.skip_code_point();
  }
std::cout << "Code point source: " << code_points[0] << "\n";
  std::vector<std::uint32_t> result_cp = to_nfkc_x(code_points);
  std::string result;
  for (auto c : result_cp) {
    utf8_encode_code_point(c, result);
  }
  return result;
}

std::vector<std::uint32_t> to_nfc_x(const std::vector<std::uint32_t>& input) {
  return compose(to_nfd_x(input));
}

std::vector<std::uint32_t> to_nfd_x(const std::vector<std::uint32_t>& input) {
  std::vector<std::uint32_t> result;
  for (auto c : input) {
    do_nfd(result, c);
  }
  sort_non_starters(result);
  return result;
}

std::vector<std::uint32_t> to_nfkc_x(const std::vector<std::uint32_t>& input) {
  return compose(to_nfkd_x(input));
}

std::vector<std::uint32_t> to_nfkd_x(const std::vector<std::uint32_t>& input) {
  std::vector<std::uint32_t> result;
  for (auto c : input) {
    do_nfkd(result, c);
  }
  sort_non_starters(result);
  return result;
}

}  // namespace unicode

