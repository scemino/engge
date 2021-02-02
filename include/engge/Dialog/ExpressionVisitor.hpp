#pragma once
#include <functional>
#include "engge/Parsers/YackParser.hpp"

namespace ng {
class DialogContextAbstract;
class ExpressionVisitor final : public Ast::AstVisitor {
public:
  explicit ExpressionVisitor(DialogContextAbstract &ctx);
  ~ExpressionVisitor() override;

  [[nodiscard]] const std::function<bool()>& getWaitAction() const { return _pWaitAction; }

private:
  void visit(const Ast::Say &node) override;
  void visit(const Ast::Code &node) override;
  void visit(const Ast::Goto &node) override;
  void visit(const Ast::Shutup &) override;
  void visit(const Ast::Pause &node) override;
  void visit(const Ast::WaitFor &node) override;
  void visit(const Ast::Parrot &node) override;
  void visit(const Ast::Dialog &node) override;
  void visit(const Ast::Override &node) override;
  void visit(const Ast::AllowObjects &node) override;
  void visit(const Ast::WaitWhile &node) override;
  void visit(const Ast::Limit &node) override;

private:
  DialogContextAbstract &context;
  std::function<bool()> _pWaitAction{nullptr};
};
}