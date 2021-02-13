#pragma once
#include <functional>
#include <utility>
#include "DialogScriptAbstract.hpp"

namespace ng {
class Engine;

class EngineDialogScript final : public DialogScriptAbstract {
public:
  explicit EngineDialogScript(Engine &engine);
  ~EngineDialogScript() final = default;

private:
  std::function<bool()> pause(ngf::TimeSpan time) final;
  std::function<bool()> say(const std::string &actor, const std::string &text) final;
  void shutup() final;
  std::function<bool()> waitFor(const std::string &actor) final;
  std::function<bool()> waitWhile(const std::string &condition) final;
  void execute(const std::string &code) final;
  [[nodiscard]] bool executeCondition(const std::string &condition) const final;

private:
  Engine &m_engine;
};
}