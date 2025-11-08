// Copyright 2024-2025 Lucas Mirelmann

#ifndef GRAMMAR_TOKEN_HPP_
#define GRAMMAR_TOKEN_HPP_

#include <string>
#include <string_view>
#include <variant>

#include "bigint/number.hpp"
#include "proto/starlark_logging.pb.h"

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
  kBigInt,
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

starlark::logging::Position operator-(const starlark::logging::Position& pos, std::size_t places);

class token {
 public:
  token(token_type type, starlark::logging::Position start, starlark::logging::Position end);
  token(token_type type, starlark::logging::Position start, starlark::logging::Position end, std::int64_t value);
  token(token_type type, starlark::logging::Position start, starlark::logging::Position end, const bigint::number& value);
  token(token_type type, starlark::logging::Position start, starlark::logging::Position end, double value);
  token(token_type type, starlark::logging::Position start, starlark::logging::Position end, const std::string& value);
  token_type type() const;
  void set_type(token_type new_type);
  std::int64_t int_value() const;
  const bigint::number& big_int_value() const;
  double double_value() const;
  const std::string& string_value() const;
  starlark::logging::Position start() const;
  starlark::logging::Position end() const;

 private:
  token_type tok_type;
  starlark::logging::Position tok_start;
  starlark::logging::Position tok_end;
  std::variant<double, std::int64_t, bigint::number, std::string> value;
};

}  // namespace grammar
}  // namespace starlark

#pragma GCC visibility pop

#endif  // GRAMMAR_TOKEN_HPP_

