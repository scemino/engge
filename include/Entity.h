#pragma once
#include <optional>
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

  void setUsePosition(const sf::Vector2f &pos)
  {
    _usePos = pos;
  }

  void setPosition(const sf::Vector2f &pos)
  {
      if (!_defaultPosition.has_value())
      {
          _defaultPosition = pos;
      }
      _transform.setPosition(pos);
  }

  sf::Vector2f getPosition() const
  {
      return _transform.getPosition();
  }

  sf::Vector2f getDefaultPosition() const
  {
      return _defaultPosition.value();
  }

  sf::Vector2f getUsePosition() const
  {
    return _usePos;
  }

  void setTrigger(int triggerNumber, std::shared_ptr<Trigger> trigger)
  {
    _triggers[triggerNumber] = trigger;
  }

  void trig(int triggerNumber)
  {
    auto it = _triggers.find(triggerNumber);
    if (it != _triggers.end())
    {
      it->second->trig();
    }
  }

protected:
  sf::Transformable _transform;

private:
  std::map<int, std::shared_ptr<Trigger>> _triggers;
  sf::Vector2f _usePos;
  std::optional<sf::Vector2f> _defaultPosition;
};
} // namespace ng
