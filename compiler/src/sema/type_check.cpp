//
// Created by vasniktel on 08.09.2019.
//

#include <absl/strings/string_view.h>
#include "absl/memory/memory.h"
#include "type_check.hpp"
#include "type.hpp"

namespace helium {

using ::absl::string_view;
using ::absl::make_unique;
using ::std::unique_ptr;
using ::absl::optional;
using ::absl::make_optional;
using ::absl::nullopt;

void TypeCheck::Visit(UnaryExpr& expr) {
  const auto& operand = expr.Operand();
  operand->Accept(*this);

  const auto& type = operand->GetType();
  if (!Is<ErrorType>(type) &&
      !type->Match(kInt.get()) &&
      !type->Match(kReal.get())) {
    reporter_.ErrorAt("Operand type must be Int or Real", expr.Op());
  }

  expr.SetType(type->Copy());

  SetIntrinsic(expr);
}

// TODO: revisit later when operator execution is determined by traits
void TypeCheck::Visit(BinaryExpr& expr) {
  const auto& left = expr.Left();
  const auto& right = expr.Right();
  left->Accept(*this);
  right->Accept(*this);

  const auto& ltype = left->GetType();
  const auto& rtype = right->GetType();
  switch (expr.Op().type) {
    case TokenType::kPlus:
    case TokenType::kMinus:
    case TokenType::kStar:
    case TokenType::kSlash: {
      // if any of the operands' types is an error -
      // bail out without checking the other one
      if (Is<ErrorType>(ltype) || Is<ErrorType>(rtype)) {
        expr.SetType(make_unique<ErrorType>());
        return;
      }

      bool error = false;
      if (!Is<SingleType>(ltype) || !Is<SingleType>(rtype)) {
        reporter_.ErrorAt("Wrong operand types for binary expr", expr.Op());
        error = true;
      }

      if (!error && !ltype->Match(rtype)) {
        reporter_.ErrorAt("Operands must have the same type", expr.Op());
        error = true;
      }

      if (!error && !ltype->Match(kInt.get()) && !ltype->Match(kReal.get())) {
        reporter_.ErrorAt("Operands must be either ints or reals", expr.Op());
        error = true;
      }

      if (error) expr.SetType(make_unique<ErrorType>());
      else expr.SetType(ltype->Copy());

      SetIntrinsic(expr);
      return;
    }

    default: assert(false && "Token is not a binary op");
  }
}

// TODO: find a better way to deal with intrinsics
void TypeCheck::SetIntrinsic(BinaryExpr& expr) const {
#define CHECK_TYPE(if_int, if_real) \
    do { \
      if (expr.GetType()->Match(kInt.get())) { \
        expr.SetIntrinsic(if_int); \
      } else if (expr.GetType()->Match(kReal.get())) { \
        expr.SetIntrinsic(if_real); \
      } \
    } while (false)

  switch (expr.Op().type) {
    case TokenType::kPlus:
      CHECK_TYPE(IntrinsicOp::kIntAdd, IntrinsicOp::kRealAdd);
      break;
    case TokenType::kMinus:
      CHECK_TYPE(IntrinsicOp::kIntSub, IntrinsicOp::kRealSub);
      break;
    case TokenType::kStar:
      CHECK_TYPE(IntrinsicOp::kIntMul, IntrinsicOp::kRealMul);
      break;
    case TokenType::kSlash:
      CHECK_TYPE(IntrinsicOp::kIntDiv, IntrinsicOp::kRealDiv);
      break;
    default:
      assert(false && "Unreachable");
  }

#undef CHECK_TYPE
}

void TypeCheck::SetIntrinsic(UnaryExpr& expr) const {
#define CHECK_TYPE(if_int, if_real) \
    do { \
      if (expr.GetType()->Match(kInt.get())) { \
        expr.SetIntrinsic(if_int); \
      } else if (expr.GetType()->Match(kReal.get())) { \
        expr.SetIntrinsic(if_real); \
      } \
    } while (false)

  switch (expr.Op().type) {
    case TokenType::kMinus:
      CHECK_TYPE(IntrinsicOp::kIntNeg, IntrinsicOp::kRealNeg);
      break;
    case TokenType::kPlus: break;
    default: assert(false && "Not an unary op");
  }

#undef CHECK_TYPE
}

optional<const Type*> TypeCheck::Lookup(absl::string_view name)  {
  for (auto check = this; check; check = check->parent_) {
    auto it = check->locals_.find(name);
    if (it != check->locals_.end()) return make_optional(it->second);
  }

  return nullopt;
}

void PatternMatcher::Visit(TypedPattern& pattern) {
  const Type* var = nullptr;
  const auto& decl = pattern.GetType();

  assert(type_);
  if (decl && decl->Match(type_)) var = decl.get();
  else if (!decl) var = type_;
  else check_.reporter_.ErrorAt("Incompatible type decl", pattern.GetName());

  if (check_.locals_.contains(pattern.GetName().lexeme)) {
    check_.reporter_.ErrorAt("Redefinition of a name is not allowed", pattern.GetName());
  } else {
    check_.locals_[pattern.GetName().lexeme] = var;
  }
}

void TypeCheck::Visit(VariableStmt& stmt) {
  stmt.GetExpr()->Accept(*this);

  const Type* expr_type = stmt.GetExpr()->GetType().get();
  PatternMatcher match(expr_type, *this);

  stmt.GetPattern()->Accept(match);
}

void TypeCheck::Visit(LiteralExpr& expr) {
  switch (expr.Value().type) {
    case TokenType::kInt:
      expr.SetType(kInt->Copy());
      return;
    case TokenType::kReal:
      expr.SetType(kReal->Copy());
      return;
    case TokenType::kChar:
      expr.SetType(kChar->Copy());
      return;
    case TokenType::kString:
      assert(false && "Unimplemented");
      return;
    case TokenType::kTrue:
    case TokenType::kFalse:
      expr.SetType(kBool->Copy());
      return;
    case TokenType::kUnit:
      expr.SetType(kUnit->Copy());
      return;
    default: assert(false && "Invalid literal expr");
  }
}

void TypeCheck::Visit(IdentifierExpr& expr) {
  if (auto var = Lookup(expr.Value().lexeme)) {
    if (*var) expr.SetType((*var)->Copy());
    else expr.SetType(make_unique<ErrorType>());
  } else {
    reporter_.ErrorAt("Undeclared identifier", expr.Value());
    expr.SetType(make_unique<ErrorType>());
  }
}

void TypeCheck::Visit(IfExpr& expr) {
  expr.Cond()->Accept(*this);
  const auto& cond_type = expr.Cond()->GetType();

  bool cond_good = !Is<ErrorType>(cond_type);
  if (cond_good && !cond_type->Match(kBool.get())) {
    reporter_.ErrorAt("Type of condition must be Bool", 0, 0); // TODO
    cond_good = false;
  }

  expr.Then()->Accept(*this);
  const auto& then_type = expr.Then()->GetType();

  if (!expr.Else()) {
    if (cond_good && !Is<ErrorType>(then_type)) {
      expr.SetType(kUnit->Copy());
    } else {
      expr.SetType(make_unique<ErrorType>());
    }

    return;
  }

  expr.Else()->Accept(*this);
  const auto& else_type = expr.Else()->GetType();

  if (Is<ErrorType>(then_type) || Is<ErrorType>(else_type)) {
    cond_good = false;
  }

  if (cond_good && !then_type->Match(else_type)) {
    reporter_.ErrorAt("Two clauses of an 'if' expression have different types", 0, 0);
    cond_good = false;
  }

  if (cond_good) expr.SetType(then_type->Copy());
  else expr.SetType(make_unique<ErrorType>());
}

// TODO: define the order of assignment execution
void TypeCheck::Visit(AssignExpr& expr) {
  assert(!expr.Receiver() && "Unimplemented"); // TODO: fix when classes are introduced
  expr.Expr()->Accept(*this);

  auto dest_type_opt = Lookup(expr.Name().lexeme);
  if (!dest_type_opt) {
    reporter_.ErrorAt("Undefined name", expr.Name());
    expr.SetType(make_unique<ErrorType>());
    return;
  }

  auto dest_type = *dest_type_opt;
  const auto& value_type = expr.Expr()->GetType();
  if (!Is<ErrorType>(dest_type) || !Is<ErrorType>(value_type)) {
    expr.SetType(make_unique<ErrorType>());
    return;
  }

  if (!dest_type->Match(value_type.get())) {
    reporter_.ErrorAt("Invalid assignment types", 0, 0);
    expr.SetType(make_unique<ErrorType>());
    return;
  }

  expr.SetType(kUnit->Copy());
}

void TypeCheck::Visit(BlockExpr& expr) {
  Type* result = kUnit.get();

  TypeCheck scope(this);
  for (const auto& stmt : expr.Body()) {
    stmt->Accept(scope);
    if (!Is<ErrorType>(result)) {
      if (stmt->IsExpr()) {
        result = static_cast<const Expr*>(stmt.get())->GetType().get();
      } else result = kUnit.get();
    }
  }

  if (result) expr.SetType(result->Copy());
}

void TypeCheck::Visit(WhileExpr& expr) {
  expr.Cond()->Accept(*this);
  const auto& cond_type = expr.Cond()->GetType();

  if (!Is<ErrorType>(cond_type) && !cond_type->Match(kBool.get())) {
    // TODO
    reporter_.ErrorAt("Type of condition expression must be Bool", 0, 0);
  }

  expr.Body()->Accept(*this);
  const auto& body_type = expr.Body()->GetType();
  if (!Is<ErrorType>(cond_type) && !Is<ErrorType>(body_type)) {
    expr.SetType(kUnit->Copy());
  } else {
    expr.SetType(make_unique<ErrorType>());
  }
}

}
