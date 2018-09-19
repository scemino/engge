#pragma once
#include <vector>
#include <string>

namespace gg
{
struct Scaling
{
    float scale;
    float yPos;
};

class RoomScaling
{
  public:
    RoomScaling() = default;
    ~RoomScaling() = default;

    const std::string &getTrigger() const { return _trigger; }
    void setTrigger(const std::string &trigger) { _trigger = trigger; }

    std::vector<Scaling> &getScalings() { return _scalings; }

  private:
    std::string _trigger;
    std::vector<Scaling> _scalings;
};
} // namespace gg
