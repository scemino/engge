#pragma once
#include <functional>
#include "engge/Parsers/YackParser.hpp"

namespace ng {
class DialogContextAbstract;
class ExpressionVisitor final : public Ast::AstVisitor {
public:
  explicit ExpressionVisitor(DialogContextAbstract &ctx);
  ~ExpressionVisitor() final;

  [[nodiscard]] const std::function<bool()>& getWaitAction() const { return m_pWaitAction; }

private:
  void visit(const Ast::Say &node) final;
  void visit(const Ast::Code &node) final;
  void visit(const Ast::Goto &node) final;
  void visit(const Ast::Shutup &) final;
  void visit(const Ast::Pause &node) final;
  void visit(const Ast::WaitFor &node) final;
  void visit(const Ast::Parrot &node) final;
  void visit(const Ast::Dialog &node) final;
  void visit(const Ast::Override &node) final;
  void visit(const Ast::AllowObjects &node) final;
  void visit(const Ast::WaitWhile &node) final;
  void visit(const Ast::Limit &node) final;

private:
  DialogContextAbstract &m_context;
  std::function<bool()> m_pWaitAction{nullptr};
};
}