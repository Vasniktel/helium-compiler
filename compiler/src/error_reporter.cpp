//
// Created by vasniktel on 29.08.2019.
//

#include <absl/strings/str_format.h>
#include "error_reporter.hpp"

namespace helium {

using ::std::string;
using ::absl::string_view;
using ::absl::StrAppendFormat;

void ErrorReporter::ErrorAt(string_view msg, int line, int col) {
  StrAppendFormat(&buffer_,
      "Error at %s:%d:%d:\n\t%s\n", file_name_, line, col, msg);
}

void ErrorReporter::ErrorAt(string_view msg, const Token& token) {
  switch (token.type) {
    case TokenType::kEol:
      StrAppendFormat(&buffer_, "Error at end of line in %s:%d:\n\t%s\n",
          file_name_, token.line - 1, msg);
      break;
    case TokenType::kEof:
      StrAppendFormat(&buffer_, "Error at end of file in %s:\n\t%s\n", file_name_, msg);
      break;
    default:
      StrAppendFormat(&buffer_, "Error at '%s' in %s:%d:%d:\n\t%s\n", token.lexeme, file_name_,
          token.line, token.col - static_cast<int>(token.lexeme.size()), msg);
  }
}

void ErrorReporter::Error(string_view msg) {
  StrAppendFormat(&buffer_, "Error in %s:\n\t%s\n", file_name_, msg);
}

}
