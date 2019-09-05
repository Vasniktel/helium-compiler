//
// Created by vasniktel on 29.08.2019.
//

#ifndef HELIUM_COMPILER_SRC_ERROR_REPORTER_HPP_
#define HELIUM_COMPILER_SRC_ERROR_REPORTER_HPP_

#include <string>
#include <utility>
#include "absl/strings/string_view.h"
#include "parser/token.hpp"

namespace helium {

// reports errors found during module compilation,
// must ensure that module object outlives this one
class ErrorReporter {
 private:
  absl::string_view file_name_;
  std::string buffer_;

 public:
  ErrorReporter() = delete;
  explicit ErrorReporter(absl::string_view file_name)
  : file_name_(file_name),
    buffer_()
  {}

  bool HadErrors() const { return !buffer_.empty(); }
  const std::string& GetErrors() const { return buffer_; }

  // TODO: think about better formatting
  void ErrorAt(absl::string_view msg, int line, int col);
  void ErrorAt(absl::string_view msg, const Token& token);
};

}

#endif //HELIUM_COMPILER_SRC_ERROR_REPORTER_HPP_
