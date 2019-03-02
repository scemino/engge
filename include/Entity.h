#pragma once
#include <memory>
#include <optional>
#include "SFML/Graphics.hpp"

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

class Entity : public sf::Drawable
{
public:
  virtual void update(const sf::Time &elapsed) {}
  virtual int getZOrder() const = 0;

  void setLit(bool isLit) { _isLit = isLit; }
  bool isLit() const { return _isLit; }

  void setUsePosition(const sf::Vector2f &pos)
  {
    _usePos = pos;
  }

  void setPosition(const sf::Vector2f &pos)
  {
    _transform.setPosition(pos);
  }

  sf::Vector2f getPosition() const
  {
    return _transform.getPosition();
  }

  virtual sf::Vector2f getDefaultPosition() const
  {
    return getPosition();
  }

  sf::Vector2f getUsePosition() const
  {
    return _usePos;
  }

  virtual void move(const sf::Vector2f &offset) = 0;

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

  virtual void trigSound(const std::string &name)
  {
  }

  virtual void drawForeground(sf::RenderTarget &target, sf::RenderStates states) const
  {
  }

protected:
  sf::Transformable _transform;

private:
  std::map<int, std::shared_ptr<Trigger>> _triggers;
  sf::Vector2f _usePos;
  bool _isLit;
};
} // namespace ng
