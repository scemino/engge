#pragma once

#include <array>
#include "SFML/Graphics.hpp"
#include "YackParser.h"
#include "Function.h"
#include "Font.h"

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
    void visit(const Ast::OnceEverCondition &node) override;
    void visit(const Ast::TempOnceCondition &node) override;

  private:
    DialogVisitor &_dialogVisitor;
    const Ast::Statement &_statement;
    bool _isAccepted;
  };

public:
  explicit DialogVisitor(DialogManager &dialogManager);

  void setEngine(Engine *pEngine) { _pEngine = pEngine; }
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
  void visit(const Ast::Override &node) override;
  void visit(const Ast::WaitFor &node) override;
  void visit(const Ast::Parrot &node) override;
  void visit(const Ast::Dialog &node) override;
  void visit(const Ast::AllowObjects &node) override;
  void visit(const Ast::WaitWhile &node) override;
  void visit(const Ast::Limit &node) override;

  static int getId(const std::string &text);
  bool acceptConditions(const Ast::Statement &statement);

private:
  Engine *_pEngine{nullptr};
  DialogManager &_dialogManager;
  std::vector<const Ast::Node *> _nodesVisited;
  std::vector<const Ast::Node *> _nodesSelected;
};

enum class DialogManagerState
{
  None,
  Active,
  WaitingForChoice
};

class DialogManager : public sf::Drawable
{
public:
  DialogManager();

  void setEngine(Engine *pEngine);
  void start(const std::string &name, const std::string &node);
  void selectLabel(const std::string &label);
  std::array<DialogSlot, 8> &getDialog() { return _dialog; }
  void update(const sf::Time &elapsed);

  DialogManagerState getState() const { return _state; }
  void addFunction(std::unique_ptr<Function> function);
  void choose(int choice);
  void setActorName(const std::string& actor);
  inline void enableParrotMode(bool enable) { _parrotModeEnabled = enable; }
  inline void setLimit(int limit) { _limit = limit; }

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
  Engine *_pEngine{nullptr};
  std::unique_ptr<Ast::CompilationUnit> _pCompilationUnit;
  Ast::Label *_pLabel{nullptr};
  std::array<DialogSlot, 8> _dialog;
  DialogManagerState _state{DialogManagerState::None};
  DialogVisitor _dialogVisitor;
  std::vector<std::unique_ptr<Function>> _functions;
  std::string _actorName;
  bool _parrotModeEnabled{true};
  int _limit{6};
};
} // namespace ng
