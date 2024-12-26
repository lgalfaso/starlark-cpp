// Copyright 2024 Lucas Mirelmann

#include "grammar/lexer.hpp"

#include <cassert>
#include <cmath>

#include <map>
#include <string>
#include <utility>

#include "grammar/numeric_parser.hpp"
#include "unicode/encode.hpp"
#include "unicode/ucd_code_points.hpp"

using unicode::utf8_encode_code_point;

namespace grammar {

namespace {

static const std::map<std::string, token_type, std::less<>> all_operators = {
  {"&", token_type::ampersand},
  {"&=", token_type::ampersand_equals},
  {"^", token_type::caret},
  {"^=", token_type::caret_equals},
  {":", token_type::colon},
  {",", token_type::comma},
  {".", token_type::dot},
  {"=", token_type::equals},
  {"==", token_type::equals_equals},
  {">", token_type::greater},
  {">=", token_type::greater_equals},
  {">>", token_type::greater_greater},
  {">>=", token_type::greater_greater_equals},
  {"{", token_type::lbrace},
  {"[", token_type::lbracket},
  {"(", token_type::lparen},
  {"<", token_type::less},
  {"<=", token_type::less_equals},
  {"<<", token_type::less_less},
  {"<<=", token_type::less_less_equals},
  {"-", token_type::minus},
  {"-=", token_type::minus_equals},
  {"!=", token_type::not_equals},
  {"%", token_type::percent},
  {"%=", token_type::percent_equals},
  {"|", token_type::pipe},
  {"|=", token_type::pipe_equals},
  {"+", token_type::plus},
  {"+=", token_type::plus_equals},
  {"}", token_type::rbrace},
  {"]", token_type::rbracket},
  {")", token_type::rparen},
  {";", token_type::semi},
  {"/", token_type::slash},
  {"/=", token_type::slash_equals},
  {"//", token_type::slash_slash},
  {"//=", token_type::slash_slash_equals},
  {"*", token_type::star},
  {"*=", token_type::star_equals},
  {"**", token_type::star_star},
  {"~", token_type::tilde},
};

static const std::map<std::string, token_type, std::less<>> all_keywords = {
  {"and", token_type::and_},
  {"break", token_type::break_},
  {"continue", token_type::continue_},
  {"def", token_type::def},
  {"elif", token_type::elif},
  {"else", token_type::else_},
  {"for", token_type::for_},
  {"if", token_type::if_},
  {"in", token_type::in},
  {"lambda", token_type::lambda},
  {"load", token_type::load},
  {"not", token_type::not_},
  {"or", token_type::or_},
  {"pass", token_type::pass},
  {"return", token_type::return_},


  {"as", token_type::as},
  {"assert", token_type::assert},
  {"async", token_type::async},
  {"await", token_type::await},
  {"class", token_type::class_},
  {"del", token_type::del},
  {"except", token_type::except},
  {"finally", token_type::finally},
  {"from", token_type::from},
  {"global", token_type::global},
  {"import", token_type::import},
  {"is", token_type::is},
  {"nonlocal", token_type::nonlocal},
  {"raise", token_type::raise},
  {"try", token_type::try_},
  {"while", token_type::while_},
  {"with", token_type::with},
  {"yield", token_type::yield},
};

std::map<char, std::vector<std::pair<std::string, token_type>>>
parse_operators(const std::map<std::string, token_type, std::less<>>& all_op) {
  std::map<char, std::vector<std::pair<std::string, token_type>>> result;
  for (auto it = all_op.rbegin(); it != all_op.rend(); it++) {
    result[it->first.at(0)].push_back(std::make_pair(it->first, it->second));
  }
  return result;
}

static const
  std::map<char, std::vector<std::pair<std::string, token_type>>>
  operators_by_starting_char = parse_operators(all_operators);

int isoctal(int c) {
  return '0' <= c && c <= '7' ? 1 : 0;
}

bignum::number parse_number(std::string_view input, const char** end_ptr) {
  int base;
  std::size_t pos = 0;
  if (input.starts_with("0x")) {
    pos += 2;
    base = 16;
  } else if (input.starts_with("0")) {
    pos += 1;
    base = 8;
  } else {
    base = 10;
  }
  bignum::number result;
  for (; pos < input.length(); ++pos) {
    int c = input[pos];
    if ('0' <= c && c <= '9') {
      c -= '0';
    } else if ('a' <= c && c <= 'z') {
      c -= 'a' - 10;
    } else if ('A' <= c && c <= 'Z') {
      c -= 'A' - 10;
    } else {
      break;
    }
    if (c >= base) {
      break;
    }
    result *= bignum::number(base);
    result += bignum::number(c);
  }
  if (end_ptr != nullptr) {
    *end_ptr = &input[pos];
  }
  return result;
}

}  // namespace

lexer::lexer(std::string_view input) : source_code(input), current(token_type::bof, get_position(), get_position()), indent_stack(1) {}

const token& lexer::current_token() const {
  return current;
}

void lexer::next_token() {
  bool after_newline = current.type() == token_type::newline;
  [[maybe_unused]] auto start_token = current.type();
  [[maybe_unused]] auto start_pos = get_position();
  tokenize();

  assert(current.type() != token_type::bof);
  assert(start_pos.pos < get_position().pos || start_token != current.type() || start_token == token_type::outdent);

  // Always have a `newline` token before `eof`.
  if (current.type() == token_type::eof && !after_newline) {
    current.set_type(token_type::newline);
  }
}

const std::vector<std::pair<position, position>>& lexer::comments() const {
  return comments_found;
}

const std::vector<std::pair<std::string, position>>& lexer::errors() const {
  return errors_found;
}

void lexer::tokenize() {
  bool parse_statement =
      (current.type() == token_type::newline || current.type() == token_type::bof) &&
      open_brackets == 0;
  consume_indentation(parse_statement);

  if (pending_indents < 0) {
    pending_indents++;
    current = token(token_type::outdent, get_position(), get_position());
    return;
  } else if (pending_indents > 0) {
    pending_indents--;
    auto pos = get_position();
    current = token(token_type::indent, pos - (indent_ignore + indent_stack.back() - indent_stack[indent_stack.size() - 2]), pos);
    return;
  }

  if (source_code.empty()) {
    current = token{token_type::eof, get_position(), get_position()};
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
     current = token{token_type::newline, start, get_position()};
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
         std::string illegal_char;
         auto start = get_position();

         utf8_encode_code_point(source_code.peek_code_point(), illegal_char);
         source_code.skip_code_point();

         auto end_pos = source_code.pos();
         last_begin_of_line += (end_pos - start.pos - 1);
         current = token{token_type::illegal, start, get_position(), illegal_char};
         add_error("Unexpected character", start);
         return;
       }
       if (auto element = all_keywords.find(identifier_name); element != all_keywords.end()) {
         current = token{element->second, start, get_position()};
       } else {
         current = token{token_type::identifier, start, get_position(), identifier_name};
       }
       return;
     }
  }
  current = token{token_type::eof, get_position(), get_position()};
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
      // TODO(lmirelmann): This should be a warning.
      add_error("Tab characters are not allowed for indentation. Use spaces instead.", start);
    } else if (source_code.peek() == '\n') {
      if (current.type() != token_type::newline &&
          current.type() != token_type::bof &&
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
  for (const auto& op : operators_by_starting_char.at(first_char)) {
    if (source_code.capture(op.first)) {
      current = token{op.second, start, get_position()};
      return;
    }
  }
  current = token{token_type::illegal, start, get_position(), std::string{} + first_char};
  source_code.skip();
}

void lexer::read_numeric() {
  auto start = get_position();
  auto optional_value = read_number(source_code);
  if (!optional_value.has_value()) {
    add_error("Unable to parse numeric value", start);
    // TODO(lmirelmann): Put into the token the illegal representation.
    current = token{token_type::illegal, start, get_position(), ""};
    return;
  }
  auto value = optional_value.value();
  if (value.find('.') != std::string::npos ||
      (!value.starts_with("0x") && value.find('e') != std::string::npos)) {
    char* end;
    double double_value = std::strtod(value.c_str(), &end);
    if (double_value == HUGE_VAL || end != &value.back() + 1) {
      add_error("Unable to parse numeric value", start);
      current = token{token_type::illegal, start, get_position(), value};
      return;
    }
    current = token{token_type::float_, start, get_position(), double_value};
  } else {
    const char* end;
    bignum::number int_value = parse_number(value, &end);
    if (end != &value.back() + 1) {
      add_error("Unable to parse numeric value", start);
      current = token{token_type::illegal, start, get_position(), value};
      return;
    }
    current = token{token_type::int_, start, get_position(), int_value};
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
      current = token{token_type::illegal, start, get_position(), result};
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
            ( is_triple &&  is_single_quote && source_code.capture("'''")) ||
            ( is_triple && !is_single_quote && source_code.capture("\"\"\""))) {
          if (found_errors) {
            current = token{token_type::illegal, start, get_position(), result};
          } else {
            current = token{is_bytes ? token_type::bytes : token_type::string, start, get_position(), result};
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
            if (!read_escaped_char(result, !is_bytes, is_bytes ? 255 : 127, 1, 3, 8)) {
              add_error("Invalid escape sequence", escape_start);
              found_errors = true;
            }
            break;
          case 'x':
            source_code.skip();
            if (!read_escaped_char(result, !is_bytes, is_bytes ? 255 : 127, 2, 2, 16)) {
              add_error("Invalid escape sequence", escape_start);
              found_errors = true;
            }
            break;
          case 'u':
            if (is_bytes) {
              add_error("Invalid escape sequence", escape_start);
              found_errors = true;
            }
            source_code.skip();
            if (!read_escaped_char(result, true, 0x10ffff, 4, 4, 16)) {
              add_error("Invalid escape sequence", escape_start);
              found_errors = true;
            }
            break;
          case 'U':
            if (is_bytes) {
              add_error("Invalid escape sequence", escape_start);
              found_errors = true;
            }
            source_code.skip();
            if (!read_escaped_char(result, true, 0x10ffff, 8, 8, 16)) {
              add_error("Invalid escape sequence", escape_start);
              found_errors = true;
            }
            break;
          default:
            add_error("Invalid escape sequence", escape_start);
            found_errors = true;
        }
        break;
      }
      case '\n':
        if (is_triple) {
          result += source_code.peek();
          source_code.skip();
          newline();
          break;
        }
        add_error("Unterminated string", get_position());
        current = token{token_type::illegal, start, get_position(), result};
        return;
      case '\r':
        source_code.skip();
        break;
      default: {
        // This is a lot of extra work to report the right column.
        auto ch = source_code.peek_code_point();
        if (ch == unicode::utf8_reader::replacement_character) {
          result += source_code.peek();
          source_code.skip();
        } else {
          auto start = get_position();
          utf8_encode_code_point(ch, result);
          source_code.skip_code_point();

          auto end_pos = source_code.pos();
          last_begin_of_line += (end_pos - start.pos - 1);
          // If the character ccc is not 0, then this is a non-starter, and could be skipped. Given that
          // it is showing as another character, then will count it.
        }
        break;
      }
    }
  }

  add_error("Unterminated string", get_position());
  current = token{token_type::illegal, start, get_position(), result};
}

