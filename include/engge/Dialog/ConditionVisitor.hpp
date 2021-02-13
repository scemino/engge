#pragma once
#include "engge/Parsers/YackParser.hpp"

namespace ng{

class DialogConditionAbstract;

class ConditionVisitor final : public Ast::AstVisitor {
public:
  explicit ConditionVisitor(const DialogConditionAbstract &context);
  [[nodiscard]] bool isAccepted() const { return m_isAccepted; }

private:
  void visit(const Ast::CodeCondition &node) override;
  void visit(const Ast::OnceCondition &node) override;
  void visit(const Ast::ShowOnceCondition &node) override;
  void visit(const Ast::OnceEverCondition &node) override;
  void visit(const Ast::TempOnceCondition &node) override;

private:
  const DialogConditionAbstract &m_context;
  bool m_isAccepted{true};
};
}