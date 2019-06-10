#pragma once
#include <memory>
#include <optional>
#include "squirrel.h"
#include "SFML/Graphics.hpp"
#include "SoundTrigger.h"

namespace ng
{
class Engine;
class Room;
class SoundDefinition;
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

  void setTouchable(bool isTouchable);
  virtual bool isTouchable() const;

  void setRenderOffset(const sf::Vector2i &offset);
  sf::Vector2i getRenderOffset() const;

  void setUsePosition(const sf::Vector2f &pos);
  void setPosition(const sf::Vector2f &pos);

  sf::Vector2f getPosition() const;
  virtual sf::Vector2f getDefaultPosition() const;
  sf::Vector2f getUsePosition() const;

  virtual void move(const sf::Vector2f &offset) = 0;

  void setTrigger(int triggerNumber, Trigger* pTrigger);
  void trig(int triggerNumber);

  virtual void trigSound(const std::string &name);
  virtual void drawForeground(sf::RenderTarget &target, sf::RenderStates states) const;

  virtual Room *getRoom() = 0;
  virtual void setFps(int fps) = 0;

  virtual HSQOBJECT &getTable() = 0;

  SoundTrigger* createSoundTrigger(Engine &engine, const std::vector<SoundDefinition*> &sounds);

protected:
  sf::Transformable _transform;

private:
  std::map<int, Trigger*> _triggers;
  std::vector<std::unique_ptr<SoundTrigger>> _soundTriggers;
  sf::Vector2f _usePos;
  bool _isLit;
  bool _isVisible{true};
  bool _isTouchable{true};
  sf::Vector2i _renderOffset;
};
} // namespace ng
