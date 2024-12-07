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
        current_token.type() == token_type::bytes) {
      parts.back() += "(";
      parts.back() += quoted(current_token.string_value());
      parts.back() += ")";
    } else if (current_token.type() == token_type::illegal) {
      parts.back() += "(";
      parts.back() += quoted(original.substr(current_token.start(), current_token.end() - current_token.start()));
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
    parts.back() += std::to_string(current_token.start());
    parts.back() += ":";
    parts.back() += std::to_string(current_token.end());
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
  EXPECT_EQ(expected, join(read_tokens(l, input)));
  EXPECT_THAT(l.errors(), IsEmpty());
}

void checkComments(std::string_view input, const std::vector<std::string>& expected_comments) {
  lexer l(input);
  read_tokens(l, input);
  std::vector<std::string> comments;
  for (const auto& [comment_start, comment_end] : l.comments()) {
    comments.emplace_back(input.substr(comment_start, comment_end - comment_start));
  }
  EXPECT_EQ(expected_comments, comments);
}

void checkErrors(std::string_view input, std::string_view expected, const std::vector<std::string>& expected_errors) {
  lexer l(input);
  EXPECT_EQ(expected, join(read_tokens(l, input)));

  std::vector<std::string> errors;
  for (const auto& [error_message, error_pos] : l.errors()) {
    errors.emplace_back(error_message);
    errors.back() += ":";
    errors.back() += std::to_string(error_pos);
  }
  EXPECT_EQ(expected_errors, errors);
}

TEST(LexerTest, Indentation) {
  check("", "NEWLINE:0:0 EOF:0:0");
  check("# Some comment", "NEWLINE:14:14 EOF:14:14");
  check("\r\n\r\n", "NEWLINE:4:4 EOF:4:4");
  check("\r\n\r1\r\r\n", "INT(1):3:4 NEWLINE:6:7 EOF:7:7");
  check("\r\n\r    1\r\r\n", "INDENT:3:7 INT(1):7:8 NEWLINE:10:11 OUTDENT:11:11 NEWLINE:11:11 EOF:11:11");
  check("# some\r\n# comment\r\n", "NEWLINE:19:19 EOF:19:19");
  check("    1", "INDENT:0:4 INT(1):4:5 NEWLINE:5:5 OUTDENT:5:5 NEWLINE:5:5 EOF:5:5");
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
  check("1", "INT(1):0:1 NEWLINE:1:1 EOF:1:1");
  check("1234567890", "INT(1234567890):0:10 NEWLINE:10:10 EOF:10:10");
  check("01234567890", "INT(1234567890):0:11 NEWLINE:11:11 EOF:11:11");
  check("0o1234567", "INT(342391):0:9 NEWLINE:9:9 EOF:9:9");
  check("0O1234567", "INT(342391):0:9 NEWLINE:9:9 EOF:9:9");
  checkErrors("0o18", "ILLEGAL(\"0o18\"):0:4 NEWLINE:4:4 EOF:4:4", { "Unable to parse numeric value:0" });
  check("0x1234567890", "INT(78187493520):0:12 NEWLINE:12:12 EOF:12:12");
  check("0X1234567890", "INT(78187493520):0:12 NEWLINE:12:12 EOF:12:12");
  check("0X1234567890ABCDEFabcdef", "INT(22007822917795467892608495):0:24 NEWLINE:24:24 EOF:24:24");
  check("12345678901234567890", "INT(12345678901234567890):0:20 NEWLINE:20:20 EOF:20:20");
}

TEST(LexerTest, Float) {
  check("1.0", "FLOAT(1.000000):0:3 NEWLINE:3:3 EOF:3:3");
  check("1234567890.0", "FLOAT(1234567890.000000):0:12 NEWLINE:12:12 EOF:12:12");
  check(".1234", "FLOAT(0.123400):0:5 NEWLINE:5:5 EOF:5:5");
  checkErrors("2e308", "ILLEGAL(\"2e308\"):0:5 NEWLINE:5:5 EOF:5:5", {"Unable to parse numeric value:0"});
}

TEST(LexerTest, Identifier) {
  check("abc", "IDENTIFIER(\"abc\"):0:3 NEWLINE:3:3 EOF:3:3");
  check("şpěćïåł", "IDENTIFIER(\"\\305\\237p\\304\\233\\304\\207\\303\\257\\303\\245\\305\\202\"):0:13 NEWLINE:13:13 EOF:13:13");
  check("r a b c", "IDENTIFIER(\"r\"):0:1 IDENTIFIER(\"a\"):2:3 IDENTIFIER(\"b\"):4:5 IDENTIFIER(\"c\"):6:7 NEWLINE:7:7 EOF:7:7");
  checkErrors("\xf2\x92\x8d\x{85}", "ILLEGAL(\"\\362\\222\\215\\205\"):0:4 NEWLINE:4:4 EOF:4:4", {"Unexpected character:0"});
}

