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
  if (type && !type->Match(kInt.get()) && !type->Match(kReal.get())) {
    reporter_.ErrorAt("Operand type must be Int or Real", expr.Op());
    return;
  }

  expr.SetType(type->Copy());
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
      if (!ltype || !rtype) return;
      if (!Is<SingleType>(ltype.get()) || !Is<SingleType>(rtype.get())) {
        reporter_.ErrorAt("Wrong operand types for binary expr", expr.Op());
        return;
      }

      if (!ltype->Match(rtype.get())) {
        reporter_.ErrorAt("Operands must have the same type", expr.Op());
        return;
      }

      if (!ltype->Match(kInt.get()) && !ltype->Match(kReal.get())) {
        reporter_.ErrorAt("Operands must be either ints or reals", expr.Op());
        return;
      }

      expr.SetType(ltype->Copy());
      return;
    }

    default: assert(false && "Token is not a binary op");
  }
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

  if (!type_ || (decl && decl->Match(type_))) var = decl.get();
  else if (!decl) var = type_;
  else check_.reporter_.ErrorAt("Incompatible type decl", pattern.GetName());

  if (check_.locals_.contains(pattern.GetName().lexeme)) {
    check_.reporter_.ErrorAt("Redefinition of a name is not allowed", pattern.GetName());
  } else {
    check_.locals_[pattern.GetName().lexeme] = var;
  }
}

void TypeCheck::Visit(VariableStmt& stmt) {
  const Type* expr_type = nullptr;

  if (stmt.GetExpr()) {
    stmt.GetExpr()->Accept(*this);
    expr_type = stmt.GetExpr()->GetType().get();
  }

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
    case TokenType::kIdentifier: {
      if (auto var = Lookup(expr.Value().lexeme)) {
        if (*var) expr.SetType((*var)->Copy());
      } else {
        reporter_.ErrorAt("Undeclared identifier", expr.Value());
      }

      return;
    }

    default: assert(false && "Invalid literal expr");
  }
}

void TypeCheck::Visit(IfExpr& expr) {
  expr.Cond()->Accept(*this);
  const auto& cond_type = expr.Cond()->GetType();

  bool cond_good = true;
  if (cond_type && !cond_type->Match(kBool.get())) {
    reporter_.ErrorAt("Type of condition must be Bool", 0, 0); // TODO
    cond_good = false;
  }

  expr.Then()->Accept(*this);
  const auto& then_type = expr.Then()->GetType();

  if (!expr.Else()) {
    if (cond_good && then_type) {
      expr.SetType(kUnit->Copy());
    }

    return;
  }

  expr.Else()->Accept(*this);
  const auto& else_type = expr.Else()->GetType();

  if (!then_type || !else_type) return;

  if (!then_type->Match(else_type.get())) {
    reporter_.ErrorAt("Two clauses of an 'if' expression have different types", 0, 0);
    return;
  }

  if (cond_good) expr.SetType(then_type->Copy());
}

// TODO: define the order of assignment execution
void TypeCheck::Visit(AssignExpr& expr) {
  expr.Expr()->Accept(*this);
  expr.Dest()->Accept(*this);

  const auto& dest_type = expr.Dest()->GetType();
  const auto& value_type = expr.Expr()->GetType();
  if (!dest_type || !value_type) return;

  if (!dest_type->Match(value_type.get())) {
    reporter_.ErrorAt("Invalid assignment types", 0, 0);
    return;
  }

  expr.SetType(kUnit->Copy());
}

void TypeCheck::Visit(BlockExpr& expr) {
  Type* result = kUnit.get();

  TypeCheck scope(this);
  for (const auto& stmt : expr.Body()) {
    stmt->Accept(scope);
    if (result) {
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

  if (cond_type && !cond_type->Match(kBool.get())) {
    // TODO
    reporter_.ErrorAt("Type of condition expression must be Bool", 0, 0);
  }

  expr.Body()->Accept(*this);
  if (cond_type && expr.Body()->GetType()) {
    expr.SetType(kUnit->Copy());
  }
}

}
