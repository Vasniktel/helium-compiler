//
// Created by vasniktel on 29.08.2019.
//

#include <utility>
#include "module.hpp"
#include "parser/parser.hpp"

namespace helium {

using ::std::move;
using ::std::string;

Module::Module(string name, string source)
: name_(move(name)),
  source_(move(source)),
  ast_(),
  reporter_(name_)
{}

bool Module::Parse() {
  auto ast = Parser::Parse(source_, reporter_);
  if (reporter_.HadErrors()) return false;
  ast_ = move(ast);
  return true;
}
}
