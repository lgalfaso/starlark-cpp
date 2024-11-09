// Copyright 2024 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <map>
#include <string>
#include <vector>

#include "grammar/lexer.hpp"
#include "grammar/quoted.hpp"

using bignum::number;
using grammar::lexer;
using grammar::quoted;
using grammar::token_type;

namespace {

std::map<token_type, std::string> mapping = {
  {token_type::ampersand, "AMPERSAND"},
  {token_type::ampersand_equals, "AMPERSAND_EQUALS"},
  {token_type::and_, "AND"},
  {token_type::as, "AS"},
  {token_type::assert, "ASSERT"},
  {token_type::break_, "BREAK"},
  {token_type::bytes, "BYTES"},
  {token_type::caret, "CARET"},
  {token_type::caret_equals, "CARET_EQUALS"},
  {token_type::class_, "CLASS"},
  {token_type::colon, "COLON"},
  {token_type::comma, "COMMA"},
  {token_type::continue_, "CONTINUE"},
  {token_type::def, "DEF"},
  {token_type::del, "DEL"},
  {token_type::dot, "DOT"},
  {token_type::elif, "ELIF"},
  {token_type::else_, "ELSE"},
  {token_type::eof, "EOF"},
  {token_type::equals, "EQUALS"},
  {token_type::equals_equals, "EQUALS_EQUALS"},
  {token_type::except, "EXCEPT"},
  {token_type::finally, "FINALLY"},
  {token_type::float_, "FLOAT"},
  {token_type::for_, "FOR"},
  {token_type::from, "FROM"},
  {token_type::global, "GLOBAL"},
  {token_type::greater, "GREATER"},
  {token_type::greater_equals, "GREATER_EQUALS"},
  {token_type::greater_greater, "GREATER_GREATER"},
  {token_type::greater_greater_equals, "GREATER_GREATER_EQUALS"},
  {token_type::identifier, "IDENTIFIER"},
  {token_type::if_, "IF"},
  {token_type::illegal, "ILLEGAL"},
  {token_type::import, "IMPORT"},
  {token_type::in, "IN"},
  {token_type::indent, "INDENT"},
  {token_type::int_, "INT"},
  {token_type::is, "IS"},
  {token_type::lambda, "LAMBDA"},
  {token_type::lbrace, "LBRACE"},
  {token_type::lbracket, "LBRACKET"},
  {token_type::less, "LESS"},
  {token_type::less_equals, "LESS_EQUALS"},
  {token_type::less_less, "LESS_LESS"},
  {token_type::less_less_equals, "LESS_LESS_EQUALS"},
  {token_type::load, "LOAD"},
  {token_type::lparen, "LPAREN"},
  {token_type::minus, "MINUS"},
  {token_type::minus_equals, "MINUS_EQUALS"},
  {token_type::newline, "NEWLINE"},
  {token_type::nonlocal, "NONLOCAL"},
  {token_type::not_, "NOT"},
  {token_type::not_equals, "NOT_EQUALS"},
  {token_type::not_in, "NOT_IN"},
  {token_type::or_, "OR"},
  {token_type::outdent, "OUTDENT"},
  {token_type::pass, "PASS"},
  {token_type::percent, "PERCENT"},
  {token_type::percent_equals, "PERCENT_EQUALS"},
  {token_type::pipe, "PIPE"},
  {token_type::pipe_equals, "PIPE_EQUALS"},
  {token_type::plus, "PLUS"},
  {token_type::plus_equals, "PLUS_EQUALS"},
  {token_type::raise, "RAISE"},
  {token_type::rbrace, "RBRACE"},
  {token_type::rbracket, "RBRACKET"},
  {token_type::return_, "RETURN"},
  {token_type::rparen, "RPAREN"},
  {token_type::semi, "SEMI"},
  {token_type::slash, "SLASH"},
  {token_type::slash_equals, "SLASH_EQUALS"},
  {token_type::slash_slash, "SLASH_SLASH"},
  {token_type::slash_slash_equals, "SLASH_SLASH_EQUALS"},
  {token_type::star, "STAR"},
  {token_type::star_equals, "STAR_EQUALS"},
  {token_type::star_star, "STAR_STAR"},
  {token_type::string, "STRING"},
  {token_type::tilde, "TILDE"},
  {token_type::try_, "TRY"},
  {token_type::while_, "WHILE"},
  {token_type::with, "WITH"},
  {token_type::yield, "YIELD"},
};

std::string to_string(const number& num) {
  std::string result;
  if (num == number::zero) {
    result = "0";
    return result;
  }
  number ref(num);
  bool neg = false;
  if (ref.sign()) {
    neg = true;
    ref.neg();
  }
  while (ref != number::zero) {
    const auto [res, rem] = number::div(ref, number(10));
    ref = res;
    auto num = rem.bits(0, 4);
    result = (char)(num + '0') + result;
  }
  if (neg) {
    result = "-" + result;
  }
  return result;
}

std::vector<std::string> readTokens(lexer& input) {
  std::vector<std::string> parts;
  do {
    input.next_token();
    const auto& current_token = input.current_token();
    parts.push_back(mapping[current_token.type()]);
    if (current_token.type() == token_type::identifier ||
        current_token.type() == token_type::string ||
        current_token.type() == token_type::bytes ||
        current_token.type() == token_type::illegal) {
      parts.back() += "(";
      parts.back() += quoted(current_token.string_value());
      parts.back() += ")";
    } else if (current_token.type() == token_type::int_) {
      parts.back() += "(";
      parts.back() += to_string(current_token.int_value());
      parts.back() += ")";
    } else if (current_token.type() == token_type::float_) {
      parts.back() += "(";
      parts.back() += std::to_string(current_token.double_value());
      parts.back() += ")";
    }
  } while (input.current_token().type() != token_type::eof);
  return parts;
}

std::string join(const std::vector<std::string>& parts) {
  std::string result;
  for (int i = 0; i < parts.size(); ++i) {
    if (i != 0) {
      result += " ";
    }
    result += parts[i];
  }
  return result;
}

void check(std::string_view input, std::string_view expected) {
  lexer l(input);
  EXPECT_EQ(expected, join(readTokens(l)));
}

void checkComments(std::string_view input, const std::vector<std::string>& expected_comments) {
  lexer l(input);
  readTokens(l);
  std::vector<std::string> comments;
  for (const auto& [comment_start, comment_end] : l.comments()) {
    comments.emplace_back(input.substr(comment_start, comment_end - comment_start));
  }
  EXPECT_EQ(expected_comments, comments);
}

void checkErrors(std::string_view input, std::string_view expected, const std::vector<std::string>& expected_errors) {
  lexer l(input);
  EXPECT_EQ(expected, join(readTokens(l)));

  std::vector<std::string> errors;
  for (const auto& [error_message, error_pos] : l.errors()) {
    errors.emplace_back(error_message);
  }
  EXPECT_EQ(expected_errors, errors);
}

TEST(LexerTest, Indentation) {
  check("", "NEWLINE EOF");
  check("# Some comment", "NEWLINE EOF");
  check("\r\n\r\n", "NEWLINE EOF");
  check("\r\n\r1\r\r\n", "INT(1) NEWLINE EOF");
  check("\r\n\r    1\r\r\n", "INDENT INT(1) NEWLINE OUTDENT NEWLINE EOF");
  check("# some\r\n# comment\r\n", "NEWLINE EOF");
  check("    1", "INDENT INT(1) NEWLINE OUTDENT NEWLINE EOF");
}

TEST(LexerTest, Comments) {
  checkComments("", {});
  checkComments("# Some comment", {" Some comment"});
  checkComments(R"starlark(
foo = "bar"  # One comment.
man = []  # Another comment.
)starlark", {" One comment.", " Another comment."});
}

TEST(LexerTest, Integer) {
  check("1", "INT(1) NEWLINE EOF");
  check("1234567890", "INT(1234567890) NEWLINE EOF");
  check("01234567890", "INT(1234567890) NEWLINE EOF");
  check("0o1234567", "INT(342391) NEWLINE EOF");
  check("0O1234567", "INT(342391) NEWLINE EOF");
  check("0x1234567890", "INT(78187493520) NEWLINE EOF");
  check("0X1234567890", "INT(78187493520) NEWLINE EOF");
  check("12345678901234567890", "INT(12345678901234567890) NEWLINE EOF");
}

TEST(LexerTest, Float) {
  check("1.0", "FLOAT(1.000000) NEWLINE EOF");
  check("1234567890.0", "FLOAT(1234567890.000000) NEWLINE EOF");
  check(".1234", "FLOAT(0.123400) NEWLINE EOF");
  checkErrors("2e308", "ILLEGAL(\"2e308\") NEWLINE EOF", {"Unable to parse numeric value"});
}

TEST(LexerTest, Identifier) {
  check("abc", "IDENTIFIER(\"abc\") NEWLINE EOF");
  check("şpěćïåł", "IDENTIFIER(\"\\305\\237p\\304\\233\\304\\207\\303\\257\\303\\245\\305\\202\") NEWLINE EOF");
  check("r a b c", "IDENTIFIER(\"r\") IDENTIFIER(\"a\") IDENTIFIER(\"b\") IDENTIFIER(\"c\") NEWLINE EOF");
}

TEST(LexerTest, SimpleFunctionCall) {
  check("foo(bar, man)",
        "IDENTIFIER(\"foo\") LPAREN IDENTIFIER(\"bar\") COMMA IDENTIFIER(\"man\") RPAREN NEWLINE EOF");
}

TEST(LexerTest, FunctionDefinition) {
  check(R"starlark(
def foo(name):
  bar(
    name = name,
    number = 1,
  )
)starlark",
        "DEF IDENTIFIER(\"foo\") LPAREN IDENTIFIER(\"name\") RPAREN COLON NEWLINE "
        "INDENT IDENTIFIER(\"bar\") LPAREN IDENTIFIER(\"name\") EQUALS IDENTIFIER(\"name\") COMMA IDENTIFIER(\"number\") EQUALS INT(1) COMMA RPAREN NEWLINE "
        "OUTDENT NEWLINE EOF");
}

