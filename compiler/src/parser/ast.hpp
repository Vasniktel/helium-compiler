//
// Created by vasniktel on 27.08.2019.
//

#ifndef HELIUM_COMPILER_SRC_AST_HPP_
#define HELIUM_COMPILER_SRC_AST_HPP_

#include <vector>
#include <utility>
#include <sema/type.hpp>
#include "absl/memory/memory.h"
#include "token.hpp"
#include "visitor.hpp"

namespace helium {

enum class IntrinsicOp {
  kNone, // used in constructor
  kRealAdd,
  kIntAdd,
  kRealSub,
  kIntSub,
  kRealMul,
  kIntMul,
  kRealDiv,
  kIntDiv,
  kRealNeg,
  kIntNeg
};

class Pattern {
 public:
  virtual ~Pattern() = default;
  virtual void Accept(PatternVisitor& visitor) = 0;
};

class TypedPattern : public Pattern {
  Token name_;
  ::std::unique_ptr<Type> type_; // Might be null, in which case type is inferred

 public:
  TypedPattern() = delete;
  TypedPattern(const Token& name, ::std::unique_ptr<Type> type)
      : name_(name),
        type_(::std::move(type))
  {}

  void Accept(PatternVisitor& visitor) override {
    visitor.Visit(*this);
  }

  const ::std::unique_ptr<Type>& GetType() const {
    return type_;
  }

  const Token& GetName() const {
    return name_;
  }
};

class AstNode {
 public:
  virtual ~AstNode() = default;
  virtual void Accept(AstVisitor& visitor) = 0;
  virtual bool IsExpr() const { return false; }
};

using AstTree = ::std::vector<::std::unique_ptr<AstNode>>;

class Expr : public AstNode {
 protected:
  ::std::unique_ptr<Type> type_;

 public:
  virtual const ::std::unique_ptr<Type>& GetType() const {
    return type_;
  }

  bool IsExpr() const final { return true; }

  virtual void SetType(::std::unique_ptr<Type> type) {
    type_ = ::std::move(type);
  }
};

class VariableStmt final : public AstNode {
  ::std::unique_ptr<Pattern> pattern_;
  ::std::unique_ptr<Expr> expr_;

 public:
  VariableStmt() = delete;
  VariableStmt(::std::unique_ptr<Pattern> pattern, ::std::unique_ptr<Expr> expr)
  : pattern_(::std::move(pattern)),
    expr_(::std::move(expr))
  {}

  const ::std::unique_ptr<Pattern>& GetPattern() const {
    return pattern_;
  }

  const ::std::unique_ptr<Expr>& GetExpr() const {
    return expr_;
  }

  void Accept(AstVisitor& visitor) override {
    visitor.Visit(*this);
  }
};

class BinaryExpr final : public Expr {
  ::std::unique_ptr<Expr> left_;
  Token op_;
  ::std::unique_ptr<Expr> right_;
  IntrinsicOp intrinsic_;

 public:
  BinaryExpr() = delete;
  BinaryExpr(::std::unique_ptr<Expr> left, Token op, ::std::unique_ptr<Expr> right)
  : left_(::std::move(left)),
    op_(op),
    right_(::std::move(right)),
    intrinsic_(IntrinsicOp::kNone)
  {}

  const ::std::unique_ptr<Expr>& Left() const { return left_; }
  const Token& Op() const { return op_; }
  const ::std::unique_ptr<Expr>& Right() const { return right_; }

  IntrinsicOp GetIntrinsic() const { return intrinsic_; }
  void SetIntrinsic(IntrinsicOp value) { intrinsic_ = value; }

  void Accept(AstVisitor& visitor) override {
    visitor.Visit(*this);
  }
};

class AssignExpr final : public Expr {
  ::std::unique_ptr<Expr> receiver_;
  Token name_;
  ::std::unique_ptr<Expr> expr_;

 public:
  AssignExpr() = delete;
  AssignExpr(::std::unique_ptr<Expr> receiver, const Token& name, ::std::unique_ptr<Expr> expr)
  : receiver_(::std::move(receiver)),
    name_(name),
    expr_(::std::move(expr))
  {}

  const ::std::unique_ptr<Expr>& Receiver() const {
    return receiver_;
  }

