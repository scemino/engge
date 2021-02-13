#include "engge/Dialog/DialogContextAbstract.hpp"
#include "engge/Dialog/ExpressionVisitor.hpp"

namespace ng {
ExpressionVisitor::ExpressionVisitor(DialogContextAbstract &ctx) : m_context(ctx) {
}
ExpressionVisitor::~ExpressionVisitor() = default;

void ExpressionVisitor::visit(const Ast::Say &node) {
  auto func = m_context.say(node.actor, node.text);
  m_pWaitAction = [func]() { return func(); };
}
void ExpressionVisitor::visit(const Ast::Code &node) {
  m_context.execute(node.code);
}
void ExpressionVisitor::visit(const Ast::Goto &node) {
  m_context.gotoLabel(node.name);
}
void ExpressionVisitor::visit(const Ast::Shutup &) {
  m_context.shutup();
}
void ExpressionVisitor::visit(const Ast::Pause &node) {
  m_pWaitAction = m_context.pause(ngf::TimeSpan::seconds(node.time));
}
void ExpressionVisitor::visit(const Ast::WaitFor &node) {
  m_pWaitAction = m_context.waitFor(node.actor);
}
void ExpressionVisitor::visit(const Ast::Parrot &node) {
  m_context.parrot(node.active);
}
void ExpressionVisitor::visit(const Ast::Dialog &node) {
  m_context.dialog(node.actor);
}
void ExpressionVisitor::visit(const Ast::Override &node) {
  m_context.override(node.node);
}
void ExpressionVisitor::visit(const Ast::AllowObjects &node) {
  m_context.allowObjects(node.allow);
}
void ExpressionVisitor::visit(const Ast::WaitWhile &node) {
  m_pWaitAction = m_context.waitWhile(node.condition);
}
void ExpressionVisitor::visit(const Ast::Limit &node) {
  m_context.limit(node.max);
}
}