TEST(LexerTest, SimpleFunctionCall) {
  check("foo(bar, man)",
        "IDENTIFIER(\"foo\"):0:3 LPAREN:3:4 IDENTIFIER(\"bar\"):4:7 COMMA:7:8 IDENTIFIER(\"man\"):9:12 RPAREN:12:13 NEWLINE:13:13 EOF:13:13");
}

TEST(LexerTest, FunctionDefinition) {
  check(R"starlark(
def foo(name):
  bar(
    name = name,
    number = 1,
  )
)starlark",
        "DEF:1:4 IDENTIFIER(\"foo\"):5:8 LPAREN:8:9 IDENTIFIER(\"name\"):9:13 RPAREN:13:14 COLON:14:15 NEWLINE:15:16 "
        "INDENT:16:18 IDENTIFIER(\"bar\"):18:21 LPAREN:21:22 IDENTIFIER(\"name\"):27:31 EQUALS:32:33 IDENTIFIER(\"name\"):34:38 COMMA:38:39 "
        "IDENTIFIER(\"number\"):44:50 EQUALS:51:52 INT(1):53:54 COMMA:54:55 RPAREN:58:59 NEWLINE:59:60 "
        "OUTDENT:60:60 NEWLINE:60:60 EOF:60:60");
}

TEST(LexerTest, Strings) {
  check(R"starlark(
foo = "bar"
)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 STRING(\"bar\"):7:12 NEWLINE:12:13 EOF:13:13");
  check(R"starlark(
foo = "b'a'r"
)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 STRING(\"b'a'r\"):7:14 NEWLINE:14:15 EOF:15:15");
  check(R"starlark(
foo = 'bar'
)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 STRING(\"bar\"):7:12 NEWLINE:12:13 EOF:13:13");
  check(R"starlark(
foo = 'b"a"r'
)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 STRING(\"b\\\"a\\\"r\"):7:14 NEWLINE:14:15 EOF:15:15");
}

