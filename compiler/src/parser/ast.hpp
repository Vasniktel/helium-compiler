//
// Created by vasniktel on 27.08.2019.
//

#ifndef HELIUM_COMPILER_SRC_AST_HPP_
#define HELIUM_COMPILER_SRC_AST_HPP_

#include <vector>
#include <utility>
#include "absl/memory/memory.h"
#include "token.hpp"

namespace helium {

struct AstNode;
struct Expr;
struct VariableStmt;
struct BinaryExpr;
struct AssignExpr;
struct UnaryExpr;
struct LiteralExpr;
struct BlockExpr;
struct IfExpr;
struct WhileExpr;

class AstVisitor;

struct AstNode {
  virtual ~AstNode() = default;
  virtual void Accept(AstVisitor& visitor) = 0;
};

class AstVisitor {
 public:
  virtual ~AstVisitor() = default;
  virtual void Visit(VariableStmt&) = 0;
  virtual void Visit(BinaryExpr&) = 0;
  virtual void Visit(UnaryExpr&) = 0;
  virtual void Visit(LiteralExpr&) = 0;
  virtual void Visit(BlockExpr&) = 0;
  virtual void Visit(IfExpr&) = 0;
  virtual void Visit(WhileExpr&) = 0;
  virtual void Visit(AssignExpr&) = 0;
  virtual void Visit(AstNode& node) {
    node.Accept(*this);
  }
};

struct Expr : public AstNode {
};

struct VariableStmt final : public AstNode {
  Token name;
  Token type; // TODO: implement support for type inference
  std::unique_ptr<Expr> expr;

  VariableStmt() = delete;
  VariableStmt(Token name, Token type, std::unique_ptr<Expr> expr)
  : name(name),
    type(type),
    expr(std::move(expr))
  {}

  void Accept(AstVisitor& visitor) override {
    visitor.Visit(*this);
  }
};

struct BinaryExpr final : public Expr {
  std::unique_ptr<Expr> left;
  Token op;
  std::unique_ptr<Expr> right;

  BinaryExpr() = delete;
  BinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
  : left(std::move(left)),
    op(op),
    right(std::move(right))
  {}

  void Accept(AstVisitor& visitor) override {
    visitor.Visit(*this);
  }
};

struct AssignExpr final : public Expr {
  std::unique_ptr<Expr> dest;
  std::unique_ptr<Expr> expr;

  AssignExpr() = delete;
  AssignExpr(std::unique_ptr<Expr> dest, std::unique_ptr<Expr> expr)
  : dest(std::move(dest)),
    expr(std::move(expr))
  {}

  void Accept(AstVisitor& visitor) override {
    visitor.Visit(*this);
  }
};

struct UnaryExpr final : public Expr {
  Token op;
  std::unique_ptr<Expr> operand;

  UnaryExpr() = delete;
  UnaryExpr(Token op, std::unique_ptr<Expr> operand)
  : op(op),
    operand(std::move(operand))
  {}

  void Accept(AstVisitor& visitor) override {
    visitor.Visit(*this);
  }
};

struct LiteralExpr final : public Expr {
  Token value;

  LiteralExpr() = delete;
  explicit LiteralExpr(Token value)
  : value(value)
  {}

  void Accept(AstVisitor& visitor) override {
    visitor.Visit(*this);
  }
};

struct BlockExpr final : public Expr {
  // TODO: decide to use absl::inlined_vector
  std::vector<std::unique_ptr<AstNode>> body;

  BlockExpr() = delete;
  explicit BlockExpr(std::vector<std::unique_ptr<AstNode>>&& body)
  : body(std::move(body))
  {}

  void Accept(AstVisitor& visitor) override {
    visitor.Visit(*this);
  }
};

struct IfExpr final : public Expr {
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Expr> then_branch;
  std::unique_ptr<Expr> else_branch;

  IfExpr() = delete;
  IfExpr(std::unique_ptr<Expr> condition, std::unique_ptr<Expr> then_branch,
         std::unique_ptr<Expr> else_branch)
  : condition(std::move(condition)),
    then_branch(std::move(then_branch)),
    else_branch(std::move(else_branch))
  {}

  void Accept(AstVisitor& visitor) override {
    visitor.Visit(*this);
  }
};

struct WhileExpr final : public Expr {
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Expr> body;

  WhileExpr() = delete;
  WhileExpr(std::unique_ptr<Expr> condition, std::unique_ptr<Expr> body)
  : condition(std::move(condition)),
    body(std::move(body))
  {}

  void Accept(AstVisitor& visitor) override {
    visitor.Visit(*this);
  }
};

}

#endif //HELIUM_COMPILER_SRC_AST_HPP_
