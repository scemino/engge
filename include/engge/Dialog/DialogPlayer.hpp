#pragma once
#include <string>
#include <ngf/System/TimeSpan.h>
#include "DialogContextAbstract.hpp"
#include "DialogConditionAbstract.hpp"

namespace ng {

enum class DialogManagerState {
  None,
  Active,
  WaitingForChoice
};

enum class DialogPlayerState {
  None,
  Start,
  Running,
  WaitingForChoice,
  WaitingEndAnimation,
  WaitingForSayingChoice
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

class DialogScriptAbstract;
class DialogPlayer final : public DialogContextAbstract, public DialogConditionAbstract {
private:
  DialogScriptAbstract &_script;

public:
  explicit DialogPlayer(DialogScriptAbstract &script);
  ~DialogPlayer() override;

  void start(const std::string &actor, const std::string &name, const std::string &node);
  void choose(int choiceId);
  void update();

  [[nodiscard]] DialogManagerState getState() const;
  [[nodiscard]] std::string getActor() const { return m_actor; }
  [[nodiscard]] std::string getDialogName() const { return m_dialogName; }

  [[nodiscard]] const std::array<const Ast::Statement *, 9> &getChoices() const { return m_choices; }
  [[nodiscard]] const std::vector<DialogConditionState>& getStates() const { return m_states; }
  std::vector<DialogConditionState>& getStates() { return m_states; }

private:
  void resetState();

  void selectLabel(const std::string &name);
  void run(Ast::Statement *pStatement);

  void addChoice(const Ast::Statement *pStatement, const Ast::Choice *pChoice);
  void clearChoices();

  [[nodiscard]] bool choicesReady() const;
  bool acceptConditions(const Ast::Statement *pStatement);

  void allowObjects(bool allow) override;
  void dialog(const std::string &actor) override;
  void execute(const std::string &code) override;
  void gotoLabel(const std::string &label) override;
  void limit(int max) override;
  void override(const std::string &label) override;
  void parrot(bool enabled) override;
  std::function<bool()> pause(ngf::TimeSpan seconds) override;
  std::function<bool()> say(const std::string &actor, const std::string &text) override;
  void shutup() override;
  std::function<bool()> waitFor(const std::string &actor) override;
  std::function<bool()> waitWhile(const std::string &condition) override;

  [[nodiscard]] bool isOnce(int32_t line) const override;
  [[nodiscard]] bool isShowOnce(int32_t line) const override;
  [[nodiscard]] bool isOnceEver(int32_t line) const override;
  [[nodiscard]] bool isTempOnce(int32_t line) const override;
  [[nodiscard]] bool executeCondition(const std::string &condition) const override;

  void running();
  bool gotoNextLabel();
  void endDialog();

private:
  std::string m_dialogName;
  std::unique_ptr<Ast::CompilationUnit> m_pCompilationUnit;
  std::array<const Ast::Statement *, 9> m_choices{};
  Ast::Label *m_pLabel{nullptr};
  int m_currentStatement{0};
  DialogPlayerState m_state{DialogPlayerState::None};
  std::string m_actor;
  bool m_parrot{true};
  bool m_allowObjects{false};
  int m_limit{6};
  std::string m_overrideLabel;
  std::function<bool()> m_pWaitAction{nullptr};
  std::vector<DialogConditionState> m_states;
  std::string m_nextLabel;
};
}