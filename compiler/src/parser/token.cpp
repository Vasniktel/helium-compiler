//
// Created by vasniktel on 27.08.2019.
//

#include "token.hpp"
#include "parser.hpp"

namespace helium {

Token Token::ParsedAs(const Parser& parser, TokenType type) {
  int start = parser.start_;
  int length = parser.curr_char_ - start;
  auto lexeme = parser.source_.substr(start, length);
  int line = parser.line_;
  int col = parser.col_;
  return {type, line, col, lexeme};
}

}
