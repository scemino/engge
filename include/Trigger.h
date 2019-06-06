#pragma once

namespace ng
{
class Trigger
{
public:
  Trigger() = default;
  virtual ~Trigger() = default;
  virtual void trig() = 0;
  bool isEnabled() const { return _isEnabled; }
  void disable() { _isEnabled = false; }

 private:
  bool _isEnabled{true};
};
}
