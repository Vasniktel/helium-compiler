//
// Created by vasniktel on 01.10.2019.
//

#ifndef HELIUM_COMPILER_SRC_PARSER_VISITOR_HPP_
#define HELIUM_COMPILER_SRC_PARSER_VISITOR_HPP_

namespace helium {

class AstNode;
class Expr;
class VariableStmt;
class BinaryExpr;
class AssignExpr;
class UnaryExpr;
class LiteralExpr;
class IdentifierExpr;
class BlockExpr;
class IfExpr;
class WhileExpr;
class Pattern;
class TypedPattern;

class AstVisitor {
 public:
  virtual ~AstVisitor() = default;
  virtual void Visit(VariableStmt& node) = 0;
  virtual void Visit(BinaryExpr& node) = 0;
  virtual void Visit(UnaryExpr& node) = 0;
  virtual void Visit(LiteralExpr& node) = 0;
  virtual void Visit(IdentifierExpr& node) = 0;
  virtual void Visit(BlockExpr& node) = 0;
  virtual void Visit(IfExpr& node) = 0;
  virtual void Visit(WhileExpr& node) = 0;
  virtual void Visit(AssignExpr& node) = 0;
};

class PatternVisitor {
 public:
  virtual ~PatternVisitor() = default;
  virtual void Visit(TypedPattern& type) = 0;
};

}

#endif //HELIUM_COMPILER_SRC_PARSER_VISITOR_HPP_
