#include "Dialog/DialogManager.h"
#include "Engine.h"
#include "Actor.h"
#include "_SayFunction.h"

namespace ng
{
class _PauseFunction : public Function
{
  public:
    _PauseFunction(sf::Time time)
        : _time(time), _done(false)
    {
    }

    bool isElapsed() { return _done && _clock.getElapsedTime() > _time; }

    virtual void operator()()
    {
        if (_done)
            return;
        _clock.restart();
        _done = true;
    }

  private:
    sf::Clock _clock;
    const sf::Time _time;
    bool _done;
};

DialogVisitor::DialogVisitor(Engine &engine, DialogManager &dialogManager)
    : _engine(engine), _dialogManager(dialogManager)
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
    _isAccepted = _dialogVisitor._engine.executeCondition(node.code);
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
    auto &actors = _engine.getActors();
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
}

void DialogVisitor::visit(const Ast::Choice &node)
{
    if (_dialogManager.getDialog()[node.number - 1].id != 0)
        return;

    auto id = getId(node.text);
    _dialogManager.getDialog()[node.number - 1].id = id;
    _dialogManager.getDialog()[node.number - 1].text = _engine.getText(id);
    _dialogManager.getDialog()[node.number - 1].label = node.gotoExp->name;
    _dialogManager.getDialog()[node.number - 1].pChoice = &node;
}

void DialogVisitor::visit(const Ast::Code &node)
{
    _engine.execute(node.code);
}

void DialogVisitor::visit(const Ast::Goto &node)
{
    _dialogManager.selectLabel(node.name);
}

void DialogVisitor::visit(const Ast::Shutup &node)
{
    // TODO: shutup
    std::cout << "TODO: shutup" << std::endl;
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
    // TODO:
    //throw std::logic_error("Expecting a talk id");
    std::cerr << "Expecting a talk id instead of: " << s << std::endl;
    return -1;
}
} // namespace ng
