#pragma once
#include <sstream>
#include <squirrel3/squirrel.h>
#include "SFML/Graphics.hpp"
#include "NonCopyable.h"
#include "TextureManager.h"
#include "NGCostume.h"
#include "NGFont.h"
#include "NGEntity.h"
#include "NGLip.h"

namespace ng
{
class NGRoom;
class NGObject;

class NGActor;

class NGEngine;

class NGActor : public NGEntity
{
private:
  class WalkingState
  {
  public:
    WalkingState(NGActor &actor);

    void setDestination(const sf::Vector2f &destination);
    void update(const sf::Time &elapsed);
    bool isWalking() const { return _isWalking; }

  private:
    NGActor &_actor;
    sf::Vector2f _destination;
    bool _isWalking;
  };

  class TalkingState : public sf::Drawable
  {
  public:
    TalkingState(NGActor &actor);

    void setTalkOffset(const sf::Vector2i &offset) { _talkOffset = offset; }
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
    void update(const sf::Time &elapsed);
    void say(int id);
    bool isTalking() const { return _isTalking; }
    void setTalkColor(sf::Color color) { _talkColor = color; }

  private:
    NGActor &_actor;
    NGFont _font;
    bool _isTalking;
    std::string _sayText;
    NGLip _lip;
    int _index;
    sf::Vector2i _talkOffset;
    sf::Color _talkColor;
    sf::Clock _clock;
  };

public:
  explicit NGActor(NGEngine &engine);
  virtual ~NGActor();

  void setName(const std::string &name) { _name = name; }
  const std::string &getName() const { return _name; }

  void setVisible(bool isVisible) { _isVisible = isVisible; }
  bool isVisible() const { return _isVisible; }

  void useWalkboxes(bool use) { _use = use; }

  int getZOrder() const override;

  void setCostume(const std::string &name, const std::string &sheet = "");
  NGCostume &getCostume() { return _costume; }
  const NGCostume &getCostume() const { return _costume; }

  void setTalkColor(sf::Color color) { _talkingState.setTalkColor(color); }
  void setTalkOffset(const sf::Vector2i &offset) { _talkingState.setTalkOffset(offset); }
  void say(int id) { _talkingState.say(id); }
  bool isTalking() const { return _talkingState.isTalking(); }

  void setColor(sf::Color color) { _color = color; }
  sf::Color getColor() { return _color; }

  void move(const sf::Vector2f &offset);
  sf::Vector2f getPosition() const { return _transform.getPosition(); }
  void setPosition(const sf::Vector2f &pos);
  void setRenderOffset(const sf::Vector2i &offset) { _renderOffset = offset; }

  NGRoom *getRoom() const { return _pRoom; }
  void setRoom(NGRoom *pRoom);

  void setHotspot(const sf::IntRect &hotspot) { _hotspot = hotspot; }
  const sf::IntRect &getHotspot() const { return _hotspot; }

  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
  void update(const sf::Time &time) override;

  void pickupObject(const std::string &icon) { _icons.push_back(icon); }
  const std::vector<std::string> &getObjects() const { return _icons; }

  void setWalkSpeed(const sf::Vector2i &speed) { _speed = speed; }
  const sf::Vector2i &getWalkSpeed() const { return _speed; }
  void walkTo(const sf::Vector2f &destination);
  bool isWalking() const { return _walkingState.isWalking(); }

private:
  NGEngine &_engine;
  const NGEngineSettings &_settings;
  NGCostume _costume;
  std::string _name;
  sf::Transformable _transform;
  sf::Color _color;
  sf::Vector2i _renderOffset;
  int _zorder;
  bool _isVisible;
  bool _use;
  NGRoom *_pRoom;
  sf::IntRect _hotspot;
  std::vector<std::string> _icons;
  WalkingState _walkingState;
  TalkingState _talkingState;
  sf::Vector2i _speed;
};
} // namespace ng
