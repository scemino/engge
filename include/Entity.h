#pragma once
#include "SFML/Graphics.hpp"

namespace ng
{
class Trigger
{
public:
  virtual void trig() = 0;
};

class Entity : public sf::Drawable
{
public:
  virtual void update(const sf::Time &elapsed) {}
  virtual int getZOrder() const = 0;

  void setTrigger(int triggerNumber, std::shared_ptr<Trigger> trigger)
  {
    _triggers.insert(std::make_pair(triggerNumber, trigger));
  }

  void trig(int triggerNumber)
  {
    auto it = _triggers.find(triggerNumber);
    if (it != _triggers.end())
    {
        it->second->trig();
    }
}

private:
  std::map<int, std::shared_ptr<Trigger>> _triggers;
};
} // namespace ng
