// Copyright 2026 Lucas Mirelmann

#include "unicode/word_break.hpp"

#include <algorithm>
#include <vector>

#include "unicode/ucd_code_points.hpp"

namespace starlark {
namespace unicode {

using ::starlark::ucd::word_break_type;
using ::starlark::ucd::is_Extended_Pictographic;

// TODO(lmirelmann): It should be possible to create a version of this that generates one element at a time.
// Based on Unicode Standard Annex #29, revision 47.
void word_break(const std::vector<std::uint32_t>& code_points, std::vector<std::uint64_t>& output) {
  if (code_points.empty()) {
    return;
  }

  int ri_count = 0;
  int previous_ri_count = 0;
  bool reduce = true;
  word_break_type reduced_previous_previous_word_break = word_break_type::kOther;
  word_break_type reduced_previous_word_break = word_break_type::kOther;
  word_break_type current_word_break = word_break_type::kOther;
  std::size_t last_reduced_next_word_break_check = 0;
  for (std::size_t pos = 0; pos <= code_points.size(); ++pos) {
    word_break_type actual_previous_word_break = current_word_break;
    if (reduce) {
      reduced_previous_previous_word_break = reduced_previous_word_break;
      reduced_previous_word_break = current_word_break;
    }
    if (pos < code_points.size()) {
      current_word_break = starlark::ucd::word_break(code_points[pos]);
    }
    previous_ri_count = ri_count;
    if (current_word_break == word_break_type::kRegional_Indicator) {
      ri_count += 1;
    } else {
      ri_count = 0;
    }
    reduce = true;

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
    if (actual_previous_word_break == word_break_type::kCR &&
        current_word_break == word_break_type::kLF) {
      continue;
    }
    // Otherwise break before and after Newlines (including CR and LF)
    // WB3a (Newline | CR | LF) ÷
    if (actual_previous_word_break == word_break_type::kNewline || actual_previous_word_break == word_break_type::kCR || actual_previous_word_break == word_break_type::kLF) {
      output.push_back(pos);
      continue;
    }
    // WB3b   ÷ (Newline | CR | LF)
    if (current_word_break == word_break_type::kNewline || current_word_break == word_break_type::kCR || current_word_break == word_break_type::kLF) {
      output.push_back(pos);
      continue;
    }
    // Do not break within emoji zwj sequences.
    // WB3c ZWJ × \p{Extended_Pictographic}
    if (actual_previous_word_break == word_break_type::kZWJ &&
        is_Extended_Pictographic(code_points[pos])) {
      continue;
    }
    // Keep horizontal whitespace together.
    // WB3d WSegSpace × WSegSpace
    if (actual_previous_word_break == word_break_type::kWSegSpace &&
        current_word_break == word_break_type::kWSegSpace) {
      continue;
    }

    // Ignore Format and Extend characters, except after sot, CR, LF, and Newline. (See Section 6.2, Replacing Ignore Rules.) This also has the effect of: Any × (Format | Extend | ZWJ)
    // WB4 X (Extend | Format | ZWJ)* → X
    if (current_word_break == word_break_type::kExtend || current_word_break == word_break_type::kFormat || current_word_break == word_break_type::kZWJ) {
      ri_count = previous_ri_count;
      reduce = false;
      continue;
    }

    // Do not break between most letters.
    // WB5 AHLetter × AHLetter
    if ((reduced_previous_word_break == word_break_type::kALetter || reduced_previous_word_break == word_break_type::kHebrew_Letter) &&
        (current_word_break == word_break_type::kALetter || current_word_break == word_break_type::kHebrew_Letter)) {
      continue;
    }

    word_break_type reduced_next_word_break = word_break_type::kOther;
    for (std::size_t i = std::max(last_reduced_next_word_break_check, pos + 1); i < code_points.size(); ++i) {
      last_reduced_next_word_break_check = i;
      auto candidate = starlark::ucd::word_break(code_points[i]);
      if (candidate == word_break_type::kExtend || candidate == word_break_type::kFormat || candidate == word_break_type::kZWJ) {
        continue;
      }
      reduced_next_word_break = candidate;
      break;
    }

    // Do not break letters across certain punctuation, such as within “e.g.” or “example.com”.
    // WB6 AHLetter × (MidLetter | MidNumLetQ) AHLetter
    if ((reduced_previous_word_break == word_break_type::kALetter || reduced_previous_word_break == word_break_type::kHebrew_Letter) &&
        (current_word_break == word_break_type::kMidLetter || current_word_break == word_break_type::kMidNumLet || current_word_break == word_break_type::kSingle_Quote) &&
        (reduced_next_word_break == word_break_type::kALetter || reduced_next_word_break == word_break_type::kHebrew_Letter)) {
      continue;
    }

    // WB7 AHLetter (MidLetter | MidNumLetQ) × AHLetter
    if ((reduced_previous_previous_word_break == word_break_type::kALetter || reduced_previous_previous_word_break == word_break_type::kHebrew_Letter) &&
        (reduced_previous_word_break == word_break_type::kMidLetter || reduced_previous_word_break == word_break_type::kMidNumLet || reduced_previous_word_break == word_break_type::kSingle_Quote) &&
        (current_word_break == word_break_type::kALetter || current_word_break == word_break_type::kHebrew_Letter)) {
      continue;
    }
    // WB7a Hebrew_Letter × Single_Quote
    if (reduced_previous_word_break == word_break_type::kHebrew_Letter &&
        current_word_break == word_break_type::kSingle_Quote) {
      continue;
    }
    // WB7b Hebrew_Letter × Double_Quote Hebrew_Letter
    if (reduced_previous_word_break == word_break_type::kHebrew_Letter &&
        current_word_break == word_break_type::kDouble_Quote &&
        reduced_next_word_break == word_break_type::kHebrew_Letter) {
      continue;
    }
    // WB7c Hebrew_Letter Double_Quote × Hebrew_Letter
    if (reduced_previous_previous_word_break == word_break_type::kHebrew_Letter &&
        reduced_previous_word_break == word_break_type::kDouble_Quote &&
        current_word_break == word_break_type::kHebrew_Letter) {
      continue;
    }

    // Do not break within sequences of digits, or digits adjacent to letters (“3a”, or “A3”).
    // WB8 Numeric × Numeric
    if (reduced_previous_word_break == word_break_type::kNumeric &&
        current_word_break == word_break_type::kNumeric) {
      continue;
    }

    // WB9 AHLetter × Numeric
    if ((reduced_previous_word_break == word_break_type::kALetter || reduced_previous_word_break == word_break_type::kHebrew_Letter) &&
        current_word_break == word_break_type::kNumeric) {
      continue;
    }

    // WB10 Numeric × AHLetter
    if (reduced_previous_word_break == word_break_type::kNumeric &&
        (current_word_break == word_break_type::kALetter || current_word_break == word_break_type::kHebrew_Letter)) {
      continue;
    }

    // Do not break within sequences, such as “3.2” or “3,456.789”.
    // WB11 Numeric (MidNum | MidNumLetQ) × Numeric
    if (reduced_previous_previous_word_break == word_break_type::kNumeric &&
        (reduced_previous_word_break == word_break_type::kMidNum || reduced_previous_word_break == word_break_type::kMidNumLet || reduced_previous_word_break == word_break_type::kSingle_Quote) &&
        current_word_break == word_break_type::kNumeric) {
      continue;
    }

    // WB12 Numeric × (MidNum | MidNumLetQ) Numeric
    if (reduced_previous_word_break == word_break_type::kNumeric &&
        (current_word_break == word_break_type::kMidNum || current_word_break == word_break_type::kMidNumLet || current_word_break == word_break_type::kSingle_Quote) &&
        reduced_next_word_break == word_break_type::kNumeric) {
      continue;
    }

    // Do not break between Katakana.
    // WB13 Katakana × Katakana
    if (reduced_previous_word_break == word_break_type::kKatakana &&
        current_word_break == word_break_type::kKatakana) {
      continue;
    }
    // Do not break from extenders.
    // WB13a (AHLetter | Numeric | Katakana | ExtendNumLet) × ExtendNumLet
    if ((reduced_previous_word_break == word_break_type::kALetter || reduced_previous_word_break == word_break_type::kHebrew_Letter || reduced_previous_word_break == word_break_type::kNumeric || reduced_previous_word_break == word_break_type::kKatakana || reduced_previous_word_break == word_break_type::kExtendNumLet) &&
        current_word_break == word_break_type::kExtendNumLet) {
      continue;
    }
    // WB13b ExtendNumLet × (AHLetter | Numeric | Katakana)
    if (reduced_previous_word_break == word_break_type::kExtendNumLet &&
        (current_word_break == word_break_type::kALetter || current_word_break == word_break_type::kHebrew_Letter || current_word_break == word_break_type::kNumeric || current_word_break == word_break_type::kKatakana)) {
      continue;
    }

    // Do not break within emoji flag sequences. That is, do not break between regional indicator (RI) symbols if there is an odd number of RI characters before the break point.
    // WB15 sot (RI RI)* RI × RI
    // WB16 [^RI] (RI RI)* RI × RI
    if (ri_count > 0) {
      if (ri_count % 2 == 0) {
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

