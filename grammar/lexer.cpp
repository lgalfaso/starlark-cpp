// Copyright 2024-2025 Lucas Mirelmann

#include "grammar/lexer.hpp"

#include <cassert>
#include <cmath>

#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "grammar/numeric_parser.hpp"
#include "unicode/encode.hpp"
#include "unicode/ucd_code_points.hpp"
#include "unicode/utf8_reader.hpp"

using ::starlark::bigint::parse_number;
using ::starlark::logging::LogLevel;
using ::starlark::logging::Position;
using ::starlark::logging::logger;
using ::starlark::unicode::utf8_encode_code_point;
using ::starlark::unicode::replacement_character_utf8;

namespace starlark {
namespace grammar {

namespace {

const std::map<std::string, token_type, std::less<>>& all_operators() {
  static const std::map<std::string, token_type, std::less<>>* result =
    new std::map<std::string, token_type, std::less<>>{
      {"&", token_type::kAmpersand},
      {"&=", token_type::kAmpersandEquals},
      {"^", token_type::kCaret},
      {"^=", token_type::kCaretEquals},
      {":", token_type::kColon},
      {",", token_type::kComma},
      {".", token_type::kDot},
      {"=", token_type::kEquals},
      {"==", token_type::kEqualsEquals},
      {">", token_type::kGreater},
      {">=", token_type::kGreaterEquals},
      {">>", token_type::kGreaterGreater},
      {">>=", token_type::kGreaterGreaterEquals},
      {"{", token_type::kLBrace},
      {"[", token_type::kLBracket},
      {"(", token_type::kLParen},
      {"<", token_type::kLess},
      {"<=", token_type::kLessEquals},
      {"<<", token_type::kLessLess},
      {"<<=", token_type::kLessLessEquals},
      {"-", token_type::kMinus},
      {"-=", token_type::kMinusEquals},
      {"!=", token_type::kNotEquals},
      {"%", token_type::kPercent},
      {"%=", token_type::kPercentEquals},
      {"|", token_type::kPipe},
      {"|=", token_type::kPipeEquals},
      {"+", token_type::kPlus},
      {"+=", token_type::kPlusEquals},
      {"}", token_type::kRBrace},
      {"]", token_type::kRBracket},
      {")", token_type::kRParen},
      {";", token_type::kSemi},
      {"/", token_type::kSlash},
      {"/=", token_type::kSlashEquals},
      {"//", token_type::kSlashSlash},
      {"//=", token_type::kSlashSlashEquals},
      {"*", token_type::kStar},
      {"*=", token_type::kStarEquals},
      {"**", token_type::kStarStar},
      {"~", token_type::kTilde},
    };

  return *result;
}

const std::map<std::string, token_type, std::less<>>& all_keywords() {
  static const std::map<std::string, token_type, std::less<>>* result =
    new std::map<std::string, token_type, std::less<>>{
      {"and", token_type::kAnd},
      {"break", token_type::kBreak},
      {"continue", token_type::kContinue},
      {"def", token_type::kDef},
      {"elif", token_type::kElif},
      {"else", token_type::kElse},
      {"for", token_type::kFor},
      {"if", token_type::kIf},
      {"in", token_type::kIn},
      {"lambda", token_type::kLambda},
      {"load", token_type::kLoad},
      {"not", token_type::kNot},
      {"or", token_type::kOr},
      {"pass", token_type::kPass},
      {"return", token_type::kReturn},


      {"as", token_type::kAs},
      {"assert", token_type::kAssert},
      {"async", token_type::kAsync},
      {"await", token_type::kAwait},
      {"class", token_type::kClass},
      {"del", token_type::kDel},
      {"except", token_type::kExcept},
      {"finally", token_type::kFinally},
      {"from", token_type::kFrom},
      {"global", token_type::kGlobal},
      {"import", token_type::kImport},
      {"is", token_type::kIs},
      {"nonlocal", token_type::kNonlocal},
      {"raise", token_type::kRaise},
      {"try", token_type::kTry},
      {"while", token_type::kWhile},
      {"with", token_type::kWith},
      {"yield", token_type::kYield},
    };

  return *result;
}

std::map<char, std::vector<std::pair<std::string, token_type>>>
parse_operators(const std::map<std::string, token_type, std::less<>>& all_op) {
  std::map<char, std::vector<std::pair<std::string, token_type>>> result;
  for (auto it = all_op.rbegin(); it != all_op.rend(); it++) {
    result[it->first.at(0)].push_back(std::make_pair(it->first, it->second));
  }
  return result;
}

const std::map<char, std::vector<std::pair<std::string, token_type>>>& operators_by_starting_char() {
  static const std::map<char, std::vector<std::pair<std::string, token_type>>>* result =
    new std::map<char, std::vector<std::pair<std::string, token_type>>>(parse_operators(all_operators()));

  return *result;
}

int isoctal(int c) {
  return '0' <= c && c <= '7' ? 1 : 0;
}

}  // namespace

lexer::lexer(std::string_view input, logger& logging) : lexer(input, grammar_options{}, logging) {}

lexer::lexer(std::string_view input, const grammar_options& opts, logger& logging) :
    opts(opts), input(input), source_code(input, false, true), current(token_type::kBof, get_position(), get_position()), indent_stack(1), logging(logging) {}

const token& lexer::current_token() const {
  return current;
}

void lexer::next_token() {
  bool after_newline = current.type() == token_type::kNewline;
  [[maybe_unused]] auto start_token = current.type();
  [[maybe_unused]] auto start_pos = get_position();
  tokenize();

  assert(current.type() != token_type::kBof);
  assert(start_pos.pos() < get_position().pos() || start_token != current.type() || start_token == token_type::kOutdent);

  // Always have a `newline` token before `eof`.
  if (current.type() == token_type::kEof && !after_newline) {
    current.set_type(token_type::kNewline);
  }
}

const std::vector<std::pair<Position, Position>>& lexer::comments() const {
  return comments_found;
}

void lexer::tokenize() {
  bool parse_statement =
      (current.type() == token_type::kNewline || current.type() == token_type::kBof) &&
      open_brackets == 0;
  consume_indentation(parse_statement);

  if (pending_indents < 0) {
    pending_indents++;
    current = token(token_type::kOutdent, get_position(), get_position());
    return;
  } else if (pending_indents > 0) {
    pending_indents--;
    auto pos = get_position();
    current = token(token_type::kIndent, pos - (indent_ignore + indent_stack.back() - indent_stack[indent_stack.size() - 2]), pos);
    return;
  }

  if (source_code.empty()) {
    current = token{token_type::kEof, get_position(), get_position()};
    return;
  }

  char next_char = source_code.peek();
  switch (next_char) {
    case '&':
    case '^':
    case ':':
    case ',':
    case '=':
    case '>':
    case '<':
    case '-':
    case '!':
    case '%':
    case '|':
    case '+':
    case ';':
    case '/':
    case '*':
    case '~':
      read_operator(next_char);
      return;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      read_numeric();
      return;
    case '.':
      if (isdigit(source_code.peek(1))) {
        read_numeric();
        return;
      }
      read_operator(next_char);
      return;
    case '{':
    case '[':
    case '(':
      open_brackets++;
      read_operator(next_char);
      return;
    case '}':
    case ']':
    case ')':
      if (open_brackets == 0) {
        add_error("Dangling bracket", get_position());
      } else {
        open_brackets--;
      }
      read_operator(next_char);
      return;
    case '\n': {
      auto start = get_position();
      source_code.skip();
      current = token{token_type::kNewline, start, get_position()};
      newline();
      return;
    }
    case '"':
    case '\'':
      read_string();
      return;
    default: {
      if ((next_char == 'r' && (source_code.next("r\"") || source_code.next("r'") || source_code.next("rb\"") || source_code.next("rb'"))) ||
          (next_char == 'b' && (source_code.next("b\"") || source_code.next("b'") || source_code.next("br\"") || source_code.next("br'")))) {
        read_string();
        return;
      }
      auto start = get_position();
      auto identifier_name = read_identifier_or_keyword();
      if (identifier_name.empty()) {
        // Skip the current character.
        auto start = get_position();
        source_code.read_code_point();
        auto end_pos = source_code.pos();
        last_begin_of_line += (end_pos - start.pos() - 1);

        auto end = get_position();
        current = token{token_type::kIllegal, start, end, std::string{input.substr(start.pos(), end.pos() - start.pos())}};
        add_error("Unexpected character", start);
        return;
      }
      const auto& keywords = all_keywords();
      if (auto element = keywords.find(identifier_name); element != keywords.end()) {
        current = token{element->second, start, get_position()};
      } else {
        current = token{token_type::kIdentifier, start, get_position(), identifier_name};
      }
      return;
    }
  }
  current = token{token_type::kEof, get_position(), get_position()};
}

void lexer::consume_indentation(bool modify_indents) {
  int indentation_length = 0;
  while (!source_code.empty()) {
    auto start = get_position();
    if (source_code.capture(" ")) {
      indentation_length++;
    } else if (source_code.capture("\r")) {
      // The char '\r` should be ignored and not handled as a whitespace.
      indent_ignore++;
    } else if (source_code.capture("\\\n") || source_code.capture("\\\r\n")) {
      newline();
    } else if (source_code.capture("\t")) {
      indentation_length++;
      add_warning("Tab characters are not allowed for indentation. Use spaces instead.", start);
    } else if (source_code.peek() == '\n') {
      if (current.type() != token_type::kNewline &&
          current.type() != token_type::kBof &&
          open_brackets == 0) {
        break;
      }
      source_code.skip();
      newline();
      indentation_length = 0;
    } else if (source_code.capture("#")) {
      auto comment_start = get_position();
      while (!source_code.empty() && source_code.peek() != '\n') {
        source_code.skip();
      }
      add_comment(comment_start, get_position());
    } else {  // End of indentation.
      break;
    }
  }

  if (!modify_indents) {
    return;
  }
  if (source_code.empty()) {
    indentation_length = 0;
  }

  if (indent_stack.back() < indentation_length) {  // Increase the indentation.
    indent_stack.push_back(indentation_length);
    pending_indents++;
  } else {  // Maybe decrease the indentation.
    while (indent_stack.back() > indentation_length) {
      indent_stack.pop_back();
      pending_indents--;
    }

    if (indent_stack.back() < indentation_length) {
      add_error("Indentation error", get_position() - 1);
    }
  }
}

void lexer::read_operator(char first_char) {
  auto start = get_position();
  for (const auto& op : operators_by_starting_char().at(first_char)) {
    if (source_code.capture(op.first)) {
      current = token{op.second, start, get_position()};
      return;
    }
  }
  source_code.skip();
  current = token{token_type::kIllegal, start, get_position(), std::string {} + first_char};
}

void lexer::read_numeric() {
  auto start = get_position();
  auto optional_value = read_number(source_code, opts.allow_binary_integer_literals);
  if (!optional_value.ok()) {
    add_error("Unable to parse numeric value", start);
    auto end = get_position();
    current = token{token_type::kIllegal, start, end, std::string{input.substr(start.pos(), end.pos() - start.pos())}};
    return;
  }
  const auto& value = *optional_value;
  if (value.find('.') != std::string::npos ||
      (!value.starts_with("0x") && value.find('e') != std::string::npos)) {
    char* end;
    double double_value = std::strtod(value.c_str(), &end);
    if (double_value == HUGE_VAL || end != &value.back() + 1) {
      add_error("Unable to parse numeric value", start);
      current = token{token_type::kIllegal, start, get_position(), value};
      return;
    }
    current = token{token_type::kFloat, start, get_position(), double_value};
  } else {
    const char* end;
    starlark::bigint::number int_value = parse_number(value, &end, 0);
    // At this stage, we do not care about the size of the bigint. This is a runtime concern.
    if (end != &value.back() + 1) {
      add_error("Unable to parse numeric value", start);
      current = token{token_type::kIllegal, start, get_position(), value};
      return;
    }
    if (int_value.bit_size() < 64) {
      current = token{token_type::kInt, start, get_position(), static_cast<std::int64_t>(int_value.at(0))};
    } else {
      current = token{token_type::kBigInt, start, get_position(), int_value};
    }
  }
}

void lexer::read_string() {
  auto start = get_position();
  std::string result;
  bool is_raw = false;
  bool is_bytes = false;
  while (!source_code.next("\"") && !source_code.next("'")) {
    if (source_code.capture("r")) {
      is_raw = true;
    } else if (source_code.capture("b")) {
      is_bytes = true;
    } else {
      add_error("Unterminated string", get_position());
      auto end = get_position();
      current = token{token_type::kIllegal, start, end, std::string{input.substr(start.pos(), end.pos() - start.pos())}};
      return;
    }
  }
  bool is_single_quote = source_code.peek() == '\'';
  source_code.skip();
  bool is_triple = source_code.capture(is_single_quote ? "''" : "\"\"");
  bool found_errors = false;
  while (!source_code.empty()) {
    switch (source_code.peek()) {
      case '\'':
      case '"':
        if ((!is_triple &&  is_single_quote && source_code.capture("'")) ||
            (!is_triple && !is_single_quote && source_code.capture("\"")) ||
            (is_triple &&  is_single_quote && source_code.capture("'''")) ||
            (is_triple && !is_single_quote && source_code.capture("\"\"\""))) {
          if (found_errors) {
            auto end = get_position();
            current = token{token_type::kIllegal, start, end, std::string{input.substr(start.pos(), end.pos() - start.pos())}};
          } else {
            current = token{is_bytes ? token_type::kBytes : token_type::kString, start, get_position(), result};
          }
          return;
        }
        result += source_code.peek();
        source_code.skip();
        break;
      case '\\': {
        auto escape_start = get_position();
        if (is_raw) {
          // Add the character '\\' and the following one, with these exceptions:
          // "\r\n" => "\n"
          // "\r" => "\n"
          result += source_code.peek();
          source_code.skip();
          if (source_code.empty()) {
            break;
          }
          if (source_code.capture("\r\n")) {
            result += "\n";
            newline();
          } else if (source_code.capture("\r")) {
            result += "\n";
          } else {
            result += source_code.peek();
            source_code.skip();
          }
          break;
        }
        source_code.skip();
        if (source_code.empty()) {
          break;
        }
        switch (source_code.peek()) {
          case '\\':
            result += "\\";
            source_code.skip();
            break;
          case '"':
            result += "\"";
            source_code.skip();
            break;
          case '\'':
            result += "'";
            source_code.skip();
            break;
          case 'a':
            result += "\a";
            source_code.skip();
            break;
          case 'b':
            result += "\b";
            source_code.skip();
            break;
          case 'f':
            result += "\f";
            source_code.skip();
            break;
          case 'n':
            result += "\n";
            source_code.skip();
            break;
          case 'r':
            result += "\r";
            source_code.skip();
            break;
          case 't':
            result += "\t";
            source_code.skip();
            break;
          case 'v':
            result += "\v";
            source_code.skip();
            break;
          case '\n':
            source_code.skip();
            newline();
            break;
          case '\r':
            source_code.skip();
            if (source_code.capture("\n")) {
              newline();
            } else {
              add_error("Invalid line continuation", get_position());
              found_errors = true;
            }
            break;
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
            if (!read_escaped_char(result, !is_bytes && opts.escaped_octal_and_hex_char_are_ascii, is_bytes || !opts.escaped_octal_and_hex_char_are_ascii ? 255 : 127, 1, 3, 8)) {
              add_error("Invalid escape sequence", escape_start);
              found_errors = true;
            }
            break;
          case 'x':
            source_code.skip();
            if (!read_escaped_char(result, !is_bytes && opts.escaped_octal_and_hex_char_are_ascii, is_bytes || !opts.escaped_octal_and_hex_char_are_ascii ? 255 : 127, 2, 2, 16)) {
              add_error("Invalid escape sequence", escape_start);
              found_errors = true;
            }
            break;
          case 'u':
            source_code.skip();
            if (!read_escaped_char(result, true, unicode::utf8_reader::kMaxCodePoint, 4, 4, 16)) {
              add_error("Invalid escape sequence", escape_start);
              found_errors = true;
            }
            break;
          case 'U':
            source_code.skip();
            if (!read_escaped_char(result, true, unicode::utf8_reader::kMaxCodePoint, 8, 8, 16)) {
              add_error("Invalid escape sequence", escape_start);
              found_errors = true;
            }
            break;
          case 'N':
            add_error("Invalid escape sequence, the escape sequence \\N is not supported.", escape_start);
            found_errors = true;
            source_code.skip();
            break;
          default:
            add_error("Invalid escape sequence", escape_start);
            found_errors = true;
            break;
        }
        break;
      }
      case '\n': {
        if (is_triple) {
          result += source_code.peek();
          source_code.skip();
          newline();
          break;
        }
        add_error("Unterminated string", get_position());
        auto end = get_position();
        current = token{token_type::kIllegal, start, end, std::string{input.substr(start.pos(), end.pos() - start.pos())}};
        return;
      }
      case '\r':
        source_code.skip();
        break;
      default: {
        // Handle a literal replacement character.
        if (source_code.capture(replacement_character_utf8())) {
          result += replacement_character_utf8();
          last_begin_of_line += replacement_character_utf8().size() - 1;
          break;
        }
        // This is a lot of extra work to report the right column.
        auto ch = source_code.peek_code_point();
        if (ch == unicode::utf8_reader::kReplacementCharacter) {
          found_errors = true;
          result += source_code.peek();
          source_code.skip();
        } else {
          auto start = get_position();
          utf8_encode_code_point(ch, result, false, false);
          // TODO(lmirelmann): Would be nice to avoid doing this extra read, but keep on handling the error case.
          source_code.read_code_point();

          auto end_pos = source_code.pos();
          last_begin_of_line += (end_pos - start.pos() - 1);
          // If the character ccc is not 0, then this is a non-starter, and could be skipped. Given that
          // it is showing as another character, then will count it.
        }
        break;
      }
    }
  }

  add_error("Unterminated string", get_position());
  auto end = get_position();
  current = token{token_type::kIllegal, start, end, std::string{input.substr(start.pos(), end.pos() - start.pos())}};
}

bool lexer::read_escaped_char(std::string& result, bool utf8_encode, int max_value, int min_size, int max_size, int base) {
  std::string value;
  int(*f)(int) = (base == 8 ? isoctal : (int(*)(int))std::isxdigit);
  // This is inefficient, but we should be making not assumptions about `source_code` and its internal storage.
  for (int i = 0; i < max_size; ++i) {
    if (source_code.empty() || !f((unsigned char)source_code.peek())) {
      if (i < min_size) {
        return false;
      }
      break;
    }
    value += source_code.peek();
    source_code.skip();
  }
  errno = 0;
  char* end;
  std::int64_t int_value = std::strtol(value.c_str(), &end, base);
  if (errno == ERANGE || end != &value.back() + 1) {
    return false;
  }
  if (int_value > max_value) {
    return false;
  }
  bool error = false;
  if (utf8_encode) {
    // It is an error if the code point is not assigned.
    // The spec is not clear, as it reads:
    //
    //   Strings
    //
    //   (...)
    //   An implementation may permit strings to hold arbitrary values of the
    //   element type, including sequences that do not denote encode valid
    //   Unicode text; or, it may disallow invalid sequences, and operations
    //   that would form them.
    //
    //
    // In other parts of the spec, it states
    //
    //   "\ud83d"                # error: invalid Unicode code point U+D83D
    //
    //
    // And at no point in time, it tries to define what is an "invalid sequence".
    //
    // Unicode talks about "invalid sequences" when talking about encoding. Given
    // that literals do not define the encoding of the characters, then this
    // leaves it open to interpretation.
    //
    // The interpretation that we are using is that this must be a character
    // that can be present on its own in a valid sequence, even when the character
    // were not assigned or were in the Do Not Emit list. This is, the literal
    // consisting of just this code point is not "ill-formed" using the
    // definition from Unicode for ill-formed.
    // With this definition, the character must be within the Unicode range,
    // and not a surrogate.
    if (unicode::is_surrogate(int_value) || !unicode::is_in_range(int_value)) {
      error = true;
    }
    utf8_encode_code_point(int_value, result, false, false);
  } else {
    result += static_cast<char>(int_value & 0xff);
  }
  return !error;
}

std::string lexer::read_identifier_or_keyword() {
  std::string result;
  bool first = true;

  while (!source_code.empty()) {
    auto start = source_code.pos();
    auto ch = source_code.peek_code_point();
    if ((first && (ch == '_' || ucd::is_XID_Start(ch))) ||
        (!first && ucd::is_XID_Continue(ch))) {
      utf8_encode_code_point(ch, result, false, false);
      source_code.read_code_point();
    } else {
      break;
    }
    auto end = source_code.pos();
    last_begin_of_line += (end - start - 1);
    // If the character ccc is not 0, then this is a non-starter, and could be skipped. Given that
    // it is showing as another character, then will count it.
    first = false;
  }
  return result;
}

void lexer::add_error(std::string_view message, Position pos) {
  logging.log(LogLevel::LOG_LEVEL_ERROR, message, module, pos);
}

void lexer::add_warning(std::string_view message, Position pos) {
  logging.log(LogLevel::LOG_LEVEL_WARNING, message, module, pos);
}

void lexer::add_comment(Position start, Position end) {
  comments_found.emplace_back(start, end);
}

Position lexer::get_position() const {
  Position result;
  result.set_row(current_line + 1);
  result.set_column(source_code.pos() - last_begin_of_line + 1);
  result.set_pos(source_code.pos());
  return result;
}

void lexer::newline() {
  current_line++;
  last_begin_of_line = source_code.pos();
  indent_ignore = 0;
}

}  // namespace grammar
}  // namespace starlark

