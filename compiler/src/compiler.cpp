//
// Created by vasniktel on 26.08.2019.
//

#include <iostream>
#include <fstream>
#include <utility>
#include <parser/ast_printer.hpp>
#include "parser/parser.hpp"
#include "compiler.hpp"
#include "error_reporter.hpp"

namespace helium {
namespace {

using ::std::vector;
using ::std::string;
using ::std::unique_ptr;
using ::std::move;
using ::absl::string_view;
using ::absl::optional;
using ::absl::nullopt;
using ::absl::make_optional;

bool ReadFile(const string& name, string& data) {
  using ::std::ifstream;
  using ::std::istream_iterator;

  ifstream fin(name);
  if (!fin) return false;

  fin.seekg(0, ::std::ios::end);
  data.reserve(fin.tellg());
  fin.seekg(0, ::std::ios::beg);

  data.assign(istream_iterator<char>{fin}, istream_iterator<char>{});
  return true;
}

}

optional<string> Compiler::FromFile(const string& name, vector<uint8_t>& out) {
  string source;
  if (!ReadFile(name, source))
    return make_optional("Unable to read from file: " + name);
  return Compile(name, source, out);
}

// TODO: remove boilerplate
optional<string> Compiler::Compile(const string& name,
    const string& source, vector<uint8_t>& out) {
  (void)out;
  ErrorReporter reporter(name);
  Interner interner;
  auto ast = Parser::Parse(source, reporter, interner);

  if (reporter.HadErrors()) {
    return make_optional(reporter.GetErrors());
  }

  TypeCheck check(reporter, interner);
  
  for (const auto& node : ast) {
    node->Accept(check);
  }

  if (reporter.HadErrors()) {
    return make_optional(reporter.GetErrors());
  }
  
  AstPrinter printer(true, ::std::cout, interner);

  for (const auto& node : ast) {
    node->Accept(printer);
  }

  return nullopt;
}

}
