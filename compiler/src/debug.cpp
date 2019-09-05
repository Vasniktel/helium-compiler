//
// Created by vasniktel on 27.08.2019.
//

#ifdef HELIUM_COMPILER_DEBUG

#include "absl/strings/str_format.h"
#include "cassert"
#include "debug.hpp"

namespace helium {

using ::std::ostream;
using ::std::endl;
using ::absl::StreamFormat;
using TT = TokenType;

ostream& operator <<(ostream& os, const Token& token) {
  const char* enum_name = nullptr;

  switch (token.type) {
    case TT::kEof:        enum_name = "EOF"; break;
    case TT::kEol:        enum_name = "EOL"; break;
    case TT::kError:      enum_name = "ERROR"; break;
    case TT::kChar:       enum_name = "CHAR"; break;
    case TT::kInt:        enum_name = "INT"; break;
    case TT::kReal:       enum_name = "REAL"; break;
    case TT::kString:     enum_name = "STRING"; break;
    case TT::kIdentifier: enum_name = "IDENTIFIER"; break;
    case TT::kVar:        enum_name = "VAR"; break;
    case TT::kWhile:      enum_name = "WHILE"; break;
    case TT::kIf:         enum_name = "IF"; break;
    case TT::kElse:       enum_name = "ELSE"; break;
    case TT::kTrue:       enum_name = "TRUE"; break;
    case TT::kFalse:      enum_name = "FALSE"; break;
    case TT::kPlus:       enum_name = "PLUS"; break;
    case TT::kMinus:      enum_name = "MINUS"; break;
    case TT::kStar:       enum_name = "STAR"; break;
    case TT::kSlash:      enum_name = "SLASH"; break;
    case TT::kLeftParen:  enum_name = "LPAREN"; break;
    case TT::kRightParen: enum_name = "RPAREN"; break;
    case TT::kLeftBrace:  enum_name = "LBRACE"; break;
    case TT::kRightBrace: enum_name = "RBRACE"; break;
    case TT::kEqual:      enum_name = "EQUAL"; break;
    case TT::kColon:      enum_name = "COLON"; break;
    default: assert(false && "Unknown enum value (forgot to handle)");
  }

  return os << StreamFormat("[%s <%s> at %d:%d]", enum_name,
      token.type == TT::kEol ? "\\n" : token.lexeme,
      token.line, token.col);
}

}

#endif
