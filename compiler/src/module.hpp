//
// Created by vasniktel on 29.08.2019.
//

#ifndef HELIUM_COMPILER_SRC_MODULE_HPP_
#define HELIUM_COMPILER_SRC_MODULE_HPP_

#include <string>
#include <sstream>
#include <vector>
#include "absl/memory/memory.h"
#include "parser/ast.hpp"
#include "error_reporter.hpp"

namespace helium {

// TODO: finish this
class Module final {
 private:
  std::string name_;
  std::string source_;
  std::vector<std::unique_ptr<AstNode>> ast_;
  ErrorReporter reporter_;

 public:
  Module() = delete;
  Module(std::string name, std::string source);

  bool Parse();
  const ErrorReporter& GetReporter() const { return reporter_; }
};

}

#endif //HELIUM_COMPILER_SRC_MODULE_HPP_
