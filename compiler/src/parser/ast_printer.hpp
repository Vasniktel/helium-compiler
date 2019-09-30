//
// Created by vasniktel on 30.08.2019.
//

#ifndef HELIUM_COMPILER_SRC_PARSER_AST_PRINTER_HPP_
#define HELIUM_COMPILER_SRC_PARSER_AST_PRINTER_HPP_

#include <ostream>
#include "ast.hpp"
#include "sema/type.hpp"
#include "sema/type_check.hpp"

namespace helium {

class AstPrinter : public AstVisitor, public PatternVisitor, public TypeVisitor {
  bool typed_;
  std::ostream& os_;
  const Interner& interner_;

 public:
  explicit AstPrinter(bool typed, std::ostream& os, const Interner& interner)
  : typed_(typed),
    os_(os),
    interner_(interner)
  {}

  void Visit(VariableStmt& node) override;
  void Visit(BinaryExpr& node) override;
  void Visit(UnaryExpr& node) override;
  void Visit(LiteralExpr& node) override;
  void Visit(IdentifierExpr& node) override;
  void Visit(BlockExpr& node) override;
  void Visit(IfExpr& node) override;
  void Visit(WhileExpr& node) override;
  void Visit(AssignExpr& node) override;
  void Visit(TypedPattern& pattern) override;
  void Visit(SingleType& type) override;
  void Visit(ErrorType& type) override;
};

}

#endif //HELIUM_COMPILER_SRC_PARSER_AST_PRINTER_HPP_
