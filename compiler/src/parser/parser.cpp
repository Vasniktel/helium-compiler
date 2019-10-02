//
// Created by vasniktel on 27.08.2019.
//

#include <vector>
#include <utility>
#include "parser.hpp"

namespace helium {
namespace {

using ::std::move;
using ::std::unique_ptr;
using ::std::vector;
using ::absl::string_view;
using ::absl::make_unique;
using ::absl::flat_hash_map;
using TT = TokenType;

bool IsAlpha(char c) {
  return (c >= 'a' && c <= 'z') ||
      (c >= 'A' && c <= 'Z') ||
      c == '_';
}

bool IsDigit(char c) {
  return c >= '0' && c <= '9';
}

bool IsAlNum(char c) {
  return IsDigit(c) || IsAlpha(c);
}

}

Parser::Parser(string_view source, ErrorReporter& reporter, Interner& interner)
: source_(source),
  size_(static_cast<int>(source.size())),
  curr_char_(0),
  start_(0),
  line_(1),
  col_(1),
  curr_token_(Token::ParsedAs(*this, TT::kEof)),
  prev_token_(Token::ParsedAs(*this, TT::kEof)),
  panic_mode_(false),
  reporter_(reporter),
  interner_(interner)
{}

char Parser::AdvanceChar() {
  if (IsAtEnd()) return '\0';

  char c = source_[curr_char_++];
  if (c == '\n') {
    line_++;
    col_ = 0; // new line is considered the first token
  }

  col_++;
  return c;
}

void Parser::ParseComment() {
  while (!IsAtEnd() && PeekChar() != '\n') {
    AdvanceChar();
  }
}

void Parser::SkipSpace() {
  for (; !IsAtEnd(); AdvanceChar()) {
    switch (PeekChar()) {
      case '\t':
      case '\r':
      case ' ': break;
      case '/':
        if (PeekNextChar() == '/') {
          ParseComment();
        }
      default:
        return;
    }
  }
}

void Parser::MakeToken(TT type) {
  prev_token_ = curr_token_;
  curr_token_ = Token::ParsedAs(*this, type);
}

void Parser::ParserError(string_view msg, const Token& token) {
  if (panic_mode_) return;
  panic_mode_ = true;
  if (token.type == TT::kError) return;
  reporter_.ErrorAt(msg, token);
}

void Parser::LexError(absl::string_view msg, int line, int col) {
  MakeToken(TT::kError);
  reporter_.ErrorAt(msg, line, col);
}

// consumes characters
bool Parser::IsValidSymbol() {
  switch (AdvanceChar()) {
    case '\n':
    case '\t':
    case '\r':
    case '\'':
    case '\"': return false;
    case '\\': return IsEscape();
    default: return true;
  }
}

bool Parser::IsEscape() {
  // '\\' has already been consumed
  switch (PeekChar()) {
    case '\'':
    case '\"':
    case '\\':
      AdvanceChar();
      return true;
    default: return IsValidSymbol();
  }
}

// TODO: define and handle escape sequences
void Parser::ParseChar() {
  const int line = line_, col = col_ - 1;
  bool valid;

  if (PeekChar() == '"') {
    AdvanceChar();
    valid = true;
  } else {
    valid = IsValidSymbol();
  }

  valid = valid && AdvanceChar() == '\'';

  if (!valid) {
    LexError("Invalid character literal", line, col);
    return;
  }

  MakeToken(TT::kChar);
}

// TODO: define and handle escape sequences
void Parser::ParseString() {
  const int line = line_, col = col_ - 1;

  bool valid = true;
  while (!IsAtEnd() && PeekChar() != '"') {
    switch (PeekChar()) {
      case '\'':
      case '\n':
      case '\t':
      case '\r':
        AdvanceChar();
        break;
      default:
        valid = valid && IsValidSymbol();
    }
  }

  if (!valid) {
    LexError("Invalid string literal", line, col);
    return;
  }

  if (AdvanceChar() != '"') {
    LexError("Missing closing \"", line, col);
    return;
  }

  MakeToken(TT::kString);
}

void Parser::ParseNumber() {
  const int line = line_, col = col_ - 1;
  while (!IsAtEnd() && IsDigit(PeekChar())) {
    AdvanceChar();
  }

  bool real = PeekChar() == '.';
  if (real) {
    do {
      AdvanceChar();
    } while (!IsAtEnd() && IsDigit(PeekChar()));
  }

  if (source_[start_] == '0' && curr_char_ - start_ > 1) {
    LexError("Invalid number literal", line, col);
    return;
  }

  MakeToken(real ? TT::kReal : TT::kInt);
}

void Parser::ParseIdentifier() {
  static const flat_hash_map<string_view, TT> keywords = {
      {"var",   TT::kVar},
      {"while", TT::kWhile},
      {"if",    TT::kIf},
      {"else",  TT::kElse},
      {"true",  TT::kTrue},
      {"false", TT::kFalse},
      {"unit",  TT::kUnit}
  };

  while (!IsAtEnd() && IsAlNum(PeekChar())) {
    AdvanceChar();
  }

  auto id = source_.substr(start_, curr_char_ - start_);
  MakeToken(keywords.contains(id) ? keywords.at(id) : TT::kIdentifier);
}

Token& Parser::NextToken() {
  SkipSpace();
  start_ = curr_char_;

  char c = AdvanceChar();
  switch (c) {
    case '\0': MakeToken(TT::kEof);
      break;
    case '\n': MakeToken(TT::kEol);
      break;
    case '{': MakeToken(TT::kLeftBrace);
      break;
    case '}': MakeToken(TT::kRightBrace);
      break;
    case '(': MakeToken(TT::kLeftParen);
      break;
    case ')': MakeToken(TT::kRightParen);
      break;
    case '+': MakeToken(TT::kPlus);
      break;
    case '-': MakeToken(TT::kMinus);
      break;
    case '*': MakeToken(TT::kStar);
      break;
    case '/': MakeToken(TT::kSlash);
      break;
    case '=': MakeToken(TT::kEqual);
      break;
    case ':': MakeToken(TT::kColon);
      break;
    case '"': ParseString();
      break;
    case '\'': ParseChar();
      break;
    default:
      if (IsAlpha(c)) ParseIdentifier();
      else if (IsDigit(c)) ParseNumber();
      else LexError("Unexpected symbol", line_, col_ - 1);
  }

  return prev_token_;
}

void Parser::SkipEolTokens() {
  while (curr_token_.type == TT::kEol) {
    NextToken();
  }
}

bool Parser::MatchToken(TT type, bool ignore_eol) {
  if (ignore_eol) SkipEolTokens();
  if (curr_token_.type != type) return false;
  NextToken();
  return true;
}

bool Parser::ConsumeToken(TT type, bool ignore_eol, string_view msg) {
  if (MatchToken(type, ignore_eol)) return true;
  ParserError(msg, prev_token_);
  return false;
}

enum class Parser::Precedence {
  kNone,
  kAssign,
  kAdd,
  kMul,
  kUnary
};

struct Parser::Rule {
  using PrefixFn = unique_ptr<Expr>(Parser::*)(bool can_assign);
  using InfixFn  = unique_ptr<Expr>(Parser::*)(unique_ptr<Expr> left);

