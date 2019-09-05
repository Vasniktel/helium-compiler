//
// Created by vasniktel on 30.08.2019.
//

#ifndef HELIUM_COMPILER_SRC_PARSER_AST_PRINTER_HPP_
#define HELIUM_COMPILER_SRC_PARSER_AST_PRINTER_HPP_

#include <ostream>
#include "ast.hpp"

namespace helium {

class AstPrinter : public AstVisitor {
  std::ostream& os_;

 public:
  explicit AstPrinter(std::ostream& os)
  : os_(os)
  {}

  using AstVisitor::Visit;
  void Visit(VariableStmt& node) override;
  void Visit(BinaryExpr& node) override;
  void Visit(UnaryExpr& node) override;
  void Visit(LiteralExpr& node) override;
  void Visit(BlockExpr& node) override;
  void Visit(IfExpr& node) override;
  void Visit(WhileExpr& node) override;
  void Visit(AssignExpr& node) override;
};

}

#endif //HELIUM_COMPILER_SRC_PARSER_AST_PRINTER_HPP_
