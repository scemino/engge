#pragma once

namespace ng
{
class Trigger
{
public:
  Trigger() : _isEnabled(true) {}
  void doTrig()
  {
    if (isEnabled())
    {
      trig();
    }
  }
  virtual void trig() = 0;
  void setEnabled(bool enabled) { _isEnabled = enabled; }
  bool isEnabled() const { return _isEnabled; }

private:
  bool _isEnabled;
};
}