  PrefixFn prefix;
  InfixFn infix;
  Precedence precedence;
};

const Parser::Rule Parser::rules_[] = {
#define TT(type) (static_cast<int>(TT::type))
    [TT(kEof)]   = {nullptr, nullptr, Precedence::kNone},
    [TT(kEol)]   = {nullptr, nullptr, Precedence::kNone},
    [TT(kError)] = {nullptr, nullptr, Precedence::kNone},

    [TT(kChar)]       = {&Parser::Literal,    nullptr, Precedence::kNone},
    [TT(kInt)]        = {&Parser::Literal,    nullptr, Precedence::kNone},
    [TT(kReal)]       = {&Parser::Literal,    nullptr, Precedence::kNone},
    [TT(kString)]     = {&Parser::Literal,    nullptr, Precedence::kNone},
    [TT(kIdentifier)] = {&Parser::Identifier, nullptr, Precedence::kNone},
    [TT(kTrue)]       = {&Parser::Literal,    nullptr, Precedence::kNone},
    [TT(kFalse)]      = {&Parser::Literal,    nullptr, Precedence::kNone},
    [TT(kUnit)]       = {&Parser::Literal,    nullptr, Precedence::kNone},

    [TT(kVar)]   = {nullptr,          nullptr, Precedence::kNone},
    [TT(kWhile)] = {&Parser::While,   nullptr, Precedence::kNone},
    [TT(kIf)]    = {&Parser::If,      nullptr, Precedence::kNone},
    [TT(kElse)]  = {nullptr,          nullptr, Precedence::kNone},

    [TT(kPlus)]       = {&Parser::Unary,    &Parser::Binary, Precedence::kAdd},
    [TT(kMinus)]      = {&Parser::Unary,    &Parser::Binary, Precedence::kAdd},
    [TT(kStar)]       = {nullptr,           &Parser::Binary, Precedence::kMul},
    [TT(kSlash)]      = {nullptr,           &Parser::Binary, Precedence::kMul},
    [TT(kLeftParen)]  = {&Parser::Grouping, nullptr,         Precedence::kNone},
    [TT(kRightParen)] = {nullptr,           nullptr,         Precedence::kNone},
    [TT(kLeftBrace)]  = {&Parser::Block,    nullptr,         Precedence::kNone},
    [TT(kRightBrace)] = {nullptr,           nullptr,         Precedence::kNone},
    [TT(kEqual)]      = {nullptr,           nullptr,         Precedence::kNone},
    [TT(kColon)]      = {nullptr,           nullptr,         Precedence::kNone},
#undef TT
};

template <typename T, typename F>
vector<unique_ptr<T>> Parser::Sequence(TT separator, TT closing, F parser) {
  if (MatchToken(closing, true)) return {};

  vector<unique_ptr<T>> result;
  while (true) {
    auto node = parser();
    if (node) result.emplace_back(move(node));

    bool has_separator = MatchToken(separator, separator != TT::kEol);
    auto maybe_sep = prev_token_;

    if (MatchToken(closing, true)) break;
    else if (MatchToken(TT::kEof, false)) {
      ParserError("Missing closing token", maybe_sep);
      break;
    } else if (!has_separator) {
      ParserError("Missing separator in a sequence", maybe_sep);
      break;
    }
  }

  return result;
}

vector<unique_ptr<AstNode>> Parser::Parse(
    string_view source, ErrorReporter& reporter, Interner& interner) {
  Parser parser(source, reporter, interner);
  parser.NextToken();
  return parser.Sequence<AstNode>(TT::kEol, TT::kEof, [&parser] {
    return parser.Statement();
  });
}

#define PARSE_EXPRESSION(var, precedence, skip_eol) \
    do { \
      panic_mode_ = false; \
      if ((skip_eol)) SkipEolTokens(); \
      var = Expression(precedence); \
      assert(reporter_.HadErrors() || var); \
    } while (false)

#define CONSTRUCT_NODE(node) \
      reporter_.HadErrors() ? nullptr : (node)

#define IGNORE(expr) \
      static_cast<void>((expr))

unique_ptr<Expr> Parser::Expression(Precedence precedence) {
  const auto& token = NextToken();
  auto prefix = rules_[static_cast<int>(token.type)].prefix;
  if (prefix == nullptr) {
    ParserError("Unexpected token", token);
    return nullptr;
  }

  bool can_assign = precedence <= Precedence::kAssign;
  auto left = (this->*prefix)(can_assign);
  if (panic_mode_) return nullptr;

  while (rules_[static_cast<int>(curr_token_.type)].precedence >= precedence) {
    const auto& op = NextToken();
    auto infix = rules_[static_cast<int>(op.type)].infix;
    left = (this->*infix)(move(left));
    if (panic_mode_) return nullptr;
  }

  if (can_assign && MatchToken(TT::kEqual, false)) {
    ParserError("Invalid assignment target", prev_token_);
    return nullptr;
  }

  return left;
}

unique_ptr<AstNode> Parser::Statement() {
  // we synchronize at statement level and higher
  panic_mode_ = false;
  if (MatchToken(TT::kVar, true)) return Variable();
  return Expression(Precedence::kAssign);
}

unique_ptr<Expr> Parser::Binary(unique_ptr<Expr> left) {
  assert(!panic_mode_);

  auto op = prev_token_;
  const auto& rule = rules_[static_cast<int>(op.type)];
  auto precedence = static_cast<Precedence>(static_cast<int>(rule.precedence) + 1);
  auto right = Expression(precedence);

  // we either had errors previously (in which case operand can be anything)
  // or we had no errors and operand must be a valid node
  assert(reporter_.HadErrors() || right);

  return CONSTRUCT_NODE(make_unique<BinaryExpr>(move(left), op, move(right)));
}

unique_ptr<Expr> Parser::Unary(bool can_assign) {
  IGNORE(can_assign);
  assert(!panic_mode_);

  auto op = prev_token_;
  auto operand = Expression(Precedence::kUnary);

  // we either had errors previously (in which case operand can be anything)
  // or we had no errors and operand must be a valid node
  assert(reporter_.HadErrors() || operand);

  return CONSTRUCT_NODE(make_unique<UnaryExpr>(op, move(operand)));
}

unique_ptr<Expr> Parser::Identifier(bool can_assign) {
  auto node = make_unique<IdentifierExpr>(prev_token_);

  if (can_assign && MatchToken(TT::kEqual, false)) {
    unique_ptr<Expr> expr;
    PARSE_EXPRESSION(expr, Precedence::kAssign, false);
    return CONSTRUCT_NODE(make_unique<AssignExpr>(nullptr, move(node), move(expr)));
  }

  return move(node);
}

unique_ptr<Expr> Parser::Literal(bool can_assign) {
  IGNORE(can_assign);
  return make_unique<LiteralExpr>(prev_token_);
}

unique_ptr<Expr> Parser::Grouping(bool can_assign) {
  IGNORE(can_assign);
  assert(!panic_mode_);

  unique_ptr<Expr> expr;
  PARSE_EXPRESSION(expr, Precedence::kAssign, true);

  ConsumeToken(TT::kRightParen, true,
      "Missing closing )");

  return CONSTRUCT_NODE(move(expr));
}

// TODO
unique_ptr<Type> Parser::ParseType(bool ignore_eol) {
  ConsumeToken(TT::kIdentifier, ignore_eol, "Invalid type syntax");
  return make_unique<SingleType>(interner_.Intern(prev_token_.lexeme));
}

unique_ptr<Pattern> Parser::ParsePattern(bool ignore_eol) {
  if (MatchToken(TT::kIdentifier, ignore_eol)) {
    auto name = prev_token_;
    unique_ptr<Type> type;
    if (MatchToken(TT::kColon, true)) {
      type = ParseType(true);
    }

    return make_unique<TypedPattern>(name, move(type));
  }

  ParserError("Unexpected token: invalid pattern", prev_token_);
  return nullptr;
}

unique_ptr<AstNode> Parser::Variable() {
  // panic mode must have been cleared up by the caller
  assert(!panic_mode_);

  auto pattern = ParsePattern(false);

  ConsumeToken(TT::kEqual, true, "Initializer expected");

  unique_ptr<Expr> expr;
  PARSE_EXPRESSION(expr, Precedence::kAssign, false);
int
  return CONSTRUCT_NODE(make_unique<VariableStmt>(move(pattern), move(expr)));
}

unique_ptr<Expr> Parser::Block(bool can_assign) {
  IGNORE(can_assign);
  assert(!panic_mode_);
  auto body = Sequence<AstNode>(TT::kEol, TT::kRightBrace, [this] {
    return Statement();
  });
  return CONSTRUCT_NODE(make_unique<BlockExpr>(move(body)));
}

unique_ptr<Expr> Parser::If(bool can_assign) {
  IGNORE(can_assign);
  assert(!panic_mode_);

  ConsumeToken(TT::kLeftParen, false,
      "Missing ( before condition in 'if' expression");

  unique_ptr<Expr> condition, then_branch, else_branch;
  PARSE_EXPRESSION(condition, Precedence::kAssign, true);

  ConsumeToken(TT::kRightParen, true,
      "Missing ) after condition in 'if' expression");

  PARSE_EXPRESSION(then_branch, Precedence::kAssign, false);

  if (MatchToken(TT::kElse, true)) {
    PARSE_EXPRESSION(else_branch, Precedence::kAssign, false);
  }

  return CONSTRUCT_NODE(make_unique<IfExpr>(
      move(condition), move(then_branch), move(else_branch)));
}

unique_ptr<Expr> Parser::While(bool can_assign) {
  IGNORE(can_assign);
  assert(!panic_mode_);

  ConsumeToken(TT::kLeftParen, false,
      "Missing ( before condition in 'if' expression");

  unique_ptr<Expr> condition, body;
  PARSE_EXPRESSION(condition, Precedence::kAssign, true);

  ConsumeToken(TT::kRightParen, true,
      "Missing ) after condition in 'if' expression");

  PARSE_EXPRESSION(body, Precedence::kAssign, false);

  return CONSTRUCT_NODE(make_unique<WhileExpr>(move(condition), move(body)));
}

#undef IGNORE
#undef CONSTRUCT_NODE
#undef PARSE_EXPRESSION

}
