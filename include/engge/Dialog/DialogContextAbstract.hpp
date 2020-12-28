#pragma once
#include <functional>
#include <string>
#include <ngf/System/TimeSpan.h>

namespace ng{
class DialogContextAbstract {
public:
  virtual ~DialogContextAbstract()  = default;

  virtual void allowObjects(bool allow) = 0;
  virtual void dialog(const std::string &actor) = 0;
  virtual void execute(const std::string &code) = 0;
  virtual void gotoLabel(const std::string &label) = 0;
  virtual void limit(int max) = 0;
  virtual void override(const std::string &label) = 0;
  virtual void parrot(bool enabled) = 0;
  virtual std::function<bool()> pause(ngf::TimeSpan seconds) = 0;
  virtual std::function<bool()> say(const std::string& actor, const std::string &text) = 0;
  virtual void shutup() = 0;
  virtual std::function<bool()> waitFor(const std::string &actor) = 0;
  virtual std::function<bool()> waitWhile(const std::string &condition) = 0;
};
}