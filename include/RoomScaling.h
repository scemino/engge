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

  void setTrigger(const std::string &trigger);
  const std::string& getName() const;
  float getScaling(float yPos) const;
  std::vector<Scaling> &getScalings();

private:
  std::string _trigger;
  std::vector<Scaling> _scalings;
};
} // namespace ng