TEST(LexerTest, StringTripleQuote) {
  check(R"starlark(
foo = """bar"""
)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 STRING(\"bar\"):7:16 NEWLINE:16:17 EOF:17:17");
  check(R"starlark(
foo = """b"a"r"""
)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 STRING(\"b\\\"a\\\"r\"):7:18 NEWLINE:18:19 EOF:19:19");
  check(R"starlark(
foo = """b'''a'''r"""
)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 STRING(\"b'''a'''r\"):7:22 NEWLINE:22:23 EOF:23:23");
  check(R"starlark(
foo = '''bar'''
)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 STRING(\"bar\"):7:16 NEWLINE:16:17 EOF:17:17");
  check(R"starlark(
foo = '''b'a'r'''
)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 STRING(\"b'a'r\"):7:18 NEWLINE:18:19 EOF:19:19");
  check(R"starlark(
foo = '''b"""a"""r'''
)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 STRING(\"b\\\"\\\"\\\"a\\\"\\\"\\\"r\"):7:22 NEWLINE:22:23 EOF:23:23");
  checkErrors(R"starlark(
foo = """bar
)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 ILLEGAL(\"\\\"\\\"\\\"bar\\n\"):7:14 NEWLINE:14:14 EOF:14:14", { "Unterminated string:14" });
}

TEST(LexerTest, StringsEscapeSequences) {
  check(R"starlark(
foo = "bar\n"
)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 STRING(\"bar\\n\"):7:14 NEWLINE:14:15 EOF:15:15");
  check(R"starlark(
foo = "bar\a\b\f\n\r\t\v"
)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 STRING(\"bar\\a\\b\\f\\n\\r\\t\\v\"):7:26 NEWLINE:26:27 EOF:27:27");
  check(R"starlark(
foo = "bar\
"
)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 STRING(\"bar\"):7:14 NEWLINE:14:15 EOF:15:15");
  check("foo = \"bar\\\r\n\"",
        "IDENTIFIER(\"foo\"):0:3 EQUALS:4:5 STRING(\"bar\"):6:14 NEWLINE:14:14 EOF:14:14");
  checkErrors("foo = \"bar\\\r\"",
      "IDENTIFIER(\"foo\"):0:3 EQUALS:4:5 ILLEGAL(\"\\\"bar\\\\\\r\\\"\"):6:13 NEWLINE:13:13 EOF:13:13",
      { "Invalid line continuation:12" });
  check(R"starlark(
foo = "bar\0"
)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 STRING(\"bar\\000\"):7:14 NEWLINE:14:15 EOF:15:15");
  check(R"starlark(
foo = "bar\1"
)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 STRING(\"bar\\001\"):7:14 NEWLINE:14:15 EOF:15:15");
  check(R"starlark(
foo = "bar\177"
)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 STRING(\"bar\\177\"):7:16 NEWLINE:16:17 EOF:17:17");
  checkErrors(R"starlark(
foo = "bar\377"
)starlark",
      "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 ILLEGAL(\"\\\"bar\\\\377\\\"\"):7:16 NEWLINE:16:17 EOF:17:17",
      { "Invalid escape sequence:11" });
  check(R"starlark(
foo = "bar\1777"
)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 STRING(\"bar\\1777\"):7:17 NEWLINE:17:18 EOF:18:18");
  check(R"starlark(
foo = "bar\x7f"
)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 STRING(\"bar\\177\"):7:16 NEWLINE:16:17 EOF:17:17");
  checkErrors(R"starlark(
foo = "bar\x80"
)starlark",
      "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 ILLEGAL(\"\\\"bar\\\\x80\\\"\"):7:16 NEWLINE:16:17 EOF:17:17",
      { "Invalid escape sequence:11" });
  check(R"starlark(
foo = "bar\u1234"
)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 STRING(\"bar\\341\\210\\264\"):7:18 NEWLINE:18:19 EOF:19:19");
  check(R"starlark(
foo = "bar\U00012345"
)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 STRING(\"bar\\360\\222\\215\\205\"):7:22 NEWLINE:22:23 EOF:23:23");
  checkErrors(R"starlark(
foo = "bar\U00012)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 ILLEGAL(\"\\\"bar\\\\U00012\"):7:18 NEWLINE:18:18 EOF:18:18",
       { "Invalid escape sequence:11", "Unterminated string:18" });
  checkErrors(R"starlark(
foo = "bar\u12")starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 ILLEGAL(\"\\\"bar\\\\u12\\\"\"):7:16 NEWLINE:16:16 EOF:16:16",
       { "Invalid escape sequence:11" });
  checkErrors(R"starlark(
foo = "bar\U00012")starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 ILLEGAL(\"\\\"bar\\\\U00012\\\"\"):7:19 NEWLINE:19:19 EOF:19:19",
       { "Invalid escape sequence:11" });
  checkErrors(R"starlark(
foo = "bar\U00012 ")starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 ILLEGAL(\"\\\"bar\\\\U00012 \\\"\"):7:20 NEWLINE:20:20 EOF:20:20",
       { "Invalid escape sequence:11" });
  checkErrors(R"starlark(
foo = "bar\UFFFFFFFF")starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 ILLEGAL(\"\\\"bar\\\\UFFFFFFFF\\\"\"):7:22 NEWLINE:22:22 EOF:22:22",
       { "Invalid escape sequence:11" });
  checkErrors(R"starlark(
foo = "bar\z")starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 ILLEGAL(\"\\\"bar\\\\z\\\"\"):7:14 NEWLINE:14:14 EOF:14:14",
       { "Invalid escape sequence:11" });
  check(R"starlark(
foo = "bar")starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 STRING(\"bar\"):7:13 NEWLINE:13:13 EOF:13:13");
}

TEST(LexerTest, RawStrings) {
  check(R"starlark(
foo = r"bar\\n"
)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 STRING(\"bar\\\\\\\\n\"):7:16 NEWLINE:16:17 EOF:17:17");
  check(R"starlark(
foo = r"\
")starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 STRING(\"\\\\\\n\"):7:12 NEWLINE:12:12 EOF:12:12");
}

TEST(LexerTest, Bytes) {
  check(R"starlark(
foo = b"bar"
)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 BYTES(\"bar\"):7:13 NEWLINE:13:14 EOF:14:14");
  check(R"starlark(
foo = b"bar\x7f"
)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 BYTES(\"bar\\177\"):7:17 NEWLINE:17:18 EOF:18:18");
  check(R"starlark(
foo = b"bar\x80"
)starlark",
        "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 BYTES(\"bar\\200\"):7:17 NEWLINE:17:18 EOF:18:18");
  checkErrors(R"starlark(
foo = b"bar\u1234"
)starlark",
      "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 ILLEGAL(\"b\\\"bar\\\\u1234\\\"\"):7:19 NEWLINE:19:20 EOF:20:20",
      { "Invalid escape sequence:12" });
  checkErrors(R"starlark(
foo = b"bar\U00012345"
)starlark",
      "IDENTIFIER(\"foo\"):1:4 EQUALS:5:6 ILLEGAL(\"b\\\"bar\\\\U00012345\\\"\"):7:23 NEWLINE:23:24 EOF:24:24",
      { "Invalid escape sequence:12" });
}

TEST(LexerTest, InNotIn) {
  check(R"starlark(
1 in [1, 2, 3]
)starlark",
        "INT(1):1:2 IN:3:5 LBRACKET:6:7 INT(1):7:8 COMMA:8:9 INT(2):10:11 COMMA:11:12 INT(3):13:14 RBRACKET:14:15 NEWLINE:15:16 EOF:16:16");
  check(R"starlark(
4 not in (1, 2, 3)
)starlark",
        "INT(4):1:2 NOT:3:6 IN:7:9 LPAREN:10:11 INT(1):11:12 COMMA:12:13 INT(2):14:15 COMMA:15:16 INT(3):17:18 RPAREN:18:19 NEWLINE:19:20 EOF:20:20");
}

}  // namespace