TEST(LexerTest, Strings) {
  check(R"starlark(
foo = "bar"
)starlark",
        "IDENTIFIER(\"foo\") EQUALS STRING(\"bar\") NEWLINE EOF");
  check(R"starlark(
foo = "b'a'r"
)starlark",
        "IDENTIFIER(\"foo\") EQUALS STRING(\"b'a'r\") NEWLINE EOF");
  check(R"starlark(
foo = 'bar'
)starlark",
        "IDENTIFIER(\"foo\") EQUALS STRING(\"bar\") NEWLINE EOF");
  check(R"starlark(
foo = 'b"a"r'
)starlark",
        "IDENTIFIER(\"foo\") EQUALS STRING(\"b\\\"a\\\"r\") NEWLINE EOF");
}

TEST(LexerTest, StringTripleQuote) {
  check(R"starlark(
foo = """bar"""
)starlark",
        "IDENTIFIER(\"foo\") EQUALS STRING(\"bar\") NEWLINE EOF");
  check(R"starlark(
foo = """b"a"r"""
)starlark",
        "IDENTIFIER(\"foo\") EQUALS STRING(\"b\\\"a\\\"r\") NEWLINE EOF");
  check(R"starlark(
foo = """b'''a'''r"""
)starlark",
        "IDENTIFIER(\"foo\") EQUALS STRING(\"b'''a'''r\") NEWLINE EOF");
  check(R"starlark(
foo = '''bar'''
)starlark",
        "IDENTIFIER(\"foo\") EQUALS STRING(\"bar\") NEWLINE EOF");
  check(R"starlark(
foo = '''b'a'r'''
)starlark",
        "IDENTIFIER(\"foo\") EQUALS STRING(\"b'a'r\") NEWLINE EOF");
  check(R"starlark(
foo = '''b"""a"""r'''
)starlark",
        "IDENTIFIER(\"foo\") EQUALS STRING(\"b\\\"\\\"\\\"a\\\"\\\"\\\"r\") NEWLINE EOF");
}

