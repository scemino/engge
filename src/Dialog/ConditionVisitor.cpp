#include "engge/Parsers/YackParser.hpp"
#include "engge/Dialog/ConditionVisitor.hpp"
#include "engge/Dialog/DialogConditionAbstract.hpp"

namespace ng {
ConditionVisitor::ConditionVisitor(const DialogConditionAbstract &context) : m_context(context) {
}

void ConditionVisitor::visit(const Ast::CodeCondition &node) {
  m_isAccepted = m_context.executeCondition(node.code);
}

void ConditionVisitor::visit(const Ast::OnceCondition &node) {
  m_isAccepted = m_context.isOnce(node.getLine());
}

void ConditionVisitor::visit(const Ast::ShowOnceCondition &node) {
  m_isAccepted = m_context.isShowOnce(node.getLine());
}

void ConditionVisitor::visit(const Ast::OnceEverCondition &node) {
  m_isAccepted = m_context.isOnceEver(node.getLine());
}

void ConditionVisitor::visit(const Ast::TempOnceCondition &node) {
  m_isAccepted = m_context.isTempOnce(node.getLine());
}
}