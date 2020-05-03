#pragma once

#include <array>
#include <optional>
#include "SFML/Graphics.hpp"
#include "Parsers/YackParser.hpp"
#include "Engine/Function.hpp"
#include "Font/GGFont.hpp"

namespace ng {
class Actor;
class Engine;

struct DialogSlot {
  int id{0};
  std::wstring text;
  std::string label;
  const Ast::Statement *pChoice{nullptr};
  mutable sf::Vector2f pos;
};

enum class DialogConditionMode {
  Once,
  ShowOnce,
  OnceEver,
  ShowOnceEver,
  TempOnce
};

struct DialogConditionState {
public:
  DialogConditionMode mode;
  std::string actorKey;
  std::string dialog;
  int32_t line;
};

enum class DialogSelectMode {
  Show,
  Choose
};

class DialogManager;
class DialogVisitor : public Ast::AstVisitor {
private:
  class ConditionStateVisitor : public Ast::AstVisitor {
  public:
    explicit ConditionStateVisitor(DialogManager& dialogManager, DialogSelectMode selectMode);
    ~ConditionStateVisitor() override = default;
    [[nodiscard]] std::optional<DialogConditionState> getState() const { return _state; }

  private:
    void visit(const Ast::OnceCondition &node) override;
    void visit(const Ast::ShowOnceCondition &node) override;
    void visit(const Ast::OnceEverCondition &node) override;
    void visit(const Ast::TempOnceCondition &node) override;

    void setState(int32_t line, DialogConditionMode mode);

  private:
    DialogManager& _dialogManager;
    DialogSelectMode _selectMode;
    std::optional<DialogConditionState> _state;
  };
  class ConditionVisitor : public Ast::AstVisitor {
  public:
    explicit ConditionVisitor(DialogVisitor &dialogVisitor);
    [[nodiscard]] bool isAccepted() const { return _isAccepted; }

  private:
    void visit(const Ast::CodeCondition &node) override;
    void visit(const Ast::OnceCondition &node) override;
    void visit(const Ast::ShowOnceCondition &node) override;
    void visit(const Ast::OnceEverCondition &node) override;
    void visit(const Ast::TempOnceCondition &node) override;

  private:
    DialogVisitor &_dialogVisitor;
    bool _isAccepted;
  };

public:
  explicit DialogVisitor(DialogManager &dialogManager);

  void setEngine(Engine *pEngine) { _pEngine = pEngine; }
  void select(const Ast::Statement &node);
  bool acceptConditions(const Ast::Statement &statement);

private:
  void visit(const Ast::Statement &node) override;
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

private:
  Engine *_pEngine{nullptr};
  DialogManager &_dialogManager;
  const Ast::Statement* _pStatement{nullptr};
};

enum class DialogManagerState {
  None,
  Active,
  WaitingForChoice
};

class DialogManager : public sf::Drawable {
private:
  static constexpr float SlidingSpeed = 25.f;

public:
  DialogManager();

  void setEngine(Engine *pEngine);
  void start(const std::string &name, const std::string &node);
  void selectLabel(const std::string &label);
  std::array<DialogSlot, 8> &getDialog() { return _dialog; }
  void update(const sf::Time &elapsed);

  void setMousePosition(sf::Vector2f pos);

  [[nodiscard]] DialogManagerState getState() const { return _state; }
  void addFunction(std::unique_ptr<Function> function);
  void choose(int choice);
  void setActorName(const std::string &actor);
  inline void enableParrotMode(bool enable) { _parrotModeEnabled = enable; }
  inline void setLimit(int limit) { _limit = limit; }
  inline void setOverride(const std::string &override) { _override = override; }

  Actor *getTalkingActor();
  std::string getDialogName() const { return _name; }
  std::vector<DialogConditionState>& getStates() { return _states; }
  const std::vector<DialogConditionState>& getStates() const { return _states; }

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
  void resetState();

private:
  Engine *_pEngine{nullptr};
  std::unique_ptr<Ast::CompilationUnit> _pCompilationUnit;
  Ast::Label *_pLabel{nullptr};
  std::array<DialogSlot, 8> _dialog;
  DialogManagerState _state{DialogManagerState::None};
  DialogVisitor _dialogVisitor;
  std::vector<std::unique_ptr<Function>> _functions;
  std::string _actorName;
  std::string _name;
  bool _parrotModeEnabled{true};
  int _limit{6};
  std::vector<std::unique_ptr<Ast::Statement>, std::allocator<std::unique_ptr<Ast::Statement>>>::iterator _currentStatement;
  std::string _override;
  sf::Vector2f _mousePos;
  std::vector<DialogConditionState> _states;
};
} // namespace ng
