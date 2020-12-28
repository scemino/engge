#pragma once
#include <functional>

namespace ng {
class DialogScriptAbstract {
public:
  virtual ~DialogScriptAbstract() = default;
  virtual std::function<bool()> pause(ngf::TimeSpan seconds) = 0;
  virtual std::function<bool()> say(const std::string &actor, const std::string &text) = 0;
  virtual void shutup() = 0;
  virtual std::function<bool()> waitFor(const std::string &actor) = 0;
  virtual std::function<bool()> waitWhile(const std::string &condition) = 0;
  virtual void execute(const std::string &code) = 0;
  [[nodiscard]] virtual bool executeCondition(const std::string &condition) const = 0;
};
}