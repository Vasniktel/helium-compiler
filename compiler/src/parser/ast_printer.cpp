//
// Created by vasniktel on 30.08.2019.
//

#include "token.hpp"
#include "ast_printer.hpp"

namespace helium {

using ::std::endl;
using TT = TokenType;

void AstPrinter::Visit(VariableStmt& node) {
  os_ << "(var " << node.name.lexeme << " : " << node.type.lexeme << ' ';
  if (node.expr) Visit(*node.expr);
  os_ << ')';
}

void AstPrinter::Visit(BinaryExpr& node) {
  os_ << '(' << node.op.lexeme << ' ';
  Visit(*node.left);
  os_ << ' ';
  Visit(*node.right);
  os_ << ')';
}

void AstPrinter::Visit(UnaryExpr& node) {
  os_ << '(' << node.op.lexeme << ' ';
  Visit(*node.operand);
  os_ << ')';
}

void AstPrinter::Visit(LiteralExpr& node) {
  auto name = "";
  
  switch (node.value.type) {
    case TokenType::kInt:
      name = "int";
      break;
    case TokenType::kReal:
      name = "real";
      break;
    case TokenType::kIdentifier:
      name = "id";
      break;
    case TokenType::kString:
      name = "string";
      break;
    case TokenType::kChar:
      name = "char";
      break;
    default: name = "lit";
  }
  
  os_ << "(" << name << ' ' << node.value.lexeme << ")";
}

void AstPrinter::Visit(IfExpr& node) {
  os_ << "(if ";
  Visit(*node.condition);
  os_ << " then ";
  Visit(*node.then_branch);
  if (node.else_branch) {
    os_ << " else ";
    Visit(*node.else_branch);
  }

  os_ << ")";
}

void AstPrinter::Visit(WhileExpr& node) {
  os_ << "(while ";
  Visit(*node.condition);
  os_ << " loop ";
  Visit(*node.body);
  os_ << ')';
}

void AstPrinter::Visit(BlockExpr& node) {
  os_ << "(block";

  for (const auto& stmt : node.body) {
    os_ << ' ';
    Visit(*stmt);
  }

  os_ << ')';
}

void AstPrinter::Visit(AssignExpr& node) {
  os_ << "(= ";
  Visit(*node.dest);
  os_ << ' ';
  Visit(*node.expr);
  os_ << ')';
}

}
