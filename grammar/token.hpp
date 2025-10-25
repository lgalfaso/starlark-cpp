// Copyright 2024-2025 Lucas Mirelmann

#ifndef GRAMMAR_TOKEN_HPP_
#define GRAMMAR_TOKEN_HPP_

#include <string>
#include <string_view>
#include <variant>

#include "bigint/number.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace grammar {

enum class token_type {
  kBof,
  kEof,

  kAmpersand,
  kAmpersandEquals,
  kAnd,
  kAs,
  kAssert,
  kAsync,
  kAwait,
  kBreak,
  kBytes,
  kCaret,
  kCaretEquals,
  kClass,
  kColon,
  kComma,
  kContinue,
  kDef,
  kDel,
  kDot,
  kElif,
  kElse,
  kEquals,
  kEqualsEquals,
  kExcept,
  kFinally,
  kFloat,
  kFor,
  kFrom,
  kGlobal,
  kGreater,
  kGreaterEquals,
  kGreaterGreater,
  kGreaterGreaterEquals,
  kIdentifier,
  kIf,
  kIllegal,
  kImport,
  kIn,
  kIndent,
  kInt,
  kIs,
  kLambda,
  kLBrace,
  kLBracket,
  kLess,
  kLessEquals,
  kLessLess,
  kLessLessEquals,
  kLoad,
  kLParen,
  kMinus,
  kMinusEquals,
  kNewline,
  kNonlocal,
  kNot,
  kNotEquals,
  kOr,
  kOutdent,
  kPass,
  kPercent,
  kPercentEquals,
  kPipe,
  kPipeEquals,
  kPlus,
  kPlusEquals,
  kRaise,
  kRBrace,
  kRBracket,
  kReturn,
  kRParen,
  kSemi,
  kSlash,
  kSlashEquals,
  kSlashSlash,
  kSlashSlashEquals,
  kStar,
  kStarEquals,
  kStarStar,
  kString,
  kTilde,
  kTry,
  kWhile,
  kWith,
  kYield,
};

struct position {
  std::size_t row;
  std::size_t column;
  std::size_t pos;
};

position operator-(const position& pos, std::size_t places);

class token {
 public:
  token(token_type type, position start, position end);
  token(token_type type, position start, position end, const bigint::number& value);
  token(token_type type, position start, position end, double value);
  token(token_type type, position start, position end, const std::string& value);
  token_type type() const;
  void set_type(token_type new_type);
  const bigint::number& int_value() const;
  double double_value() const;
  const std::string& string_value() const;
  position start() const;
  position end() const;

 private:
  token_type tok_type;
  position tok_start;
  position tok_end;
  std::variant<double, bigint::number, std::string> value;
};

}  // namespace grammar
}  // namespace starlark

#pragma GCC visibility pop

#endif  // GRAMMAR_TOKEN_HPP_