TEST(LexerTest, StringsEscapeSequences) {
  check(R"starlark(
foo = "bar\n"
)starlark",
        "IDENTIFIER(\"foo\") EQUALS STRING(\"bar\\n\") NEWLINE EOF");
  check(R"starlark(
foo = "bar\a\b\f\n\r\t\v"
)starlark",
        "IDENTIFIER(\"foo\") EQUALS STRING(\"bar\\a\\b\\f\\n\\r\\t\\v\") NEWLINE EOF");
  check(R"starlark(
foo = "bar\
"
)starlark",
        "IDENTIFIER(\"foo\") EQUALS STRING(\"bar\") NEWLINE EOF");
  check("foo = \"bar\\\r\n\"",
        "IDENTIFIER(\"foo\") EQUALS STRING(\"bar\") NEWLINE EOF");
  check("foo = \"bar\\\r\"",
        "IDENTIFIER(\"foo\") EQUALS ILLEGAL(\"bar\") ILLEGAL(\"\") NEWLINE EOF");
  check(R"starlark(
foo = "bar\0"
)starlark",
        "IDENTIFIER(\"foo\") EQUALS STRING(\"bar\\000\") NEWLINE EOF");
  check(R"starlark(
foo = "bar\1"
)starlark",
        "IDENTIFIER(\"foo\") EQUALS STRING(\"bar\\001\") NEWLINE EOF");
  check(R"starlark(
foo = "bar\177"
)starlark",
        "IDENTIFIER(\"foo\") EQUALS STRING(\"bar\\177\") NEWLINE EOF");
  check(R"starlark(
foo = "bar\377"
)starlark",
        "IDENTIFIER(\"foo\") EQUALS ILLEGAL(\"bar\") ILLEGAL(\"\") NEWLINE EOF");
  check(R"starlark(
foo = "bar\1777"
)starlark",
        "IDENTIFIER(\"foo\") EQUALS STRING(\"bar\\1777\") NEWLINE EOF");
  check(R"starlark(
foo = "bar\x7f"
)starlark",
        "IDENTIFIER(\"foo\") EQUALS STRING(\"bar\\177\") NEWLINE EOF");
  check(R"starlark(
foo = "bar\x80"
)starlark",
        "IDENTIFIER(\"foo\") EQUALS ILLEGAL(\"bar\") ILLEGAL(\"\") NEWLINE EOF");
  check(R"starlark(
foo = "bar\u1234"
)starlark",
        "IDENTIFIER(\"foo\") EQUALS STRING(\"bar\\341\\210\\264\") NEWLINE EOF");
  check(R"starlark(
foo = "bar\U00012345"
)starlark",
        "IDENTIFIER(\"foo\") EQUALS STRING(\"bar\\360\\222\\215\\205\") NEWLINE EOF");
}

