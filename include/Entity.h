#pragma once
#include <memory>
#include <optional>
#include "squirrel.h"
#include "SFML/Graphics.hpp"
#include "SoundTrigger.h"
#include "ScriptObject.h"

namespace ng
{
class Engine;
class Room;
class SoundDefinition;
class Trigger;
class Entity : public ScriptObject, public sf::Drawable
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
  sf::Vector2f getUsePosition() const;

  void setOffset(const sf::Vector2f &offset);
  sf::Vector2f getOffset() const;

  void setTrigger(int triggerNumber, Trigger* pTrigger);
  void trig(int triggerNumber);

  virtual void trigSound(const std::string &name);
  virtual void drawForeground(sf::RenderTarget &target, sf::RenderStates states) const;

  virtual Room *getRoom() = 0;
  virtual const Room *getRoom() const = 0;
  virtual void setFps(int fps) = 0;

  virtual HSQOBJECT &getTable() = 0;

  SoundTrigger* createSoundTrigger(Engine &engine, const std::vector<SoundDefinition*> &sounds);

protected:
  sf::Transform getTransform() const;
  sf::Transformable _transform;

private:
  std::map<int, Trigger*> _triggers;
  std::vector<std::unique_ptr<SoundTrigger>> _soundTriggers;
  sf::Vector2f _usePos;
  sf::Vector2f _offset;
  bool _isLit{true};
  bool _isVisible{true};
  bool _isTouchable{true};
  sf::Vector2i _renderOffset;
};
} // namespace ng
