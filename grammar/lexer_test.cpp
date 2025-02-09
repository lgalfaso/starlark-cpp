// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <map>
#include <string>
#include <vector>

#include "grammar/lexer.hpp"
#include "grammar/options.hpp"
#include "grammar/quoted.hpp"

using starlark::bignum::number;
using starlark::grammar::options;
using starlark::grammar::lexer;
using starlark::grammar::logger;
using starlark::grammar::quoted;
using starlark::grammar::token_type;
using testing::IsEmpty;

namespace {

const std::map<token_type, std::string> mapping = {
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
  {token_type::bof, "BOF"},
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

std::vector<std::string> read_tokens(lexer& input, std::string_view original) {
  std::vector<std::string> parts;
  do {
    input.next_token();
    const auto& current_token = input.current_token();
    parts.push_back(mapping.at(current_token.type()));
    if (current_token.type() == token_type::identifier ||
        current_token.type() == token_type::string ||
        current_token.type() == token_type::bytes ||
        current_token.type() == token_type::illegal) {
      parts.back() += "(";
      parts.back() += quoted(current_token.string_value());
      parts.back() += ")";
    } else if (current_token.type() == token_type::int_) {
      parts.back() += "(";
      parts.back() += current_token.int_value().to_string(10);
      parts.back() += ")";
    } else if (current_token.type() == token_type::float_) {
      parts.back() += "(";
      parts.back() += std::to_string(current_token.double_value());
      parts.back() += ")";
    }
    parts.back() += ":";
    parts.back() += std::to_string(current_token.start().row);
    parts.back() += ",";
    parts.back() += std::to_string(current_token.start().column);
    parts.back() += ":";
    parts.back() += std::to_string(current_token.end().row);
    parts.back() += ",";
    parts.back() += std::to_string(current_token.end().column);
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

void check(std::string_view input, std::string_view expected, options options) {
  logger logging;
  lexer l(input, options, logging);
  EXPECT_EQ(expected, join(read_tokens(l, input)));
  EXPECT_THAT(logging, IsEmpty());
}

void check(std::string_view input, std::string_view expected) {
  check(input, expected, options{});
}

void checkComments(std::string_view input, const std::vector<std::string>& expected_comments) {
  logger logging;
  lexer l(input, logging);
  read_tokens(l, input);
  std::vector<std::string> comments;
  for (const auto& [comment_start, comment_end] : l.comments()) {
    comments.emplace_back(input.substr(comment_start.pos, comment_end.pos - comment_start.pos));
  }
  EXPECT_EQ(expected_comments, comments);
}

void checkErrors(std::string_view input, std::string_view expected, const std::vector<std::string>& expected_errors) {
  logger logging;
  lexer l(input, logging);
  EXPECT_EQ(expected, join(read_tokens(l, input)));

  std::vector<std::string> errors;
  for (const auto& log_entry : logging) {
    errors.emplace_back(log_entry.message);
    errors.back() += ":";
    errors.back() += std::to_string(log_entry.pos.row);
    errors.back() += ",";
    errors.back() += std::to_string(log_entry.pos.column);
  }
  EXPECT_EQ(expected_errors, errors);
}

TEST(LexerTest, Indentation) {
  check("", "NEWLINE:1,1:1,1 EOF:1,1:1,1");
  check("# Some comment", "NEWLINE:1,15:1,15 EOF:1,15:1,15");
  check("\r\n\r\n", "NEWLINE:3,1:3,1 EOF:3,1:3,1");
  check("\r\n\r1\r\r\n", "INT(1):2,2:2,3 NEWLINE:2,5:2,6 EOF:3,1:3,1");
  check("\r\n\r    1\r\r\n", "INDENT:2,1:2,6 INT(1):2,6:2,7 NEWLINE:2,9:2,10 OUTDENT:3,1:3,1 NEWLINE:3,1:3,1 EOF:3,1:3,1");
  check("# some\r\n# comment\r\n", "NEWLINE:3,1:3,1 EOF:3,1:3,1");
  check("    1", "INDENT:1,1:1,5 INT(1):1,5:1,6 NEWLINE:1,6:1,6 OUTDENT:1,6:1,6 NEWLINE:1,6:1,6 EOF:1,6:1,6");
  check("    # Comment\n1", "INT(1):2,1:2,2 NEWLINE:2,2:2,2 EOF:2,2:2,2");
  checkErrors(R"starlark(
def foo():
    pass
  pass
)starlark",
      "DEF:2,1:2,4 IDENTIFIER(\"foo\"):2,5:2,8 LPAREN:2,8:2,9 RPAREN:2,9:2,10 COLON:2,10:2,11 NEWLINE:2,11:2,12 INDENT:3,1:3,5 PASS:3,5:3,9 NEWLINE:3,9:3,10 OUTDENT:4,3:4,3 PASS:4,3:4,7 NEWLINE:4,7:4,8 EOF:5,1:5,1",
      { "Indentation error:4,2" });
}

TEST(LexerTest, Comments) {
  checkComments("", {});
  checkComments("# Some comment", {" Some comment"});
  checkComments(R"starlark(
foo = "bar"  # One comment.
man = []  # Another comment.
)starlark", {" One comment.", " Another comment."});
  check("a = [1 + # Comment\n 1]", "IDENTIFIER(\"a\"):1,1:1,2 EQUALS:1,3:1,4 LBRACKET:1,5:1,6 INT(1):1,6:1,7 PLUS:1,8:1,9 INT(1):2,2:2,3 RBRACKET:2,3:2,4 NEWLINE:2,4:2,4 EOF:2,4:2,4");
  check("a = 1 + \\\n 1", "IDENTIFIER(\"a\"):1,1:1,2 EQUALS:1,3:1,4 INT(1):1,5:1,6 PLUS:1,7:1,8 INT(1):2,2:2,3 NEWLINE:2,3:2,3 EOF:2,3:2,3");
}

TEST(LexerTest, Integer) {
  check("1", "INT(1):1,1:1,2 NEWLINE:1,2:1,2 EOF:1,2:1,2");
  check("1234567890", "INT(1234567890):1,1:1,11 NEWLINE:1,11:1,11 EOF:1,11:1,11");
  check("01234567890", "INT(1234567890):1,1:1,12 NEWLINE:1,12:1,12 EOF:1,12:1,12");
  check("0o1234567", "INT(342391):1,1:1,10 NEWLINE:1,10:1,10 EOF:1,10:1,10");
  check("0O1234567", "INT(342391):1,1:1,10 NEWLINE:1,10:1,10 EOF:1,10:1,10");
  checkErrors("0o18", "ILLEGAL(\"0o18\"):1,1:1,5 NEWLINE:1,5:1,5 EOF:1,5:1,5", { "Unable to parse numeric value:1,1" });
  check("0x1234567890", "INT(78187493520):1,1:1,13 NEWLINE:1,13:1,13 EOF:1,13:1,13");
  check("0X1234567890", "INT(78187493520):1,1:1,13 NEWLINE:1,13:1,13 EOF:1,13:1,13");
  check("0X1234567890ABCDEFabcdef", "INT(22007822917795467892608495):1,1:1,25 NEWLINE:1,25:1,25 EOF:1,25:1,25");
  check("12345678901234567890", "INT(12345678901234567890):1,1:1,21 NEWLINE:1,21:1,21 EOF:1,21:1,21");
}

TEST(LexerTest, Float) {
  check("1.0", "FLOAT(1.000000):1,1:1,4 NEWLINE:1,4:1,4 EOF:1,4:1,4");
  check("1234567890.0", "FLOAT(1234567890.000000):1,1:1,13 NEWLINE:1,13:1,13 EOF:1,13:1,13");
  check(".1234", "FLOAT(0.123400):1,1:1,6 NEWLINE:1,6:1,6 EOF:1,6:1,6");
  checkErrors("2e308", "ILLEGAL(\"2e308\"):1,1:1,6 NEWLINE:1,6:1,6 EOF:1,6:1,6", {"Unable to parse numeric value:1,1"});
}

TEST(LexerTest, Identifier) {
  check("abc", "IDENTIFIER(\"abc\"):1,1:1,4 NEWLINE:1,4:1,4 EOF:1,4:1,4");
  check("şpěćïåł", "IDENTIFIER(\"\\305\\237p\\304\\233\\304\\207\\303\\257\\303\\245\\305\\202\"):1,1:1,8 NEWLINE:1,8:1,8 EOF:1,8:1,8");
  check("r a b c", "IDENTIFIER(\"r\"):1,1:1,2 IDENTIFIER(\"a\"):1,3:1,4 IDENTIFIER(\"b\"):1,5:1,6 IDENTIFIER(\"c\"):1,7:1,8 NEWLINE:1,8:1,8 EOF:1,8:1,8");
  check("_r", "IDENTIFIER(\"_r\"):1,1:1,3 NEWLINE:1,3:1,3 EOF:1,3:1,3");
  checkErrors("\xf2\x92\x8d\x{85}", "ILLEGAL(\"\\362\\222\\215\\205\"):1,1:1,2 NEWLINE:1,2:1,2 EOF:1,2:1,2", {"Unexpected character:1,1"});
}

TEST(LexerTest, SimpleFunctionCall) {
  check("foo(bar, man)",
        "IDENTIFIER(\"foo\"):1,1:1,4 LPAREN:1,4:1,5 IDENTIFIER(\"bar\"):1,5:1,8 COMMA:1,8:1,9 IDENTIFIER(\"man\"):1,10:1,13 RPAREN:1,13:1,14 NEWLINE:1,14:1,14 EOF:1,14:1,14");
}

TEST(LexerTest, FunctionDefinition) {
  check(R"starlark(
def foo(name):
  bar(
    name = name,
    number = 1,
  )
)starlark",
        "DEF:2,1:2,4 IDENTIFIER(\"foo\"):2,5:2,8 LPAREN:2,8:2,9 IDENTIFIER(\"name\"):2,9:2,13 RPAREN:2,13:2,14 COLON:2,14:2,15 NEWLINE:2,15:2,16 "
        "INDENT:3,1:3,3 IDENTIFIER(\"bar\"):3,3:3,6 LPAREN:3,6:3,7 IDENTIFIER(\"name\"):4,5:4,9 EQUALS:4,10:4,11 IDENTIFIER(\"name\"):4,12:4,16 COMMA:4,16:4,17 "
        "IDENTIFIER(\"number\"):5,5:5,11 EQUALS:5,12:5,13 INT(1):5,14:5,15 COMMA:5,15:5,16 RPAREN:6,3:6,4 NEWLINE:6,4:6,5 "
        "OUTDENT:7,1:7,1 NEWLINE:7,1:7,1 EOF:7,1:7,1");
}

TEST(LexerTest, Strings) {
  check(R"starlark(
foo = "bar"
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 STRING(\"bar\"):2,7:2,12 NEWLINE:2,12:2,13 EOF:3,1:3,1");
  check(R"starlark(
foo = "b'a'r"
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 STRING(\"b'a'r\"):2,7:2,14 NEWLINE:2,14:2,15 EOF:3,1:3,1");
  check(R"starlark(
foo = 'bar'
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 STRING(\"bar\"):2,7:2,12 NEWLINE:2,12:2,13 EOF:3,1:3,1");
  check(R"starlark(
foo = 'b"a"r'
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 STRING(\"b\\\"a\\\"r\"):2,7:2,14 NEWLINE:2,14:2,15 EOF:3,1:3,1");
  check(R"starlark("\"\'\\")starlark",
        "STRING(\"\\\"'\\\\\"):1,1:1,9 NEWLINE:1,9:1,9 EOF:1,9:1,9");
  check(R"starlark(
foo = "şpěćïåł"
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 STRING(\"\\305\\237p\\304\\233\\304\\207\\303\\257\\303\\245\\305\\202\"):2,7:2,16 NEWLINE:2,16:2,17 EOF:3,1:3,1");
  // It is not clear whether this is the right behavior for the column positions as this is an incomplete Unicode character.
  check("foo = \"\364\215\264\"",
        "IDENTIFIER(\"foo\"):1,1:1,4 EQUALS:1,5:1,6 STRING(\"\\364\\215\\264\"):1,7:1,12 NEWLINE:1,12:1,12 EOF:1,12:1,12");
}

TEST(LexerTest, StringTripleQuote) {
  check(R"starlark(
foo = """bar"""
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 STRING(\"bar\"):2,7:2,16 NEWLINE:2,16:2,17 EOF:3,1:3,1");
  check(R"starlark(
foo = """b"a"r"""
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 STRING(\"b\\\"a\\\"r\"):2,7:2,18 NEWLINE:2,18:2,19 EOF:3,1:3,1");
  check(R"starlark(
foo = """b'''a'''r"""
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 STRING(\"b'''a'''r\"):2,7:2,22 NEWLINE:2,22:2,23 EOF:3,1:3,1");
  check(R"starlark(
foo = '''bar'''
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 STRING(\"bar\"):2,7:2,16 NEWLINE:2,16:2,17 EOF:3,1:3,1");
  check(R"starlark(
foo = '''b'a'r'''
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 STRING(\"b'a'r\"):2,7:2,18 NEWLINE:2,18:2,19 EOF:3,1:3,1");
  check(R"starlark(
foo = '''b"""a"""r'''
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 STRING(\"b\\\"\\\"\\\"a\\\"\\\"\\\"r\"):2,7:2,22 NEWLINE:2,22:2,23 EOF:3,1:3,1");
  checkErrors(R"starlark(
foo = """bar
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 ILLEGAL(\"\\\"\\\"\\\"bar\\n\"):2,7:3,1 NEWLINE:3,1:3,1 EOF:3,1:3,1", { "Unterminated string:3,1" });
}

TEST(LexerTest, StringsEscapeSequences) {
  check(R"starlark(
foo = "bar\n"
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 STRING(\"bar\\n\"):2,7:2,14 NEWLINE:2,14:2,15 EOF:3,1:3,1");
  check(R"starlark(
foo = "bar\a\b\f\n\r\t\v"
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 STRING(\"bar\\a\\b\\f\\n\\r\\t\\v\"):2,7:2,26 NEWLINE:2,26:2,27 EOF:3,1:3,1");
  check(R"starlark(
foo = "bar\
"
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 STRING(\"bar\"):2,7:3,2 NEWLINE:3,2:3,3 EOF:4,1:4,1");
  check("foo = \"bar\\\r\n\"",
        "IDENTIFIER(\"foo\"):1,1:1,4 EQUALS:1,5:1,6 STRING(\"bar\"):1,7:2,2 NEWLINE:2,2:2,2 EOF:2,2:2,2");
  checkErrors("foo = \"bar\\\r\"",
      "IDENTIFIER(\"foo\"):1,1:1,4 EQUALS:1,5:1,6 ILLEGAL(\"\\\"bar\\\\\\r\\\"\"):1,7:1,14 NEWLINE:1,14:1,14 EOF:1,14:1,14",
      { "Invalid line continuation:1,13" });
  check(R"starlark(
foo = "bar\0"
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 STRING(\"bar\\000\"):2,7:2,14 NEWLINE:2,14:2,15 EOF:3,1:3,1");
  check(R"starlark(
foo = "bar\1"
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 STRING(\"bar\\001\"):2,7:2,14 NEWLINE:2,14:2,15 EOF:3,1:3,1");
  check(R"starlark(
foo = "bar\177"
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 STRING(\"bar\\177\"):2,7:2,16 NEWLINE:2,16:2,17 EOF:3,1:3,1");
  checkErrors(R"starlark(
foo = "bar\377"
)starlark",
      "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 ILLEGAL(\"\\\"bar\\\\377\\\"\"):2,7:2,16 NEWLINE:2,16:2,17 EOF:3,1:3,1",
      { "Invalid escape sequence:2,11" });
  check(R"starlark(
foo = "bar\1777"
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 STRING(\"bar\\1777\"):2,7:2,17 NEWLINE:2,17:2,18 EOF:3,1:3,1");
  check(R"starlark(
foo = "bar\x7f"
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 STRING(\"bar\\177\"):2,7:2,16 NEWLINE:2,16:2,17 EOF:3,1:3,1");
  checkErrors(R"starlark(
foo = "bar\x80"
)starlark",
      "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 ILLEGAL(\"\\\"bar\\\\x80\\\"\"):2,7:2,16 NEWLINE:2,16:2,17 EOF:3,1:3,1",
      { "Invalid escape sequence:2,11" });
  check(R"starlark(
foo = "bar\u1234"
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 STRING(\"bar\\341\\210\\264\"):2,7:2,18 NEWLINE:2,18:2,19 EOF:3,1:3,1");
  check(R"starlark(
foo = "bar\U00012345"
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 STRING(\"bar\\360\\222\\215\\205\"):2,7:2,22 NEWLINE:2,22:2,23 EOF:3,1:3,1");
  checkErrors(R"starlark(
foo = "bar
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 ILLEGAL(\"\\\"bar\"):2,7:2,11 NEWLINE:2,11:2,12 EOF:3,1:3,1",
       { "Unterminated string:2,11" });
  checkErrors(R"starlark(
foo = "bar\U00012)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 ILLEGAL(\"\\\"bar\\\\U00012\"):2,7:2,18 NEWLINE:2,18:2,18 EOF:2,18:2,18",
       { "Invalid escape sequence:2,11", "Unterminated string:2,18" });
  checkErrors(R"starlark(
foo = "bar\u12")starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 ILLEGAL(\"\\\"bar\\\\u12\\\"\"):2,7:2,16 NEWLINE:2,16:2,16 EOF:2,16:2,16",
       { "Invalid escape sequence:2,11" });
  checkErrors(R"starlark(
foo = "bar\U00012")starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 ILLEGAL(\"\\\"bar\\\\U00012\\\"\"):2,7:2,19 NEWLINE:2,19:2,19 EOF:2,19:2,19",
       { "Invalid escape sequence:2,11" });
  checkErrors(R"starlark(
foo = "bar\U00012 ")starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 ILLEGAL(\"\\\"bar\\\\U00012 \\\"\"):2,7:2,20 NEWLINE:2,20:2,20 EOF:2,20:2,20",
       { "Invalid escape sequence:2,11" });
  checkErrors(R"starlark(
foo = "bar\UFFFFFFFF")starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 ILLEGAL(\"\\\"bar\\\\UFFFFFFFF\\\"\"):2,7:2,22 NEWLINE:2,22:2,22 EOF:2,22:2,22",
       { "Invalid escape sequence:2,11" });
  checkErrors(R"starlark(
print ("\N{LATIN SMALL LETTER CLOSED OMEGA}")
)starlark",
        "IDENTIFIER(\"print\"):2,1:2,6 LPAREN:2,7:2,8 ILLEGAL(\"\\\"\\\\N{LATIN SMALL LETTER CLOSED OMEGA}\\\"\"):2,8:2,45 RPAREN:2,45:2,46 NEWLINE:2,46:2,47 EOF:3,1:3,1",
       { "Invalid escape sequence, the escape sequence \\N is not supported.:2,9" });
  checkErrors(R"starlark(
foo = "bar\z")starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 ILLEGAL(\"\\\"bar\\\\z\\\"\"):2,7:2,14 NEWLINE:2,14:2,14 EOF:2,14:2,14",
       { "Invalid escape sequence:2,11" });
  check(R"starlark(
foo = "bar")starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 STRING(\"bar\"):2,7:2,13 NEWLINE:2,13:2,13 EOF:2,13:2,13");
  check(R"starlark(
foo = "\200")starlark",
      "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 STRING(\"\\200\"):2,7:2,13 NEWLINE:2,13:2,13 EOF:2,13:2,13",
      options{
        .escaped_octal_and_hex_char_are_ascii = false,
  });
  checkErrors(R"starlark(
foo = "bar\ud83d")starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 ILLEGAL(\"\\\"bar\\\\ud83d\\\"\"):2,7:2,18 NEWLINE:2,18:2,18 EOF:2,18:2,18",
       { "Invalid escape sequence:2,11" });
}

TEST(LexerTest, RawStrings) {
  check(R"starlark(
foo = r"bar\\n"
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 STRING(\"bar\\\\\\\\n\"):2,7:2,16 NEWLINE:2,16:2,17 EOF:3,1:3,1");
  check(R"starlark(
foo = r"\
")starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 STRING(\"\\\\\\n\"):2,7:2,12 NEWLINE:2,12:2,12 EOF:2,12:2,12");
  check("foo = r\"\\\r\n\"",
        "IDENTIFIER(\"foo\"):1,1:1,4 EQUALS:1,5:1,6 STRING(\"\\\\\\n\"):1,7:2,2 NEWLINE:2,2:2,2 EOF:2,2:2,2");
}

TEST(LexerTest, Bytes) {
  check(R"starlark(
foo = b"bar"
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 BYTES(\"bar\"):2,7:2,13 NEWLINE:2,13:2,14 EOF:3,1:3,1");
  check(R"starlark(
foo = b"bar\x7f"
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 BYTES(\"bar\\177\"):2,7:2,17 NEWLINE:2,17:2,18 EOF:3,1:3,1");
  check(R"starlark(
foo = b"bar\x80"
)starlark",
        "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 BYTES(\"bar\\200\"):2,7:2,17 NEWLINE:2,17:2,18 EOF:3,1:3,1");
  check(R"starlark(
foo = b"bar\u1234"
)starlark",
      "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 BYTES(\"bar\\341\\210\\264\"):2,7:2,19 NEWLINE:2,19:2,20 EOF:3,1:3,1");
  check(R"starlark(
foo = b"bar\U00012345"
)starlark",
      "IDENTIFIER(\"foo\"):2,1:2,4 EQUALS:2,5:2,6 BYTES(\"bar\\360\\222\\215\\205\"):2,7:2,23 NEWLINE:2,23:2,24 EOF:3,1:3,1");
}

TEST(LexerTest, InNotIn) {
  check(R"starlark(
1 in [1, 2, 3]
)starlark",
        "INT(1):2,1:2,2 IN:2,3:2,5 LBRACKET:2,6:2,7 INT(1):2,7:2,8 COMMA:2,8:2,9 INT(2):2,10:2,11 COMMA:2,11:2,12 INT(3):2,13:2,14 RBRACKET:2,14:2,15 NEWLINE:2,15:2,16 EOF:3,1:3,1");
  check(R"starlark(
4 not in (1, 2, 3)
)starlark",
        "INT(4):2,1:2,2 NOT:2,3:2,6 IN:2,7:2,9 LPAREN:2,10:2,11 INT(1):2,11:2,12 COMMA:2,12:2,13 INT(2):2,14:2,15 COMMA:2,15:2,16 INT(3):2,17:2,18 RPAREN:2,18:2,19 NEWLINE:2,19:2,20 EOF:3,1:3,1");
}

TEST(LexerTest, Errors) {
  checkErrors("!\364\215\264\253", "ILLEGAL(\"!\"):1,1:1,2 ILLEGAL(\"\\364\\215\\264\\253\"):1,2:1,3 NEWLINE:1,3:1,3 EOF:1,3:1,3", { "Unexpected character:1,2" });
}

}  // namespace

