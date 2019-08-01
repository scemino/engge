#pragma once

namespace ng
{
class Trigger
{
  public:
    Trigger() = default;
    virtual ~Trigger() = default;
    void trig()
    {
        if (!isEnabled())
            return;
        trigCore();
    }
    bool isEnabled() const { return _isEnabled; }
    void disable() { _isEnabled = false; }

  protected:
    virtual void trigCore() = 0;

  private:
    bool _isEnabled{true};
};
} // namespace ng
