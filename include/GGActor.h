#pragma once
#include <sstream>
#include <squirrel3/squirrel.h>
#include "SFML/Graphics.hpp"
#include "NonCopyable.h"
#include "TextureManager.h"
#include "GGCostume.h"
#include "GGFont.h"
#include "GGEntity.h"

namespace gg
{
class GGRoom;
class GGObject;
class GGActor : public GGEntity
{
public:
  explicit GGActor(TextureManager &textureManager);
  virtual ~GGActor();

  void setName(const std::string &name) { _name = name; }
  const std::string &getName() const { return _name; }

  void setVisible(bool isVisible) { _isVisible = isVisible; }
  bool isVisible() const { return _isVisible; }

  void useWalkboxes(bool use) { _use = use; }

  void setTable(HSQOBJECT table) { _table = table; }
  HSQOBJECT getTable() const { return _table; }

  int getZOrder() const override { return _zorder; }

  void setCostume(const std::string &name, const std::string &sheet = "");
  GGCostume &getCostume() { return _costume; }
  const GGCostume &getCostume() const { return _costume; }

  void setTalkColor(sf::Color color) { _talkColor = color; }
  sf::Color getTalkColor() { return _talkColor; }
  void setTalkOffset(const sf::Vector2i &offset) { _talkOffset = offset; }
  void say(const std::string &text) { _sayText = text; }
  bool isTalking() const { return !_sayText.empty(); }

  void setColor(sf::Color color) { _color = color; }
  sf::Color getColor() { return _color; }

  void move(const sf::Vector2f &offset);
  sf::Vector2f getPosition() const { return _transform.transformPoint(0, 0); }
  void setPosition(const sf::Vector2f &pos);
  void setRenderOffset(const sf::Vector2i &offset) { _renderOffset = offset; }

  GGRoom *getRoom() const { return _pRoom; }
  void setRoom(GGRoom *pRoom) { _pRoom = pRoom; }

  void setHotspot(const sf::IntRect &hotspot) { _hotspot = hotspot; }
  const sf::IntRect &getHotspot() const { return _hotspot; }

  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
  void update(const sf::Time &time) override;

  void pickupObject(const std::string& icon) { _icons.push_back(icon); }
  const std::vector<std::string>& getObjects() { return _icons; }

private:
  const GGEngineSettings &_settings;
  GGCostume _costume;
  std::string _name;
  sf::Transform _transform;
  sf::Color _color;
  sf::Color _talkColor;
  sf::Vector2i _renderOffset, _talkOffset;
  GGFont _font;
  std::string _sayText;
  int _zorder;
  HSQOBJECT _table;
  bool _isVisible;
  bool _use;
  GGRoom *_pRoom;
  sf::IntRect _hotspot;
  std::vector<std::string> _icons;
};
} // namespace gg
