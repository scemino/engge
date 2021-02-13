#pragma once
#include <string>

namespace ng {
class Trigger {
public:
  Trigger() = default;
  virtual ~Trigger() = default;
  void trig() {
    if (!isEnabled())
      return;
    trigCore();
  }
  [[nodiscard]] bool isEnabled() const { return m_isEnabled; }
  void disable() { m_isEnabled = false; }
  virtual std::string getName() = 0;

protected:
  virtual void trigCore() = 0;

private:
  bool m_isEnabled{true};
};
} // namespace ng
