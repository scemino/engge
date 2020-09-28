#include "engge/Dialog/DialogContextAbstract.hpp"
#include "engge/Dialog/ExpressionVisitor.hpp"

namespace ng {
ExpressionVisitor::ExpressionVisitor(DialogContextAbstract &ctx) : context(ctx) {
}
ExpressionVisitor::~ExpressionVisitor() = default;

void ExpressionVisitor::visit(const Ast::Say &node) {
  auto func = context.say(node.actor, node.text);
  _pWaitAction = [func]() { return func(); };
}
void ExpressionVisitor::visit(const Ast::Code &node) {
  context.execute(node.code);
}
void ExpressionVisitor::visit(const Ast::Goto &node) {
  context.gotoLabel(node.name);
}
void ExpressionVisitor::visit(const Ast::Shutup &) {
  context.shutup();
}
void ExpressionVisitor::visit(const Ast::Pause &node) {
  _pWaitAction = context.pause(sf::seconds(node.time));
}
void ExpressionVisitor::visit(const Ast::WaitFor &node) {
  _pWaitAction = context.waitFor(node.actor);
}
void ExpressionVisitor::visit(const Ast::Parrot &node) {
  context.parrot(node.active);
}
void ExpressionVisitor::visit(const Ast::Dialog &node) {
  context.dialog(node.actor);
}
void ExpressionVisitor::visit(const Ast::Override &node) {
  context.override(node.node);
}
void ExpressionVisitor::visit(const Ast::AllowObjects &node) {
  context.allowObjects(node.allow);
}
void ExpressionVisitor::visit(const Ast::WaitWhile &node) {
  _pWaitAction = context.waitWhile(node.condition);
}
void ExpressionVisitor::visit(const Ast::Limit &node) {
  context.limit(node.max);
}
}