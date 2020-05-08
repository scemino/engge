#pragma once
#include "Parsers/YackParser.hpp"

namespace ng{

class DialogConditionAbstract;

class ConditionVisitor : public Ast::AstVisitor {
public:
  explicit ConditionVisitor(const DialogConditionAbstract &context);
  [[nodiscard]] bool isAccepted() const { return _isAccepted; }

private:
  void visit(const Ast::CodeCondition &node) override;
  void visit(const Ast::OnceCondition &node) override;
  void visit(const Ast::ShowOnceCondition &node) override;
  void visit(const Ast::OnceEverCondition &node) override;
  void visit(const Ast::TempOnceCondition &node) override;

private:
  const DialogConditionAbstract &_context;
  bool _isAccepted{true};
};
}