TEST(LexerTest, RawStrings) {
  check(R"starlark(
foo = r"bar\\n"
)starlark",
        "IDENTIFIER(\"foo\") EQUALS STRING(\"bar\\\\\\\\n\") NEWLINE EOF");
  check(R"starlark(
foo = r"\
")starlark",
        "IDENTIFIER(\"foo\") EQUALS STRING(\"\\\\\\n\") NEWLINE EOF");
}

TEST(LexerTest, Bytes) {
  check(R"starlark(
foo = b"bar"
)starlark",
        "IDENTIFIER(\"foo\") EQUALS BYTES(\"bar\") NEWLINE EOF");
  check(R"starlark(
foo = b"bar\x7f"
)starlark",
        "IDENTIFIER(\"foo\") EQUALS BYTES(\"bar\\177\") NEWLINE EOF");
  check(R"starlark(
foo = b"bar\x80"
)starlark",
        "IDENTIFIER(\"foo\") EQUALS BYTES(\"bar\\200\") NEWLINE EOF");
  check(R"starlark(
foo = b"bar\u1234"
)starlark",
        "IDENTIFIER(\"foo\") EQUALS ILLEGAL(\"bar\") IDENTIFIER(\"u1234\") ILLEGAL(\"\") NEWLINE EOF");
  check(R"starlark(
foo = b"bar\U00012345"
)starlark",
        "IDENTIFIER(\"foo\") EQUALS ILLEGAL(\"bar\") IDENTIFIER(\"U00012345\") ILLEGAL(\"\") NEWLINE EOF");
}

}  // namespace

