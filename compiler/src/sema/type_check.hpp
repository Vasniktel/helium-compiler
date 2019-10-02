//
// Created by vasniktel on 08.09.2019.
//

#ifndef HELIUM_COMPILER_SRC_TYPE_CHECK_HPP_
#define HELIUM_COMPILER_SRC_TYPE_CHECK_HPP_

#include <absl/container/flat_hash_map.h>
#include "absl/types/optional.h"
#include <parser/ast.hpp>
#include <utility>
#include "error_reporter.hpp"
#include "interner.hpp"

namespace helium {

class TypeCheck;

class PatternMatcher : public PatternVisitor {
  const Type* type_;
  TypeCheck& check_;

 public:
  PatternMatcher() = delete;
  explicit PatternMatcher(const Type* type, TypeCheck& check)
  : type_(type),
    check_(check)
  {}

  void Visit(TypedPattern& pattern) override;
};

class TypeCheck : public AstVisitor {
  friend class PatternMatcher;

  TypeCheck* parent_; // Enclosing scope's type check
  absl::flat_hash_map<::absl::string_view, const Type*> locals_;
  ErrorReporter& reporter_;

  const std::shared_ptr<Type> kInt;
  const std::shared_ptr<Type> kReal;
  const std::shared_ptr<Type> kUnit;
  const std::shared_ptr<Type> kChar;
  const std::shared_ptr<Type> kBool;

  TypeCheck(
      TypeCheck* parent, ErrorReporter& reporter,
      std::shared_ptr<Type> kInt,
      std::shared_ptr<Type> kReal,
      std::shared_ptr<Type> kUnit,
      std::shared_ptr<Type> kChar,
      std::shared_ptr<Type> kBool)
  : parent_(parent),
    locals_(),
    reporter_(reporter),
    kInt(std::move(kInt)),
    kReal(std::move(kReal)),
    kUnit(std::move(kUnit)),
    kChar(std::move(kChar)),
    kBool(std::move(kBool))
  {}

  explicit TypeCheck(TypeCheck* parent)
  : TypeCheck(
      parent,
      parent->reporter_,
      parent->kInt,
      parent->kReal,
      parent->kUnit,
      parent->kChar,
      parent->kBool)
  {}

 public:
  TypeCheck() = delete;
  TypeCheck(ErrorReporter& reporter, Interner& interner)
  : TypeCheck(nullptr, reporter,
              std::make_shared<SingleType>(interner.Intern("Int")),
              std::make_shared<SingleType>(interner.Intern("Real")),
              std::make_shared<SingleType>(interner.Intern("Unit")),
              std::make_shared<SingleType>(interner.Intern("Char")),
              std::make_shared<SingleType>(interner.Intern("Bool")))
  {}

  void Visit(VariableStmt& stmt) override;
  void Visit(BinaryExpr& expr) override;
  void Visit(UnaryExpr& expr) override;
  void Visit(LiteralExpr& expr) override;
  void Visit(IdentifierExpr& expr) override;
  void Visit(AssignExpr& expr) override;
  void Visit(BlockExpr& expr) override;
  void Visit(IfExpr& expr) override;
  void Visit(WhileExpr& expr) override;

 private:
  ::absl::optional<const Type*> Lookup(absl::string_view name);

  // Set intrinsic on binary and unary expressions
  // Expect exprs already have been type checked
  void SetIntrinsic(BinaryExpr& expr) const;
  void SetIntrinsic(UnaryExpr& expr) const;
};

}

#endif //HELIUM_COMPILER_SRC_TYPE_CHECK_HPP_
