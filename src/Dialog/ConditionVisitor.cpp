#include "engge/Parsers/YackParser.hpp"
#include "engge/Dialog/ConditionVisitor.hpp"
#include "engge/Dialog/DialogConditionAbstract.hpp"

namespace ng {
ConditionVisitor::ConditionVisitor(const DialogConditionAbstract &context) : _context(context) {
}

void ConditionVisitor::visit(const Ast::CodeCondition &node) {
  _isAccepted = _context.executeCondition(node.code);
}

void ConditionVisitor::visit(const Ast::OnceCondition &node) {
  _isAccepted = _context.isOnce(node.getLine());
}

void ConditionVisitor::visit(const Ast::ShowOnceCondition &node) {
  _isAccepted = _context.isShowOnce(node.getLine());
}

void ConditionVisitor::visit(const Ast::OnceEverCondition &node) {
  _isAccepted = _context.isOnceEver(node.getLine());
}

void ConditionVisitor::visit(const Ast::TempOnceCondition &node) {
  _isAccepted = _context.isTempOnce(node.getLine());
}
}