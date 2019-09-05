//
// Created by vasniktel on 04.09.2019.
//

#include <gtest/gtest.h>
#include <parser/parser.hpp>
#include <parser/ast_printer.hpp>
#include "absl/strings/string_view.h"

namespace helium {
namespace {

using ::std::stringstream;
using ::absl::string_view;

void ParseTest(string_view input, string_view expected, bool success) {
  ErrorReporter reporter("");
  stringstream ss;
  AstPrinter printer(ss);

  auto ast = Parser::Parse(input, reporter);

  if (success) {
    ASSERT_FALSE(reporter.HadErrors());
  } else {
    ASSERT_TRUE(reporter.HadErrors());
  }

  for (const auto& node : ast) {
    printer.Visit(*node);
  }

  ASSERT_EQ(ss.str(), expected);
}

#define PARSE_SUCCESS(input, expected_ast) \
    ASSERT_NO_FATAL_FAILURE(ParseTest((input), (expected_ast), true))

#define PARSE_FAILURE(input) \
    ASSERT_NO_FATAL_FAILURE(ParseTest((input), "", false))

}

TEST(Lexer, SkipsSpace) {
  PARSE_SUCCESS(" \n\n  \t \t   \r  ", "");
}

TEST(Lexer, IntegerLiteral) {
  PARSE_SUCCESS("0", "(int 0)");
  PARSE_FAILURE("  0024 ");
  PARSE_FAILURE(" 0233   ");
  PARSE_SUCCESS(" \n  \t2342  ", "(int 2342)");
}

TEST(Lexer, RealLiteral) {
  PARSE_SUCCESS("  \n \t 2332.33 ", "(real 2332.33)");
  PARSE_SUCCESS("  234.  \n", "(real 234.)");
  PARSE_FAILURE(" 00.23 ");
  PARSE_FAILURE("  0230. ");
  PARSE_FAILURE(" \n \t \r .44 ");
  PARSE_SUCCESS(" 443.00  ", "(real 443.00)");
}

TEST(Lexer, Identifier) {
  PARSE_SUCCESS(" _  ", "(id _)");
  PARSE_SUCCESS(" mdieZN3920_", "(id mdieZN3920_)");
  PARSE_SUCCESS(" __03  ", "(id __03)");
  PARSE_SUCCESS("  a__0  ", "(id a__0)");
}

TEST(Lexer, CharLiteral) {
  PARSE_SUCCESS("'3'", "(char '3')");
  PARSE_SUCCESS(R"('"')", R"((char '"'))");
  PARSE_SUCCESS(R"('\"')", R"((char '\"'))");
  PARSE_SUCCESS(R"('\'')", R"((char '\''))");
  PARSE_SUCCESS(R"('\"')", R"((char '\"'))");
  PARSE_SUCCESS(R"('\n')", R"((char '\n'))");
  PARSE_SUCCESS(R"('\t')", R"((char '\t'))");
  PARSE_SUCCESS(R"('\r')", R"((char '\r'))");
  PARSE_SUCCESS(R"('\\')", R"((char '\\'))");
  PARSE_SUCCESS("' '", "(char ' ')");
  PARSE_FAILURE("'\n'");
  PARSE_FAILURE("'\t'");
  PARSE_FAILURE("'\r'");
  PARSE_FAILURE("'e");
  PARSE_FAILURE("''");
  PARSE_FAILURE("'wew'");
  PARSE_FAILURE("'w\n'");
  PARSE_FAILURE(R"('\')");
  PARSE_FAILURE("'\\\n'");
  PARSE_FAILURE("'\\\t'");
  PARSE_FAILURE("'\\\r'");
}

TEST(Parser, VarStmt) {
  PARSE_SUCCESS("var k : int \n\n", "(var k : int )");
  PARSE_SUCCESS("var k : sdd = (2 + 22)\n", "(var k : sdd (+ (int 2) (int 22)))");
  PARSE_SUCCESS("var \n qw \n\n : \n q \n = \n 4", "(var qw : q (int 4))");
  PARSE_FAILURE("var 34");
}

}
