#pragma once
#include <vector>
#include <string>

namespace ng
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

  const std::string &getTrigger() const;
  void setTrigger(const std::string &trigger);
  float getScaling(float yPos) const;
  std::vector<Scaling> &getScalings();

private:
  std::string _trigger;
  std::vector<Scaling> _scalings;
};
} // namespace ng
