#include "Dialog/DialogManager.h"
#include "Engine.h"
#include "Actor.h"
#include "Logger.h"
#include "_SayFunction.h"
#include "_ExecuteCodeFunction.h"
#include "_ShutupFunction.h"
#include "_PauseFunction.h"
#include "../_Util.h"

namespace ng
{
DialogVisitor::DialogVisitor(DialogManager &dialogManager)
    : _dialogManager(dialogManager)
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
    : _dialogVisitor(dialogVisitor), _statement(statement), _isAccepted(true)
{
}

void DialogVisitor::ConditionVisitor::visit(const Ast::CodeCondition &node)
{
    const auto &actors = _dialogVisitor._pEngine->getActors();
    
    // check if the code corresponds to an actor name
    const auto name = node.code;
    auto it = std::find_if(actors.cbegin(), actors.cend(), [&name](const std::unique_ptr<Actor> &actor) {
        return actor->getKey() == name; });
    if (it != actors.end())
    {
        // yes, so we check if the current actor is the given actor name
        std::string code("currentActor==");
        code.append(name);
        _isAccepted = _dialogVisitor._pEngine->executeCondition(code);
        return;
    }

    // no it's not an actor, executes the code
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

void DialogVisitor::ConditionVisitor::visit(const Ast::OnceEverCondition &node)
{
    // TODO: OnceEverCondition
    _isAccepted = true;
}

void DialogVisitor::ConditionVisitor::visit(const Ast::TempOnceCondition &node)
{
    // TODO: TempOnceCondition
    _isAccepted = true;
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
    if(node.actor=="agent")
    {
        pActor = _pEngine->getCurrentActor();
    }
    else
    {
        auto name = node.actor;
        for (auto &actor : actors)
        {
            if (actor->getKey() == name)
            {
                pActor = actor.get();
                break;
            }
        }
    }
    auto id = getId(node.text);
    if (id > 0)
    {
        auto say = std::make_unique<_SayFunction>(*pActor, node.text);
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
        text = _pEngine->executeDollar(text);
    }
    auto id = getId(text);
    _dialogManager.getDialog()[node.number - 1].id = id;
    _dialogManager.getDialog()[node.number - 1].text = _pEngine->getText(id);
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
    trace("TODO: waitfor");
}

void DialogVisitor::visit(const Ast::Override &node)
{
    // TODO: override
    trace("TODO: override");
}

void DialogVisitor::visit(const Ast::Parrot &node)
{
    _dialogManager.enableParrotMode(node.active);
}

void DialogVisitor::visit(const Ast::Dialog &node)
{
    _dialogManager.setActorName(node.actor);
}

void DialogVisitor::visit(const Ast::AllowObjects &node)
{
    // TODO: allowObjects
    trace("TODO: allowObjects");
}

void DialogVisitor::visit(const Ast::WaitWhile &node)
{
    // TODO: waitWhile
    trace("TODO: waitWhile");
}

void DialogVisitor::visit(const Ast::Limit &node)
{
    // TODO: limit
    trace("TODO: limit");
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
