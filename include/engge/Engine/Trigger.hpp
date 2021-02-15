#pragma once
#include <string>

namespace ng {
class Trigger {
public:
  Trigger();
  virtual ~Trigger();

  void trig();

  [[nodiscard]] bool isEnabled() const;
  void disable();

  virtual std::string getName() = 0;

protected:
  virtual void trigCore() = 0;

private:
  bool m_isEnabled{true};
};
} // namespace ng
