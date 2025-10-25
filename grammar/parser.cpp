// Copyright 2024-2025 Lucas Mirelmann

#include "grammar/parser.hpp"

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "unicode/normalization.hpp"
#include "third-party/defer.hpp"

using google::protobuf::Arena;
using google::protobuf::RepeatedPtrField;
using starlark::ast::Argument;
using starlark::ast::AssignStmt;
using starlark::ast::BinaryExpr;
using starlark::ast::BinaryExpr;
using starlark::ast::CompClause;
using starlark::ast::DefStmt;
using starlark::ast::Entry;
using starlark::ast::Expression;
using starlark::ast::File;
using starlark::ast::ForStmt;
using starlark::ast::Identifier;
using starlark::ast::IfStmt;
using starlark::ast::LambdaExpr;
using starlark::ast::Parameter;
using starlark::ast::Statement;
using starlark::ast::UnaryExpr;
using starlark::logging::LogLevel;
using starlark::logging::Position;
using starlark::logging::logger;
using starlark::unicode::to_nfkc;

namespace starlark {
namespace grammar {

namespace {

const std::map<token_type, std::pair<int, BinaryExpr::BinaryOperator>> operator_precedence = {
  {token_type::kOr, {1, BinaryExpr::OR}},
  {token_type::kAnd, {2, BinaryExpr::AND}},
  {token_type::kNot, {3, BinaryExpr::UNKNOWN}},  // As a prefix.
  {token_type::kEqualsEquals, {4, BinaryExpr::EQUALS_EQUALS}},
  {token_type::kNotEquals, {4, BinaryExpr::BANG_EQUALS}},
  {token_type::kLess, {4, BinaryExpr::LESS_THAN}},
  {token_type::kGreater, {4, BinaryExpr::GREATER_THAN}},
  {token_type::kLessEquals, {4, BinaryExpr::LESS_THAN_EQUALS}},
  {token_type::kGreaterEquals, {4, BinaryExpr::GREATER_THAN_EQUALS}},
  {token_type::kIn, {4, BinaryExpr::IN}},
  {token_type::kPipe, {5, BinaryExpr::PIPE}},
  {token_type::kCaret, {6, BinaryExpr::HAT}},
  {token_type::kAmpersand, {7, BinaryExpr::AMPERSAND}},
  {token_type::kLessLess, {8, BinaryExpr::LESS_THAN_LESS_THAN}},
  {token_type::kGreaterGreater, {8, BinaryExpr::GREATER_THAN_GREATER_THAN}},
  {token_type::kMinus, {9, BinaryExpr::MINUS}},
  {token_type::kPlus, {9, BinaryExpr::PLUS}},
  {token_type::kStar, {10, BinaryExpr::STAR}},
  {token_type::kPercent, {10, BinaryExpr::PERCENT}},
  {token_type::kSlash, {10, BinaryExpr::SLASH}},
  {token_type::kSlashSlash, {10, BinaryExpr::SLASH_SLASH}},
};

constexpr int MAX_PRECEDENCE = 11;

const std::map<token_type, AssignStmt::AssignOperator> assign_ops = {
  {token_type::kEquals, AssignStmt::EQUALS},
  {token_type::kPlusEquals, AssignStmt::PLUS_EQUALS},
  {token_type::kMinusEquals, AssignStmt::MINUS_EQUALS},
  {token_type::kStarEquals, AssignStmt::STAR_EQUALS},
  {token_type::kSlashEquals, AssignStmt::SLASH_EQUALS},
  {token_type::kSlashSlashEquals, AssignStmt::SLASH_SLASH_EQUALS},
  {token_type::kPercentEquals, AssignStmt::PERCENT_EQUALS},
  {token_type::kAmpersandEquals, AssignStmt::AMPERSAND_EQUALS},
  {token_type::kPipeEquals, AssignStmt::PIPE_EQUALS},
  {token_type::kCaretEquals, AssignStmt::HAT_EQUALS},
  {token_type::kLessLessEquals, AssignStmt::LESS_LESS_EQUALS},
  {token_type::kGreaterGreaterEquals, AssignStmt::GREATER_GREATER_EQUALS},
};

bool is_target(const Expression* base) {
  std::vector<const Expression*> frames{base};
  do {
    auto top = frames.back();
    frames.pop_back();
    if (top != nullptr) {
      switch (top->expression_type_case()) {
        case Expression::kDotExpression:
        case Expression::kSliceExpression:
        case Expression::kIdentifier:
          break;
        case Expression::kCallExpression:
        case Expression::kIntValue:
        case Expression::kFloatValue:
        case Expression::kStringValue:
        case Expression::kBytesValue:
        case Expression::kListComprehension:
        case Expression::kDictionaryExpression:
        case Expression::kDictionaryComprehension:
        case Expression::kIfExpression:
        case Expression::kUnaryExpression:
        case Expression::kBinaryExpression:
        case Expression::kLambdaExpression:
        case Expression::EXPRESSION_TYPE_NOT_SET:
          return false;
        case Expression::kListExpression:
          for (const auto& item : top->list_expression().element()) {
            frames.push_back(&item);
          }
          break;
        case Expression::kTuple:
          for (const auto& element : top->tuple().value()) {
            frames.push_back(&element);
          }
          break;
      }
    }
  } while (!frames.empty());
  return true;
}

enum class parser_state {
  kParseStatement,
  kParseStatementDef_0,
  kParseStatementDefFinal,
  kParseStatementIf_0,
  kParseStatementIfElif,
  kParseStatementIfElse,
  kParseStatementFor_0,
  kParseStatementFor_1,
  kParseStatementFor_2,
  kParseStatementForFinal,
  kParseStatementExpression_0,
  kParseSuite,
  kParseSuiteStatementList,
  kParseSimpleStatement,
  kParseSimpleStatement_0,
  kParseSimpleStatementFinal,
  kParseSmallStatement,
  kParseParameters,
  kParseExpression,
  kParseExpression_0,
  kParseExpression_1,
  kParseTest,
  kParseTest_0,
  kParseTest_1,
  kParseTestP,
  kParseTestP_0,
  kParseLambda,
  kParseLambda_0,
  kParseLambdaFinal,
  kParsePrimary,
  kParsePrimary_0,
  kParsePrimaryCall_0,
  kParsePrimaryIndex_0,
  kParsePrimaryIndex_1,
  kParsePrimaryIndex_2,
  kParsePrimaryIndexFinal,
  kParseOperand,
  kParseOperandExpression_0,
  kParseList,
  kParseList_0,
  kParseListIndex_0,
  kParseListComprehensionFinal,
  kParseListFinal,
  kParseDict,
  kParseDict_0,
  kParseDictIndex_0,
  kParseDictComprehensionFinal,
  kParseDictFinal,
  kParseEntry,
  kParseEntry_0,
  kParseCompClauses,
  kParseCompClauses_0,
  kParseArgument,
  kParseArgument_0,
  kParseResolveTest,
  kParseResolveExpression,
};

struct frame {
  parser_state state;
  union {
    RepeatedPtrField<Statement>* statements;
    Statement* statement;
    DefStmt* def_statement;
    IfStmt* if_statement;
    ForStmt* for_statement;
    struct {
      Expression* for_loop_variables;
      bool for_loop_variables_first;
    };
    LambdaExpr* lambda;
    struct {
      Argument* argument;
      Argument* previous_argument;
    };
    Entry* entry;
    struct {
      RepeatedPtrField<CompClause>* comp_clauses;
      bool first_comp_clause;
    };
    struct {
      CompClause* comp_clause;
      Expression* comp_clause_primary;
      bool comp_clause_primary_first_expression_part;
      bool comp_clause_resolve_in_test;
    };
    struct {
      RepeatedPtrField<Parameter>* parameters;
      bool parse_parameters_allow_trailing_comma;
      bool parse_parameters_first;
      bool found_star_parameter;
      bool found_star_star_parameter;
      bool previous_parameter_was_bare_star;
    };
    struct {
      Expression* expression;
      bool expression_allow_trailing_comma;
    };
    struct {
      Expression* test;
      int test_p_precedence;
      bool test_p_0_first;
    };
    struct {
      Expression* primary = nullptr;
      Argument* previous_call_argument = nullptr;
      bool primary_must_be_target = false;
    };
  };
};

const std::set<std::string> predeclared_symbols = {
    "None",     "True",      "False",    "abs",     "any",       "all",
    "bool",     "bytes",     "dict",     "dir",     "enumerate", "float",
    "fail",     "getattr",   "hasattr",  "hash",    "int",       "len",
    "list",     "max",       "min",      "print",   "range",     "repr",
    "reversed", "set",       "sorted",   "str",     "tuple",     "type",
    "zip",
};

}  // namespace

parser::parser(std::string_view input, logger& logging) : parser(input, options{}, {}, logging) {
}

parser::parser(std::string_view input, const options& opts, const std::set<std::string, std::less<>>& bindings, logger& logging)
    : opts(opts), lex(input, opts, logging), logging(logging), base_bindings(bindings), nested_loops(1) {
  base_bindings.insert(predeclared_symbols.begin(), predeclared_symbols.end());
  lex.next_token();
}

File* parser::parse_file(Arena& arena) {
  File* result = Arena::Create<File>(&arena);

  // Predeclared block.
  create_block(base_bindings, {}, nullptr);
  // Module block.
  create_block({}, {}, result->mutable_module_binding());
  // File block.
  create_block({}, {}, result->mutable_file_binding());

  while (lex.current_token().type() != token_type::kEof) {
    if (lex.current_token().type() == token_type::kNewline) {
      lex.next_token();
    } else {
      parse_statement(*result->mutable_statement());
    }
  }
  assert(parse_parameter_identifiers.empty());
  assert(nested_loops.size() == 1 && nested_loops[0] == 0);
  assert(is_top_level_block());

  // Force the resolution of the predefined, module and file blocks.
  while (!parser_blocks.empty()) {
    drop_block();
  }
  identifier_positions.clear();
  return result;
}

bool parser::capture(token_type expected_token) {
  if (!is_current(expected_token)) {
    return false;
  }
  lex.next_token();
  return true;
}

bool parser::is_current(token_type expected_token) const {
  return lex.current_token().type() == expected_token;
}

bool parser::expect(token_type expected_token) {
  if (capture(expected_token)) {
    return true;
  }
  switch (expected_token) {
    case token_type::kAmpersand:              add_error("Expected AMPERSAND");              break;
    case token_type::kAmpersandEquals:        add_error("Expected AMPERSAND_EQUALS");       break;
    case token_type::kAnd:                    add_error("Expected AND");                    break;
    case token_type::kAs:                     add_error("Expected AS");                     break;
    case token_type::kAssert:                 add_error("Expected ASSERT");                 break;
    case token_type::kAsync:                  add_error("Expected ASYNC");                  break;
    case token_type::kAwait:                  add_error("Expected AWAIT");                  break;
    case token_type::kBof:                    add_error("Expected BOF");                    break;
    case token_type::kBreak:                  add_error("Expected BREAK");                  break;
    case token_type::kBytes:                  add_error("Expected BYTES");                  break;
    case token_type::kCaret:                  add_error("Expected CARET");                  break;
    case token_type::kCaretEquals:            add_error("Expected CARET_EQUALS");           break;
    case token_type::kClass:                  add_error("Expected CLASS");                  break;
    case token_type::kColon:                  add_error("Expected COLON");                  break;
    case token_type::kComma:                  add_error("Expected COMMA");                  break;
    case token_type::kContinue:               add_error("Expected CONTINUE");               break;
    case token_type::kDef:                    add_error("Expected DEF");                    break;
    case token_type::kDel:                    add_error("Expected DEL");                    break;
    case token_type::kDot:                    add_error("Expected DOT");                    break;
    case token_type::kElif:                   add_error("Expected ELIF");                   break;
    case token_type::kElse:                   add_error("Expected ELSE");                   break;
    case token_type::kEof:                    add_error("Expected EOF");                    break;
    case token_type::kEquals:                 add_error("Expected EQUALS");                 break;
    case token_type::kEqualsEquals:           add_error("Expected EQUALS_EQUALS");          break;
    case token_type::kExcept:                 add_error("Expected EXCEPT");                 break;
    case token_type::kFinally:                add_error("Expected FINALLY");                break;
    case token_type::kFloat:                  add_error("Expected FLOAT");                  break;
    case token_type::kFor:                    add_error("Expected FOR");                    break;
    case token_type::kFrom:                   add_error("Expected FROM");                   break;
    case token_type::kGlobal:                 add_error("Expected GLOBAL");                 break;
    case token_type::kGreater:                add_error("Expected GREATER");                break;
    case token_type::kGreaterEquals:          add_error("Expected GREATER_EQUALS");         break;
    case token_type::kGreaterGreater:         add_error("Expected GREATER_GREATER");        break;
    case token_type::kGreaterGreaterEquals:   add_error("Expected GREATER_GREATER_EQUALS"); break;
    case token_type::kIdentifier:             add_error("Expected IDENTIFIER");             break;
    case token_type::kIf:                     add_error("Expected IF");                     break;
    case token_type::kIllegal:                add_error("Expected ILLEGAL");                break;
    case token_type::kImport:                 add_error("Expected IMPORT");                 break;
    case token_type::kIn:                     add_error("Expected IN");                     break;
    case token_type::kIndent:                 add_error("Expected INDENT");                 break;
    case token_type::kInt:                    add_error("Expected INT");                    break;
    case token_type::kIs:                     add_error("Expected IS");                     break;
    case token_type::kLambda:                 add_error("Expected LAMBDA");                 break;
    case token_type::kLBrace:                 add_error("Expected LBRACE");                 break;
    case token_type::kLBracket:               add_error("Expected LBRACKET");               break;
    case token_type::kLess:                   add_error("Expected LESS");                   break;
    case token_type::kLessEquals:             add_error("Expected LESS_EQUALS");            break;
    case token_type::kLessLess:               add_error("Expected LESS_LESS");              break;
    case token_type::kLessLessEquals:         add_error("Expected LESS_LESS_EQUALS");       break;
    case token_type::kLoad:                   add_error("Expected LOAD");                   break;
    case token_type::kLParen:                 add_error("Expected LPAREN");                 break;
    case token_type::kMinus:                  add_error("Expected MINUS");                  break;
    case token_type::kMinusEquals:            add_error("Expected MINUS_EQUALS");           break;
    case token_type::kNewline:                add_error("Expected NEWLINE");                break;
    case token_type::kNonlocal:               add_error("Expected NONLOCAL");               break;
    case token_type::kNot:                    add_error("Expected NOT");                    break;
    case token_type::kNotEquals:              add_error("Expected NOT_EQUALS");             break;
    case token_type::kOr:                     add_error("Expected OR");                     break;
    case token_type::kOutdent:                add_error("Expected OUTDENT");                break;
    case token_type::kPass:                   add_error("Expected PASS");                   break;
    case token_type::kPercent:                add_error("Expected PERCENT");                break;
    case token_type::kPercentEquals:          add_error("Expected PERCENT_EQUALS");         break;
    case token_type::kPipe:                   add_error("Expected PIPE");                   break;
    case token_type::kPipeEquals:             add_error("Expected PIPE_EQUALS");            break;
    case token_type::kPlus:                   add_error("Expected PLUS");                   break;
    case token_type::kPlusEquals:             add_error("Expected PLUS_EQUALS");            break;
    case token_type::kRaise:                  add_error("Expected RAISE");                  break;
    case token_type::kRBrace:                 add_error("Expected RBRACE");                 break;
    case token_type::kRBracket:               add_error("Expected RBRACKET");               break;
    case token_type::kReturn:                 add_error("Expected RETURN");                 break;
    case token_type::kRParen:                 add_error("Expected RPAREN");                 break;
    case token_type::kSemi:                   add_error("Expected SEMI");                   break;
    case token_type::kSlash:                  add_error("Expected SLASH");                  break;
    case token_type::kSlashEquals:            add_error("Expected SLASH_EQUALS");           break;
    case token_type::kSlashSlash:             add_error("Expected SLASH_SLASH");            break;
    case token_type::kSlashSlashEquals:       add_error("Expected SLASH_SLASH_EQUALS");     break;
    case token_type::kStar:                   add_error("Expected STAR");                   break;
    case token_type::kStarEquals:             add_error("Expected STAR_EQUALS");            break;
    case token_type::kStarStar:               add_error("Expected STAR_STAR");              break;
    case token_type::kString:                 add_error("Expected STRING");                 break;
    case token_type::kTilde:                  add_error("Expected TILDE");                  break;
    case token_type::kTry:                    add_error("Expected TRY");                    break;
    case token_type::kWhile:                  add_error("Expected WHILE");                  break;
    case token_type::kWith:                   add_error("Expected WITH");                   break;
    case token_type::kYield:                  add_error("Expected YIELD");                  break;
  }
  return false;
}

void parser::add_error(std::string_view message) {
  add_error(message, lex.current_token().start());
}

void parser::add_error(std::string_view message, const Position& pos) {
  logging.log(LogLevel::LOG_LEVEL_ERROR, message, module, pos);
  recover = true;
}

void parser::add_warning(std::string_view message) {
  logging.log(LogLevel::LOG_LEVEL_WARNING, message, module, lex.current_token().start());
  recover = true;
}

void parser::parse_statement(RepeatedPtrField<Statement>& statements) {
  std::vector<frame> frames;
  frames.emplace_back(frame{
      .state = parser_state::kParseStatement,
      .statements = &statements,
  });

  do {
    const frame top = frames.back();
    frames.pop_back();
    switch (top.state) {
      case parser_state::kParseStatement:
        if (capture(token_type::kDef)) {
          found_non_load = true;
          if (!opts.allow_function_definitions) {
            add_error("Function definitions not allowed");
          }
          DefStmt* def_statement = top.statements->Add()->mutable_def_statement();
          if (!set_identifier(*def_statement->mutable_function_name())) {
            add_error("Expected an identifier");
            break;
          }

          // If this is a top-level function definition, then check whether this is causing a redefinition
          // with a previous `load` statement.
          if (is_top_level_block()) {
            if (parser_blocks.back().identifiers.contains(def_statement->function_name().nfkc_name())) {
              add_error(std::format("`def` statement redefines previously defined `load` symbol '{}'", def_statement->function_name().name()));
            }
          }
          bind(def_statement->function_name());
          resolve(def_statement->mutable_function_name(), 0);

          if (!expect(token_type::kLParen)) {
            break;
          }
          nested_loops.push_back(0);
          frames.emplace_back(frame{
            .state = parser_state::kParseStatementDefFinal,
            .def_statement = def_statement,
          });
          frames.emplace_back(frame{
            .state = parser_state::kParseStatementDef_0,
            .def_statement = def_statement,
          });
          frames.emplace_back(frame{
            .state = parser_state::kParseParameters,
            .parameters = def_statement->mutable_parameter(),
            .parse_parameters_allow_trailing_comma = true,
            .parse_parameters_first = true,
            .found_star_parameter = false,
            .found_star_star_parameter = false,
            .previous_parameter_was_bare_star = false,
          });
        } else if (capture(token_type::kIf)) {
          found_non_load = true;
          if (nested_loops.size() == 1) {
            add_error("`if` statements are not allowed at the top level");
          }
          IfStmt* if_statement = top.statements->Add()->mutable_if_statement();
          // It is unclear whether the attempt to parse the `elif` and `else` blocks should be
          // defined here or in parse_statement_if_0. This difference is important when there
          // are errors in the parsing and how should we attempt to recover from these errors.
          frames.emplace_back(frame{
            .state = parser_state::kParseStatementIfElse,
            .if_statement = if_statement,
          });
          frames.emplace_back(frame{
            .state = parser_state::kParseStatementIfElif,
            .if_statement = if_statement,
          });
          frames.emplace_back(frame{
            .state = parser_state::kParseStatementIf_0,
            .statements = if_statement->mutable_statement(),
          });
          frames.emplace_back(frame{
            .state = parser_state::kParseResolveTest,
            .test = if_statement->mutable_test(),
          });
          frames.emplace_back(frame{
            .state = parser_state::kParseTest,
            .test = if_statement->mutable_test(),
          });
        } else if (capture(token_type::kFor)) {
          found_non_load = true;
          if (nested_loops.size() == 1) {
            add_error("`for` statements are not allowed at the top level");
          }
          ForStmt* for_statement = top.statements->Add()->mutable_for_statement();
          Expression* loop_variables = for_statement->mutable_loop_variables();
          nested_loops.back()++;
          frames.emplace_back(frame{
            .state = parser_state::kParseStatementForFinal,
          });
          frames.emplace_back(frame{
            .state = parser_state::kParseStatementFor_2,
            .statements = for_statement->mutable_statement(),
          });
          frames.emplace_back(frame{
            .state = parser_state::kParseStatementFor_1,
            .for_statement = for_statement,
          });
          frames.emplace_back(frame{
            .state = parser_state::kParseStatementFor_0,
            .for_loop_variables = loop_variables,
            .for_loop_variables_first = true,
          });
          frames.emplace_back(frame{
            .state = parser_state::kParsePrimary,
            .primary = loop_variables,
            .primary_must_be_target = true,
          });
        } else {
          frames.emplace_back(frame{
            .state = parser_state::kParseSimpleStatement,
            .statements = top.statements,
          });
        }
        break;
      case parser_state::kParseStatementDef_0:
        create_block(parse_parameter_identifiers.back().first, parse_parameter_identifiers.back().second, top.def_statement->mutable_function_binding());
        parse_parameter_identifiers.pop_back();
        if (!expect(token_type::kRParen)) {
          break;
        }
        if (!expect(token_type::kColon)) {
          break;
        }
        frames.emplace_back(frame{
          .state = parser_state::kParseSuite,
          .statements = top.def_statement->mutable_statement(),
        });
        break;
      case parser_state::kParseStatementDefFinal:
        drop_block();
        nested_loops.pop_back();
        break;
      case parser_state::kParseStatementIf_0:
        if (!expect(token_type::kColon)) {
          break;
        }
        frames.emplace_back(frame{
          .state = parser_state::kParseSuite,
          .statements = top.statements,
        });
        break;
      case parser_state::kParseStatementIfElif:
        if (capture(token_type::kElif)) {
          auto* elif = top.if_statement->add_elif();
          frames.emplace_back(top);
          frames.emplace_back(frame{
            .state = parser_state::kParseStatementIf_0,
            .statements = elif->mutable_statement(),
          });
          frames.emplace_back(frame{
            .state = parser_state::kParseResolveTest,
            .test = elif->mutable_test(),
          });
          frames.emplace_back(frame{
            .state = parser_state::kParseTest,
            .test = elif->mutable_test(),
          });
        }
        break;
      case parser_state::kParseStatementIfElse:
        if (capture(token_type::kElse)) {
          frames.emplace_back(frame{
            .state = parser_state::kParseStatementIf_0,
            .statements = top.if_statement->mutable_else_statement(),
          });
        }
        break;
      case parser_state::kParseStatementFor_0:
        if (capture(token_type::kComma)) {
          // If there are multiple loop variables, transform it into a tuple.
          if (top.for_loop_variables_first) {
            Expression* first_test = Arena::Create<Expression>(top.for_loop_variables->GetArena());
            first_test->Swap(top.for_loop_variables);
            first_test->Swap(top.for_loop_variables->mutable_tuple()->add_value());
          }
          Expression* loop_variable = top.for_loop_variables->mutable_tuple()->add_value();
          frames.emplace_back(frame{
            .state = parser_state::kParseStatementFor_0,
            .for_loop_variables = top.for_loop_variables,
            .for_loop_variables_first = false,
          });
          frames.emplace_back(frame{
            .state = parser_state::kParsePrimary,
            .primary = loop_variable,
            .primary_must_be_target = true,
          });
        } else {
          bind_and_resolve(top.for_loop_variables);
        }
        break;
      case parser_state::kParseStatementFor_1:
        if (!expect(token_type::kIn)) {
          break;
        }
        frames.emplace_back(frame{
          .state = parser_state::kParseResolveExpression,
          .expression = top.for_statement->mutable_expression(),
        });
        frames.emplace_back(frame{
          .state = parser_state::kParseExpression,
          .expression = top.for_statement->mutable_expression(),
          .expression_allow_trailing_comma = false,
        });
        break;
      case parser_state::kParseStatementFor_2:
        if (!expect(token_type::kColon)) {
          break;
        }
        frames.emplace_back(frame{
          .state = parser_state::kParseSuite,
          .statements = top.statements,
        });
        break;
      case parser_state::kParseStatementForFinal:
        nested_loops.back()--;
        break;
      case parser_state::kParseSuite:
        if (capture(token_type::kNewline)) {
          if (!expect(token_type::kIndent)) {
            break;
          }
          frames.emplace_back(frame{
            .state = parser_state::kParseSuiteStatementList,
            .statements = top.statements,
          });
        } else {
          frames.emplace_back(frame{
            .state = parser_state::kParseSimpleStatement,
            .statements = top.statements,
          });
        }
        break;
      case parser_state::kParseSuiteStatementList:
        if (lex.current_token().type() != token_type::kOutdent && lex.current_token().type() != token_type::kEof) {
          frames.emplace_back(top);
          frames.emplace_back(frame{
            .state = parser_state::kParseStatement,
            .statements = top.statements,
          });
          break;
        }
        expect(token_type::kOutdent);
        break;
      case parser_state::kParseSimpleStatement:
        frames.emplace_back(frame{
          .state = parser_state::kParseSimpleStatementFinal,
        });
        frames.emplace_back(frame{
          .state = parser_state::kParseSimpleStatement_0,
          .statements = top.statements,
        });
        frames.emplace_back(frame{
          .state = parser_state::kParseSmallStatement,
          .statement = top.statements->Add(),
        });
        break;
      case parser_state::kParseSimpleStatement_0:
        if (capture(token_type::kSemi)) {
          if (is_current(token_type::kNewline)) {
            break;
          }
          frames.emplace_back(top);
          frames.emplace_back(frame{
            .state = parser_state::kParseSmallStatement,
             .statement = top.statements->Add(),
          });
        }
        break;
      case parser_state::kParseSimpleStatementFinal:
        if (recover) {
          while (lex.current_token().type() != token_type::kNewline && lex.current_token().type() != token_type::kEof) {
            lex.next_token();
          }
        }
        if (!expect(token_type::kNewline)) {
          break;
        }
        recover = false;
        break;
      case parser_state::kParseSmallStatement:
        switch (lex.current_token().type()) {
          case token_type::kReturn:
            found_non_load = true;
            if (nested_loops.size() == 1) {
              add_error("Unexpected RETURN");
            }
            top.statement->mutable_return_statement();
            lex.next_token();
            if (!is_current(token_type::kNewline)) {
              frames.emplace_back(frame{
                .state = parser_state::kParseResolveExpression,
                .expression = top.statement->mutable_return_statement()->mutable_expression(),
              });
              frames.emplace_back(frame{
                .state = parser_state::kParseExpression,
                .expression = top.statement->mutable_return_statement()->mutable_expression(),
                .expression_allow_trailing_comma = false,
              });
            }
            break;
          case token_type::kLoad: {
            std::set<std::string, std::less<>> symbols;
            if (found_non_load && opts.require_load_statements_first) {
              add_error("`load` statements must appear before other statements");
            }
            if (nested_loops.size() != 1) {
              add_error("`load` statement not at top level");
            }
            lex.next_token();
            if (!expect(token_type::kLParen)) {
              break;
            }
            if (!is_current(token_type::kString)) {
              add_error("Expected STRING");
              break;
            }
            top.statement->mutable_load_statement()->set_module(lex.current_token().string_value());
            lex.next_token();
            bool loaded_symbol = false;
            while (capture(token_type::kComma)) {
              if (is_current(token_type::kRParen)) {
                break;
              }
              loaded_symbol = true;
              auto* load_param = top.statement->mutable_load_statement()->add_load_param();
              if (is_current(token_type::kIdentifier)) {
                set_identifier(*load_param->mutable_local_name());
                if (!expect(token_type::kEquals)) {
                  break;
                }
              }
              if (!is_current(token_type::kString)) {
                add_error("Expected STRING");
                break;
              }
              if (!opts.allow_load_private_symbols &&
                  lex.current_token().string_value().starts_with("_")) {
                add_error(std::string {"Cannot import private symbol '"} + lex.current_token().string_value() + "'");
              }
              load_param->set_remote_name(lex.current_token().string_value());
              if (!load_param->has_local_name()) {
                load_param->mutable_local_name()->set_name(load_param->remote_name());
                load_param->mutable_local_name()->set_nfkc_name(to_nfkc(load_param->remote_name()));
              }
              resolve(load_param->mutable_local_name(), 0);
              if (!symbols.emplace(load_param->local_name().nfkc_name()).second) {
                add_error(std::format("`load` statement defines '{}' more than once", load_param->local_name().name()));
              }
              if (parser_blocks.back().identifiers.contains(load_param->local_name().nfkc_name()) && !opts.allow_top_level_rebinding) {
                add_error(std::format("`load` statement redefines previously defined value '{}'", load_param->local_name().name()));
              }
              // The spec does not specify whether it is an error to bind to the file block multiple times.
              // We are taking the possition that if `allow_top_level_rebinding` is `false`, then this is not allowed.
              if (!parser_blocks.back().identifiers.emplace(load_param->local_name().nfkc_name()).second && !opts.allow_top_level_rebinding) {
                add_error(std::format("Multiple bindings for the top-level load symbol '{}'", load_param->local_name().name()));
              }
              lex.next_token();
            }
            if (!loaded_symbol) {
              add_error("Expect to load at least one symbol");
            }
            expect(token_type::kRParen);
            break;
          }
          case token_type::kBreak:
            found_non_load = true;
            if (nested_loops.back() == 0) {
              add_error("Unexpected BREAK");
            }
            top.statement->mutable_break_statement();
            lex.next_token();
            break;
          case token_type::kContinue:
            found_non_load = true;
            if (nested_loops.back() == 0) {
              add_error("Unexpected CONTINUE");
            }
            top.statement->mutable_continue_statement();
            lex.next_token();
            break;
          case token_type::kPass:
            found_non_load = true;
            top.statement->mutable_pass_statement();
            lex.next_token();
            break;
          default: {
            found_non_load |= lex.current_token().type() != token_type::kString;
            frames.emplace_back(frame{
              .state = parser_state::kParseStatementExpression_0,
              .statement = top.statement,
            });
            frames.emplace_back(frame{
              .state = parser_state::kParseExpression,
              .expression = top.statement->mutable_expression_statement(),
              .expression_allow_trailing_comma = false,
            });
            break;
          }
        }
        break;
      case parser_state::kParseStatementExpression_0:
        found_non_load |= !top.statement->expression_statement().has_string_value();
        if (auto op = assign_ops.find(lex.current_token().type()); op != assign_ops.end()) {
          found_non_load = true;
          if (!is_target(&top.statement->expression_statement())) {
            // Report the error and continue to parse this as an expression
            add_error("Exprecting TARGET");
          }
          if (op->first != token_type::kEquals && (
              top.statement->expression_statement().has_tuple() ||
              top.statement->expression_statement().has_list_expression() ||
              top.statement->expression_statement().has_list_comprehension() ||
              top.statement->expression_statement().has_dictionary_expression() ||
              top.statement->expression_statement().has_dictionary_comprehension())) {
            add_error("target is an illegal expression for augmented assignment");
          }
          {
            AssignStmt* assign_statement = Arena::Create<AssignStmt>(top.statement->GetArena());
            assign_statement->mutable_lhs()->Swap(top.statement->mutable_expression_statement());
            assign_statement->Swap(top.statement->mutable_assign_statement());
          }
          // Assignements and augmented assignment are considered a binding.
          bind_and_resolve(top.statement->mutable_assign_statement()->mutable_lhs());
          top.statement->mutable_assign_statement()->set_op(op->second);
          lex.next_token();
          frames.emplace_back(frame{
            .state = parser_state::kParseResolveExpression,
            .expression = top.statement->mutable_assign_statement()->mutable_rhs(),
          });
          frames.emplace_back(frame{
            .state = parser_state::kParseExpression,
            .expression = top.statement->mutable_assign_statement()->mutable_rhs(),
            .expression_allow_trailing_comma = false,
          });
        } else {
          resolve(top.statement->mutable_expression_statement(), 0);
        }
        break;
      case parser_state::kParseExpression:
        frames.emplace_back(frame{
          .state = parser_state::kParseExpression_0,
          .expression = top.expression,
          .expression_allow_trailing_comma = top.expression_allow_trailing_comma,
        });
        frames.emplace_back(frame{
          .state = parser_state::kParseTest,
          .test = top.expression,
        });
        break;
      case parser_state::kParseExpression_0:
        if (is_current(token_type::kComma)) {
          {
            Expression* first_test = Arena::Create<Expression>(top.expression->GetArena());
            first_test->Swap(top.expression);
            first_test->Swap(top.expression->mutable_tuple()->add_value());
          }
          frames.emplace_back(frame{
            .state = parser_state::kParseExpression_1,
            .expression = top.expression,
            .expression_allow_trailing_comma = top.expression_allow_trailing_comma,
          });
        }
        break;
      case parser_state::kParseExpression_1:
        if (capture(token_type::kComma)) {
          if (is_current(token_type::kNewline) ||
              is_current(token_type::kEquals) ||
              is_current(token_type::kRBrace) ||
              is_current(token_type::kRBracket) ||
              is_current(token_type::kRParen) ||
              is_current(token_type::kSemi)) {
            if (!top.expression_allow_trailing_comma) {
              add_error("Unexpected COMMA");
            }
            break;
          }
          frames.emplace_back(top);
          frames.emplace_back(frame{
            .state = parser_state::kParseTest,
            .test = top.expression->mutable_tuple()->add_value(),
          });
        }
        break;
      case parser_state::kParseTest:
        if (is_current(token_type::kLambda)) {
          frames.emplace_back(frame{
            .state = parser_state::kParseLambda,
            .lambda = top.test->mutable_lambda_expression(),
          });
        } else {
          frames.emplace_back(frame{
            .state = parser_state::kParseTest_0,
            .test = top.test,
          });
          frames.emplace_back(frame{
            .state = parser_state::kParseTestP,
            .test = top.test,
            .test_p_precedence = 0,
          });
        }
        break;
      case parser_state::kParseTest_0:
        if (capture(token_type::kIf)) {
          {
            Expression* new_result = Arena::Create<Expression>(top.test->GetArena());
            new_result->mutable_if_expression()->mutable_if_value()->Swap(top.test);
            new_result->Swap(top.test);
          }
          frames.emplace_back(frame{
            .state = parser_state::kParseTest_1,
            .test = top.test,
          });
          frames.emplace_back(frame{
            .state = parser_state::kParseTestP,
            .test = top.test->mutable_if_expression()->mutable_if_test(),
            .test_p_precedence = 0,
          });
        }
        break;
      case parser_state::kParseTest_1:
        if (!expect(token_type::kElse)) {
          break;
        }
        frames.emplace_back(frame{
          .state = parser_state::kParseTest,
          .test = top.test->mutable_if_expression()->mutable_else_value(),
        });
        break;
      case parser_state::kParseTestP:
        if (top.test_p_precedence >= MAX_PRECEDENCE) {
          Expression* result_ref = top.test;
          for (;;) {
            if (capture(token_type::kPlus)) {
              result_ref->mutable_unary_expression()->set_operator_(UnaryExpr::PLUS);
            } else if (capture(token_type::kMinus)) {
              result_ref->mutable_unary_expression()->set_operator_(UnaryExpr::MINUS);
            } else if (capture(token_type::kTilde)) {
              result_ref->mutable_unary_expression()->set_operator_(UnaryExpr::TILDE);
            } else {
              break;
            }
            result_ref = result_ref->mutable_unary_expression()->mutable_test();
          }
          frames.emplace_back(frame{
            .state = parser_state::kParsePrimary,
            .primary = result_ref,
            .primary_must_be_target = false,
          });
          break;
        }
        if (top.test_p_precedence == operator_precedence.at(token_type::kNot).first) {
          Expression* result_ref = top.test;
          for (;;) {
            if (!capture(token_type::kNot)) {
              break;
            }
            result_ref->mutable_unary_expression()->set_operator_(UnaryExpr::NOT);
            result_ref = result_ref->mutable_unary_expression()->mutable_test();
          }
          frames.emplace_back(frame{
            .state = parser_state::kParseTestP,
            .test = result_ref,
            .test_p_precedence = top.test_p_precedence + 1,
          });
          break;
        }
        frames.emplace_back(frame{
          .state = parser_state::kParseTestP_0,
          .test = top.test,
          .test_p_precedence = top.test_p_precedence,
          .test_p_0_first = true,
        });
        frames.emplace_back(frame{
          .state = parser_state::kParseTestP,
          .test = top.test,
          .test_p_precedence = top.test_p_precedence + 1,
        });
        break;
      case parser_state::kParseTestP_0:
        if (is_current(token_type::kNot)) {
          if (top.test_p_precedence != operator_precedence.at(token_type::kIn).first) {
            break;
          }
          if (!top.test_p_0_first) {
            add_error("Comparison operators are not associative. Use parens.");
          }
          lex.next_token();
          if (!expect(token_type::kIn)) {
            break;
          }
          {
            Expression* new_result = Arena::Create<Expression>(top.test->GetArena());
            new_result->mutable_binary_expression()->mutable_lhs()->Swap(top.test);
            new_result->Swap(top.test);
          }
          top.test->mutable_binary_expression()->set_operator_(BinaryExpr::NOT_IN);
          frames.emplace_back(frame{
            .state = parser_state::kParseTestP_0,
            .test = top.test,
            .test_p_precedence = top.test_p_precedence,
            .test_p_0_first = false,
          });
          frames.emplace_back(frame{
            .state = parser_state::kParseTestP,
            .test = top.test->mutable_binary_expression()->mutable_rhs(),
            .test_p_precedence = top.test_p_precedence + 1,
          });
        } else if (auto next_op = operator_precedence.find(lex.current_token().type()); next_op != operator_precedence.end()) {
          if (top.test_p_precedence != next_op->second.first) {
            break;
          }
          if (!top.test_p_0_first && top.test_p_precedence == operator_precedence.at(token_type::kEqualsEquals).first) {
            add_error("Comparison operators are not associative. Use parens.");
          }
          lex.next_token();
          {
            Expression* new_result = Arena::Create<Expression>(top.test->GetArena());
            new_result->mutable_binary_expression()->mutable_lhs()->Swap(top.test);
            new_result->Swap(top.test);
          }
          top.test->mutable_binary_expression()->set_operator_(next_op->second.second);
          frames.emplace_back(frame{
            .state = parser_state::kParseTestP_0,
            .test = top.test,
            .test_p_precedence = top.test_p_precedence,
            .test_p_0_first = false,
          });
          frames.emplace_back(frame{
            .state = parser_state::kParseTestP,
            .test = top.test->mutable_binary_expression()->mutable_rhs(),
            .test_p_precedence = top.test_p_precedence + 1,
          });
        }
        break;
      case parser_state::kParsePrimary:
        frames.emplace_back(frame{
          .state = parser_state::kParsePrimary_0,
          .primary = top.primary,
          .primary_must_be_target = top.primary_must_be_target,
        });
        frames.emplace_back(frame{
          .state = parser_state::kParseOperand,
          .primary = top.primary,
        });
        break;
      case parser_state::kParsePrimary_0:
        if (capture(token_type::kDot)) {
          {
            Expression* new_result = Arena::Create<Expression>(top.primary->GetArena());
            new_result->mutable_dot_expression()->mutable_primary_expression()->Swap(top.primary);
            new_result->Swap(top.primary);
          }
          if (!set_identifier(*top.primary->mutable_dot_expression()->mutable_identifier())) {
            add_error("Expecting IDENTIFIER");
            break;
          }
          frames.emplace_back(top);
        } else if (capture(token_type::kLParen)) {
          {
            Expression* new_result = Arena::Create<Expression>(top.primary->GetArena());
            new_result->mutable_call_expression()->mutable_primary_expression()->Swap(top.primary);
            new_result->Swap(top.primary);
          }
          frames.emplace_back(top);
          if (capture(token_type::kRParen)) {
            break;
          }
          auto* new_argument = top.primary->mutable_call_expression()->add_argument();
          frames.emplace_back(frame{
            .state = parser_state::kParsePrimaryCall_0,
            .primary = top.primary,
            .primary_must_be_target = top.primary_must_be_target,
            .previous_call_argument = new_argument,
          });
          frames.emplace_back(frame{
            .state = parser_state::kParseArgument,
            .argument = new_argument,
            .previous_argument = nullptr,
          });
        } else if (capture(token_type::kLBracket)) {
          {
            Expression* new_result = Arena::Create<Expression>(top.primary->GetArena());
            new_result->mutable_slice_expression()->mutable_primary_expression()->Swap(top.primary);
            new_result->Swap(top.primary);
          }
          frames.emplace_back(top);
          frames.emplace_back(frame{
            .state = parser_state::kParsePrimaryIndexFinal,
          });
          if (capture(token_type::kColon)) {
            top.primary->mutable_slice_expression()->mutable_slice();
            frames.emplace_back(frame{
              .state = parser_state::kParsePrimaryIndex_1,
              .primary = top.primary,
              .primary_must_be_target = top.primary_must_be_target,
            });
          } else {
            frames.emplace_back(frame{
              .state = parser_state::kParsePrimaryIndex_0,
              .primary = top.primary,
              .primary_must_be_target = top.primary_must_be_target,
            });
            // TODO(lmirelmann): Change this once https://github.com/bazelbuild/starlark/issues/291
            // is resolved. The current implementation follows the grammar from Starlark, but this
            // is not the same as the grammar from Python.
            frames.emplace_back(frame{
              .state = parser_state::kParseExpression,
              .expression = top.primary->mutable_slice_expression()->mutable_index(),
              .expression_allow_trailing_comma = true,
            });
          }
        } else {
          if (top.primary_must_be_target && !is_target(top.primary)) {
            add_error("Expecting a TARGET");
          }
        }
        break;
      case parser_state::kParsePrimaryCall_0:
        if (capture(token_type::kComma)) {
          if (capture(token_type::kRParen)) {
            break;
          }
          auto* new_argument = top.primary->mutable_call_expression()->add_argument();
          frames.emplace_back(frame{
            .state = parser_state::kParsePrimaryCall_0,
            .primary = top.primary,
            .primary_must_be_target = top.primary_must_be_target,
            .previous_call_argument = new_argument,
          });
          frames.emplace_back(frame{
            .state = parser_state::kParseArgument,
            .argument = new_argument,
            .previous_argument = top.previous_call_argument,
          });
        } else {
          expect(token_type::kRParen);
        }
        break;
      case parser_state::kParsePrimaryIndex_0:
        if (capture(token_type::kColon)) {
          // TODO(lmirelmann): Solve this issue once https://github.com/bazelbuild/starlark/issues/291
          // is resolved.
          // if (!top.primary->slice_expression().index().has_value()) {
          //   add_error("Unexpected TUPLE");
          // }
          {
            Expression* expression = Arena::Create<Expression>(top.primary->GetArena());
            expression->Swap(top.primary->mutable_slice_expression()->mutable_index());
            top.primary->mutable_slice_expression()->mutable_slice()->mutable_start()->Swap(expression);
          }
          frames.emplace_back(frame{
            .state = parser_state::kParsePrimaryIndex_1,
            .primary = top.primary,
            .primary_must_be_target = top.primary_must_be_target,
          });
        }
        break;
      case parser_state::kParsePrimaryIndex_1:
        frames.emplace_back(frame{
          .state = parser_state::kParsePrimaryIndex_2,
          .primary = top.primary,
          .primary_must_be_target = top.primary_must_be_target,
        });
        if (!is_current(token_type::kColon) && !is_current(token_type::kRBracket)) {
          frames.emplace_back(frame{
            .state = parser_state::kParseTest,
            .test = top.primary->mutable_slice_expression()->mutable_slice()->mutable_end(),
          });
        }
        break;
      case parser_state::kParsePrimaryIndex_2:
        if (capture(token_type::kColon) && !is_current(token_type::kRBracket)) {
          frames.emplace_back(frame{
            .state = parser_state::kParseTest,
            .test = top.primary->mutable_slice_expression()->mutable_slice()->mutable_step(),
          });
        }
        break;
      case parser_state::kParsePrimaryIndexFinal:
        expect(token_type::kRBracket);
        break;
      case parser_state::kParseOperand:
        if (is_current(token_type::kInt)) {
          top.primary->set_int_value(lex.current_token().int_value().to_string(10));
          lex.next_token();
        } else if (is_current(token_type::kIdentifier)) {
          set_identifier(*top.primary->mutable_identifier());
        } else if (is_current(token_type::kFloat)) {
          top.primary->set_float_value(lex.current_token().double_value());
          lex.next_token();
        } else if (is_current(token_type::kString)) {
          top.primary->set_string_value(lex.current_token().string_value());
          lex.next_token();
        } else if (is_current(token_type::kBytes)) {
          top.primary->set_bytes_value(lex.current_token().string_value());
          lex.next_token();
        } else if (is_current(token_type::kLBracket)) {
          frames.emplace_back(frame{
            .state = parser_state::kParseList,
            .primary = top.primary,
          });
        } else if (is_current(token_type::kLBrace)) {
          frames.emplace_back(frame{
            .state = parser_state::kParseDict,
            .primary = top.primary,
          });
        } else if (capture(token_type::kLParen)) {
          if (capture(token_type::kRParen)) {
            top.primary->mutable_tuple();
          } else {
            frames.emplace_back(frame{
              .state = parser_state::kParseOperandExpression_0,
            });
            frames.emplace_back(frame{
              .state = parser_state::kParseExpression,
              .expression = top.primary,
              .expression_allow_trailing_comma = true,
            });
          }
        } else {
          add_error("Unexpected token");
        }
        break;
      case parser_state::kParseOperandExpression_0:
        if (!expect(token_type::kRParen)) {
          break;
        }
        break;
      case parser_state::kParseList:
        if (!expect(token_type::kLBracket)) {
          break;
        }
        if (capture(token_type::kRBracket)) {
          top.primary->mutable_list_expression();
          break;
        }

        create_block({}, {}, nullptr);
        frames.emplace_back(frame{
          .state = parser_state::kParseListFinal,
          .primary = top.primary,
        });
        frames.emplace_back(frame{
          .state = parser_state::kParseList_0,
          .primary = top.primary,
        });
        frames.emplace_back(frame{
          .state = parser_state::kParseTest,
          .test = top.primary->mutable_list_expression()->add_element(),
        });
        break;
      case parser_state::kParseList_0:
        switch (lex.current_token().type()) {
          case token_type::kFor:
            {
              Expression* expression = Arena::Create<Expression>(top.primary->GetArena());
              expression->Swap(&top.primary->mutable_list_expression()->mutable_element()->at(0));
              expression->Swap(top.primary->mutable_list_comprehension()->mutable_test());
            }
            parser_blocks.back().id_store = top.primary->mutable_list_comprehension()->mutable_comprehension_binding();
            frames.emplace_back(frame{
              .state = parser_state::kParseListComprehensionFinal,
              .primary = top.primary,
            });
            frames.emplace_back(frame{
              .state = parser_state::kParseCompClauses,
              .comp_clauses = top.primary->mutable_list_comprehension()->mutable_clause(),
              .first_comp_clause = true,
            });
            break;
          case token_type::kRBracket:
          case token_type::kComma:
            frames.emplace_back(frame{
              .state = parser_state::kParseListIndex_0,
              .primary = top.primary,
            });
            break;
          default:
            break;
        }
        break;
      case parser_state::kParseListIndex_0:
        if (capture(token_type::kComma)) {
          if (is_current(token_type::kRBracket)) {
            break;
          }
          frames.emplace_back(top);
          frames.emplace_back(frame{
            .state = parser_state::kParseTest,
            .test = top.primary->mutable_list_expression()->add_element(),
          });
        }
        break;
      case parser_state::kParseListComprehensionFinal:
        resolve(top.primary->mutable_list_comprehension()->mutable_test(), 0);
        break;
      case parser_state::kParseListFinal:
        drop_block();
        if (top.primary->expression_type_case() == Expression::kListComprehension) {
          resolve(top.primary->mutable_list_comprehension()->mutable_clause(0)->mutable_for_clause()->mutable_in(), 1);
        }
        if (!expect(token_type::kRBracket)) {
          break;
        }
        break;
      case parser_state::kParseDict:
        if (!expect(token_type::kLBrace)) {
          break;
        }
        if (capture(token_type::kRBrace)) {
          top.primary->mutable_dictionary_expression();
          break;
        }
        create_block({}, {}, nullptr);
        frames.emplace_back(frame{
          .state = parser_state::kParseDictFinal,
          .primary = top.primary,
        });
        frames.emplace_back(frame{
          .state = parser_state::kParseDict_0,
          .primary = top.primary,
        });
        frames.emplace_back(frame{
          .state = parser_state::kParseEntry,
          .entry = top.primary->mutable_dictionary_expression()->add_entry(),
        });
        break;
      case parser_state::kParseDict_0:
        switch (lex.current_token().type()) {
          case token_type::kFor:
            {
              Entry* entry = Arena::Create<Entry>(top.primary->GetArena());
              entry->Swap(&top.primary->mutable_dictionary_expression()->mutable_entry()->at(0));
              entry->Swap(top.primary->mutable_dictionary_comprehension()->mutable_entry());
            }
            parser_blocks.back().id_store = top.primary->mutable_dictionary_comprehension()->mutable_comprehension_binding();
            frames.emplace_back(frame{
              .state = parser_state::kParseDictComprehensionFinal,
              .primary = top.primary,
            });
            frames.emplace_back(frame{
              .state = parser_state::kParseCompClauses,
              .comp_clauses = top.primary->mutable_dictionary_comprehension()->mutable_clause(),
              .first_comp_clause = true,
            });
            break;
          case token_type::kRBrace:
          case token_type::kComma:
            frames.emplace_back(frame{
              .state = parser_state::kParseDictIndex_0,
              .primary = top.primary,
            });
            break;
          default:
            break;
        }
        break;
      case parser_state::kParseDictIndex_0:
        if (capture(token_type::kComma)) {
          if (is_current(token_type::kRBrace)) {
            break;
          }
          frames.emplace_back(top);
          frames.emplace_back(frame{
            .state = parser_state::kParseEntry,
            .entry = top.primary->mutable_dictionary_expression()->add_entry(),
          });
        }
        break;
      case parser_state::kParseDictComprehensionFinal:
        resolve(top.primary->mutable_dictionary_comprehension()->mutable_entry()->mutable_key(), 0);
        resolve(top.primary->mutable_dictionary_comprehension()->mutable_entry()->mutable_value(), 0);
        break;
      case parser_state::kParseDictFinal:
        drop_block();
        if (top.primary->expression_type_case() == Expression::kDictionaryComprehension) {
          resolve(top.primary->mutable_dictionary_comprehension()->mutable_clause(0)->mutable_for_clause()->mutable_in(), 1);
        }
        expect(token_type::kRBrace);
        break;
      case parser_state::kParseEntry:
        frames.emplace_back(frame{
          .state = parser_state::kParseEntry_0,
          .entry = top.entry,
        });
        frames.emplace_back(frame{
          .state = parser_state::kParseTest,
          .test = top.entry->mutable_key(),
        });
        break;
      case parser_state::kParseEntry_0:
        if (!expect(token_type::kColon)) {
          break;
        }
        frames.emplace_back(frame{
          .state = parser_state::kParseTest,
          .test = top.entry->mutable_value(),
        });
        break;
      case parser_state::kParseCompClauses:
        if (capture(token_type::kFor)) {
          auto* comp_clause = top.comp_clauses->Add();
          auto* comp_clause_primary = comp_clause->mutable_for_clause()->mutable_loop_variables();
          frames.emplace_back(frame{
            .state = parser_state::kParseCompClauses,
            .comp_clauses = top.comp_clauses,
            .first_comp_clause = false,
          });
          frames.emplace_back(frame{
            .state = parser_state::kParseCompClauses_0,
            .comp_clause = comp_clause,
            .comp_clause_primary = comp_clause_primary,
            .comp_clause_primary_first_expression_part = true,
            .comp_clause_resolve_in_test = !top.first_comp_clause,
          });
          frames.emplace_back(frame{
            .state = parser_state::kParsePrimary,
            .primary = comp_clause_primary,
            .primary_must_be_target = true,
          });
        } else if (capture(token_type::kIf)) {
          auto* comp_clause = top.comp_clauses->Add();
          frames.emplace_back(top);
          frames.emplace_back(frame{
            .state = parser_state::kParseResolveTest,
            .test = comp_clause->mutable_if_clause(),
          });
          // Have to avoid parsing this as an `IfExpr`.
          // This is also not allowing a lambda to be used.
          // Context: https://github.com/bazelbuild/bazel/issues/24469
          frames.emplace_back(frame{
            .state = parser_state::kParseTestP,
            .test = comp_clause->mutable_if_clause(),
            .test_p_precedence = 0,
          });
        }
        break;
      case parser_state::kParseCompClauses_0:
        if (capture(token_type::kComma)) {
          // If there are multiple loop varaibles, transform into a tuple.
          if (top.comp_clause_primary_first_expression_part) {
            Expression* first_primary = Arena::Create<Expression>(top.comp_clause_primary->GetArena());
            first_primary->Swap(top.comp_clause_primary);
            first_primary->Swap(top.comp_clause_primary->mutable_tuple()->add_value());
          }
          auto* comp_clause_primary = top.comp_clause_primary->mutable_tuple()->add_value();
          frames.emplace_back(frame{
            .state = parser_state::kParseCompClauses_0,
            .comp_clause = top.comp_clause,
            .comp_clause_primary = top.comp_clause_primary,
            .comp_clause_primary_first_expression_part = false,
            .comp_clause_resolve_in_test = top.comp_clause_resolve_in_test,
          });
          frames.emplace_back(frame{
            .state = parser_state::kParsePrimary,
            .primary = comp_clause_primary,
            .primary_must_be_target = true,
          });
        } else {
          bind_and_resolve(top.comp_clause_primary);
          if (!expect(token_type::kIn)) {
            break;
          }
          // The first `for in` test should be reslved one frame up.
          if (top.comp_clause_resolve_in_test) {
            frames.emplace_back(frame{
              .state = parser_state::kParseResolveTest,
              .test = top.comp_clause->mutable_for_clause()->mutable_in(),
            });
          }
          // Do not allow `IfExpr` nor lambdas.
          frames.emplace_back(frame{
            .state = parser_state::kParseTestP,
            .test = top.comp_clause->mutable_for_clause()->mutable_in(),
            .test_p_precedence = 0,
          });
        }
        break;
      case parser_state::kParseArgument:
        // Check that the order is (not all elements must be present, but the order is strict):
        // - positional arguments
        // - keyword arguments
        // - At most one *args
        // - At most one **kwargs
        if (capture(token_type::kStar)) {
          if (!opts.allow_varadic_arguments) {
            // Report the error, but keep on parsing.
            add_error("Varadic arguments are not allowed");
          }
          if (top.previous_argument != nullptr &&
              top.previous_argument->has_star_argument()) {
            add_error("Duplicate *args");
          }
          if (top.previous_argument != nullptr &&
              top.previous_argument->has_star_star_argument()) {
            add_error("**kwargs must be the last argument");
          }
          frames.emplace_back(frame{
            .state = parser_state::kParseTest,
            .test = top.argument->mutable_star_argument(),
          });
        } else if (capture(token_type::kStarStar)) {
          if (!opts.allow_varadic_arguments) {
            // Report the error, but keep on parsing.
            add_error("Varadic arguments are not allowed");
          }
          if (top.previous_argument != nullptr &&
              top.previous_argument->has_star_star_argument()) {
            add_error("Duplicate **kwargs");
          }
          frames.emplace_back(frame{
            .state = parser_state::kParseTest,
            .test = top.argument->mutable_star_star_argument(),
          });
        } else {
          if (top.previous_argument != nullptr &&
              (top.previous_argument->has_star_argument() ||
               top.previous_argument->has_star_star_argument())) {
            add_error("Non-varadic arguments must be before varadic arguments");
          }
          if (lex.current_token().type() == token_type::kIdentifier) {
            frames.emplace_back(frame{
              .state = parser_state::kParseArgument_0,
              .argument = top.argument,
              .previous_argument = top.previous_argument,
            });
          }
          frames.emplace_back(frame{
            .state = parser_state::kParseTest,
            .test = top.argument->mutable_value(),
          });
        }
        break;
      case parser_state::kParseArgument_0:
        if (capture(token_type::kEquals)) {
          // The reason this is able to detect that this is not an identifier between brackets is
          // because this rule is only called when the argument value started with an identifier.
          if (!top.argument->value().has_identifier()) {
            add_error("Expected identifier for named arguments");
            break;
          }
          {
            Identifier* id = Arena::Create<Identifier>(top.argument->GetArena());
            id->Swap(top.argument->mutable_value()->mutable_identifier());
            id->Swap(top.argument->mutable_named_argument()->mutable_identifier());
          }
          frames.emplace_back(frame{
            .state = parser_state::kParseTest,
            .test = top.argument->mutable_named_argument()->mutable_value(),
          });
        } else {
          if (top.previous_argument != nullptr &&
              top.previous_argument->has_named_argument()) {
            add_error("Positional arguments must come before named arguments");
          }
        }
        break;
      case parser_state::kParseLambda:
        if (!opts.allow_function_definitions) {
          add_error("Function definitions not allowed");
        }
        expect(token_type::kLambda);
        frames.emplace_back(frame{
          .state = parser_state::kParseLambda_0,
          .lambda = top.lambda,
        });
        frames.emplace_back(frame{
          .state = parser_state::kParseParameters,
          .parameters = top.lambda->mutable_parameter(),
          .parse_parameters_allow_trailing_comma = false,
          .parse_parameters_first = true,
          .found_star_parameter = false,
          .found_star_star_parameter = false,
          .previous_parameter_was_bare_star = false,
        });
        break;
      case parser_state::kParseLambda_0:
        create_block(parse_parameter_identifiers.back().first,
                     parse_parameter_identifiers.back().second,
                     top.lambda->mutable_function_binding());
        parse_parameter_identifiers.pop_back();
        frames.emplace_back(frame{
          .state = parser_state::kParseLambdaFinal,
          .lambda = top.lambda,
        });
        if (!expect(token_type::kColon)) {
          break;
        }
        frames.emplace_back(frame{
          .state = parser_state::kParseResolveTest,
          .test = top.lambda->mutable_test(),
        });
        frames.emplace_back(frame{
          .state = parser_state::kParseTest,
          .test = top.lambda->mutable_test(),
        });
        break;
      case parser_state::kParseLambdaFinal:
        drop_block();
        break;
      case parser_state::kParseParameters:
        if (top.parse_parameters_first || capture(token_type::kComma)) {
          if (top.parse_parameters_first) {
            parse_parameter_identifiers.emplace_back(std::set<std::string, std::less<>>{}, std::set<Identifier*>{});
          }
          if (is_current(token_type::kIdentifier)) {
            if (top.found_star_star_parameter) {
              add_error("arguments cannot follow var-keyword argument");
            }
            frames.emplace_back(frame{
              .state = parser_state::kParseParameters,
              .parameters = top.parameters,
              .parse_parameters_allow_trailing_comma = top.parse_parameters_allow_trailing_comma,
              .parse_parameters_first = false,
              .found_star_parameter = top.found_star_parameter,
              .found_star_star_parameter = top.found_star_star_parameter,
              .previous_parameter_was_bare_star = false,
            });
            Parameter* param = top.parameters->Add();
            set_identifier(*param->mutable_identifier());
            if (capture(token_type::kEquals)) {
              frames.emplace_back(frame{
                .state = parser_state::kParseResolveTest,
                .test = param->mutable_initialization(),
              });
              frames.emplace_back(frame{
                .state = parser_state::kParseTest,
                .test = param->mutable_initialization(),
              });
            }
            if (!parse_parameter_identifiers.back().first.emplace(param->identifier().nfkc_name()).second) {
              add_error(std::format("duplicate argument '{}' in function definition", param->identifier().name()));
            }
            // The parameters need to be resolved. The issue is that the block does not yet exists so there
            // is a need to store the Identifiers and resolve them later.
            parse_parameter_identifiers.back().second.insert(param->mutable_identifier());
          } else if (capture(token_type::kStar)) {
            if (top.found_star_parameter) {
              add_error("* argument may appear only once");
            }
            if (top.found_star_star_parameter) {
              add_error("arguments cannot follow var-keyword argument");
            }
            frames.emplace_back(frame{
              .state = parser_state::kParseParameters,
              .parameters = top.parameters,
              .parse_parameters_allow_trailing_comma = top.parse_parameters_allow_trailing_comma,
              .parse_parameters_first = false,
              .found_star_parameter = true,
              .found_star_star_parameter = top.found_star_star_parameter,
              .previous_parameter_was_bare_star = !is_current(token_type::kIdentifier),
            });
            Parameter* param = top.parameters->Add();
            param->mutable_star();
            if (is_current(token_type::kIdentifier)) {
              set_identifier(*param->mutable_identifier());
              if (!parse_parameter_identifiers.back().first.emplace(param->identifier().nfkc_name()).second) {
                add_error(std::format("duplicate argument '{}' in function definition", param->identifier().name()));
              }
              parse_parameter_identifiers.back().second.insert(param->mutable_identifier());
            }
          } else if (capture(token_type::kStarStar)) {
            if (top.previous_parameter_was_bare_star) {
              add_error("named arguments must follow bare *");
            }
            if (top.found_star_star_parameter) {
              add_error("arguments cannot follow var-keyword argument");
            }
            frames.emplace_back(frame{
              .state = parser_state::kParseParameters,
              .parameters = top.parameters,
              .parse_parameters_allow_trailing_comma = top.parse_parameters_allow_trailing_comma,
              .parse_parameters_first = false,
              .found_star_parameter = top.found_star_parameter,
              .found_star_star_parameter = true,
              .previous_parameter_was_bare_star = false,
            });
            Parameter* param = top.parameters->Add();
            param->mutable_star_star();
            if (!set_identifier(*param->mutable_identifier())) {
              add_error("Expected identifier after STAR_STAR when parsing parameters");
            } else {
              if (!parse_parameter_identifiers.back().first.emplace(param->identifier().nfkc_name()).second) {
                add_error(std::format("duplicate argument '{}' in function definition", param->identifier().name()));
              }
              parse_parameter_identifiers.back().second.insert(param->mutable_identifier());
            }
          } else {
            if (top.previous_parameter_was_bare_star) {
              add_error("named arguments must follow bare *");
            }
            if (!top.parse_parameters_first && !top.parse_parameters_allow_trailing_comma) {
              add_error("Unexpected COMMA");
            }
          }
        } else {
          if (top.previous_parameter_was_bare_star) {
            add_error("named arguments must follow bare *");
          }
        }
        break;
      case parser_state::kParseResolveTest:
        resolve(top.test, 0);
        break;
      case parser_state::kParseResolveExpression:
        resolve(top.expression, 0);
        break;
    }
  } while (!frames.empty());
}

void parser::bind_and_resolve(Expression* base) {
  resolve(base, 0);
  std::vector<Expression*> frames{base};
  do {
    auto top = frames.back();
    frames.pop_back();
    if (top != nullptr) {
      switch (top->expression_type_case()) {
        case Expression::kDotExpression:
        case Expression::kSliceExpression:
          break;
        case Expression::kIdentifier:
          if (is_top_level_block() &&
              parser_blocks.back().identifiers.contains(top->identifier().nfkc_name())) {
            add_error(std::format("Variable '{}' redefines symbol previously defined by a load statement", top->identifier().name()));
          }
          bind(top->identifier());
          break;
        case Expression::kListExpression:
          for (auto& item : *top->mutable_list_expression()->mutable_element()) {
            frames.push_back(&item);
          }
          break;
        case Expression::kCallExpression:
        case Expression::kIntValue:
        case Expression::kFloatValue:
        case Expression::kStringValue:
        case Expression::kBytesValue:
        case Expression::kListComprehension:
        case Expression::kDictionaryExpression:
        case Expression::kDictionaryComprehension:
        case Expression::kIfExpression:
        case Expression::kUnaryExpression:
        case Expression::kBinaryExpression:
        case Expression::kLambdaExpression:
        case Expression::EXPRESSION_TYPE_NOT_SET:
          add_warning("Unexpected expression to bind");
          break;
        case Expression::kTuple:
          for (auto& element : *top->mutable_tuple()->mutable_value()) {
            frames.push_back(&element);
          }
          break;
      }
    }
  } while (!frames.empty());
}

void parser::bind(const Identifier& identifier) {
  // The spec states "It is a static error to bind a global variable already explicitly bound in the file"
  // This leaves to interpretation whether a `def` can be rebound. The rule followed is that `def` and variables should
  // be consistent, follow the same rules, and be handled equally.
  if (is_top_level_block()) {
    if (!parser_blocks[parser_blocks.size() - 2].identifiers.emplace(identifier.nfkc_name()).second && !opts.allow_top_level_rebinding) {
      add_error(std::format("Multiple bindings for the top-level symbol '{}'", identifier.name()));
    }
  } else {
    parser_blocks.back().identifiers.emplace(identifier.nfkc_name());
  }
}

bool parser::set_identifier(Identifier& identifier) {
  if (!is_current(token_type::kIdentifier)) {
    return false;
  }
  auto name = lex.current_token().string_value();
  identifier.set_name(name);
  identifier.set_nfkc_name(to_nfkc(name));
  identifier_positions[&identifier] = lex.current_token().start();
  lex.next_token();
  return true;
}

void parser::create_block(const std::set<std::string, std::less<>>& symbols,
                    const std::set<Identifier*>& identifiers,
                    google::protobuf::RepeatedPtrField<std::string>* binding) {
  parser_blocks.emplace_back(symbols, binding);
  for (auto* id : identifiers) {
    resolve(id, 0);
  }
}

void parser::drop_block() {
  if (parser_blocks.back().id_store != nullptr) {
    for (const auto& binding : parser_blocks.back().identifiers) {
      *parser_blocks.back().id_store->Add() = binding;
    }
  }
  for (auto& entry : parser_blocks.back().to_resolve) {
    auto pos = parser_blocks.back().identifiers.find(entry.first->nfkc_name());
    if (pos == parser_blocks.back().identifiers.end()) {
      if (parser_blocks.size() == 1) {
        add_error(std::format("name '{}' is not defined", entry.first->name()), identifier_positions[entry.first]);
        entry.first->set_frame(-1);
        entry.first->set_pos_in_frame(-1);
      } else {
        auto new_pos = entry.second + (parser_blocks.back().id_store == nullptr ? 0 : 1);
        parser_blocks[parser_blocks.size() - 2].to_resolve.emplace_back(entry.first, new_pos);
      }
    } else {
      entry.first->set_frame(entry.second);
      entry.first->set_pos_in_frame(std::distance(parser_blocks.back().identifiers.begin(), pos));
    }
  }
  parser_blocks.pop_back();
}

void parser::resolve(Identifier* identifier, int base_frame) {
  parser_blocks.back().to_resolve.emplace_back(identifier, base_frame);
}

void parser::resolve(Expression* base, int base_frame) {
  std::vector<Expression*> frames{base};
  do {
    auto top = frames.back();
    frames.pop_back();
    if (top != nullptr) {
      switch (top->expression_type_case()) {
        case Expression::kDotExpression:
          frames.push_back(top->mutable_dot_expression()->mutable_primary_expression());
          break;
        case Expression::kSliceExpression:
          frames.push_back(top->mutable_slice_expression()->mutable_primary_expression());
          if (top->slice_expression().has_index()) {
            frames.push_back(top->mutable_slice_expression()->mutable_index());
          }
          if (top->slice_expression().slice().has_start()) {
            frames.push_back(top->mutable_slice_expression()->mutable_slice()->mutable_start());
          }
          if (top->slice_expression().slice().has_end()) {
            frames.push_back(top->mutable_slice_expression()->mutable_slice()->mutable_end());
          }
          if (top->slice_expression().slice().has_step()) {
            frames.push_back(top->mutable_slice_expression()->mutable_slice()->mutable_step());
          }
          break;
        case Expression::kIdentifier:
          resolve(top->mutable_identifier(), base_frame);
          break;
        case Expression::kCallExpression:
          frames.push_back(top->mutable_call_expression()->mutable_primary_expression());
          for (auto& arg : *top->mutable_call_expression()->mutable_argument()) {
            if (arg.has_value()) {
              frames.push_back(arg.mutable_value());
            }
            if (arg.has_named_argument()) {
              frames.push_back(arg.mutable_named_argument()->mutable_value());
            }
            if (arg.has_star_argument()) {
              frames.push_back(arg.mutable_star_argument());
            }
            if (arg.has_star_star_argument()) {
              frames.push_back(arg.mutable_star_star_argument());
            }
          }
          break;
        case Expression::kIntValue:
        case Expression::kFloatValue:
        case Expression::kStringValue:
        case Expression::kBytesValue:
        case Expression::kListComprehension:
        case Expression::kDictionaryComprehension:
        default:
          break;
        case Expression::kDictionaryExpression:
          for (auto& item : *top->mutable_dictionary_expression()->mutable_entry()) {
            frames.push_back(item.mutable_key());
            frames.push_back(item.mutable_value());
          }
          break;
        case Expression::kListExpression:
          for (auto& item : *top->mutable_list_expression()->mutable_element()) {
            frames.push_back(&item);
          }
          break;
        case Expression::EXPRESSION_TYPE_NOT_SET:
          add_warning("Unexpected expression to resolve");
          break;
        case Expression::kTuple:
          for (auto& element : *top->mutable_tuple()->mutable_value()) {
            frames.push_back(&element);
          }
          break;
        case Expression::kIfExpression:
          frames.push_back(top->mutable_if_expression()->mutable_if_value());
          frames.push_back(top->mutable_if_expression()->mutable_if_test());
          frames.push_back(top->mutable_if_expression()->mutable_else_value());
          break;
        case Expression::kUnaryExpression:
          frames.push_back(top->mutable_unary_expression()->mutable_test());
          break;
        case Expression::kBinaryExpression:
          frames.push_back(top->mutable_binary_expression()->mutable_lhs());
          frames.push_back(top->mutable_binary_expression()->mutable_rhs());
          break;
        case Expression::kLambdaExpression:
          break;
      }
    }
  } while (!frames.empty());
}

bool parser::is_top_level_block() const {
  return parser_blocks.size() == 3;
}

}  // namespace grammar
}  // namespace starlark

