//
// Created by vasniktel on 27.08.2019.
//

#ifndef HELIUM_COMPILER_SRC_TOKEN_HPP_
#define HELIUM_COMPILER_SRC_TOKEN_HPP_

#include "absl/strings/string_view.h"

namespace helium {

enum class TokenType {
  kEof,
  kEol,
  kError,

  kChar,
  kInt,
  kReal,
  kString,
  kIdentifier,
  kTrue,
  kFalse,
  kUnit,

  kVar,
  kWhile,
  kIf,
  kElse,

  kPlus,
  kMinus,
  kStar,
  kSlash,
  kLeftParen,
  kRightParen,
  kLeftBrace,
  kRightBrace,
  kEqual,
  kColon
};

class Parser;

struct Token final {
  TokenType type;
  int line;
  int col;
  absl::string_view lexeme;

  Token() = delete;
  static Token ParsedAs(const Parser& parser, TokenType type);
};

}

#endif //HELIUM_COMPILER_SRC_TOKEN_HPP_
