// Copyright 2026 Lucas Mirelmann

#include "unicode/word_break.hpp"

#include <vector>

#include "unicode/ucd_code_points.hpp"

namespace starlark {
namespace unicode {

using ::starlark::ucd::word_break_type;
using ::starlark::ucd::is_Regional_Indicator;
using ::starlark::ucd::is_Extended_Pictographic;

// Based on Unicode Standard Annex #29, revision 47.
void word_break(const std::vector<std::uint32_t>& code_points, std::vector<std::uint64_t>& output) {
  if (code_points.empty()) {
    return;
  }
  std::vector<word_break_type> break_types;
  for (const auto& code_point : code_points) {
    auto wb = starlark::ucd::word_break(code_point);
    break_types.push_back(wb);
  }

  std::vector<word_break_type> previous_break_types;
  std::vector<std::uint32_t> previous_code_points;
  for (std::size_t pos = 0; pos <= code_points.size(); ++pos) {
    if (pos != code_points.size()) {
      previous_break_types.push_back(break_types[pos]);
      previous_code_points.push_back(code_points[pos]);
    }

    // Break at the start and end of text, unless the text is empty.
    // WB1 sot ÷ Any
    if (pos == 0) {
      output.push_back(pos);
      continue;
    }

    // WB2 Any ÷ eot
    if (pos == code_points.size()) {
      output.push_back(pos);
      continue;
    }

    // Do not break within CRLF.
    // WB3 CR × LF
    if (break_types[pos - 1] == word_break_type::kCR &&
        break_types[pos] == word_break_type::kLF) {
      continue;
    }
    // Otherwise break before and after Newlines (including CR and LF)
    // WB3a (Newline | CR | LF) ÷
    if (break_types[pos - 1] == word_break_type::kNewline || break_types[pos - 1] == word_break_type::kCR || break_types[pos - 1] == word_break_type::kLF) {
      output.push_back(pos);
      continue;
    }
    // WB3b   ÷ (Newline | CR | LF)
    if (break_types[pos] == word_break_type::kNewline || break_types[pos] == word_break_type::kCR || break_types[pos] == word_break_type::kLF) {
      output.push_back(pos);
      continue;
    }
    // Do not break within emoji zwj sequences.
    // WB3c ZWJ × \p{Extended_Pictographic}
    if (break_types[pos - 1] == word_break_type::kZWJ &&
        is_Extended_Pictographic(code_points[pos])) {
      continue;
    }
    // Keep horizontal whitespace together.
    // WB3d WSegSpace × WSegSpace
    if (break_types[pos - 1] == word_break_type::kWSegSpace &&
        break_types[pos] == word_break_type::kWSegSpace) {
      continue;
    }

    // Ignore Format and Extend characters, except after sot, CR, LF, and Newline. (See Section 6.2, Replacing Ignore Rules.) This also has the effect of: Any × (Format | Extend | ZWJ)
    // WB4 X (Extend | Format | ZWJ)* → X
    if (break_types[pos] == word_break_type::kExtend || break_types[pos] == word_break_type::kFormat || break_types[pos] == word_break_type::kZWJ) {
      previous_break_types.pop_back();
      previous_code_points.pop_back();
      continue;
    }
    word_break_type previous_word_break = previous_break_types[previous_break_types.size() - 2];
    word_break_type current_word_break = previous_break_types.back();

    // Do not break between most letters.
    // WB5 AHLetter × AHLetter
    if ((previous_word_break == word_break_type::kALetter || previous_word_break == word_break_type::kHebrew_Letter) &&
        (current_word_break == word_break_type::kALetter || current_word_break == word_break_type::kHebrew_Letter)) {
      continue;
    }

    word_break_type next_word_break = word_break_type::kOther;
    for (int i = pos + 1; i < break_types.size(); ++i) {
      if (break_types[i] == word_break_type::kExtend || break_types[i] == word_break_type::kFormat || break_types[i] == word_break_type::kZWJ) {
        continue;
      }
      next_word_break = break_types[i];
      break;
    }

    // Do not break letters across certain punctuation, such as within “e.g.” or “example.com”.
    // WB6 AHLetter × (MidLetter | MidNumLetQ) AHLetter
    if (pos + 1 < code_points.size() &&
        (previous_word_break == word_break_type::kALetter || previous_word_break == word_break_type::kHebrew_Letter) &&
        (current_word_break == word_break_type::kMidLetter || current_word_break == word_break_type::kMidNumLet || current_word_break == word_break_type::kSingle_Quote) &&
        (next_word_break == word_break_type::kALetter || next_word_break == word_break_type::kHebrew_Letter)) {
      continue;
    }

    word_break_type previous_previous_word_break = word_break_type::kOther;
    if (previous_break_types.size() > 2) {
      previous_previous_word_break = previous_break_types[previous_break_types.size() - 3];
    }

    // WB7 AHLetter (MidLetter | MidNumLetQ) × AHLetter
    if ((previous_previous_word_break == word_break_type::kALetter || previous_previous_word_break == word_break_type::kHebrew_Letter) &&
        (previous_word_break == word_break_type::kMidLetter || previous_word_break == word_break_type::kMidNumLet || previous_word_break == word_break_type::kSingle_Quote) &&
        (current_word_break == word_break_type::kALetter || current_word_break == word_break_type::kHebrew_Letter)) {
      continue;
    }
    // WB7a Hebrew_Letter × Single_Quote
    if (previous_word_break == word_break_type::kHebrew_Letter &&
        current_word_break == word_break_type::kSingle_Quote) {
      continue;
    }
    // WB7b Hebrew_Letter × Double_Quote Hebrew_Letter
    if (previous_word_break == word_break_type::kHebrew_Letter &&
        current_word_break == word_break_type::kDouble_Quote &&
        next_word_break == word_break_type::kHebrew_Letter) {
      continue;
    }
    // WB7c Hebrew_Letter Double_Quote × Hebrew_Letter
    if (previous_previous_word_break == word_break_type::kHebrew_Letter &&
        previous_word_break == word_break_type::kDouble_Quote &&
        current_word_break == word_break_type::kHebrew_Letter) {
      continue;
    }

    // Do not break within sequences of digits, or digits adjacent to letters (“3a”, or “A3”).
    // WB8 Numeric × Numeric
    if (previous_word_break == word_break_type::kNumeric &&
        current_word_break == word_break_type::kNumeric) {
      continue;
    }

    // WB9 AHLetter × Numeric
    if ((previous_word_break == word_break_type::kALetter || previous_word_break == word_break_type::kHebrew_Letter) &&
        current_word_break == word_break_type::kNumeric) {
      continue;
    }

    // WB10 Numeric × AHLetter
    if (previous_word_break == word_break_type::kNumeric &&
        (current_word_break == word_break_type::kALetter || current_word_break == word_break_type::kHebrew_Letter)) {
      continue;
    }

    // Do not break within sequences, such as “3.2” or “3,456.789”.
    // WB11 Numeric (MidNum | MidNumLetQ) × Numeric
    if (previous_previous_word_break == word_break_type::kNumeric &&
        (previous_word_break == word_break_type::kMidNum || previous_word_break == word_break_type::kMidNumLet || previous_word_break == word_break_type::kSingle_Quote) &&
        current_word_break == word_break_type::kNumeric) {
      continue;
    }

    // WB12 Numeric × (MidNum | MidNumLetQ) Numeric
    if (pos + 1 < code_points.size() &&
        previous_word_break == word_break_type::kNumeric &&
        (current_word_break == word_break_type::kMidNum || current_word_break == word_break_type::kMidNumLet || current_word_break == word_break_type::kSingle_Quote) &&
        next_word_break == word_break_type::kNumeric) {
      continue;
    }

    // Do not break between Katakana.
    // WB13 Katakana × Katakana
    if (previous_word_break == word_break_type::kKatakana &&
        current_word_break == word_break_type::kKatakana) {
      continue;
    }
    // Do not break from extenders.
    // WB13a (AHLetter | Numeric | Katakana | ExtendNumLet) × ExtendNumLet
    if ((previous_word_break == word_break_type::kALetter || previous_word_break == word_break_type::kHebrew_Letter || previous_word_break == word_break_type::kNumeric || previous_word_break == word_break_type::kKatakana || previous_word_break == word_break_type::kExtendNumLet) &&
        current_word_break == word_break_type::kExtendNumLet) {
      continue;
    }
    // WB13b ExtendNumLet × (AHLetter | Numeric | Katakana)
    if (previous_word_break == word_break_type::kExtendNumLet &&
        (current_word_break == word_break_type::kALetter || current_word_break == word_break_type::kHebrew_Letter || current_word_break == word_break_type::kNumeric || current_word_break == word_break_type::kKatakana)) {
      continue;
    }

    // Do not break within emoji flag sequences. That is, do not break between regional indicator (RI) symbols if there is an odd number of RI characters before the break point.
    // WB15 sot (RI RI)* RI × RI
    // WB16 [^RI] (RI RI)* RI × RI
    auto previous_pos = previous_code_points.size() - 1;
    if (is_Regional_Indicator(previous_code_points[previous_pos])) {
      int previous_RI = 0;
      for (auto i = previous_pos; i > 0; --i) {
        if (!is_Regional_Indicator(previous_code_points[i - 1])) {
          break;
        }
        previous_RI++;
      }
      if (previous_RI % 2 == 1) {
        continue;
      }
    }

    // Otherwise, break everywhere (including around ideographs).
    // WB999 Any ÷ Any
    output.push_back(pos);
  }

  return;
}

}  // namespace unicode
}  // namespace starlark

