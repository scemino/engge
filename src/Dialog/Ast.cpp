#include "engge/Parsers/YackParser.hpp"

namespace ng::Ast {
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
void Override::accept(AstVisitor &visitor) { visitor.visit(*this); }
void WaitFor::accept(AstVisitor &visitor) { visitor.visit(*this); }
void Pause::accept(AstVisitor &visitor) { visitor.visit(*this); }
void Parrot::accept(AstVisitor &visitor) { visitor.visit(*this); }
void Dialog::accept(AstVisitor &visitor) { visitor.visit(*this); }
void AllowObjects::accept(AstVisitor &visitor) { visitor.visit(*this); }
void WaitWhile::accept(AstVisitor &visitor) { visitor.visit(*this); }
void Limit::accept(AstVisitor &visitor) { visitor.visit(*this); }
void OnceEverCondition::accept(AstVisitor &visitor) { visitor.visit(*this); }
void TempOnceCondition::accept(AstVisitor &visitor) { visitor.visit(*this); }

AstVisitor::~AstVisitor() = default;
void AstVisitor::visit(const Statement &node) { defaultVisit(node); }
void AstVisitor::visit(const Label &node) { defaultVisit(node); }
void AstVisitor::visit(const Say &node) { defaultVisit(node); }
void AstVisitor::visit(const Choice &node) { defaultVisit(node); }
void AstVisitor::visit(const Code &node) { defaultVisit(node); }
void AstVisitor::visit(const Goto &node) { defaultVisit(node); }
void AstVisitor::visit(const CodeCondition &node) { defaultVisit(node); }
void AstVisitor::visit(const OnceCondition &node) { defaultVisit(node); }
void AstVisitor::visit(const ShowOnceCondition &node) { defaultVisit(node); }
void AstVisitor::visit(const OnceEverCondition &node) { defaultVisit(node); }
void AstVisitor::visit(const TempOnceCondition &node) { defaultVisit(node); }
void AstVisitor::visit(const Shutup &node) { defaultVisit(node); }
void AstVisitor::visit(const Pause &node) { defaultVisit(node); }
void AstVisitor::visit(const WaitFor &node) { defaultVisit(node); }
void AstVisitor::visit(const Parrot &node) { defaultVisit(node); }
void AstVisitor::visit(const Dialog &node) { defaultVisit(node); }
void AstVisitor::visit(const Override &node) { defaultVisit(node); }
void AstVisitor::visit(const AllowObjects &node) { defaultVisit(node); }
void AstVisitor::visit(const WaitWhile &node) { defaultVisit(node); }
void AstVisitor::visit(const Limit &node) { defaultVisit(node); }
void AstVisitor::defaultVisit(const Node &) {}

} // namespace Ast