bool lexer::read_escaped_char(std::string& result, bool utf8_encode, int max_value, int min_size, int max_size, int base) {
  std::string value;
  int(*f)(int) = (base == 8 ? isoctal : (int(*)(int))std::isxdigit);
  // This is inefficient, but we should be making not assumptions about `source_code` and its internal storage.
  for (int i = 0; i < max_size; ++i) {
    if (source_code.empty() || !f((unsigned char)source_code.peek())) {
      if (i + 1 < min_size) {
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
  if (utf8_encode) {
    utf8_encode_code_point(int_value, result);
  } else {
    result += (char)(int_value & 0xff);
  }
  return true;
}

std::string lexer::read_identifier_or_keyword() {
  std::string result;
  bool first = true;

  while (!source_code.empty()) {
    auto start = source_code.pos();
    auto ch = source_code.peek_code_point();
    if ((first && (ch == '_' || ucd::is_XID_Start(ch))) ||
        (!first && ucd::is_XID_Continue(ch))) {
      utf8_encode_code_point(ch, result);
      source_code.skip_code_point();
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

void lexer::add_error(std::string_view message, position pos) {
  errors_found.emplace_back(message, pos);
}

void lexer::add_comment(position start, position end) {
  comments_found.emplace_back(start, end);
}

position lexer::get_position() const {
  // TODO(lmirelmann): This is not taking into consideration multi-byte and continuations characters.
  return position{
    .row = current_line + 1,
    .column = source_code.pos() - last_begin_of_line + 1,
    .pos = source_code.pos(),
  };
}

void lexer::newline() {
  current_line++;
  last_begin_of_line = source_code.pos();
  indent_ignore = 0;
}

}  // namespace grammar

