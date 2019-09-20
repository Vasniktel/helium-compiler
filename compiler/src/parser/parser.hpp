//
// Created by vasniktel on 27.08.2019.
//

#ifndef HELIUM_COMPILER_SRC_PARSER_HPP_
#define HELIUM_COMPILER_SRC_PARSER_HPP_

#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
#include "absl/memory/memory.h"
#include "token.hpp"
#include "ast.hpp"
#include "error_reporter.hpp"

namespace helium {

class Parser final {
 private:
  friend struct Token;

  absl::string_view source_;
  int size_;
  int curr_char_;
  int start_;
  int line_;
  int col_;
  Token curr_token_;
  Token prev_token_;
  bool panic_mode_;
  ErrorReporter& reporter_;
  Interner& interner_;

 public:
  Parser() = delete;
  static std::vector<std::unique_ptr<AstNode>> Parse(
      absl::string_view source, ErrorReporter& reporter, Interner& interner);

 private:
  enum class Precedence;
  struct Rule;
  static const Rule rules_[];

  explicit Parser(absl::string_view source, ErrorReporter& reporter, Interner& interner);

  void SkipSpace();
  void MakeToken(TokenType type);
  char AdvanceChar();
  void ParseComment();
  void ParseString();
  void ParseChar();
  bool IsValidSymbol();
  bool IsEscape();
  void ParseNumber();
  void ParseIdentifier();

  bool IsAtEnd() { return curr_char_ == size_; }
  char PeekChar() { return source_[curr_char_]; }
  char PeekNextChar() { return IsAtEnd() ? '\0' : source_[curr_char_ + 1]; }

  Token& NextToken();
  bool MatchToken(TokenType type, bool ignore_eol);
  bool ConsumeToken(TokenType type, bool ignore_eol, absl::string_view msg);
  void SkipEolTokens();

  std::unique_ptr<AstNode> Statement();
  std::unique_ptr<AstNode> Variable();

  std::unique_ptr<Expr> Expression(Precedence precedence);
  std::unique_ptr<Expr> Binary(std::unique_ptr<Expr> left);
  std::unique_ptr<Expr> Unary(bool can_assign);
  std::unique_ptr<Expr> Literal(bool can_assign);
  std::unique_ptr<Expr> While(bool can_assign);
  std::unique_ptr<Expr> If(bool can_assign);
  std::unique_ptr<Expr> Grouping(bool can_assign);
  std::unique_ptr<Expr> Block(bool can_assign);

  template <typename T, typename F>
  std::vector<std::unique_ptr<T>> Sequence(TokenType separator, TokenType closing, F parser);

  std::unique_ptr<Pattern> ParsePattern(bool ignore_eol);
  std::unique_ptr<Type> ParseType(bool ignore_eol);

  // TODO: make better error reporting
  void LexError(absl::string_view msg, int line, int col);
  void ParserError(absl::string_view msg, const Token& token);
};

}

#endif //HELIUM_COMPILER_SRC_PARSER_HPP_
