//
// Created by vasniktel on 30.08.2019.
//

#include "token.hpp"
#include "ast_printer.hpp"

namespace helium {

using ::std::endl;
using TT = TokenType;

void AstPrinter::Visit(VariableStmt& node) {
  os_ << "(var ";
  node.GetPattern()->Accept(*this);
  if (node.GetExpr()) {
    os_ << ' ';
    node.GetExpr()->Accept(*this);
  }
  os_ << ')';
}

void AstPrinter::Visit(BinaryExpr& node) {
  os_ << '(' << node.Op().lexeme;
  
  if (typed_) {
    os_ << ':';
    node.GetType()->Accept(*this);
  }
  
  os_ << ' ';
  node.Left()->Accept(*this);
  os_ << ' ';
  node.Right()->Accept(*this);
  os_ << ')';
}

void AstPrinter::Visit(UnaryExpr& node) {
  os_ << '(' << node.Op().lexeme;

  if (typed_) {
    os_ << ':';
    node.GetType()->Accept(*this);
  }

  os_ << ' ';
  node.Operand()->Accept(*this);
  os_ << ')';
}

void AstPrinter::Visit(LiteralExpr& node) {
  auto name = "";
  
  switch (node.Value().type) {
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
  
  os_ << "(" << name;

  if (typed_) {
    os_ << ':';
    node.GetType()->Accept(*this);
  }

  os_ << ' ' << node.Value().lexeme << ")";
}

void AstPrinter::Visit(IfExpr& node) {
  os_ << "(if";

  if (typed_) {
    os_ << ':';
    node.GetType()->Accept(*this);
  }

  os_ << ' ';
  node.Cond()->Accept(*this);
  os_ << " then ";
  node.Then()->Accept(*this);
  if (node.Else()) {
    os_ << " else ";
    node.Else()->Accept(*this);
  }

  os_ << ")";
}

void AstPrinter::Visit(WhileExpr& node) {
  os_ << "(while";

  if (typed_) {
    os_ << ':';
    node.GetType()->Accept(*this);
  }

  os_ << ' ';
  node.Cond()->Accept(*this);
  os_ << " loop ";
  node.Body()->Accept(*this);
  os_ << ')';
}

void AstPrinter::Visit(BlockExpr& node) {
  os_ << "(block";

  if (typed_) {
    os_ << ':';
    node.GetType()->Accept(*this);
  }
  
  for (const auto& stmt : node.Body()) {
    os_ << ' ';
    stmt->Accept(*this);
  }

  os_ << ')';
}

void AstPrinter::Visit(AssignExpr& node) {
  os_ << "(=";
  
  if (typed_) {
    os_ << ':';
    node.GetType()->Accept(*this);
  }

  os_ << ' ';
  node.Dest()->Accept(*this);
  os_ << ' ';
  node.Expr()->Accept(*this);
  os_ << ')';
}

void AstPrinter::Visit(SingleType& type) {
  os_ << *interner_.LookUp(type.GetTypeData()); // optional should not be empty
}

void AstPrinter::Visit(TypedPattern& pattern) {
  os_ << pattern.GetName().lexeme;
  if (pattern.GetType()) {
    os_ << " : ";
    pattern.GetType()->Accept(*this);
  }
}

}
