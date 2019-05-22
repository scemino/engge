#pragma once
#include <memory>
#include <optional>
#include "SFML/Graphics.hpp"

namespace ng
{
class Room;
class Trigger;
class Entity : public sf::Drawable
{
public:
  virtual void update(const sf::Time &elapsed);
  virtual int getZOrder() const = 0;

  void setVisible(bool isVisible);
  bool isVisible() const;

  void setLit(bool isLit);
  bool isLit() const;

  void setUsePosition(const sf::Vector2f &pos);
  void setPosition(const sf::Vector2f &pos);

  sf::Vector2f getPosition() const;
  virtual sf::Vector2f getDefaultPosition() const;
  sf::Vector2f getUsePosition() const;

  virtual void move(const sf::Vector2f &offset) = 0;

  void setTrigger(int triggerNumber, std::shared_ptr<Trigger> trigger);
  void trig(int triggerNumber);

  virtual void trigSound(const std::string &name);
  virtual void drawForeground(sf::RenderTarget &target, sf::RenderStates states) const;

  virtual Room *getRoom() = 0;

protected:
  sf::Transformable _transform;

private:
  std::map<int, std::shared_ptr<Trigger>> _triggers;
  sf::Vector2f _usePos;
  bool _isLit;
  bool _isVisible{true};
};
} // namespace ng
