#include "Dialog/DialogManager.h"
#include "Engine.h"
#include "Actor.h"
#include "_SayFunction.h"
#include "_ExecuteCodeFunction.h"
#include "_ShutupFunction.h"
#include "_PauseFunction.h"
#include "../_NGUtil.h"

namespace ng
{
DialogVisitor::DialogVisitor(DialogManager &dialogManager)
    : _pEngine(nullptr), _dialogManager(dialogManager)
{
}
void DialogVisitor::visit(const Ast::Statement &node)
{
    if (!acceptConditions(node))
        return;
    node.expression->accept(*this);
}

void DialogVisitor::visit(const Ast::Label &node)
{
    for (auto &statement : node.statements)
    {
        statement->accept(*this);
    }
}

DialogVisitor::ConditionVisitor::ConditionVisitor(DialogVisitor &dialogVisitor, const Ast::Statement &statement)
    : _dialogVisitor(dialogVisitor), _isAccepted(true), _statement(statement)
{
}

void DialogVisitor::ConditionVisitor::visit(const Ast::CodeCondition &node)
{
    _isAccepted = _dialogVisitor._pEngine->executeCondition(node.code);
}

void DialogVisitor::ConditionVisitor::visit(const Ast::OnceCondition &node)
{
    _isAccepted = std::find(_dialogVisitor._nodesSelected.begin(), _dialogVisitor._nodesSelected.end(), _statement.expression.get()) == _dialogVisitor._nodesSelected.end();
}

void DialogVisitor::ConditionVisitor::visit(const Ast::ShowOnceCondition &node)
{
    _isAccepted = std::find(_dialogVisitor._nodesVisited.begin(), _dialogVisitor._nodesVisited.end(), _statement.expression.get()) == _dialogVisitor._nodesVisited.end();
}

bool DialogVisitor::acceptConditions(const Ast::Statement &statement)
{
    ConditionVisitor conditionVisitor(*this, statement);
    for (auto &condition : statement.conditions)
    {
        condition->accept(conditionVisitor);
        if (!conditionVisitor.isAccepted())
            break;
    }
    if (conditionVisitor.isAccepted())
    {
        _nodesVisited.push_back(statement.expression.get());
    }
    return conditionVisitor.isAccepted();
}

void DialogVisitor::visit(const Ast::Say &node)
{
    auto &actors = _pEngine->getActors();
    Actor *pActor = nullptr;
    for (auto &actor : actors)
    {
        if (actor->getName() == node.actor)
        {
            pActor = actor.get();
            break;
        }
    }
    auto id = getId(node.text);
    if (id > 0)
    {
        auto say = std::make_unique<_SayFunction>(*pActor, id);
        _dialogManager.addFunction(std::move(say));
    }
    else
    {
        auto anim = node.text.substr(2, node.text.length() - 3);
        std::stringstream s;
        s << "actorPlayAnimation(" << node.actor << ", \"" << anim << "\", NO)";
        auto executeCode = std::make_unique<_ExecuteCodeFunction>(*_pEngine, s.str());
        _dialogManager.addFunction(std::move(executeCode));
    }
}

void DialogVisitor::visit(const Ast::Choice &node)
{
    if (_dialogManager.getDialog()[node.number - 1].id != 0)
        return;

    auto text = node.text;
    if (text[0] == '$')
    {
        text =  _pEngine->executeDollar(text);
    }
    auto id = getId(text);
    _dialogManager.getDialog()[node.number - 1].id = id;
    _dialogManager.getDialog()[node.number - 1].text = towstring(text);
    _dialogManager.getDialog()[node.number - 1].label = node.gotoExp->name;
    _dialogManager.getDialog()[node.number - 1].pChoice = &node;
}

void DialogVisitor::visit(const Ast::Code &node)
{
    auto executeCode = std::make_unique<_ExecuteCodeFunction>(*_pEngine, node.code);
    _dialogManager.addFunction(std::move(executeCode));
}

void DialogVisitor::visit(const Ast::Goto &node)
{
    _dialogManager.selectLabel(node.name);
}

void DialogVisitor::visit(const Ast::Shutup &node)
{
    auto actor = _pEngine->getCurrentActor();
    if (actor)
    {
        auto shutup = std::make_unique<_ShutupFunction>(*_pEngine);
        _dialogManager.addFunction(std::move(shutup));
    }
}

void DialogVisitor::visit(const Ast::Pause &node)
{
    auto pause = std::make_unique<_PauseFunction>(sf::seconds(node.time));
    _dialogManager.addFunction(std::move(pause));
}

void DialogVisitor::visit(const Ast::WaitFor &node)
{
    // TODO: waitfor
    std::cout << "TODO: waitfor" << std::endl;
}

int DialogVisitor::getId(const std::string &text)
{
    std::string s(text);
    if (s[0] == '@')
    {
        s = s.substr(1);

        auto id = std::strtol(s.c_str(), nullptr, 10);
        return id;
    }
    return -1;
}
} // namespace ng
