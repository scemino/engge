#include "Dialog/YackParser.h"

namespace ng::Ast
{
void Statement::accept(AstVisitor &visitor) { visitor.visit(*this); }
void Label::accept(AstVisitor &visitor) { visitor.visit(*this); }
void Say::accept(AstVisitor &visitor) { visitor.visit(*this); }
void Choice::accept(AstVisitor &visitor) { visitor.visit(*this); }
void Code::accept(AstVisitor &visitor) { visitor.visit(*this); }
void Goto::accept(AstVisitor &visitor) { visitor.visit(*this); }
void CodeCondition::accept(AstVisitor &visitor) { visitor.visit(*this); }
void OnceCondition::accept(AstVisitor &visitor) { visitor.visit(*this); }
void ShowOnceCondition::accept(AstVisitor &visitor) { visitor.visit(*this); }
void Shutup::accept(AstVisitor &visitor) { visitor.visit(*this); }
void WaitFor::accept(AstVisitor &visitor) { visitor.visit(*this); }
void Pause::accept(AstVisitor &visitor) { visitor.visit(*this); }
void Parrot::accept(AstVisitor &visitor) { visitor.visit(*this); }

void AstVisitor::visit(const Statement &node) { defaultVisit(node); }
void AstVisitor::visit(const Label &node) { defaultVisit(node); }
void AstVisitor::visit(const Say &node) { defaultVisit(node); }
void AstVisitor::visit(const Choice &node) { defaultVisit(node); }
void AstVisitor::visit(const Code &node) { defaultVisit(node); }
void AstVisitor::visit(const Goto &node) { defaultVisit(node); }
void AstVisitor::visit(const CodeCondition &node) { defaultVisit(node); }
void AstVisitor::visit(const OnceCondition &node) { defaultVisit(node); }
void AstVisitor::visit(const ShowOnceCondition &node) { defaultVisit(node); }
void AstVisitor::visit(const Shutup &node) { defaultVisit(node); }
void AstVisitor::visit(const Pause &node) { defaultVisit(node); }
void AstVisitor::visit(const WaitFor &node) { defaultVisit(node); }
void AstVisitor::visit(const Parrot &node) { defaultVisit(node); }
void AstVisitor::defaultVisit(const Node &node) {}

} // namespace Ast