  const Token& Name() const {
    return name_;
  }

  const ::std::unique_ptr<Expr>& Expr() const {
    return expr_;
  }

  void Accept(AstVisitor& visitor) override {
    visitor.Visit(*this);
  }
};

class UnaryExpr final : public Expr {
  Token op_;
  ::std::unique_ptr<Expr> operand_;
  IntrinsicOp intrinsic_;

 public:
  UnaryExpr() = delete;
  UnaryExpr(Token op, ::std::unique_ptr<Expr> operand)
  : op_(op),
    operand_(::std::move(operand)),
    intrinsic_(IntrinsicOp::kNone)
  {}

  const Token& Op() const { return op_; }
  const ::std::unique_ptr<Expr>& Operand() const { return operand_; }

  IntrinsicOp GetIntrinsic() const { return intrinsic_; }
  void SetIntrinsic(IntrinsicOp value) { intrinsic_ = value; }

  void  Accept(AstVisitor& visitor) override {
    visitor.Visit(*this);
  }
};

// One of:
//  kChar,
//  kInt,
//  kReal,
//  kString,
//  kTrue,
//  kFalse,
//  kUnit
class LiteralExpr final : public Expr {
  Token value_;

 public:
  LiteralExpr() = delete;
  explicit LiteralExpr(Token value)
  : value_(value)
  {}

  const Token& Value() const { return value_; }

  void Accept(AstVisitor& visitor) override {
    visitor.Visit(*this);
  }
};

class IdentifierExpr final : public Expr {
  Token value_;

  // depth in the locals array in vm
  // TODO: reconsider this when functions are added
  uint16_t local_depth_;

 public:
  IdentifierExpr() = delete;
  explicit IdentifierExpr(const Token& value)
  : value_(value),
    local_depth_(0)
  {}

  const Token& Value() const { return value_; }

  uint16_t GetDepth() const { return local_depth_; }
  void SetDepth(uint16_t depth) { local_depth_ = depth; }

  void Accept(AstVisitor& visitor) override {
    visitor.Visit(*this);
  }
};

class BlockExpr final : public Expr {
  // TODO: decide to use absl::inlined_vector
  ::std::vector<::std::unique_ptr<AstNode>> body_;

 public:
  BlockExpr() = delete;
  explicit BlockExpr(::std::vector<::std::unique_ptr<AstNode>>&& body)
  : body_(::std::move(body))
  {}

  const ::std::vector<::std::unique_ptr<AstNode>>& Body() const {
    return body_;
  }

  void Accept(AstVisitor& visitor) override {
    visitor.Visit(*this);
  }
};

class IfExpr final : public Expr {
  ::std::unique_ptr<Expr> cond_;
  ::std::unique_ptr<Expr> then_;
  ::std::unique_ptr<Expr> else_;

 public:
  IfExpr() = delete;
  IfExpr(::std::unique_ptr<Expr> condition, ::std::unique_ptr<Expr> then_branch,
         ::std::unique_ptr<Expr> else_branch)
  : cond_(::std::move(condition)),
    then_(::std::move(then_branch)),
    else_(::std::move(else_branch))
  {}

  const ::std::unique_ptr<Expr>& Cond() const { return cond_; }
  const ::std::unique_ptr<Expr>& Then() const { return then_; }
  const ::std::unique_ptr<Expr>& Else() const { return else_; }

  void Accept(AstVisitor& visitor) override {
    visitor.Visit(*this);
  }
};

class WhileExpr final : public Expr {
  ::std::unique_ptr<Expr> cond_;
  ::std::unique_ptr<Expr> body_;

 public:
  WhileExpr() = delete;
  WhileExpr(::std::unique_ptr<Expr> condition, ::std::unique_ptr<Expr> body)
  : cond_(::std::move(condition)),
    body_(::std::move(body))
  {}

  const ::std::unique_ptr<Expr>& Cond() const { return cond_; }
  const ::std::unique_ptr<Expr>& Body() const { return body_; }

  void Accept(AstVisitor& visitor) override {
    visitor.Visit(*this);
  }
};

}

#endif //HELIUM_COMPILER_SRC_AST_HPP_
