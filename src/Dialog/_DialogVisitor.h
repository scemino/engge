#pragma once

#include "GGEngine.h"
#include "Dialog/YackTokenReader.h"
#include "Dialog/YackParser.h"

namespace gg
{
class _DialogVisitor : public Ast::AstVisitor
{
  public:
    _DialogVisitor(GGEngine &engine, DialogManager &dialogManager)
        : _engine(engine), _dialogManager(dialogManager)
    {
    }
    virtual void visit(const Ast::Statement &node)
    {
        _conditionAccepted = true;
        for (auto &condition : node.conditions)
        {
            condition->accept(*this);
            if (!_conditionAccepted)
                break;
        }
        if (_conditionAccepted)
        {
            node.expression->accept(*this);
        }
    }

    virtual void visit(const Ast::Label &node)
    {
        for (auto &statement : node.statements)
        {
            statement->accept(*this);
        }
    }

    virtual void visit(const Ast::Say &node)
    {
        auto &actors = _engine.getActors();
        GGActor *pActor = nullptr;
        for (auto &actor : actors)
        {
            if (actor->getName() == node.actor)
            {
                pActor = actor.get();
                break;
            }
        }
        pActor->say(getText(node.text));
    }

    virtual void visit(const Ast::Choice &node)
    {
        if (_engine.getDialog()[node.number - 1].text.empty())
        {
            _engine.getDialog()[node.number - 1].text = getText(node.text);
            _engine.getDialog()[node.number - 1].label = node.gotoExp->name;
        }
    }

    virtual void visit(const Ast::Code &node)
    {
        _engine.execute(node.code);
    }

    virtual void visit(const Ast::Goto &node)
    {
        _dialogManager.selectLabel(node.name);
    }

    virtual void visit(const Ast::CodeCondition &node)
    {
        _conditionAccepted = _engine.executeCondition(node.code);
    }

    virtual void visit(const Ast::OnceCondition &node)
    {
        // TODO: once
    }

    virtual void visit(const Ast::ShowOnceCondition &node)
    {
        // TODO: showonce
    }

    virtual void visit(const Ast::Shutup &node)
    {
        // TODO: shutup
    }

    virtual void visit(const Ast::Pause &node)
    {
        // TODO: pause
    }

    virtual void visit(const Ast::WaitFor &node)
    {
        // TODO: waitfor
    }

    std::string getText(const std::string &text)
    {
        std::string s(text);
        if (s[0] == '@')
        {
            s = s.substr(1);

            auto id = std::strtol(s.c_str(), nullptr, 10);
            return _engine.getText(id);
        }
        return text;
    }

  private:
    GGEngine &_engine;
    DialogManager &_dialogManager;
    bool _conditionAccepted;
};
} // namespace gg
