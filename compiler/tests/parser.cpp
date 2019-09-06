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
    EXPECT_FALSE(reporter.HadErrors());
  } else {
    EXPECT_TRUE(reporter.HadErrors());
  }

  for (const auto& node : ast) {
    printer.Visit(*node);
  }

  EXPECT_EQ(ss.str(), expected);
}

#define PARSE_SUCCESS(input, expected_ast) \
    EXPECT_NO_FATAL_FAILURE(ParseTest((input), (expected_ast), true))

#define PARSE_FAILURE(input) \
    EXPECT_NO_FATAL_FAILURE(ParseTest((input), "", false))

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
  PARSE_FAILURE("'\nq'");
  PARSE_FAILURE(R"('\')");
  PARSE_FAILURE("'\\\n'");
  PARSE_FAILURE("'\\\t'");
  PARSE_FAILURE("'\\\r'");
}

TEST(Lexer, StringLiteral) {
  PARSE_SUCCESS(R"("")", R"((string ""))");
  PARSE_SUCCESS(R"("o3v _@\n\t\r\a\f\\")", R"((string "o3v _@\n\t\r\a\f\\"))");
  PARSE_SUCCESS(R"("\"")", R"((string "\""))");
  PARSE_SUCCESS(R"("\'")", R"((string "\'"))");
  PARSE_SUCCESS("\"\n\t\r\"", "(string \"\n\t\r\")");
  PARSE_SUCCESS(R"("\0")", R"((string "\0"))");
  PARSE_FAILURE(R"("sd)");
  PARSE_FAILURE("\"\\\n\"");
  PARSE_FAILURE("\"\\\t\"");
  PARSE_FAILURE("\"\\\r\"");
  PARSE_FAILURE(R"("\")");
}

TEST(Parser, VarStmt) {
  PARSE_SUCCESS("var k : int \n\n", "(var k : int )");
  PARSE_SUCCESS("var k : sdd = (2 + 22)\n", "(var k : sdd (+ (int 2) (int 22)))");
  PARSE_SUCCESS("var qw \n\n : \n q \n = 4", "(var qw : q (int 4))");
  PARSE_FAILURE("var 34");
}

TEST(Parser, IfExpr) {
  PARSE_SUCCESS("if (1) {}", "(if (int 1) then (block))");
  PARSE_SUCCESS("if ({}) 0 else if (1) {} else {}",
      "(if (block) then (int 0) else (if (int 1) then (block) else (block)))");
  PARSE_SUCCESS("if  (\n1\n)1else {}",
      "(if (int 1) then (int 1) else (block))");
  PARSE_SUCCESS("if (0) if (1) 1 else 2",
      "(if (int 0) then (if (int 1) then (int 1) else (int 2)))");
  PARSE_SUCCESS("if (0) {} \n else {}", "(if (int 0) then (block) else (block))");
  PARSE_FAILURE("if (0) \n");
  PARSE_FAILURE("if () {}");
  PARSE_FAILURE("if (0) {} else ");
  PARSE_FAILURE("if {}");
  PARSE_FAILURE("if \n (0) {}");
  PARSE_FAILURE("if (0) \n {}");
  PARSE_FAILURE("if (0) {} else \n {}");
}

TEST(Parser, WhileExpr) {
  PARSE_SUCCESS("while (0) {}", "(while (int 0) loop (block))");
  PARSE_SUCCESS("while (\n0\n) 1", "(while (int 0) loop (int 1))");
  PARSE_FAILURE("while \n (0) 1");
  PARSE_FAILURE("while (1) \n {}");
  PARSE_FAILURE("while (1) ");
  PARSE_FAILURE("while () {}");
}

TEST(Parser, BlockExpr) {
  PARSE_SUCCESS("{}", "(block)");
  PARSE_SUCCESS("{1}", "(block (int 1))");
  PARSE_SUCCESS("{\n\n}", "(block)");
  PARSE_SUCCESS("{{}}", "(block (block))");
  PARSE_SUCCESS("{ \n a = 4 \n}", "(block (= (id a) (int 4)))");
  PARSE_SUCCESS("{\n 3 \n\n 4 \n\n }", "(block (int 3) (int 4))");
  PARSE_SUCCESS("{\n 3 \n 4 }", "(block (int 3) (int 4))");
  PARSE_SUCCESS("{ 3 \n 4 \n\n }", "(block (int 3) (int 4))");
  PARSE_FAILURE("{\n 3  4 \n\n }");
  PARSE_FAILURE("{\n");
  PARSE_FAILURE("{ hello\n world");
}

TEST(Parser, Unexpected) {
  PARSE_FAILURE("}");
  PARSE_FAILURE("=");
  PARSE_FAILURE("if if (0) {}");
}

TEST(Parser, Precedence) {
  PARSE_SUCCESS("2 + 3 * 1", "(+ (int 2) (* (int 3) (int 1)))");
  PARSE_SUCCESS("2 - 3 * 1", "(- (int 2) (* (int 3) (int 1)))");
  PARSE_SUCCESS("3 / 1 - 2", "(- (/ (int 3) (int 1)) (int 2))");
  PARSE_SUCCESS("-+3", "(- (+ (int 3)))");
  PARSE_SUCCESS("2--1", "(- (int 2) (- (int 1)))");
  PARSE_SUCCESS("2 + (k = 1)", "(+ (int 2) (= (id k) (int 1)))");
  PARSE_SUCCESS("k = a = b", "(= (id k) (= (id a) (id b)))");
  PARSE_SUCCESS("k = -a", "(= (id k) (- (id a)))");
  PARSE_SUCCESS("-(k = 1)", "(- (= (id k) (int 1)))");
  PARSE_SUCCESS("2 + {}", "(+ (int 2) (block))");
  PARSE_SUCCESS("2 + if (0) {}", "(+ (int 2) (if (int 0) then (block)))");
  PARSE_SUCCESS("-if (0) {} else {} - 2",
      "(- (if (int 0) then (block) else (- (block) (int 2))))");
  PARSE_SUCCESS("2 * while (0) 1", "(* (int 2) (while (int 0) loop (int 1)))");
  PARSE_SUCCESS("if (0) {} - 1", "(if (int 0) then (- (block) (int 1)))");
  PARSE_SUCCESS("(if (0) {}) - 1", "(- (if (int 0) then (block)) (int 1))");
  PARSE_FAILURE("1 = a");
  PARSE_FAILURE("a + b = c");
}

}
