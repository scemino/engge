#pragma once

#include <array>
#include "SFML/Graphics.hpp"
#include "YackParser.h"
#include "Function.h"
#include "FntFont.h"

namespace ng
{
class Engine;

struct DialogSlot
{
  int id;
  std::wstring text;
  std::string label;
  const Ast::Choice *pChoice;
};

class DialogManager;
class DialogVisitor : public Ast::AstVisitor
{
private:
  class ConditionVisitor : public Ast::AstVisitor
  {
  public:
    ConditionVisitor(DialogVisitor &dialogVisitor, const Ast::Statement &statement);
    bool isAccepted() const { return _isAccepted; }

  private:
    void visit(const Ast::CodeCondition &node) override;
    void visit(const Ast::OnceCondition &node) override;
    void visit(const Ast::ShowOnceCondition &node) override;

  private:
    DialogVisitor &_dialogVisitor;
    const Ast::Statement &_statement;
    bool _isAccepted;
  };

public:
  DialogVisitor(Engine &engine, DialogManager &dialogManager);
  void select(const Ast::Node &node) { _nodesSelected.push_back(&node); }

private:
  void visit(const Ast::Statement &node) override;
  void visit(const Ast::Label &node) override;
  void visit(const Ast::Say &node) override;
  void visit(const Ast::Choice &node) override;
  void visit(const Ast::Code &node) override;
  void visit(const Ast::Goto &node) override;
  void visit(const Ast::Shutup &node) override;
  void visit(const Ast::Pause &node) override;
  void visit(const Ast::WaitFor &node) override;

  int getId(const std::string &text);
  bool acceptConditions(const Ast::Statement &statement);

private:
  Engine &_engine;
  DialogManager &_dialogManager;
  std::vector<const Ast::Node *> _nodesVisited;
  std::vector<const Ast::Node *> _nodesSelected;
};

class DialogManager : public sf::Drawable
{
public:
  DialogManager(Engine &engine);

  void start(const std::string &name, const std::string &node);
  void selectLabel(const std::string &label);
  std::array<DialogSlot, 8> &getDialog() { return _dialog; }
  void update(const sf::Time &elapsed);

  bool isActive() const { return _isActive; }
  void addFunction(std::unique_ptr<Function> function);

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
  Engine &_engine;
  std::unique_ptr<Ast::CompilationUnit> _pCompilationUnit;
  Ast::Label *_pLabel;
  std::array<DialogSlot, 8> _dialog;
  sf::Vector2f _mousePos;
  bool _isActive;
  DialogVisitor _dialogVisitor;
  std::vector<std::unique_ptr<Function>> _functions;
  FntFont _font;
};
} // namespace ng
