#pragma once
#include <functional>
#include <utility>
#include "DialogScriptAbstract.hpp"

namespace ng {
class Engine;

class EngineDialogScript : public DialogScriptAbstract {
public:
  explicit EngineDialogScript(Engine &engine);
  ~EngineDialogScript() override = default;

private:
  std::function<bool()> pause(ngf::TimeSpan time) override;
  std::function<bool()> say(const std::string &actor, const std::string &text) override;
  void shutup() override;
  std::function<bool()> waitFor(const std::string &actor) override;
  std::function<bool()> waitWhile(const std::string &condition) override;
  void execute(const std::string &code) override;
  [[nodiscard]] bool executeCondition(const std::string &condition) const override;

private:
  Engine &_engine;
};
}